#pragma once

#include "CoreMinimal.h"
#include "Economy/PopStratum.h"
#include "PopGroup.generated.h"

/**
 * FPopGroup — Agregado de POPs de um único estrato dentro de uma província.
 *
 * Population: headcount (não fracionado). Wealth: capital acumulado pelo
 * grupo (Bourgeoisie usa para auto-investir; outros para qualidade de vida).
 * Loyalty: [0..1], baseline neutra; vira militancy real com Politics.
 *
 * EmployedThisMonth e WageEarnedLastMonth são caches da última fase do tick
 * para alimentar a UI sem ter que reconsultar Buildings.
 */
USTRUCT(BlueprintType)
struct STRATEGOSCORE_API FPopGroup
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Pop")
	EPopStratum Stratum = EPopStratum::Laborer;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Pop")
	int32 Population = 0;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Pop")
	float Wealth = 0.f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Pop")
	float Loyalty = 1.0f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Pop|Cache")
	int32 EmployedThisMonth = 0;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Pop|Cache")
	float WageEarnedLastMonth = 0.f;
};
