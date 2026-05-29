#include "BattleEffects.h"
#include "StrategosBattle.h"

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

static void DistributeCardDamage(FBattleSide& Target, float TotalDamage)
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

// ─────────────────────────────────────────────────────────────────────────────
// UEffect_DamageRegiment
// ─────────────────────────────────────────────────────────────────────────────

void UEffect_DamageRegiment::Apply(FBattleContext& Ctx,
                                    FBattleSide& Source, FBattleSide& Target)
{
	FBattleSide& ActualTarget = bTargetEnemy ? Target : Source;

	// Calcula dano extra com base no FightingPower da fonte
	const float ExtraDamage = Source.ComputeFightingPower() * FMath::Abs(DamageMultiplier);
	DistributeCardDamage(ActualTarget, ExtraDamage);

	UE_LOG(LogStrategosBattle, Log,
		TEXT("Effect_DamageRegiment: %.0f dano extra ao %s"),
		ExtraDamage, bTargetEnemy ? TEXT("inimigo") : TEXT("próprio lado"));
}

FText UEffect_DamageRegiment::GetDescriptionText() const
{
	return FText::Format(
		NSLOCTEXT("Battle", "DmgRegFmt", "{0}% dano {1}"),
		FText::AsNumber(FMath::RoundToInt(DamageMultiplier * 100)),
		bTargetEnemy
			? NSLOCTEXT("Battle", "ToEnemy", "ao inimigo")
			: NSLOCTEXT("Battle", "ToSelf", "a si mesmo"));
}

// ─────────────────────────────────────────────────────────────────────────────
// UEffect_MoraleShift
// ─────────────────────────────────────────────────────────────────────────────

void UEffect_MoraleShift::Apply(FBattleContext& Ctx,
                                 FBattleSide& Source, FBattleSide& Target)
{
	FBattleSide& ActualTarget = bTargetEnemy ? Target : Source;
	ActualTarget.Morale = FMath::Clamp(ActualTarget.Morale + MoraleDelta, 0.f, 100.f);

	UE_LOG(LogStrategosBattle, Log,
		TEXT("Effect_MoraleShift: moral %+.1f ao %s → %.1f"),
		MoraleDelta,
		bTargetEnemy ? TEXT("inimigo") : TEXT("próprio"),
		ActualTarget.Morale);
}

FText UEffect_MoraleShift::GetDescriptionText() const
{
	return FText::Format(
		NSLOCTEXT("Battle", "MoraleFmt", "Moral {0}{1}"),
		MoraleDelta >= 0
			? FText::FromString(TEXT("+"))
			: FText::FromString(TEXT("")),
		FText::AsNumber(FMath::RoundToInt(MoraleDelta)));
}

// ─────────────────────────────────────────────────────────────────────────────
// UEffect_AddPersistent
// ─────────────────────────────────────────────────────────────────────────────

void UEffect_AddPersistent::Apply(FBattleContext& Ctx,
                                   FBattleSide& Source, FBattleSide& Target)
{
	FActiveBattleEffect NewEffect;
	NewEffect.EffectId        = EffectId;
	NewEffect.Label           = EffectLabel;
	NewEffect.Value           = Value;
	NewEffect.RoundsRemaining = Duration;
	NewEffect.bPositive       = Value >= 0.f;
	NewEffect.EffectType      = EffectType;

	bool bAttackerSide = (&Source == &Ctx.Attacker);
	if (bTargetEnemy) bAttackerSide = !bAttackerSide;

	if (bAttackerSide)
	{
		Ctx.AttackerEffects.Add(NewEffect);
	}
	else
	{
		Ctx.DefenderEffects.Add(NewEffect);
	}

	UE_LOG(LogStrategosBattle, Log,
		TEXT("Effect_AddPersistent: '%s' (%.2f, %d rounds) ao %s"),
		*EffectId.ToString(), Value, Duration,
		bTargetEnemy ? TEXT("inimigo") : TEXT("próprio"));
}

