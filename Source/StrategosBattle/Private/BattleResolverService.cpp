#include "BattleResolverService.h"
#include "BattleCardAsset.h"
#include "StrategosBattle.h"
#include "World/WorldState.h"
#include "World/Army.h"

// ─────────────────────────────────────────────────────────────────────────────
// Helpers locais — mesma fórmula que BattleSubsystem (Stage 4: unificar)
// ─────────────────────────────────────────────────────────────────────────────

namespace
{
	constexpr float DamageScaleFactor      = 0.08f;
	constexpr float RoutThreshold          = 25.f;
	constexpr int32 MaxEngagementRounds    = 3;
	constexpr float EarlyExitStrengthRatio = 0.5f;
	constexpr float EarlyExitMoraleThresh  = 30.f;

	static FBattleSide BuildSideLocal(
		int32 NationId, const TArray<FName>& ArmyIds, UWorldState* WorldState)
	{
		FBattleSide Side;
		Side.NationId = NationId;
		Side.Morale   = 100.f;
		Side.Supply   = 1.f;
		Side.Cohesion = 1.f;

		for (const FName& ArmyId : ArmyIds)
		{
			const UArmy* Army = WorldState ? WorldState->GetArmy(ArmyId) : nullptr;

			FRegimentBattleState Reg;
			Reg.RegimentId     = FGuid::NewGuid();
			Reg.SourceArmyId   = ArmyId;
			Reg.Type           = ERegimentType::Infantry;
			Reg.InitialStrength = Army ? FMath::Max(1, Army->ManpowerCount) : 1000;
			Reg.CurrentStrength = Reg.InitialStrength;
			Reg.ATQ = (Army && Army->BaseStats.ATQ > 0) ? Army->BaseStats.ATQ : 50;
			Reg.DEF = (Army && Army->BaseStats.DEF > 0) ? Army->BaseStats.DEF : 50;
			Reg.MOR = (Army && Army->BaseStats.MOR > 0) ? Army->BaseStats.MOR : 50;
			Reg.Morale          = 100.f;
			Reg.OrganizationLeft = 1.f;
			Reg.Stance          = EBattleStance::Hold;

			Side.Regiments.Add(Reg);
		}
		return Side;
	}

	static float PositionCoeffLocal(EBattlePosition Src, EBattlePosition Tgt)
	{
		float Mod = 1.0f;
		switch (Src)
		{
		case EBattlePosition::HighGround: Mod = 1.20f; break;
		case EBattlePosition::Flank:      Mod = 1.10f; break;
		case EBattlePosition::Frontline:  Mod = 1.00f; break;
		case EBattlePosition::Rear:       Mod = 0.80f; break;
		case EBattlePosition::Crossing:   Mod = 0.70f; break;
		}
		if (Tgt == EBattlePosition::HighGround) Mod *= 0.85f;
		return Mod;
	}

	static float TerrainCoeffLocal(EBattleTerrain Terrain, const FBattleSide& Side)
	{
		float Mod = 1.0f;
		switch (Terrain)
		{
		case EBattleTerrain::Forest:   Mod = 0.90f; break;
		case EBattleTerrain::Hills:    Mod = 0.90f; break;
		case EBattleTerrain::Mountain: Mod = 0.80f; break;
		case EBattleTerrain::River:    Mod = 0.85f; break;
		case EBattleTerrain::Urban:    Mod = 0.90f; break;
		default:                        Mod = 1.00f; break;
		}
		return Mod * PositionCoeffLocal(Side.Position, EBattlePosition::Frontline);
	}

	static float WeatherCoeffLocal(EBattleWeather Weather)
	{
		switch (Weather)
		{
		case EBattleWeather::Rain:  return 0.90f;
		case EBattleWeather::Storm: return 0.70f;
		case EBattleWeather::Fog:   return 0.80f;
		case EBattleWeather::Snow:  return 0.75f;
		default:                    return 1.00f;
		}
	}

	static float ComputeDamageLocal(const FBattleSide& Source, const FBattleSide& Target,
	                                EBattleTerrain Terrain, EBattleWeather Weather)
	{
		const float Power    = Source.ComputeFightingPower();
		const float Modifier = TerrainCoeffLocal(Terrain, Source)
		                     * WeatherCoeffLocal(Weather)
		                     * FMath::Clamp(0.5f + Source.Morale / 200.f, 0.25f, 1.f)
		                     * FMath::Clamp(0.5f + Source.Supply * 0.5f, 0.25f, 1.f)
		                     * PositionCoeffLocal(Source.Position, Target.Position);

		const float AvgDEF      = Target.ComputeAverageDEF();
		const float DefReduction = 1.f - FMath::Clamp(AvgDEF / 200.f, 0.f, 0.75f);

		return Power * Modifier * DefReduction * DamageScaleFactor;
	}

