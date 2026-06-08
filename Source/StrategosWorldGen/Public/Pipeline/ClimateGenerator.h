#pragma once

#include "CoreMinimal.h"

class FWorldGenContext;

/**
 * Estagios 4-7 — Coastline, Temperatura, Vento e Precipitacao.
 *
 * Modelos fechados e baratos (sem feedback, sem sazonalidade): suficientes
 * para alimentar a classificacao de biomas Whittaker.
 */
namespace StrategosWorldGen::Climate
{
	/** Estagio 4: marca bIsWater (Height < SeaLevel) e bIsCoast (terra tocando agua). */
	STRATEGOSWORLDGEN_API void DetectWaterAndCoast(FWorldGenContext& Ctx);

	/** Estagio 5: gradiente de latitude + lapse rate por altitude. */
	STRATEGOSWORLDGEN_API void ComputeTemperature(FWorldGenContext& Ctx);

	/** Direcao do vento por banda de latitude (Hadley/Ferrel/Polar). */
	STRATEGOSWORLDGEN_API FVector2D GetWindForCell(const FVector2D& Position, float EquatorY, float HalfHeight);

	/** Estagio 7: nuvens emitidas na borda upwind marcham e despejam chuva. */
	STRATEGOSWORLDGEN_API void ComputePrecipitation(FWorldGenContext& Ctx);
}
