#include "Pipeline/VoronoiBuilder.h"
#include "Pipeline/WorldGenMath.h"
#include "StrategosWorldGen.h"

THIRD_PARTY_INCLUDES_START
#include "delaunator.hpp"
THIRD_PARTY_INCLUDES_END

namespace StrategosWorldGen::Voronoi
{
	namespace
	{
		using StrategosWorldGen::Math::Circumcenter;

		/**
		 * Preenche neighbors e poligonos de cada celula a partir da Delaunay.
		 *
		 * Abordagem por bucket (em vez de iterar half-edges manualmente):
		 *  - vizinhos: cada aresta de triangulo liga dois pontos-semente;
		 *  - poligono: os triangulos que tocam um ponto, com circuncentros
		 *    ordenados angularmente ao redor do ponto (celulas Voronoi sao
		 *    convexas, entao a ordenacao angular da o winding correto).
		 */
		void BuildTopology(const delaunator::Delaunator& D, FWorldGenResult& Out)
		{
			const int32 NumPoints = Out.Cells.Num();
			const int32 NumTriangles = static_cast<int32>(D.triangles.size() / 3);

			TArray<TSet<int32>> NeighborSets;
			NeighborSets.SetNum(NumPoints);

			TArray<TArray<int32>> TrianglesOfPoint;
			TrianglesOfPoint.SetNum(NumPoints);

			for (int32 t = 0; t < NumTriangles; ++t)
			{
				const int32 P0 = static_cast<int32>(D.triangles[3 * t]);
				const int32 P1 = static_cast<int32>(D.triangles[3 * t + 1]);
				const int32 P2 = static_cast<int32>(D.triangles[3 * t + 2]);

				NeighborSets[P0].Add(P1); NeighborSets[P0].Add(P2);
				NeighborSets[P1].Add(P0); NeighborSets[P1].Add(P2);
				NeighborSets[P2].Add(P0); NeighborSets[P2].Add(P1);

				TrianglesOfPoint[P0].Add(t);
				TrianglesOfPoint[P1].Add(t);
				TrianglesOfPoint[P2].Add(t);
			}

			for (int32 i = 0; i < NumPoints; ++i)
			{
				// Ordem estavel (CLAUDE.md: TSet/TMap order nao e garantida): o BFS
				// do heightmap consome a Stream na ordem dos vizinhos, entao fixamos.
				Out.Cells[i].Neighbors = NeighborSets[i].Array();
				Out.Cells[i].Neighbors.Sort();

				TArray<int32>& Tris = TrianglesOfPoint[i];
				const FVector2D Center = Out.Cells[i].Position;

				Tris.Sort([&](int32 A, int32 B)
				{
					const FVector2D DA = Out.VoronoiVertices[A] - Center;
					const FVector2D DB = Out.VoronoiVertices[B] - Center;
					return FMath::Atan2(DA.Y, DA.X) < FMath::Atan2(DB.Y, DB.X);
				});

				Out.Cells[i].PolygonVertices = Tris;
			}

			// Pontos do casco convexo => poligono aberto => marca como borda.
			const int32 NumHalfEdges = static_cast<int32>(D.triangles.size());
			for (int32 e = 0; e < NumHalfEdges; ++e)
			{
				if (D.halfedges[e] == delaunator::INVALID_INDEX)
				{
					const int32 PA = static_cast<int32>(D.triangles[e]);
					const int32 Next = (e % 3 == 2) ? e - 2 : e + 1;
					const int32 PB = static_cast<int32>(D.triangles[Next]);
					Out.Cells[PA].bIsBorder = true;
					Out.Cells[PB].bIsBorder = true;
				}
			}
		}
	}

	bool Build(const TArray<FVector2D>& Points, FWorldGenResult& OutResult)
	{
		if (Points.Num() < 3)
		{
			UE_LOG(LogStrategosWorldGen, Error, TEXT("Voronoi: precisa de >= 3 pontos (recebeu %d)."), Points.Num());
			return false;
		}

		std::vector<double> Coords;
		Coords.reserve(Points.Num() * 2);
		for (const FVector2D& P : Points)
		{
			Coords.push_back(static_cast<double>(P.X));
			Coords.push_back(static_cast<double>(P.Y));
		}

		try
		{
			delaunator::Delaunator D(Coords);

			OutResult.Cells.Reset();
			OutResult.Cells.SetNum(Points.Num());
			for (int32 i = 0; i < Points.Num(); ++i)
			{
				OutResult.Cells[i].Position = Points[i];
			}

			const int32 NumTriangles = static_cast<int32>(D.triangles.size() / 3);
			OutResult.VoronoiVertices.Reset();
			OutResult.VoronoiVertices.Reserve(NumTriangles);
			for (int32 t = 0; t < NumTriangles; ++t)
			{
				const FVector2D& A = Points[D.triangles[3 * t]];
				const FVector2D& B = Points[D.triangles[3 * t + 1]];
				const FVector2D& C = Points[D.triangles[3 * t + 2]];
				OutResult.VoronoiVertices.Add(Circumcenter(A, B, C));
			}

			BuildTopology(D, OutResult);
			return true;
		}
		catch (const std::exception& Ex)
		{
			UE_LOG(LogStrategosWorldGen, Error, TEXT("delaunator falhou: %hs"), Ex.what());
			return false;
		}
	}

	TArray<FVector2D> LloydRelax(const TArray<FVector2D>& Points, const FIntPoint& MapSize, int32 Iterations)
	{
		TArray<FVector2D> Current = Points;

		for (int32 Iter = 0; Iter < Iterations; ++Iter)
		{
			FWorldGenResult Temp;
			Temp.MapSize = MapSize;
			if (!Build(Current, Temp))
			{
				break;
			}

			TArray<FVector2D> Relaxed;
			Relaxed.SetNum(Current.Num());
			for (int32 i = 0; i < Current.Num(); ++i)
			{
				const FWorldCell& Cell = Temp.Cells[i];

				// Celulas de borda tem poligono aberto: nao mexe, so clampa.
				if (Cell.bIsBorder || Cell.PolygonVertices.Num() == 0)
				{
					Relaxed[i] = Current[i];
					continue;
				}

				FVector2D Centroid = FVector2D::ZeroVector;
				for (int32 V : Cell.PolygonVertices)
				{
					Centroid += Temp.VoronoiVertices[V];
				}
				Centroid /= Cell.PolygonVertices.Num();

				Relaxed[i] = FVector2D(
					FMath::Clamp(Centroid.X, 0.0f, static_cast<float>(MapSize.X)),
					FMath::Clamp(Centroid.Y, 0.0f, static_cast<float>(MapSize.Y)));
			}

			Current = MoveTemp(Relaxed);
		}

		return Current;
	}
}