	static void DistributeDamageLocal(FBattleSide& Target, float TotalDamage)
	{
		int32 ActiveCount = 0;
		for (const FRegimentBattleState& Reg : Target.Regiments)
		{
			if (Reg.IsActive()) ++ActiveCount;
		}
		if (ActiveCount == 0 || TotalDamage <= 0.f) return;

		const float Share = TotalDamage / ActiveCount;
		for (FRegimentBattleState& Reg : Target.Regiments)
		{
			if (!Reg.IsActive()) continue;
			Reg.CurrentStrength = FMath::Max(0, Reg.CurrentStrength - FMath::RoundToInt(Share));
			const float OrgDmg = Share / FMath::Max(1, Reg.InitialStrength) * 2.f;
			Reg.OrganizationLeft = FMath::Clamp(Reg.OrganizationLeft - OrgDmg, 0.f, 1.f);
		}
	}

	static void UpdateMoraleLocal(FBattleSide& Side, int32 StrLost)
	{
		if (Side.bRouted) return;
		const int32 Init = Side.TotalInitialStrength();
		if (Init <= 0) return;

		const float LossRatio = static_cast<float>(StrLost) / Init;
		const float MorFactor = 1.f - FMath::Clamp(Side.ComputeAverageMOR() / 100.f, 0.f, 1.f) * 0.5f;
		float Delta = -LossRatio * 100.f * 2.f * MorFactor;
		if (Side.Position == EBattlePosition::Crossing) Delta -= 3.f;
		if (Side.Position == EBattlePosition::Rear)     Delta -= 2.f;
		if (Side.Supply < 0.5f)                         Delta -= 2.f;
		Side.Morale = FMath::Clamp(Side.Morale + Delta, 0.f, 100.f);
	}

	static void AssignPositionsLocal(FBattleContext& Ctx)
	{
		Ctx.Attacker.Position = EBattlePosition::Frontline;
		Ctx.Defender.Position = EBattlePosition::Frontline;
		switch (Ctx.Terrain)
		{
		case EBattleTerrain::River:
			Ctx.Attacker.Position = EBattlePosition::Crossing;   break;
		case EBattleTerrain::Hills:
		case EBattleTerrain::Mountain:
		case EBattleTerrain::Urban:
			Ctx.Defender.Position = EBattlePosition::HighGround; break;
		default: break;
		}
		if (Ctx.Type == EBattleType::Raid)
		{
			Ctx.Attacker.Position = EBattlePosition::Flank;
			Ctx.Defender.Position = EBattlePosition::Rear;
		}
		else if (Ctx.Type == EBattleType::Siege)
		{
			Ctx.Attacker.Position = EBattlePosition::Crossing;
			Ctx.Defender.Position = EBattlePosition::HighGround;
		}
	}
} // namespace

// ─────────────────────────────────────────────────────────────────────────────
// UBattleResolverService
// ─────────────────────────────────────────────────────────────────────────────