FText UEffect_AddPersistent::GetDescriptionText() const
{
	return FText::Format(
		NSLOCTEXT("Battle", "PersistFmt", "{0} por {1} rounds"),
		EffectLabel,
		FText::AsNumber(Duration));
}

// ─────────────────────────────────────────────────────────────────────────────
// UEffect_RepositionSide
// ─────────────────────────────────────────────────────────────────────────────

bool UEffect_RepositionSide::CanApply(const FBattleContext& Ctx,
                                       const FBattleSide& Source,
                                       const FBattleSide& Target) const
{
	const FBattleSide& ActualTarget = bTargetEnemy ? Target : Source;
	return ActualTarget.Position != NewPosition;
}

void UEffect_RepositionSide::Apply(FBattleContext& Ctx,
                                    FBattleSide& Source, FBattleSide& Target)
{
	FBattleSide& ActualTarget = bTargetEnemy ? Target : Source;
	const EBattlePosition Old = ActualTarget.Position;
	ActualTarget.Position = NewPosition;

	UE_LOG(LogStrategosBattle, Log,
		TEXT("Effect_RepositionSide: posição %d → %d ao %s"),
		static_cast<int32>(Old),
		static_cast<int32>(NewPosition),
		bTargetEnemy ? TEXT("inimigo") : TEXT("próprio"));
}

FText UEffect_RepositionSide::GetDescriptionText() const
{
	return NSLOCTEXT("Battle", "RepositionDesc", "Reposiciona flanco");
}

// ─────────────────────────────────────────────────────────────────────────────
// UEffect_DrawCards
// ─────────────────────────────────────────────────────────────────────────────

void UEffect_DrawCards::Apply(FBattleContext& Ctx,
                               FBattleSide& Source, FBattleSide& Target)
{
	int32 Drawn = 0;
	for (int32 i = 0; i < CardsToDraw && !Source.DrawPile.IsEmpty(); ++i)
	{
		Source.Hand.Add(Source.DrawPile.Last());
		Source.DrawPile.Pop();
		++Drawn;
	}

	UE_LOG(LogStrategosBattle, Log,
		TEXT("Effect_DrawCards: comprou %d carta(s)"), Drawn);
}

FText UEffect_DrawCards::GetDescriptionText() const
{
	return FText::Format(
		NSLOCTEXT("Battle", "DrawFmt", "Compra {0} carta(s)"),
		FText::AsNumber(CardsToDraw));
}

// ─────────────────────────────────────────────────────────────────────────────
// UEffect_ExhaustEnemyCard
// ─────────────────────────────────────────────────────────────────────────────

bool UEffect_ExhaustEnemyCard::CanApply(const FBattleContext& /*Ctx*/,
                                         const FBattleSide& /*Source*/,
                                         const FBattleSide& Target) const
{
	return !Target.Hand.IsEmpty();
}

void UEffect_ExhaustEnemyCard::Apply(FBattleContext& /*Ctx*/,
                                      FBattleSide& /*Source*/, FBattleSide& Target)
{
	if (Target.Hand.IsEmpty()) return;

	int32 BestIdx = 0;
	int32 BestPrio = Target.Hand[0] ? Target.Hand[0]->Priority : 0;
	for (int32 i = 1; i < Target.Hand.Num(); ++i)
	{
		if (Target.Hand[i] && Target.Hand[i]->Priority > BestPrio)
		{
			BestPrio = Target.Hand[i]->Priority;
			BestIdx  = i;
		}
	}

	UBattleCardAsset* Exhausted = Target.Hand[BestIdx];
	Target.Hand.RemoveAt(BestIdx);
	if (Exhausted)
	{
		Target.ExhaustPile.Add(Exhausted);
		UE_LOG(LogStrategosBattle, Log,
			TEXT("Effect_ExhaustEnemyCard: '%s' esgotada da mão inimiga."),
			*Exhausted->DisplayName.ToString());
	}
}

FText UEffect_ExhaustEnemyCard::GetDescriptionText() const
{
	return NSLOCTEXT("Battle", "ExhaustEnemyDesc", "Esgotar melhor carta inimiga");
}
