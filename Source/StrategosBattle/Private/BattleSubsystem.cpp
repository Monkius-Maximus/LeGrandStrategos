#include "BattleSubsystem.h"
#include "StrategosBattle.h"
#include "BattleCardAsset.h"
#include "BattleEffects.h"
#include "BattleAIController.h"
#include "BattleAIProfile.h"
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
			TEXT("InitBattle: batalha já ativa."));
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

	BattleRNG = FRandomStream(Context.Seed);

	Context.Attacker = BuildSide(Proposal.AttackerNationId,
	                             Proposal.AttackerArmyIds, WS);
	Context.Defender = BuildSide(Proposal.DefenderNationId,
	                             Proposal.DefenderArmyIds, WS);
	Context.Attacker.bHasInitiative = true;

	AssignInitialPositions(Context);

	// Monta decks fallback (Stage 4: sem registry de cartas ainda)
	Context.Attacker.DrawPile = BuildFallbackDeck(this);
	ShuffleDeck(Context.Attacker.DrawPile);

	Context.Defender.DrawPile = BuildFallbackDeck(this);
	ShuffleDeck(Context.Defender.DrawPile);

	// Cria AI controllers (Stage 6)
	AttackerAI = NewObject<UBattleAIController>(this);
	AttackerAI->Initialize(this, 0, nullptr);

	DefenderAI = NewObject<UBattleAIController>(this);
	DefenderAI->Initialize(this, 1, nullptr);

	bActive = true;
	RoundsInCurrentPhase = 0;

	UE_LOG(LogStrategosBattle, Log,
		TEXT("BattleSubsystem: %s iniciada — Atk:%d Def:%d"),
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
	GatherAndResolveDeclarations();

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
// Gestão de deck — Etapa 4-5
// ─────────────────────────────────────────────────────────────────────────────

void UBattleSubsystem::DrawForBothSides()
{
	DrawCards(Context.Attacker, HandSize);
	DrawCards(Context.Defender, HandSize);
}

void UBattleSubsystem::DrawCards(FBattleSide& Side, int32 TargetHandSize)
{
	while (Side.Hand.Num() < TargetHandSize)
	{
		if (Side.DrawPile.IsEmpty())
		{
			if (Side.DiscardPile.IsEmpty()) break;
			Side.DrawPile = Side.DiscardPile;
			Side.DiscardPile.Empty();
			ShuffleDeck(Side.DrawPile);
		}
		Side.Hand.Add(Side.DrawPile.Last());
		Side.DrawPile.Pop();
	}
}

void UBattleSubsystem::ShuffleDeck(TArray<UBattleCardAsset*>& Deck)
{
	// Fisher-Yates com BattleRNG (determinístico)
	for (int32 i = Deck.Num() - 1; i > 0; --i)
	{
		const int32 j = BattleRNG.RandRange(0, i);
		Deck.Swap(i, j);
	}
}

void UBattleSubsystem::RefreshCommandPoints()
{
	Context.Attacker.CommandPoints = Context.Attacker.MaxCommandPoints;
	Context.Defender.CommandPoints = Context.Defender.MaxCommandPoints;
}

void UBattleSubsystem::GatherAndResolveDeclarations()
{
	FBattleDeclaration AttDecl;
	FBattleDeclaration DefDecl;

	if (AttackerAI)
	{
		AttDecl = AttackerAI->ChooseDeclaration(
			Context, Context.Attacker, Context.Defender);
	}
	if (DefenderAI)
	{
		DefDecl = DefenderAI->ChooseDeclaration(
			Context, Context.Defender, Context.Attacker);
	}

	ResolveDeclarations(AttDecl, DefDecl);
}

void UBattleSubsystem::ResolveDeclarations(const FBattleDeclaration& AttDecl,
                                             const FBattleDeclaration& DefDecl)
{
	// Agrupa todas as cartas com side e initiative
	struct FCardPlay
	{
		UBattleCardAsset* Card;
		int32 SideIndex;
		int32 Initiative;
	};

	TArray<FCardPlay> AllPlays;

	auto EnqueuePlays = [&](const FBattleDeclaration& Decl, int32 SideIdx)
	{
		FBattleSide& Side = (SideIdx == 0) ? Context.Attacker : Context.Defender;
		for (UBattleCardAsset* Card : Decl.CardsToPlay)
		{
			if (!Card || Card->CommandCost > Side.CommandPoints) continue;
			FCardPlay Play;
			Play.Card       = Card;
			Play.SideIndex  = SideIdx;
			Play.Initiative = ComputeCardInitiative(Side, Card);
			AllPlays.Add(Play);
			Side.CommandPoints -= Card->CommandCost;
		}
	};

	EnqueuePlays(AttDecl, 0);
	EnqueuePlays(DefDecl, 1);

	// Ordena por initiative DESC; empate = BattleRNG (determinístico)
	AllPlays.Sort([](const FCardPlay& A, const FCardPlay& B)
	{
		return A.Initiative != B.Initiative
			? A.Initiative > B.Initiative
			: A.SideIndex < B.SideIndex;  // atacante desempata
	});

	// Aplica efeitos em ordem
	for (const FCardPlay& Play : AllPlays)
	{
		FBattleSide& Source = (Play.SideIndex == 0)
			? Context.Attacker : Context.Defender;
		FBattleSide& Target = (Play.SideIndex == 0)
			? Context.Defender : Context.Attacker;

		for (UBattleEffect* Effect : Play.Card->Effects)
		{
			if (!Effect) continue;
			if (Effect->CanApply(Context, Source, Target))
			{
				Effect->Apply(Context, Source, Target);
			}
		}

		// Move carta para Discard ou Exhaust
		Source.Hand.Remove(Play.Card);
		if (Play.Card->bExhaustOnPlay)
			Source.ExhaustPile.Add(Play.Card);
		else
			Source.DiscardPile.Add(Play.Card);

		LogEntry(Play.SideIndex, EBattleLogType::CardPlayed,
			Play.Card->DisplayName.ToString(),
			Play.Card->CommandCost);

		OnCardPlayed.Broadcast(Play.SideIndex, FName(*Play.Card->DisplayName.ToString()));
	}
}

int32 UBattleSubsystem::ComputeCardInitiative(const FBattleSide& Side,
                                               const UBattleCardAsset* Card) const
{
	int32 Init = Card ? Card->Priority : 0;
	Init += Side.bHasInitiative ? 5 : 0;
	return Init;
}

TArray<UBattleCardAsset*> UBattleSubsystem::BuildFallbackDeck(UObject* Outer)
{
	// 5 cartas placeholder — substituir por registry quando DataAssets estiverem prontos.
	TArray<UBattleCardAsset*> Deck;

	auto MakeCard = [&](
		const TCHAR* Name,
		ECardCategory Cat,
		int32 Cost,
		int32 Prio) -> UBattleCardAsset*
	{
		UBattleCardAsset* Card = NewObject<UBattleCardAsset>(Outer);
		Card->DisplayName  = FText::FromString(FString(Name));
		Card->Category     = Cat;
		Card->CommandCost  = Cost;
		Card->Priority     = Prio;
		Card->Timing       = ECardTiming::OnPlay;
		Card->bExhaustOnPlay = false;
		return Card;
	};

	// 1. Carga (Assault, 2 CP)
	{
		UBattleCardAsset* Card = MakeCard(TEXT("Carga"), ECardCategory::Assault, 2, 8);
		Card->Description = FText::FromString(TEXT("Ordena carga frontal. Dano aumentado, moral inimiga reduzida."));

		UEffect_DamageRegiment* Dmg = NewObject<UEffect_DamageRegiment>(Card);
		Dmg->DamageMultiplier = 0.4f;
		Dmg->bTargetEnemy = true;
		Card->Effects.Add(Dmg);

		UEffect_MoraleShift* Mor = NewObject<UEffect_MoraleShift>(Card);
		Mor->MoraleDelta = -8.f;
		Mor->bTargetEnemy = true;
		Card->Effects.Add(Mor);

		Deck.Add(Card);
	}

	// 2. Segurar a Linha (Support, 1 CP)
	{
		UBattleCardAsset* Card = MakeCard(TEXT("Segurar a Linha"), ECardCategory::Support, 1, 5);
		Card->Description = FText::FromString(TEXT("Reforça a formação. Adiciona bônus defensivo por 2 rounds."));

		UEffect_AddPersistent* Eff = NewObject<UEffect_AddPersistent>(Card);
		Eff->EffectId    = FName("HoldLine_DEF");
		Eff->EffectLabel = FText::FromString(TEXT("Linha Sólida"));
		Eff->Value       = 0.20f;
		Eff->Duration    = 2;
		Eff->EffectType  = EActiveEffectType::DefenseModifier;
		Eff->bTargetEnemy = false;
		Card->Effects.Add(Eff);

		Deck.Add(Card);
	}

	// 3. Ataque de Flanco (Maneuver, 2 CP)
	{
		UBattleCardAsset* Card = MakeCard(TEXT("Ataque de Flanco"), ECardCategory::Maneuver, 2, 6);
		Card->Description = FText::FromString(TEXT("Reposiciona as tropas para flanquear o inimigo."));

		UEffect_RepositionSide* Repo = NewObject<UEffect_RepositionSide>(Card);
		Repo->NewPosition  = EBattlePosition::Flank;
		Repo->bTargetEnemy = false;
		Card->Effects.Add(Repo);

		UEffect_DamageRegiment* Dmg = NewObject<UEffect_DamageRegiment>(Card);
		Dmg->DamageMultiplier = 0.25f;
		Dmg->bTargetEnemy = true;
		Card->Effects.Add(Dmg);

		Deck.Add(Card);
	}

	// 4. Reagrupar (Support, 1 CP)
	{
		UBattleCardAsset* Card = MakeCard(TEXT("Reagrupar"), ECardCategory::Support, 1, 4);
		Card->Description = FText::FromString(TEXT("Eleva a moral das tropas. Recupera +15 de moral."));

		UEffect_MoraleShift* Mor = NewObject<UEffect_MoraleShift>(Card);
		Mor->MoraleDelta  = 15.f;
		Mor->bTargetEnemy = false;
		Card->Effects.Add(Mor);

		Deck.Add(Card);
	}

	// 5. Emboscada (Stratagem, 3 CP — válida apenas em Setup)
	{
		UBattleCardAsset* Card = MakeCard(TEXT("Emboscada"), ECardCategory::Stratagem, 3, 10);
		Card->Description = FText::FromString(TEXT("Ataque surpresa. Altíssimo dano e desmora o inimigo."));
		Card->ValidPhases = { EBattlePhase::Setup };
		Card->bExhaustOnPlay = true;

		UEffect_DamageRegiment* Dmg = NewObject<UEffect_DamageRegiment>(Card);
		Dmg->DamageMultiplier = 0.8f;
		Dmg->bTargetEnemy = true;
		Card->Effects.Add(Dmg);

		UEffect_MoraleShift* Mor = NewObject<UEffect_MoraleShift>(Card);
		Mor->MoraleDelta  = -15.f;
		Mor->bTargetEnemy = true;
		Card->Effects.Add(Mor);

		Deck.Add(Card);
	}

	// Duplica o deck para ter mais rodadas de cartas (2x cada)
	TArray<UBattleCardAsset*> DeckCopy = Deck;
	Deck.Append(DeckCopy);

	return Deck;
}

// ─────────────────────────────────────────────────────────────────────────────
// Loop interno
// ─────────────────────────────────────────────────────────────────────────────

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

	auto TickEffects = [](TArray<FActiveBattleEffect>& Effects)
	{
		for (int32 i = Effects.Num() - 1; i >= 0; --i)
		{
			if (--Effects[i].RoundsRemaining <= 0)
				Effects.RemoveAt(i);
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

	default: return false;
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
	if (Context.CurrentPhase != EBattlePhase::Resolved)
	{
		Context.CurrentPhase = EBattlePhase::Resolved;
	}

	EBattleOutcome Outcome;
	if (Context.Attacker.IsDefeated() && !Context.Defender.IsDefeated())
		Outcome = EBattleOutcome::DefenderVictory;
	else if (Context.Defender.IsDefeated() && !Context.Attacker.IsDefeated())
		Outcome = EBattleOutcome::AttackerVictory;
	else if (Context.Attacker.IsDefeated() && Context.Defender.IsDefeated())
		Outcome = Context.Attacker.Morale >= Context.Defender.Morale
			? EBattleOutcome::AttackerVictory
			: EBattleOutcome::DefenderVictory;
	else
		Outcome = EBattleOutcome::Stalemate;

	FBattleResult Result;
	Result.BattleId          = Context.BattleId;
	Result.Outcome           = Outcome;
	Result.CombatLog         = Context.Log;
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

	Result.SupplyConsumed        = TotalLost / 1000.f * 0.1f;
	Result.ProvinceControlChange =
		Outcome == EBattleOutcome::AttackerVictory ?  1 :
		Outcome == EBattleOutcome::DefenderVictory ? -1 : 0;

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
	FCombatTickResult Res;
	const float BasePower = Source.ComputeFightingPower();

	Res.TerrainMod  = TerrainCoeff(Source);
	Res.WeatherMod  = WeatherCoeff();
	Res.MoraleMod   = MoraleCoeff(Source.Morale);
	Res.SupplyMod   = SupplyCoeff(Source.Supply);
	Res.PositionMod = PositionCoeff(Source.Position, Target.Position);
	Res.EffectsMod  = ActiveEffectsModifier(Source, EActiveEffectType::AttackModifier);

	const float Modifier = Res.TerrainMod * Res.WeatherMod * Res.MoraleMod
	                     * Res.SupplyMod  * Res.PositionMod * Res.EffectsMod;

	const float AvgDEF      = Target.ComputeAverageDEF()
	                         * ActiveEffectsModifier(Target, EActiveEffectType::DefenseModifier);
	const float DefReduction = 1.f - FMath::Clamp(AvgDEF / 200.f, 0.f, 0.75f);

	Res.TotalDamage = BasePower * Modifier * DefReduction * DamageScaleFactor;
	return Res;
}

void UBattleSubsystem::DistributeDamage(FBattleSide& Target, float TotalDamage)
{
	int32 Active = 0;
	for (const FRegimentBattleState& Reg : Target.Regiments)
	{
		if (Reg.IsActive()) ++Active;
	}
	if (Active == 0 || TotalDamage <= 0.f) return;

	const float Share = TotalDamage / Active;
	for (FRegimentBattleState& Reg : Target.Regiments)
	{
		if (!Reg.IsActive()) continue;
		Reg.CurrentStrength = FMath::Max(0,
			Reg.CurrentStrength - FMath::RoundToInt(Share));
		const float OrgDmg = Share / FMath::Max(1, Reg.InitialStrength) * 2.f;
		Reg.OrganizationLeft = FMath::Clamp(Reg.OrganizationLeft - OrgDmg, 0.f, 1.f);
	}
}

void UBattleSubsystem::UpdateSideMorale(FBattleSide& Side, int32 StrengthLost)
{
	if (Side.bRouted) return;
	const int32 Init = Side.TotalInitialStrength();
	if (Init <= 0) return;

	const float LossRatio = static_cast<float>(StrengthLost) / Init;
	const float MorFactor = 1.f
		- FMath::Clamp(Side.ComputeAverageMOR() / 100.f, 0.f, 1.f) * 0.5f;

	float Delta = -LossRatio * 100.f * 2.f * MorFactor;

	// Bônus de moral por MoraleRegen de cartas ativas
	Delta += ActiveEffectsModifier(Side, EActiveEffectType::MoraleRegen);

	if (Side.Position == EBattlePosition::Crossing) Delta -= 3.f;
	if (Side.Position == EBattlePosition::Rear)     Delta -= 2.f;
	if (Side.Supply < 0.5f)                         Delta -= 2.f;

	Side.Morale = FMath::Clamp(Side.Morale + Delta, 0.f, 100.f);

	if (Delta < -0.5f)
	{
		LogEntry(
			(&Side == &Context.Attacker) ? 0 : 1,
			EBattleLogType::MoraleChanged,
			FString::Printf(TEXT("Moral %.1f (delta %.1f)"), Side.Morale, Delta),
			FMath::RoundToInt(Delta));
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// Coeficientes e efeitos ativos
// ─────────────────────────────────────────────────────────────────────────────

float UBattleSubsystem::ActiveEffectsModifier(
	const FBattleSide& Side, EActiveEffectType Type) const
{
	const TArray<FActiveBattleEffect>& Effects =
		(&Side == &Context.Attacker) ? Context.AttackerEffects : Context.DefenderEffects;

	float Total = 0.f;
	for (const FActiveBattleEffect& Eff : Effects)
	{
		if (Eff.EffectType == Type) Total += Eff.Value;
	}

	// Para multiplicadores: 1 + soma; para MoraleRegen: retorna soma plana
	return (Type == EActiveEffectType::MoraleRegen)
		? Total
		: FMath::Max(0.1f, 1.f + Total);
}

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
	Entry.Round          = Context.CurrentRound;
	Entry.Phase          = Context.CurrentPhase;
	Entry.ActorSideIndex = SideIdx;
	Entry.Type           = Type;
	Entry.Description    = Description;
	Entry.NumericValue   = NumericValue;
	Entry.TargetId       = TargetId;
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
		Reg.RegimentId      = FGuid::NewGuid();
		Reg.SourceArmyId    = ArmyId;
		Reg.Type            = ERegimentType::Infantry;
		Reg.InitialStrength = Army ? FMath::Max(1, Army->ManpowerCount) : 1000;
		Reg.ATQ = (Army && Army->BaseStats.ATQ > 0) ? Army->BaseStats.ATQ : 50;
		Reg.DEF = (Army && Army->BaseStats.DEF > 0) ? Army->BaseStats.DEF : 50;
		Reg.MOR = (Army && Army->BaseStats.MOR > 0) ? Army->BaseStats.MOR : 50;
		Reg.CurrentStrength  = Reg.InitialStrength;
		Reg.Morale           = 100.f;
		Reg.OrganizationLeft = 1.f;
		Reg.Stance           = EBattleStance::Hold;

		if (!Army)
		{
			UE_LOG(LogStrategosBattle, Warning,
				TEXT("BuildSide: exército '%s' não encontrado — usando defaults."),
				*ArmyId.ToString());
		}

		Side.Regiments.Add(Reg);
	}
	return Side;
}

void UBattleSubsystem::AssignInitialPositions(FBattleContext& Ctx)
{
	Ctx.Attacker.Position = EBattlePosition::Frontline;
	Ctx.Defender.Position = EBattlePosition::Frontline;

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
