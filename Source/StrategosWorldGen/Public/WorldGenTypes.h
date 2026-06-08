#pragma once

#include "CoreMinimal.h"
#include "WorldGenTypes.generated.h"

/**
 * EBiomeType — Classificacao Whittaker (temperatura x precipitacao).
 *
 * 12 biomas terrestres + Ocean + River, suficientes para o pipeline de
 * estudo. Espelha grosso modo a paleta do Fantasy Map Generator.
 */
UENUM()
enum class EBiomeType : uint8
{
	None,
	Ocean,
	Glacier,
	PolarDesert,
	Tundra,
	ColdDesert,
	Steppe,
	TemperateGrassland,
	TemperateForest,
	TemperateRainforest,
	Desert,
	Savanna,
	TropicalSeasonalForest,
	TropicalRainforest,
	HotDesert,
	River
};

/**
 * EBlobType — Operadores de heightmap (espelho do heightmap-templates do FMG).
 *
 * Cada operador deforma o campo de altura de um jeito; templates combinam
 * varios numa receita (ver UHeightmapGenerator).
 */
UENUM()
enum class EBlobType : uint8
{
	Hill,       // colina redonda
	Mountain,   // pico isolado grande
	Range,      // cordilheira alongada
	Pit,        // depressao redonda
	Trough,     // depressao alongada
	Strait,     // canal vertical
	Add,        // soma constante
	Multiply,   // multiplica tudo
	Smooth      // suaviza
};

/**
 * FWorldCell — Celula Voronoi do mapa.
 *
 * Vizinhanca por indices em FWorldGenResult::Cells (nunca ponteiros: o array
 * pode realocar e indices sobrevivem a serializacao/save).
 *
 * Os campos nao sao UPROPERTY de proposito: FWorldGenResult e um container
 * de runtime do pipeline, nao um asset reflectido. A persistencia, quando
 * existir, sera por serializacao custom no UWorldGenDataAsset.
 */
USTRUCT()
struct FWorldCell
{
	GENERATED_BODY()

	FVector2D Position = FVector2D::ZeroVector;
	TArray<int32> Neighbors;        // indices em Cells[]
	TArray<int32> PolygonVertices;  // indices em VoronoiVertices[]

	float Height = 0.0f;            // 0..1, sea level default 0.2
	float Temperature = 0.0f;       // graus C
	float Precipitation = 0.0f;     // mm/ano (escala arbitraria)
	EBiomeType Biome = EBiomeType::None;

	bool bIsWater = false;
	bool bIsCoast = false;
	bool bIsBorder = false;         // toca a borda do mapa
	bool bIsRiver = false;
};

/**
 * FWorldGenResult — Saida completa do pipeline.
 *
 * Array contiguo de celulas + geometria Voronoi para render. Tudo o que vem
 * depois (estados, provincias) consome este resultado em passos separados.
 */
USTRUCT()
struct FWorldGenResult
{
	GENERATED_BODY()

	int32 Seed = 0;
	FIntPoint MapSize = FIntPoint(1024, 1024);
	float SeaLevel = 0.2f;
	float EquatorY = 512.0f;        // Y (em pixels) onde fica o equador

	TArray<FWorldCell> Cells;
	TArray<FVector2D> VoronoiVertices;
};
