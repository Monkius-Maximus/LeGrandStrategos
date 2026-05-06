#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ProductionModifierAsset.generated.h"

class UTexture2D;

/**
 * UProductionModifierAsset — Toggle ativo num prédio.
 *
 * Modifiers no mesmo MutexGroup são exclusivos (só 1 ativo por vez por
 * prédio). MutexGroup vazio = livre (player pode acumular sem
 * conflito). v1 usa 3 mutex groups: "Pace", "Labor", "Quality".
 *
 * Multiplicadores são compostos multiplicativamente sobre o valor
 * baseline da PM ativa.
 */
UCLASS(BlueprintType)
class STRATEGOSCORE_API UProductionModifierAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifier")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifier")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifier")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifier")
	FText Description;

	/** Vazio = livre. Não-vazio = exclusivo dentro do grupo. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifier|Mutex")
	FName MutexGroup;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifier|Effect", meta = (ClampMin = "0.1"))
	float ThroughputMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifier|Effect", meta = (ClampMin = "0.1"))
	float InputCostMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifier|Effect", meta = (ClampMin = "0.1"))
	float WageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifier|Effect", meta = (ClampMin = "0.1"))
	float MaintenanceMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifier|Effect")
	float MonthlyLoyaltyDelta = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifier|Gating")
	FName RequiredTechId;
};
