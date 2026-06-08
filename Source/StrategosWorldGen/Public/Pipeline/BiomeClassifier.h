#pragma once

#include "CoreMinimal.h"
#include "WorldGenTypes.h"

class FWorldGenContext;

/**
 * Estagio 8 — Biomas (Whittaker). Matriz fixa temperatura x precipitacao.
 * Lookup, nao calculo.
 */
namespace StrategosWorldGen::Biome
{
	/** Classifica uma celula isolada. */
	STRATEGOSWORLDGEN_API EBiomeType Classify(float TempC, float PrecipMm, bool bIsWater);

	/** Aplica Classify a todas as celulas do contexto. */
	STRATEGOSWORLDGEN_API void ClassifyAll(FWorldGenContext& Ctx);
}