FBattleResult UBattleResolverService::ResolveQuick(
	const FBattleProposal& Proposal, UWorldState* WorldState)
{
	FBattleContext Ctx;
	Ctx.BattleId   = FGuid::NewGuid();
	Ctx.Type       = Proposal.Type;
	Ctx.Terrain    = Proposal.Terrain;
	Ctx.Weather    = Proposal.Weather;
	Ctx.ProvinceId = Proposal.ProvinceId;
	Ctx.Seed       = static_cast<int32>(GetTypeHash(Ctx.BattleId)) ^ Proposal.ProvinceId;

	Ctx.Attacker = BuildSideLocal(Proposal.AttackerNationId,
	                              Proposal.AttackerArmyIds, WorldState);
	Ctx.Defender = BuildSideLocal(Proposal.DefenderNationId,
	                              Proposal.DefenderArmyIds, WorldState);
	Ctx.Attacker.bHasInitiative = true;

	AssignPositionsLocal(Ctx);

	// Simula o loop de fases sem emitir eventos
	const int32 PhaseMaxRounds[] = { 1, MaxEngagementRounds, 1, 1 };
	int32 PhaseIdx = 0;

	while (!Ctx.IsBattleOver() && PhaseIdx < 4)
	{
		const EBattlePhase Phase = static_cast<EBattlePhase>(PhaseIdx);
		Ctx.CurrentPhase = Phase;

		int32 RoundsInPhase = 0;
		const int32 MaxR = PhaseMaxRounds[PhaseIdx];

		while (RoundsInPhase < MaxR && !Ctx.IsBattleOver())
		{
			const int32 AttBefore = Ctx.Attacker.TotalCurrentStrength();
			const int32 DefBefore = Ctx.Defender.TotalCurrentStrength();

			const float AttDmg = ComputeDamageLocal(
				Ctx.Attacker, Ctx.Defender, Ctx.Terrain, Ctx.Weather);
			const float DefDmg = ComputeDamageLocal(
				Ctx.Defender, Ctx.Attacker, Ctx.Terrain, Ctx.Weather);

			DistributeDamageLocal(Ctx.Defender, AttDmg);
			DistributeDamageLocal(Ctx.Attacker, DefDmg);

			const int32 AttLost = AttBefore - Ctx.Attacker.TotalCurrentStrength();
			const int32 DefLost = DefBefore - Ctx.Defender.TotalCurrentStrength();

			UpdateMoraleLocal(Ctx.Attacker, AttLost);
			UpdateMoraleLocal(Ctx.Defender, DefLost);

			if (Ctx.Attacker.Morale < RoutThreshold) Ctx.Attacker.bRouted = true;
			if (Ctx.Defender.Morale < RoutThreshold) Ctx.Defender.bRouted = true;

			++RoundsInPhase;
			++Ctx.TotalRounds;

			// Saída antecipada do Engagement
			if (Phase == EBattlePhase::Engagement)
			{
				const bool bEarlyExit =
					Ctx.Attacker.StrengthRatio() < EarlyExitStrengthRatio
					|| Ctx.Defender.StrengthRatio() < EarlyExitStrengthRatio
					|| Ctx.Attacker.Morale < EarlyExitMoraleThresh
					|| Ctx.Defender.Morale < EarlyExitMoraleThresh;
				if (bEarlyExit) break;
			}
		}

		++PhaseIdx;
	}

	Ctx.CurrentPhase = EBattlePhase::Resolved;

	// Monta resultado
	EBattleOutcome Outcome;
	if (Ctx.Attacker.IsDefeated() && !Ctx.Defender.IsDefeated())
	{
		Outcome = EBattleOutcome::DefenderVictory;
	}
	else if (Ctx.Defender.IsDefeated() && !Ctx.Attacker.IsDefeated())
	{
		Outcome = EBattleOutcome::AttackerVictory;
	}
	else if (Ctx.Attacker.IsDefeated() && Ctx.Defender.IsDefeated())
	{
		Outcome = Ctx.Attacker.Morale >= Ctx.Defender.Morale
			? EBattleOutcome::AttackerVictory
			: EBattleOutcome::DefenderVictory;
	}
	else
	{
		Outcome = EBattleOutcome::Stalemate;
	}

	FBattleResult Result;
	Result.BattleId          = Ctx.BattleId;
	Result.Outcome           = Outcome;
	Result.MoraleHitAttacker = FMath::Max(0.f, 100.f - Ctx.Attacker.Morale);
	Result.MoraleHitDefender = FMath::Max(0.f, 100.f - Ctx.Defender.Morale);

	float TotalLost = 0.f;
	auto CollectLosses = [&](const FBattleSide& Side)
	{
		for (const FRegimentBattleState& Reg : Side.Regiments)
		{
			const int32 Lost = Reg.InitialStrength - Reg.CurrentStrength;
			if (Lost > 0)
			{
				Result.ArmyStrengthLost.FindOrAdd(Reg.SourceArmyId) += Lost;
				TotalLost += Lost;
			}
		}
	};
	CollectLosses(Ctx.Attacker);
	CollectLosses(Ctx.Defender);

	Result.SupplyConsumed         = TotalLost / 1000.f * 0.1f;
	Result.ProvinceControlChange  =
		Outcome == EBattleOutcome::AttackerVictory ?  1 :
		Outcome == EBattleOutcome::DefenderVictory ? -1 : 0;

	UE_LOG(LogStrategosBattle, Log,
		TEXT("ResolveQuick: %s encerrada em %d rounds — outcome=%d"),
		*Result.BattleId.ToString(), Ctx.TotalRounds,
		static_cast<int32>(Outcome));

	return Result;
}
