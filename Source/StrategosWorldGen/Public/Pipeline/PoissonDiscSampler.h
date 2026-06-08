#pragma once

#include "CoreMinimal.h"

/**
 * Estagio 1 — Sampling de pontos (Bridson 2007, "Fast Poisson Disk Sampling").
 *
 * Distribui pontos com distancia minima MinDist entre eles: celulas de
 * tamanho razoavelmente uniforme com jitter natural. Determinismo via Stream.
 */
namespace StrategosWorldGen::PoissonDisc
{
	/** Gera pontos no retangulo [0,Size). K = candidatos por ponto ativo. */
	STRATEGOSWORLDGEN_API TArray<FVector2D> Sample(
		const FIntPoint& Size,
		float MinDist,
		FRandomStream& Stream,
		int32 K = 30);
}
