#include "BattleSubsystem.h"
#include "StrategosBattle.h"
#include "World/WorldState.h"
#include "World/Army.h"
#include "Game/StrategosGameState.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"

// ─────────────────────────────────────────────────────────────────────────────
// Lifecycle
// ─────────────────────────────────────────────────────────────────────────────

void UBattleSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogStrategosBattle, Log, TEXT("BattleSubsystem initialized."));
}

void UBattleSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UBattleSubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

// ─────────────────────────────────────────────────────────────────────────────
// API pública
// ─────────────────────────────────────────────────────────────────────────────

bool UBattleSubsystem::InitBattle(const FBattleProposal& Proposal)
{
	if (bActive)
	{
		UE_LOG(LogStrategosBattle, Warning,
			TEXT("InitBattle: batalha já ativa — ignore proposal."));
		return false;
	}

	if (Proposal.AttackerArmyIds.IsEmpty() || Proposal.DefenderArmyIds.IsEmpty())
	{
		UE_LOG(LogStrategosBattle, Warning,
			TEXT("InitBattle: proposal sem exércitos em um dos lados."));
		return false;
	}

	const UWorldState* WS = ResolveWorldState();

	Context = FBattleContext{};
	Context.BattleId     = FGuid::NewGuid();
	Context.Type         = Proposal.Type;
	Context.Terrain      = Proposal.Terrain;
	Context.Weather      = Proposal.Weather;
	Context.ProvinceId   = Proposal.ProvinceId;
	Context.CurrentPhase = EBattlePhase::Setup;
	Context.CurrentRound = 0;
	Context.TotalRounds  = 0;
	Context.Seed         = static_cast<int32>(GetTypeHash(Context.BattleId))
	                      ^ Proposal.ProvinceId;

	Context.Attacker = BuildSide(Proposal.AttackerNationId,
	                             Proposal.AttackerArmyIds, WS);
	Context.Defender = BuildSide(Proposal.DefenderNationId,
	                             Proposal.DefenderArmyIds, WS);

	Context.Attacker.bHasInitiative = true;

	AssignInitialPositions(Context);

	bActive = true;
	RoundsInCurrentPhase = 0;

	UE_LOG(LogStrategosBattle, Log,
		TEXT("BattleSubsystem: batalha iniciada %s — Atk:%d Def:%d"),
		*Context.BattleId.ToString(),
		Context.Attacker.TotalCurrentStrength(),
		Context.Defender.TotalCurrentStrength());

	OnBattleStarted.Broadcast();
	return true;
}

