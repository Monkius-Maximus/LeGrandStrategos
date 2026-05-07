#pragma once

#include "CoreMinimal.h"
#include "TaxLevel.generated.h"

UENUM(BlueprintType)
enum class ETaxLevel : uint8
{
	VeryLow		UMETA(DisplayName = "Very Low (0.5x)"),
	Low			UMETA(DisplayName = "Low (0.75x)"),
	Medium		UMETA(DisplayName = "Medium (1.0x)"),
	High		UMETA(DisplayName = "High (1.5x)"),
	VeryHigh	UMETA(DisplayName = "Very High (2.0x)")
};

namespace StrategosTax
{
	FORCEINLINE float Multiplier(ETaxLevel Level)
	{
		switch (Level)
		{
			case ETaxLevel::VeryLow:  return 0.50f;
			case ETaxLevel::Low:      return 0.75f;
			case ETaxLevel::Medium:   return 1.00f;
			case ETaxLevel::High:     return 1.50f;
			case ETaxLevel::VeryHigh: return 2.00f;
			default:                  return 1.00f;
		}
	}

	/** Loyalty mensal aplicada ao estrato taxado (negativa para níveis altos). */
	FORCEINLINE float LoyaltyDelta(ETaxLevel Level)
	{
		switch (Level)
		{
			case ETaxLevel::VeryLow:  return +0.01f;
			case ETaxLevel::Low:      return +0.005f;
			case ETaxLevel::Medium:   return  0.f;
			case ETaxLevel::High:     return -0.01f;
			case ETaxLevel::VeryHigh: return -0.03f;
			default:                  return  0.f;
		}
	}
}
