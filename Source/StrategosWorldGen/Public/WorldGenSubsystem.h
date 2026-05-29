#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WorldGenTypes.h"
#include "WorldGenParams.h"
#include "Pipeline/WorldGenDebugRenderer.h"
#include "WorldGenSubsystem.generated.h"

class UTexture2D;
class UWorldGenDataAsset;

/**
 * UWorldGenSubsystem — Entry point do pipeline experimental de worldgen.
 *
 * GameInstanceSubsystem (independente de mundo carregado): gera um mundo a
 * partir de FWorldGenParams (ou de um UWorldGenDataAsset) e expoe um render
 * de debug para inspecao visual. O resultado fica em LastResult para um
 * eventual passo posterior (estados/provincias) consumir.
 *
 * Este modulo e um sub-projeto de estudo, nao parte do MVP do jogo.
 */
UCLASS()
class STRATEGOSWORLDGEN_API UWorldGenSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Roda o pipeline completo e guarda o resultado em LastResult. */
	UFUNCTION(BlueprintCallable, Category = "WorldGen")
	bool GenerateWorld(const FWorldGenParams& Params);

	/** Conveniencia: gera a partir de um preset. */
	UFUNCTION(BlueprintCallable, Category = "WorldGen")
	bool GenerateFromAsset(const UWorldGenDataAsset* Asset);

	/** Rasteriza o ultimo resultado num UTexture2D transiente. */
	UFUNCTION(BlueprintCallable, Category = "WorldGen")
	UTexture2D* RenderDebugTexture(EWorldGenRenderMode Mode = EWorldGenRenderMode::Biomes);

	UFUNCTION(BlueprintPure, Category = "WorldGen")
	bool HasResult() const { return bHasResult; }

	UFUNCTION(BlueprintPure, Category = "WorldGen")
	int32 GetCellCount() const { return LastResult.Cells.Num(); }

	const FWorldGenResult& GetLastResult() const { return LastResult; }

private:
	FWorldGenResult LastResult;
	bool bHasResult = false;
};
