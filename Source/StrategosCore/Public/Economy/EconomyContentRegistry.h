#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EconomyContentRegistry.generated.h"

class UGoodAsset;
class UProductionMethodAsset;
class UProductionModifierAsset;
class UBuildingTypeAsset;

/**
 * UEconomyContentRegistry — Catálogo plugável dos DataAssets econômicos.
 *
 * Aponta para todos os UGoodAsset / UProductionMethodAsset /
 * UProductionModifierAsset / UBuildingTypeAsset que existem no jogo.
 * O EconomySubsystem consulta este registro em Initialize para montar
 * lookups por Id, em vez de varrer o asset registry inteiro.
 *
 * Designers/modders editam um único registry asset (`DA_EconomyRegistry`)
 * para ativar/desativar conteúdo sem tocar código.
 */
UCLASS(BlueprintType)
class STRATEGOSCORE_API UEconomyContentRegistry : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Registry")
	TArray<TSoftObjectPtr<UGoodAsset>> Goods;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Registry")
	TArray<TSoftObjectPtr<UProductionMethodAsset>> ProductionMethods;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Registry")
	TArray<TSoftObjectPtr<UProductionModifierAsset>> ProductionModifiers;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Registry")
	TArray<TSoftObjectPtr<UBuildingTypeAsset>> BuildingTypes;
};
