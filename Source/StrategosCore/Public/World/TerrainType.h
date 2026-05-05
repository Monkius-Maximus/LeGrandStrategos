#pragma once

#include "CoreMinimal.h"
#include "TerrainType.generated.h"

UENUM(BlueprintType)
enum class ETerrainType : uint8
{
	Plains		UMETA(DisplayName = "Plains"),
	Forest		UMETA(DisplayName = "Forest"),
	Hills		UMETA(DisplayName = "Hills"),
	Mountains	UMETA(DisplayName = "Mountains"),
	Desert		UMETA(DisplayName = "Desert"),
	Tundra		UMETA(DisplayName = "Tundra"),
	Marsh		UMETA(DisplayName = "Marsh"),
	Coast		UMETA(DisplayName = "Coast"),
	Water		UMETA(DisplayName = "Water")
};
