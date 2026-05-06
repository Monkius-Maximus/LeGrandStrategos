#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WorldBootstrapAsset.generated.h"

class UDataTable;

/**
 * UWorldBootstrapAsset — receita de cenário inicial.
 *
 * Aglutina referências às DataTables que definem o mundo de partida
 * (províncias, nações, exércitos, ideias nacionais) e qual nação o
 * jogador controla. Designers criam um asset destes por cenário
 * (sandbox padrão, 1836, ducados feudais, etc) e apontam o GameMode.
 */
UCLASS(BlueprintType)
class STRATEGOSCORE_API UWorldBootstrapAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bootstrap")
	FText ScenarioName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bootstrap")
	int32 StartYear = 1836;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bootstrap")
	int32 StartMonth = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bootstrap")
	int32 StartDay = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bootstrap")
	FName PlayerNationId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bootstrap|Tables", meta = (RequiredAssetDataTags = "RowStructure=/Script/StrategosCore.ProvinceRow"))
	TSoftObjectPtr<UDataTable> ProvincesTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bootstrap|Tables", meta = (RequiredAssetDataTags = "RowStructure=/Script/StrategosCore.NationRow"))
	TSoftObjectPtr<UDataTable> NationsTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bootstrap|Tables", meta = (RequiredAssetDataTags = "RowStructure=/Script/StrategosCore.ArmyRow"))
	TSoftObjectPtr<UDataTable> ArmiesTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bootstrap|Tables", meta = (RequiredAssetDataTags = "RowStructure=/Script/StrategosCore.NationalIdeaRow"))
	TSoftObjectPtr<UDataTable> NationalIdeasTable;
};
