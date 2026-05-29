#include "BattleCardAsset.h"
#include "StrategosBattle.h"

// ─────────────────────────────────────────────────────────────────────────────
// UBattleEffect
// ─────────────────────────────────────────────────────────────────────────────

bool UBattleEffect::CanApply(const FBattleContext& Ctx,
                              const FBattleSide& Source,
                              const FBattleSide& Target) const
{
	return true;
}

void UBattleEffect::Apply(FBattleContext& Ctx,
                           FBattleSide& Source,
                           FBattleSide& Target)
{
	UE_LOG(LogStrategosBattle, Warning,
		TEXT("UBattleEffect::Apply chamado na base — override necessário."));
}

FText UBattleEffect::GetDescriptionText() const
{
	return NSLOCTEXT("Battle", "EffectBase", "Efeito");
}

// ─────────────────────────────────────────────────────────────────────────────
// UBattleCardAsset
// ─────────────────────────────────────────────────────────────────────────────

bool UBattleCardAsset::IsValidInContext(const FBattleContext& Ctx,
                                         const FBattleSide& Side) const
{
	// Fase
	if (!ValidPhases.IsEmpty() && !ValidPhases.Contains(Ctx.CurrentPhase))
	{
		return false;
	}

	// Terreno
	if (!Conditions.RequiredTerrain.IsEmpty()
		&& !Conditions.RequiredTerrain.Contains(Ctx.Terrain))
	{
		return false;
	}

	// Fase (duplica ValidPhases para suportar ambas as abordagens)
	if (!Conditions.RequiredPhases.IsEmpty()
		&& !Conditions.RequiredPhases.Contains(Ctx.CurrentPhase))
	{
		return false;
	}

	// Moral mínima
	if (Side.Morale < Conditions.MinMorale) return false;

	// Suprimento mínimo
	if (Side.Supply < Conditions.MinSupply) return false;

	// Command Points
	if (CommandCost > Side.CommandPoints) return false;

	return true;
}