bool UBattleSubsystem::ProcessRound()
{
	if (!bActive || Context.IsBattleOver())
	{
		return false;
	}

	DrawForBothSides();
	RefreshCommandPoints();

	int32 AttStrLost = 0;
	int32 DefStrLost = 0;
	ApplyCombatTick(AttStrLost, DefStrLost);

	CheckMoraleAndRout(AttStrLost, DefStrLost);
	EndRound();

	if (!Context.IsBattleOver() && ShouldAdvancePhase())
	{
		AdvancePhase();
	}

	if (Context.IsBattleOver())
	{
		const FBattleResult Result = Finalize();
		bActive = false;
		OnBattleFinished.Broadcast(Result);
	}

	return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Loop interno
// ─────────────────────────────────────────────────────────────────────────────

void UBattleSubsystem::DrawForBothSides()
{
	// Stage 4: preencher mão com cartas do DrawPile
}

void UBattleSubsystem::RefreshCommandPoints()
{
	// Stage 4: CommandPoints = MaxCommandPoints + bônus de comandante
}

void UBattleSubsystem::ApplyCombatTick(int32& OutAttStrLost, int32& OutDefStrLost)
{
	const FCombatTickResult AttToDef = ComputeDamage(Context.Attacker, Context.Defender);
	const FCombatTickResult DefToAtt = ComputeDamage(Context.Defender, Context.Attacker);

	const int32 DefBefore = Context.Defender.TotalCurrentStrength();
	const int32 AttBefore = Context.Attacker.TotalCurrentStrength();

	DistributeDamage(Context.Defender, AttToDef.TotalDamage);
	DistributeDamage(Context.Attacker, DefToAtt.TotalDamage);

	OutAttStrLost = AttBefore - Context.Attacker.TotalCurrentStrength();
	OutDefStrLost = DefBefore - Context.Defender.TotalCurrentStrength();

	LogEntry(0, EBattleLogType::DamageDealt,
		FString::Printf(TEXT("Atacante causou %.0f de dano"), AttToDef.TotalDamage),
		FMath::RoundToInt(AttToDef.TotalDamage));

	LogEntry(1, EBattleLogType::DamageDealt,
		FString::Printf(TEXT("Defensor causou %.0f de dano"), DefToAtt.TotalDamage),
		FMath::RoundToInt(DefToAtt.TotalDamage));
}

void UBattleSubsystem::CheckMoraleAndRout(int32 AttStrLost, int32 DefStrLost)
{
	UpdateSideMorale(Context.Attacker, AttStrLost);
	UpdateSideMorale(Context.Defender, DefStrLost);

	if (!Context.Attacker.bRouted && Context.Attacker.Morale < RoutThreshold)
	{
		Context.Attacker.bRouted = true;
		LogEntry(0, EBattleLogType::SideRouted, TEXT("Atacante entrou em fuga!"));
		OnSideRouted.Broadcast(0);
	}

	if (!Context.Defender.bRouted && Context.Defender.Morale < RoutThreshold)
	{
		Context.Defender.bRouted = true;
		LogEntry(1, EBattleLogType::SideRouted, TEXT("Defensor entrou em fuga!"));
		OnSideRouted.Broadcast(1);
	}
}

void UBattleSubsystem::EndRound()
{
	Context.CurrentRound++;
	Context.TotalRounds++;
	RoundsInCurrentPhase++;

	// Decrementa efeitos ativos de cartas (Stage 4)
	auto TickEffects = [](TArray<FActiveBattleEffect>& Effects)
	{
		for (int32 i = Effects.Num() - 1; i >= 0; --i)
		{
			if (--Effects[i].RoundsRemaining <= 0)
			{
				Effects.RemoveAt(i);
			}
		}
	};
	TickEffects(Context.AttackerEffects);
	TickEffects(Context.DefenderEffects);

	OnRoundEnded.Broadcast(Context.CurrentRound);
}

bool UBattleSubsystem::ShouldAdvancePhase() const
{
	switch (Context.CurrentPhase)
	{
	case EBattlePhase::Setup:
		return RoundsInCurrentPhase >= 1;

	case EBattlePhase::Engagement:
		return RoundsInCurrentPhase >= MaxEngagementRounds
			|| Context.Attacker.StrengthRatio() < EarlyExitStrengthRatio
			|| Context.Defender.StrengthRatio() < EarlyExitStrengthRatio
			|| Context.Attacker.Morale < EarlyExitMoraleThresh
			|| Context.Defender.Morale < EarlyExitMoraleThresh;

	case EBattlePhase::Climax:
	case EBattlePhase::Pursuit:
		return RoundsInCurrentPhase >= 1;

	default:
		return false;
	}
}

void UBattleSubsystem::AdvancePhase()
{
	const EBattlePhase Old = Context.CurrentPhase;
	EBattlePhase Next = EBattlePhase::Resolved;

	switch (Old)
	{
	case EBattlePhase::Setup:      Next = EBattlePhase::Engagement; break;
	case EBattlePhase::Engagement: Next = EBattlePhase::Climax;     break;
	case EBattlePhase::Climax:     Next = EBattlePhase::Pursuit;    break;
	case EBattlePhase::Pursuit:    Next = EBattlePhase::Resolved;   break;
	default: break;
	}

	Context.CurrentPhase = Next;
	RoundsInCurrentPhase = 0;

	LogEntry(-1, EBattleLogType::PhaseChanged,
		FString::Printf(TEXT("Fase: %d → %d"),
			static_cast<int32>(Old), static_cast<int32>(Next)));

	OnPhaseChanged.Broadcast(Old, Next);
}

FBattleResult UBattleSubsystem::Finalize()
{
	// Garante phase = Resolved no contexto
	if (Context.CurrentPhase != EBattlePhase::Resolved)
	{
		Context.CurrentPhase = EBattlePhase::Resolved;
	}

	EBattleOutcome Outcome;
	if (Context.Attacker.IsDefeated() && !Context.Defender.IsDefeated())
	{
		Outcome = EBattleOutcome::DefenderVictory;
	}
	else if (Context.Defender.IsDefeated() && !Context.Attacker.IsDefeated())
	{
		Outcome = EBattleOutcome::AttackerVictory;
	}
	else if (Context.Attacker.IsDefeated() && Context.Defender.IsDefeated())
	{
		// Ambos derrotados: quem tinha mais morale vence
		Outcome = Context.Attacker.Morale >= Context.Defender.Morale
			? EBattleOutcome::AttackerVictory
			: EBattleOutcome::DefenderVictory;
	}
	else
	{
		Outcome = EBattleOutcome::Stalemate;
	}

	FBattleResult Result;
	Result.BattleId  = Context.BattleId;
	Result.Outcome   = Outcome;
	Result.CombatLog = Context.Log;

	Result.MoraleHitAttacker = FMath::Max(0.f, 100.f - Context.Attacker.Morale);
	Result.MoraleHitDefender = FMath::Max(0.f, 100.f - Context.Defender.Morale);

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
	CollectLosses(Context.Attacker);
	CollectLosses(Context.Defender);

	Result.SupplyConsumed = TotalLost / 1000.f * 0.1f;

	switch (Outcome)
	{
	case EBattleOutcome::AttackerVictory: Result.ProvinceControlChange =  1; break;
	case EBattleOutcome::DefenderVictory: Result.ProvinceControlChange = -1; break;
	default:                              Result.ProvinceControlChange =  0; break;
	}

	LogEntry(-1, EBattleLogType::BattleResolved,
		FString::Printf(TEXT("Batalha encerrada. Resultado: %d"),
			static_cast<int32>(Outcome)),
		static_cast<int32>(Outcome));

	UE_LOG(LogStrategosBattle, Log,
		TEXT("BattleSubsystem: %s encerrada — outcome=%d att=%d%% def=%d%%"),
		*Context.BattleId.ToString(),
		static_cast<int32>(Outcome),
		FMath::RoundToInt(Context.Attacker.StrengthRatio() * 100),
		FMath::RoundToInt(Context.Defender.StrengthRatio() * 100));

	return Result;
}

// ─────────────────────────────────────────────────────────────────────────────
// Cálculo de dano
// ─────────────────────────────────────────────────────────────────────────────

FCombatTickResult UBattleSubsystem::ComputeDamage(
	const FBattleSide& Source, const FBattleSide& Target) const
{
	FCombatTickResult Result;

	const float BasePower    = Source.ComputeFightingPower();
	Result.TerrainMod        = TerrainCoeff(Source);
	Result.WeatherMod        = WeatherCoeff();
	Result.MoraleMod         = MoraleCoeff(Source.Morale);
	Result.SupplyMod         = SupplyCoeff(Source.Supply);
	Result.PositionMod       = PositionCoeff(Source.Position, Target.Position);

	const float Modifier = Result.TerrainMod * Result.WeatherMod
	                     * Result.MoraleMod  * Result.SupplyMod
	                     * Result.PositionMod;

	const float AvgDEF      = Target.ComputeAverageDEF();
	const float DefReduction = 1.f - FMath::Clamp(AvgDEF / 200.f, 0.f, 0.75f);

	Result.TotalDamage = BasePower * Modifier * DefReduction * DamageScaleFactor;
	return Result;
}

void UBattleSubsystem::DistributeDamage(FBattleSide& Target, float TotalDamage)
{
	int32 ActiveCount = 0;
	for (const FRegimentBattleState& Reg : Target.Regiments)
	{
		if (Reg.IsActive()) ++ActiveCount;
	}

	if (ActiveCount == 0 || TotalDamage <= 0.f) return;

	const float SharePerReg = TotalDamage / ActiveCount;

	for (FRegimentBattleState& Reg : Target.Regiments)
	{
		if (!Reg.IsActive()) continue;

		const int32 StrDmg = FMath::RoundToInt(SharePerReg);
		Reg.CurrentStrength = FMath::Max(0, Reg.CurrentStrength - StrDmg);

		// Organização degrade mais rápido que força (pânico)
		const float OrgDmg = SharePerReg / FMath::Max(1, Reg.InitialStrength) * 2.f;
		Reg.OrganizationLeft = FMath::Clamp(Reg.OrganizationLeft - OrgDmg, 0.f, 1.f);
	}
}

void UBattleSubsystem::UpdateSideMorale(FBattleSide& Side, int32 StrengthLost)
{
	if (Side.bRouted) return;

	const int32 InitStr = Side.TotalInitialStrength();
	if (InitStr <= 0) return;

	const float LossRatio = static_cast<float>(StrengthLost) / InitStr;

	// MOR reduz a degradação de moral (unidades com alto MOR aguentam mais)
	const float AvgMOR    = Side.ComputeAverageMOR();
	const float MorFactor = 1.f - FMath::Clamp(AvgMOR / 100.f, 0.f, 1.f) * 0.5f;

	float Delta = -LossRatio * 100.f * 2.f * MorFactor;

	// Penalidade de posição
	if (Side.Position == EBattlePosition::Crossing) Delta -= 3.f;
	if (Side.Position == EBattlePosition::Rear)     Delta -= 2.f;

	// Penalidade de suprimento
	if (Side.Supply < 0.5f) Delta -= 2.f;

	Side.Morale = FMath::Clamp(Side.Morale + Delta, 0.f, 100.f);

	if (Delta < -0.5f)
	{
		LogEntry(
			(&Side == &Context.Attacker) ? 0 : 1,
			EBattleLogType::MoraleChanged,
			FString::Printf(TEXT("Moral %.1f → %.1f"), Side.Morale - Delta, Side.Morale),
			FMath::RoundToInt(Delta));
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// Coeficientes
// ─────────────────────────────────────────────────────────────────────────────

float UBattleSubsystem::TerrainCoeff(const FBattleSide& Side) const
{
	float Mod = 1.0f;
	switch (Context.Terrain)
	{
	case EBattleTerrain::Forest:   Mod = 0.90f; break;
	case EBattleTerrain::Hills:    Mod = 0.90f; break;
	case EBattleTerrain::Mountain: Mod = 0.80f; break;
	case EBattleTerrain::River:    Mod = 0.85f; break;
	case EBattleTerrain::Urban:    Mod = 0.90f; break;
	default:                        Mod = 1.00f; break;
	}
	return Mod * PositionCoeff(Side.Position, EBattlePosition::Frontline);
}

float UBattleSubsystem::WeatherCoeff() const
{
	switch (Context.Weather)
	{
	case EBattleWeather::Rain:  return 0.90f;
	case EBattleWeather::Storm: return 0.70f;
	case EBattleWeather::Fog:   return 0.80f;
	case EBattleWeather::Snow:  return 0.75f;
	default:                    return 1.00f;
	}
}

float UBattleSubsystem::MoraleCoeff(float Morale) const
{
	// Abaixo de 50 de moral, efetividade começa a cair
	return FMath::Clamp(0.5f + Morale / 200.f, 0.25f, 1.0f);
}

float UBattleSubsystem::SupplyCoeff(float Supply) const
{
	return FMath::Clamp(0.5f + Supply * 0.5f, 0.25f, 1.0f);
}

float UBattleSubsystem::PositionCoeff(EBattlePosition Src, EBattlePosition Tgt) const
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
	// Atacar posição elevada é mais difícil
	if (Tgt == EBattlePosition::HighGround) Mod *= 0.85f;
	return Mod;
}

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

void UBattleSubsystem::LogEntry(int32 SideIdx, EBattleLogType Type,
	const FString& Description, int32 NumericValue, FGuid TargetId)
{
	FBattleLogEntry Entry;
	Entry.Round        = Context.CurrentRound;
	Entry.Phase        = Context.CurrentPhase;
	Entry.ActorSideIndex = SideIdx;
	Entry.Type         = Type;
	Entry.Description  = Description;
	Entry.NumericValue = NumericValue;
	Entry.TargetId     = TargetId;
	Context.Log.Add(Entry);
}

FBattleSide UBattleSubsystem::BuildSide(
	int32 NationId, const TArray<FName>& ArmyIds,
	const UWorldState* WorldState) const
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
		Reg.RegimentId    = FGuid::NewGuid();
		Reg.SourceArmyId  = ArmyId;
		Reg.Type          = ERegimentType::Infantry;

		if (Army)
		{
			Reg.InitialStrength = FMath::Max(1, Army->ManpowerCount);
			Reg.ATQ = Army->BaseStats.ATQ > 0 ? Army->BaseStats.ATQ : 50;
			Reg.DEF = Army->BaseStats.DEF > 0 ? Army->BaseStats.DEF : 50;
			Reg.MOR = Army->BaseStats.MOR > 0 ? Army->BaseStats.MOR : 50;
		}
		else
		{
			// Fallback: exército não encontrado no WorldState
			UE_LOG(LogStrategosBattle, Warning,
				TEXT("BuildSide: exército '%s' não encontrado — usando defaults."),
				*ArmyId.ToString());
			Reg.InitialStrength = 1000;
			Reg.ATQ = 50;
			Reg.DEF = 50;
			Reg.MOR = 50;
		}

		Reg.CurrentStrength = Reg.InitialStrength;
		Reg.Morale          = 100.f;
		Reg.OrganizationLeft = 1.f;
		Reg.Stance          = EBattleStance::Hold;

		Side.Regiments.Add(Reg);
	}

	return Side;
}

void UBattleSubsystem::AssignInitialPositions(FBattleContext& Ctx)
{
	Ctx.Attacker.Position = EBattlePosition::Frontline;
	Ctx.Defender.Position = EBattlePosition::Frontline;

	// Terreno confere vantagem posicional ao defensor
	switch (Ctx.Terrain)
	{
	case EBattleTerrain::River:
		Ctx.Attacker.Position = EBattlePosition::Crossing;
		break;
	case EBattleTerrain::Hills:
	case EBattleTerrain::Mountain:
	case EBattleTerrain::Urban:
		Ctx.Defender.Position = EBattlePosition::HighGround;
		break;
	default:
		break;
	}

	// Tipo de batalha sobrepõe terreno
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

UWorldState* UBattleSubsystem::ResolveWorldState() const
{
	const UWorld* World = GetWorld();
	if (!World) return nullptr;

	AStrategosGameState* GS = World->GetGameState<AStrategosGameState>();
	return GS ? GS->GetWorldState() : nullptr;
}
