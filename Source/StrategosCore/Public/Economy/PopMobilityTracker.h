#pragma once

#include "CoreMinimal.h"
#include "PopMobilityTracker.generated.h"

/**
 * FPopMobilityTracker — Contador acumulativo da "Mobilidade Social"
 * (doc §6, metáfora do Copo D'água).
 *
 * QualityOfLifePoints sobe quando o estrato tem acesso pleno a needs
 * (basket cheio + luxo opcional) e cai em meses de escassez. A cada
 * tick mensal, o EconomySubsystem checa se o tracker passou
 * PromotionThreshold por >= MinMonthsAtThreshold meses; se sim,
 * promove uma fração (5–10%) dos POPs ao próximo estrato.
 *
 * Mesma estrutura serve à demoção (DemotionThreshold inferior a zero).
 */
USTRUCT(BlueprintType)
struct STRATEGOSCORE_API FPopMobilityTracker
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Mobility")
	float QualityOfLifePoints = 0.f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Mobility", meta = (ClampMin = "0"))
	int32 MonthsAboveThreshold = 0;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Mobility", meta = (ClampMin = "0"))
	int32 MonthsBelowThreshold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mobility|Tuning")
	float PromotionThreshold = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mobility|Tuning")
	float DemotionThreshold = -50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mobility|Tuning", meta = (ClampMin = "1"))
	int32 MinMonthsAtThreshold = 12;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mobility|Tuning", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PromotionFraction = 0.07f;
};
