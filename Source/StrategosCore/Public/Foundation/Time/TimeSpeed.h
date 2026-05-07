#pragma once

#include "CoreMinimal.h"
#include "TimeSpeed.generated.h"

UENUM(BlueprintType)
enum class ETimeSpeed : uint8
{
	Paused	UMETA(DisplayName = "Paused"),
	Slow	UMETA(DisplayName = "Slow (1x)"),
	Normal	UMETA(DisplayName = "Normal (2x)"),
	Fast	UMETA(DisplayName = "Fast (4x)"),
	Fastest	UMETA(DisplayName = "Fastest (8x)")
};
