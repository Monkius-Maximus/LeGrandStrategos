#pragma once

#include "CoreMinimal.h"
#include "WorldGenTypes.h"

/**
 * Utilidades geometricas compartilhadas entre estagios.
 */
namespace StrategosWorldGen::Math
{
	/** Circuncentro do triangulo ABC (= vertice Voronoi do dual). */
	STRATEGOSWORLDGEN_API FVector2D Circumcenter(const FVector2D& A, const FVector2D& B, const FVector2D& C);

	/** Indice da celula cujo Position e o mais proximo de P (busca linear). */
	STRATEGOSWORLDGEN_API int32 FindNearestCell(const TArray<FWorldCell>& Cells, const FVector2D& P);

	/**
	 * FSpatialGrid — Hash espacial uniforme sobre os centros das celulas.
	 *
	 * Acelera o nearest-cell do render de debug (ingenuo seria O(N) por pixel).
	 */
	class STRATEGOSWORLDGEN_API FSpatialGrid
	{
	public:
		void Build(const TArray<FWorldCell>& Cells, const FIntPoint& MapSize, float CellSize);
		int32 FindNearest(const TArray<FWorldCell>& Cells, const FVector2D& P) const;

	private:
		float CellSize = 16.0f;
		int32 GridW = 0;
		int32 GridH = 0;
		TArray<TArray<int32>> Buckets;
	};
}
