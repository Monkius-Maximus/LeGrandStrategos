#include "BattleAIController.h"
#include "BattleSubsystem.h"
#include "BattleAIProfile.h"
#include "BattleCardAsset.h"
#include "StrategosBattle.h"

void UBattleAIController::Initialize(UBattleSubsystem* InOwner,
                                      int32 InSideIndex,
                                      UBattleAIProfile* InProfile)
{
	Owner     = InOwner;
	SideIndex = InSideIndex;
	Profile   = InProfile;
}

FBattleDeclaration UBattleAIController::ChooseDeclaration(
	const FBattleContext& Ctx,
	const FBattleSide& Self,
	const FBattleSide& Enemy) const
{
	FBattleDeclaration Declaration;
	int32 RemainingCP = Self.CommandPoints;

	// Ordena cartas na mão por score (melhor primeiro) — iteração sem cópia pesada
	struct FScoredCard
	{
		UBattleCardAsset* Card;
		float Score;
	};

	TArray<FScoredCard> Scored;
	Scored.Reserve(Self.Hand.Num());

	for (UBattleCardAsset* Card : Self.Hand)
	{
		if (!Card) continue;
		if (!Card->IsValidInContext(Ctx, Self)) continue;

		FScoredCard Entry;
		Entry.Card  = Card;
		Entry.Score = ScoreCard(Card, Ctx, Self, Enemy);
		Scored.Add(Entry);
	}

	Scored.Sort([](const FScoredCard& A, const FScoredCard& B)
	{
		return A.Score > B.Score;
	});

	// Cobre cartas com score positivo dentro do orçamento de CP
	for (const FScoredCard& Entry : Scored)
	{
		if (Entry.Score <= 0.f) break;
		if (Entry.Card->CommandCost > RemainingCP) continue;

		Declaration.CardsToPlay.Add(Entry.Card);
		RemainingCP -= Entry.Card->CommandCost;

		// Limite: máx 2 cartas por declaração em Stage 6 (sem lookahead)
		if (Declaration.CardsToPlay.Num() >= 2) break;
	}

	return Declaration;
}

float UBattleAIController::ScoreCard(const UBattleCardAsset* Card,
                                      const FBattleContext& Ctx,
                                      const FBattleSide& Self,
                                      const FBattleSide& Enemy) const
{
	float Score = 0.f;
	const float Aggression    = Profile ? Profile->Aggression    : 0.5f;
	const float RiskTolerance = Profile ? Profile->RiskTolerance : 0.5f;
	const float TerrainAware  = Profile ? Profile->TerrainAwareness : 0.5f;
	const float CatBias       = Profile ? Profile->GetCategoryBias(Card->Category) : 1.f;

	switch (Card->Category)
	{
	case ECardCategory::Assault:
		// Vale mais quanto mais agressivo e quanto mais fraco o inimigo está
		Score = 10.f * Aggression
		      + 5.f * (1.f - Enemy.StrengthRatio())
		      + 5.f * (Enemy.Morale < 50.f ? 1.f : 0.f);
		break;

	case ECardCategory::Support:
		// Vale mais quando a moral própria está baixa ou suprimento em risco
		Score = 8.f * (1.f - Self.Morale / 100.f)
		      + 6.f * (1.f - Self.Supply)
		      + 4.f * (1.f - Aggression);
		break;

	case ECardCategory::Maneuver:
		// Vale mais quando em posição ruim (Crossing / Rear)
	{
		const bool bBadPosition = (Self.Position == EBattlePosition::Crossing
		                        || Self.Position == EBattlePosition::Rear);
		Score = 9.f * (bBadPosition ? 1.f : 0.2f) * TerrainAware
		      + 3.f * Aggression;
		break;
	}

	case ECardCategory::Stratagem:
		// Vale mais no início (Setup/Engagement) e com alta tolerância a risco
		Score = 7.f * RiskTolerance
		      + (Ctx.CurrentPhase == EBattlePhase::Setup   ? 5.f : 0.f)
		      + (Ctx.CurrentPhase == EBattlePhase::Engagement ? 3.f : 0.f);
		break;

	case ECardCategory::Reaction:
		// Guardado para resposta — pontuado baixo na DeclarePhase normal
		Score = 2.f;
		break;
	}

	// Custo de CP (card custosa precisa valer mais)
	Score -= Card->CommandCost * 1.5f;

	// Bias de categoria do perfil
	Score *= CatBias;

	// Etapa 8: blend com lookahead simulado conforme temperatura do perfil
	const float Temp = Profile ? Profile->LookaheadTemperature : 0.f;
	if (Temp > 0.f && Card->Timing == ECardTiming::OnPlay)
	{
		const float StateScore = SimulateCardImpact(Card, Ctx, Self, Enemy);
		Score = FMath::Lerp(Score, StateScore, FMath::Clamp(Temp, 0.f, 1.f));
	}

	return Score;
}

float UBattleAIController::EvaluateState(
	const FBattleSide& Self, const FBattleSide& Enemy) const
{
	return Self.StrengthRatio()    * 50.f
	     + (Self.Morale  / 100.f) * 30.f
	     - Enemy.StrengthRatio()   * 40.f
	     - (Enemy.Morale / 100.f) * 20.f;
}

float UBattleAIController::SimulateCardImpact(
	const UBattleCardAsset* Card,
	const FBattleContext& Ctx,
	const FBattleSide& Self,
	const FBattleSide& Enemy) const
{
	FBattleContext SimCtx = Ctx;
	const bool bSelfIsAttacker = (&Self == &Ctx.Attacker);
	FBattleSide& SimSelf  = bSelfIsAttacker ? SimCtx.Attacker : SimCtx.Defender;
	FBattleSide& SimEnemy = bSelfIsAttacker ? SimCtx.Defender : SimCtx.Attacker;

	for (UBattleEffect* Effect : Card->Effects)
	{
		if (!Effect) continue;
		if (Effect->CanApply(SimCtx, SimSelf, SimEnemy))
		{
			Effect->Apply(SimCtx, SimSelf, SimEnemy);
		}
	}

	return EvaluateState(SimSelf, SimEnemy);
}
