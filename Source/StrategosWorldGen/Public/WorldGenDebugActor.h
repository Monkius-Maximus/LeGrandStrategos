#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldGenParams.h"
#include "Pipeline/WorldGenDebugRenderer.h"
#include "WorldGenDebugActor.generated.h"

class UTexture2D;

/**
 * AWorldGenDebugActor — Teste de 1 clique do pipeline de worldgen.
 *
 * Arraste para um nivel, ajuste Params no painel de detalhes e de Play (PIE).
 * No BeginPlay gera o mundo e exporta PNG(s) de debug para
 * <Projeto>/Saved/WorldGen/ — abra a pasta para inspecionar o resultado
 * (ou anexe o PNG numa conversa para validacao). Zero assets necessarios.
 *
 * Modulo experimental; nao faz parte do MVP.
 */
UCLASS()
class STRATEGOSWORLDGEN_API AWorldGenDebugActor : public AActor
{
	GENERATED_BODY()

public:
	AWorldGenDebugActor();

	/** Parametros de geracao. Mesma seed = mesmo mundo. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen")
	FWorldGenParams Params;

	/** Modo exportado quando bExportAllModes = false. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen")
	EWorldGenRenderMode Mode = EWorldGenRenderMode::Biomes;

	/** Exporta todos os modos (RandomCells..Biomes) num PNG cada. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen")
	bool bExportAllModes = true;

	/** Gera automaticamente no BeginPlay. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen")
	bool bGenerateOnBeginPlay = true;

	/** Roda o pipeline e exporta o(s) PNG(s). Tambem chamavel via Blueprint. */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "WorldGen")
	void GenerateAndExport();

	/** Ultima textura gerada (modo Mode), util para exibir num material/UI. */
	UFUNCTION(BlueprintPure, Category = "WorldGen")
	UTexture2D* GetLastTexture() const { return LastTexture; }

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> LastTexture = nullptr;

	static FString ModeName(EWorldGenRenderMode InMode);
};
