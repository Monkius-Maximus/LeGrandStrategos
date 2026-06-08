#pragma once

#include "CoreMinimal.h"
#include "WorldGenTypes.h"

/**
 * Estagio 2 — Voronoi via delaunator-cpp.
 *
 * delaunator calcula a triangulacao de Delaunay; o Voronoi e o dual: cada
 * vertice Voronoi e o circuncentro de um triangulo. A celula de cada ponto-
 * semente e o poligono dos circuncentros dos triangulos que o tocam.
 */
namespace StrategosWorldGen::Voronoi
{
	/**
	 * Constroi celulas, vizinhanca e poligonos a partir dos pontos.
	 * Retorna false se delaunator falhar (poucos pontos / colineares).
	 */
	STRATEGOSWORLDGEN_API bool Build(const TArray<FVector2D>& Points, FWorldGenResult& OutResult);

	/**
	 * Relaxamento de Lloyd: move cada ponto ao centroide do seu poligono e
	 * recalcula. Iterations = 1-2 deixa as celulas mais uniformes (default FMG).
	 */
	STRATEGOSWORLDGEN_API TArray<FVector2D> LloydRelax(
		const TArray<FVector2D>& Points,
		const FIntPoint& MapSize,
		int32 Iterations);
}
