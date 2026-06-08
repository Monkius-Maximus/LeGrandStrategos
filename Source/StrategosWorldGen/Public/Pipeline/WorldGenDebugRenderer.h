#pragma once

#include "CoreMinimal.h"
#include "WorldGenTypes.h"
#include "WorldGenDebugRenderer.generated.h"

class UTexture2D;

/**
 * Modos de visualizacao de debug, espelhando a progressao da Secao 16:
 * cada modo valida um estagio do pipeline.
 */
UENUM(BlueprintType)
enum class EWorldGenRenderMode : uint8
{
	RandomCells,  // cor aleatoria por celula (valida Voronoi)
	Height,       // escala de cinza (valida heightmap)
	Coast,        // azul/marrom (valida coastline)
	Temperature,  // escala termica (valida temperatura)
	Precipitation,// seco->umido
	Biomes        // paleta tipo FMG + rios
};

/**
 * Estagio 11 — Render de debug (Opcao A: UTexture2D rasterizada).
 *
 * Pinta cada pixel pela cor da celula mais proxima. Usa hash espacial para
 * evitar o nearest-cell O(N) por pixel.
 */
namespace StrategosWorldGen::DebugRender
{
	STRATEGOSWORLDGEN_API FColor BiomeColor(EBiomeType Biome);

	/** Preenche OutPixels (MapSize.X * MapSize.Y, row-major) com a cor por pixel. */
	STRATEGOSWORLDGEN_API bool BuildPixels(const FWorldGenResult& Result, EWorldGenRenderMode Mode, TArray<FColor>& OutPixels);

	/** Gera uma UTexture2D transiente MapSize.X x MapSize.Y. Pode retornar nullptr. */
	STRATEGOSWORLDGEN_API UTexture2D* RenderToTexture(const FWorldGenResult& Result, EWorldGenRenderMode Mode);

	/** Exporta o resultado como PNG no caminho absoluto dado. Retorna false em falha. */
	STRATEGOSWORLDGEN_API bool SaveToPng(const FString& AbsolutePath, const FWorldGenResult& Result, EWorldGenRenderMode Mode);
}
