#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WorldGenParams.h"
#include "WorldGenDataAsset.generated.h"

/**
 * UWorldGenDataAsset — Configuracao designer-editavel de um preset de mundo.
 *
 * Guarda apenas os FWorldGenParams (entrada). O FWorldGenResult (saida) e um
 * container de runtime com containers aninhados (TArray<TArray>) que nao sao
 * reflectiveis como UPROPERTY; quando a persistencia for necessaria, vira via
 * serializacao custom (Serialize) — fora do escopo do pipeline de estudo.
 *
 * Segue a filosofia "DataAssets > codigo hardcoded" do projeto.
 */
UCLASS(BlueprintType)
class STRATEGOSWORLDGEN_API UWorldGenDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen")
	FWorldGenParams Params;
};
