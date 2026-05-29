#include "Pipeline/PoissonDiscSampler.h"

namespace StrategosWorldGen::PoissonDisc
{
	TArray<FVector2D> Sample(const FIntPoint& Size, float MinDist, FRandomStream& Stream, int32 K)
	{
		TArray<FVector2D> Points;

		if (Size.X <= 0 || Size.Y <= 0 || MinDist <= 0.0f)
		{
			return Points;
		}

		const float CellSize = MinDist / FMath::Sqrt(2.0f);
		const int32 GridW = FMath::CeilToInt(Size.X / CellSize);
		const int32 GridH = FMath::CeilToInt(Size.Y / CellSize);

		TArray<int32> Grid;
		Grid.Init(INDEX_NONE, GridW * GridH);

		TArray<int32> Active;

		auto GridCoords = [&](const FVector2D& P, int32& OutX, int32& OutY)
		{
			OutX = FMath::Clamp(FMath::FloorToInt(P.X / CellSize), 0, GridW - 1);
			OutY = FMath::Clamp(FMath::FloorToInt(P.Y / CellSize), 0, GridH - 1);
		};

		auto Register = [&](const FVector2D& P)
		{
			const int32 Index = Points.Add(P);
			int32 GX, GY;
			GridCoords(P, GX, GY);
			Grid[GY * GridW + GX] = Index;
			Active.Add(Index);
		};

		// Pick inicial.
		Register(FVector2D(Stream.FRandRange(0.0f, Size.X), Stream.FRandRange(0.0f, Size.Y)));

		while (Active.Num() > 0)
		{
			const int32 ActiveIdx = Stream.RandRange(0, Active.Num() - 1);
			const FVector2D Origin = Points[Active[ActiveIdx]];
			bool bFound = false;

			for (int32 i = 0; i < K; ++i)
			{
				const float Angle = Stream.FRandRange(0.0f, 2.0f * PI);
				const float Radius = Stream.FRandRange(MinDist, 2.0f * MinDist);
				const FVector2D Candidate = Origin + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * Radius;

				if (Candidate.X < 0.0f || Candidate.X >= Size.X || Candidate.Y < 0.0f || Candidate.Y >= Size.Y)
				{
					continue;
				}

				int32 CX, CY;
				GridCoords(Candidate, CX, CY);

				bool bValid = true;
				for (int32 dy = -2; dy <= 2 && bValid; ++dy)
				{
					for (int32 dx = -2; dx <= 2 && bValid; ++dx)
					{
						const int32 NX = CX + dx;
						const int32 NY = CY + dy;
						if (NX < 0 || NX >= GridW || NY < 0 || NY >= GridH)
						{
							continue;
						}
						const int32 Neighbor = Grid[NY * GridW + NX];
						if (Neighbor == INDEX_NONE)
						{
							continue;
						}
						if (FVector2D::Distance(Points[Neighbor], Candidate) < MinDist)
						{
							bValid = false;
						}
					}
				}

				if (bValid)
				{
					Register(Candidate);
					bFound = true;
					break;
				}
			}

			if (!bFound)
			{
				Active.RemoveAtSwap(ActiveIdx);
			}
		}

		return Points;
	}
}
