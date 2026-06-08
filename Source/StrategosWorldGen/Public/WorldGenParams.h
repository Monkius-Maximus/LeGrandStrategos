#pragma once

#include "CoreMinimal.h"
#include "WorldGenParams.generated.h"

/**
 * EWorldTemplate — Receita de heightmap a aplicar.
 */
UENUM(BlueprintType)
enum class EWorldTemplate : uint8
{
	Continent,
	Archipelago,
	Pangaea
};

/**
 * FWorldGenParams — Parametros de entrada do pipeline.
 *
 * Editavel no editor / Blueprint. Todos os estagios derivam seus valores
 * daqui; nada de constantes magicas espalhadas. Mesma seed + mesmos params
 * = mesmo mundo (determinismo).
 */
USTRUCT(BlueprintType)
struct STRATEGOSWORLDGEN_API FWorldGenParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geral")
	int32 Seed = 1337;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geral")
	FIntPoint MapSize = FIntPoint(1024, 1024);

	/** Distancia minima entre pontos no Poisson-disc. ~9 => ~10k celulas em 1024x1024. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sampling", meta = (ClampMin = "2.0"))
	float PoissonMinDist = 9.0f;

	/** Iteracoes de relaxamento de Lloyd (0 = nenhum). FMG usa ~1-2. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sampling", meta = (ClampMin = "0", ClampMax = "5"))
	int32 LloydIterations = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap")
	EWorldTemplate Template = EWorldTemplate::Continent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SeaLevel = 0.2f;

	/** Amplitude do Perlin somado ao heightmap para quebrar a redondeza dos blobs. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float PerlinAmplitude = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clima")
	float EquatorTemp = 27.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clima")
	float PoleTemp = -25.0f;

	/** Altura (m) correspondente a Height=1.0, usada no lapse rate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clima", meta = (ClampMin = "100.0"))
	float MaxAltitudeM = 5000.0f;

	/** Vapor inicial de cada nuvem emitida na borda upwind. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clima", meta = (ClampMin = "0.0"))
	float InitialMoisture = 120.0f;

	/** Fluxo acumulado acima do qual uma celula vira rio. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rios", meta = (ClampMin = "0.0"))
	float RiverFlowThreshold = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rios")
	bool bGenerateRivers = true;
};
