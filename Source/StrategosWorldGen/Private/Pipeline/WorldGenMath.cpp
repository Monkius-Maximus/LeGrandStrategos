#include "Pipeline/WorldGenMath.h"

namespace StrategosWorldGen::Math
{
	FVector2D Circumcenter(const FVector2D& A, const FVector2D& B, const FVector2D& C)
	{
		const double Ax = A.X, Ay = A.Y;
		const double Bx = B.X, By = B.Y;
		const double Cx = C.X, Cy = C.Y;

		const double D = 2.0 * (Ax * (By - Cy) + Bx * (Cy - Ay) + Cx * (Ay - By));
		if (FMath::IsNearlyZero(D))
		{
			// Triangulo degenerado (colinear): centroide como fallback.
			return FVector2D((A.X + B.X + C.X) / 3.0f, (A.Y + B.Y + C.Y) / 3.0f);
		}

		const double A2 = Ax * Ax + Ay * Ay;
		const double B2 = Bx * Bx + By * By;
		const double C2 = Cx * Cx + Cy * Cy;

		const double Ux = (A2 * (By - Cy) + B2 * (Cy - Ay) + C2 * (Ay - By)) / D;
		const double Uy = (A2 * (Cx - Bx) + B2 * (Ax - Cx) + C2 * (Bx - Ax)) / D;

		return FVector2D(static_cast<float>(Ux), static_cast<float>(Uy));
	}

	int32 FindNearestCell(const TArray<FWorldCell>& Cells, const FVector2D& P)
	{
		int32 Best = INDEX_NONE;
		float BestDistSq = TNumericLimits<float>::Max();
		for (int32 i = 0; i < Cells.Num(); ++i)
		{
			const float DistSq = FVector2D::DistSquared(Cells[i].Position, P);
			if (DistSq < BestDistSq)
			{
				BestDistSq = DistSq;
				Best = i;
			}
		}
		return Best;
	}

	void FSpatialGrid::Build(const TArray<FWorldCell>& Cells, const FIntPoint& MapSize, float InCellSize)
	{
		CellSize = FMath::Max(1.0f, InCellSize);
		GridW = FMath::Max(1, FMath::CeilToInt(MapSize.X / CellSize));
		GridH = FMath::Max(1, FMath::CeilToInt(MapSize.Y / CellSize));

		Buckets.Empty();
		Buckets.SetNum(GridW * GridH);

		for (int32 i = 0; i < Cells.Num(); ++i)
		{
			const int32 GX = FMath::Clamp(FMath::FloorToInt(Cells[i].Position.X / CellSize), 0, GridW - 1);
			const int32 GY = FMath::Clamp(FMath::FloorToInt(Cells[i].Position.Y / CellSize), 0, GridH - 1);
			Buckets[GY * GridW + GX].Add(i);
		}
	}

	int32 FSpatialGrid::FindNearest(const TArray<FWorldCell>& Cells, const FVector2D& P) const
	{
		if (Buckets.Num() == 0)
		{
			return FindNearestCell(Cells, P);
		}

		const int32 CX = FMath::Clamp(FMath::FloorToInt(P.X / CellSize), 0, GridW - 1);
		const int32 CY = FMath::Clamp(FMath::FloorToInt(P.Y / CellSize), 0, GridH - 1);

		int32 Best = INDEX_NONE;
		float BestDistSq = TNumericLimits<float>::Max();

		// Anel crescente ate achar candidato e cobrir a distancia do melhor.
		for (int32 Ring = 0; Ring < FMath::Max(GridW, GridH); ++Ring)
		{
			for (int32 dy = -Ring; dy <= Ring; ++dy)
			{
				for (int32 dx = -Ring; dx <= Ring; ++dx)
				{
					// So o perimetro do anel atual.
					if (FMath::Max(FMath::Abs(dx), FMath::Abs(dy)) != Ring)
					{
						continue;
					}
					const int32 NX = CX + dx;
					const int32 NY = CY + dy;
					if (NX < 0 || NX >= GridW || NY < 0 || NY >= GridH)
					{
						continue;
					}
					for (int32 Index : Buckets[NY * GridW + NX])
					{
						const float DistSq = FVector2D::DistSquared(Cells[Index].Position, P);
						if (DistSq < BestDistSq)
						{
							BestDistSq = DistSq;
							Best = Index;
						}
					}
				}
			}

			// Achou algo e o anel ja cobre a distancia: pode parar.
			if (Best != INDEX_NONE)
			{
				const float RingDist = Ring * CellSize;
				if (RingDist * RingDist >= BestDistSq)
				{
					break;
				}
			}
		}

		return Best != INDEX_NONE ? Best : FindNearestCell(Cells, P);
	}
}
