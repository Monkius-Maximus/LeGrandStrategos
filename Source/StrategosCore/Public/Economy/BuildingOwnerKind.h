#pragma once

#include "CoreMinimal.h"
#include "BuildingOwnerKind.generated.h"

UENUM(BlueprintType)
enum class EBuildingOwnerKind : uint8
{
	Government	UMETA(DisplayName = "Government"),
	Private		UMETA(DisplayName = "Private (Bourgeoisie)")
};

UENUM(BlueprintType)
enum class EBuildingCategory : uint8
{
	Farm,
	Mine,
	Plantation,
	Forester,
	Workshop,
	Factory,
	Refinery
};
