#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Economy/GoodAmount.h"
#include "Economy/BuildingOwnerKind.h"
#include "BuildingTypeAsset.generated.h"

class UProductionMethodAsset;

/**
 * UBuildingTypeAsset — Definição de um tipo de prédio construível.
 *
 * O conjunto AvailableMethods determina o leque de PMs que o player pode
 * trocar no prédio em runtime. Pelo menos uma PM deve estar fora de
 * gating tech para a v1 (caso contrário o prédio inicia sem produzir).
 *
 * bRequiresRawResource + RequiredResourceId restringem onde o prédio
 * pode ser construído (Mine só em prov. com IronOre/Coal potential > 0).
 */
UCLASS(BlueprintType)
class STRATEGOSCORE_API UBuildingTypeAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
	EBuildingCategory Category = EBuildingCategory::Workshop;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building|Methods")
	TArray<TSoftObjectPtr<UProductionMethodAsset>> AvailableMethods;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building|Methods")
	TSoftObjectPtr<UProductionMethodAsset> DefaultMethod;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building|Construction")
	TArray<FGoodAmount> ConstructionCost;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building|Construction", meta = (ClampMin = "1"))
	int32 ConstructionDays = 30;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building|Construction", meta = (ClampMin = "0.0"))
	float ConstructionMonetaryCost = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building|Limits", meta = (ClampMin = "1"))
	int32 MaxLevel = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building|Placement")
	bool bRequiresRawResource = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building|Placement")
	FName RequiredResourceId;
};
