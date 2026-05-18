#include "HUD/StrategosArmyCardWidget.h"
#include "StrategosUI.h"
#include "World/Army.h"
#include "World/Nation.h"
#include "World/UnitTypeAsset.h"
#include "World/WorldState.h"
#include "Game/StrategosGameState.h"
#include "Engine/World.h"

void UStrategosArmyCardWidget::SetArmy(UArmy* InArmy)
{
	BoundArmy = InArmy;
	OnArmyBound(InArmy);
}

void UStrategosArmyCardWidget::SetVariant(EArmyCardVariant InVariant)
{
	Variant = InVariant;
}

// ── Header ────────────────────────────────────────────────────────────────────

FText UStrategosArmyCardWidget::GetUnitDisplayName() const
{
	if (!BoundArmy) return FText::GetEmpty();
	if (UUnitTypeAsset* UT = BoundArmy->UnitType.LoadSynchronous())
	{
		return UT->DisplayName;
	}
	return BoundArmy->DisplayName;
}

FText UStrategosArmyCardWidget::GetUnitRole() const
{
	if (!BoundArmy) return FText::GetEmpty();
	if (UUnitTypeAsset* UT = BoundArmy->UnitType.LoadSynchronous())
	{
		return UT->Role;
	}
	return FText::GetEmpty();
}

FLinearColor UStrategosArmyCardWidget::GetNationColor() const
{
	if (!BoundArmy) return FLinearColor::White;
	const UWorld* W = GetWorld();
	if (!W) return FLinearColor::White;
	const AStrategosGameState* GS = W->GetGameState<AStrategosGameState>();
	if (!GS || !GS->GetWorldState()) return FLinearColor::White;
	if (const UNation* N = GS->GetWorldState()->GetNation(BoundArmy->OwnerNationId))
	{
		return FLinearColor(N->Color);
	}
	return FLinearColor::White;
}

int32 UStrategosArmyCardWidget::GetMaintenanceCost() const
{
	if (!BoundArmy) return 0;
	return BoundArmy->BaseStats.CST;
}

// ── Portrait ─────────────────────────────────────────────────────────────────

UTexture2D* UStrategosArmyCardWidget::GetPortrait() const
{
	if (!BoundArmy) return nullptr;
	if (UUnitTypeAsset* UT = BoundArmy->UnitType.LoadSynchronous())
	{
		return UT->Portrait.LoadSynchronous();
	}
	return nullptr;
}

EUnitState UStrategosArmyCardWidget::GetUnitState() const
{
	return BoundArmy ? BoundArmy->State : EUnitState::Ready;
}

// ── Stats ────────────────────────────────────────────────────────────────────

FArmyStats UStrategosArmyCardWidget::GetEffectiveStats() const
{
	if (!BoundArmy) return {};
	FArmyStats Effective = BoundArmy->BaseStats;
	for (const FArmyModifier& Mod : BoundArmy->ActiveModifiers)
	{
		// Simplified: modifiers tagged by Id prefix (ATQ_, DEF_, etc.)
		const FString ModId = Mod.Id.ToString();
		const float Delta   = Mod.bPositive ? Mod.Value : -Mod.Value;

		if      (ModId.StartsWith(TEXT("ATQ"))) Effective.ATQ += FMath::RoundToInt(Delta);
		else if (ModId.StartsWith(TEXT("DEF"))) Effective.DEF += FMath::RoundToInt(Delta);
		else if (ModId.StartsWith(TEXT("MOB"))) Effective.MOB += FMath::RoundToInt(Delta);
		else if (ModId.StartsWith(TEXT("MOR"))) Effective.MOR += FMath::RoundToInt(Delta);
	}
	return Effective;
}

int32 UStrategosArmyCardWidget::GetManpower() const
{
	return BoundArmy ? BoundArmy->ManpowerCount : 0;
}

// ── Trait ────────────────────────────────────────────────────────────────────

FText UStrategosArmyCardWidget::GetPrimaryTrait() const
{
	if (!BoundArmy) return FText::GetEmpty();
	if (UUnitTypeAsset* UT = BoundArmy->UnitType.LoadSynchronous())
	{
		return UT->PrimaryTrait;
	}
	return FText::GetEmpty();
}

FText UStrategosArmyCardWidget::GetPrimaryTraitDescription() const
{
	if (!BoundArmy) return FText::GetEmpty();
	if (UUnitTypeAsset* UT = BoundArmy->UnitType.LoadSynchronous())
	{
		return UT->PrimaryTraitDescription;
	}
	return FText::GetEmpty();
}

// ── XP / Level ───────────────────────────────────────────────────────────────

int32 UStrategosArmyCardWidget::GetExperienceXP() const
{
	return BoundArmy ? BoundArmy->ExperienceXP : 0;
}

int32 UStrategosArmyCardWidget::GetExperienceLevel() const
{
	return BoundArmy ? BoundArmy->ExperienceLevel : 0;
}

float UStrategosArmyCardWidget::GetXPProgress() const
{
	if (!BoundArmy) return 0.f;
	const int32 XPIntoLevel = BoundArmy->ExperienceXP % XPPerLevel;
	return static_cast<float>(XPIntoLevel) / XPPerLevel;
}

// ── Modifiers ────────────────────────────────────────────────────────────────

TArray<FArmyModifier> UStrategosArmyCardWidget::GetActiveModifiers() const
{
	if (!BoundArmy) return {};
	return BoundArmy->ActiveModifiers;
}
