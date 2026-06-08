#include "Pipeline/HeightmapGenerator.h"
#include "Pipeline/WorldGenMath.h"
#include "WorldGenContext.h"
#include "Containers/Queue.h"

namespace StrategosWorldGen::Heightmap
{
	void AddBlob(FWorldGenContext& Ctx, int32 StartCell, float Power, float Radius, float Sharpness)
	{
		TArray<FWorldCell>& Cells = Ctx.Result.Cells;
		if (!Cells.IsValidIndex(StartCell))
		{
			return;
		}

		// Contribuicao deste blob (assinada), separada para o BFS nao realimentar.
		TArray<float> Delta;
		Delta.Init(0.0f, Cells.Num());
		Delta[StartCell] = Power;

		TArray<bool> Visited;
		Visited.Init(false, Cells.Num());
		Visited[StartCell] = true;

		TQueue<int32> Queue;
		Queue.Enqueue(StartCell);

		while (!Queue.IsEmpty())
		{
			int32 Current = INDEX_NONE;
			Queue.Dequeue(Current);
			const float H = Delta[Current];

			for (int32 N : Cells[Current].Neighbors)
			{
				if (Visited[N])
				{
					continue;
				}
				const float Jitter = 1.0f - Sharpness + Ctx.Stream.FRand() * Sharpness * 2.0f;
				const float NewH = H * Radius * Jitter;
				if (FMath::Abs(NewH) < 0.01f)
				{
					continue;
				}
				Delta[N] = NewH;
				Visited[N] = true;
				Queue.Enqueue(N);
			}
		}

		for (int32 i = 0; i < Cells.Num(); ++i)
		{
			Cells[i].Height = FMath::Clamp(Cells[i].Height + Delta[i], 0.0f, 1.0f);
		}
	}

	void MultiplyHeights(FWorldGenContext& Ctx, float Factor)
	{
		for (FWorldCell& Cell : Ctx.Result.Cells)
		{
			Cell.Height = FMath::Clamp(Cell.Height * Factor, 0.0f, 1.0f);
		}
	}

	void AddRange(FWorldGenContext& Ctx, int32 StartCell, float Power, float Radius, float Sharpness, int32 Length)
	{
		TArray<FWorldCell>& Cells = Ctx.Result.Cells;
		if (!Cells.IsValidIndex(StartCell))
		{
			return;
		}

		const float Angle = Ctx.Stream.FRandRange(0.0f, 2.0f * PI);
		const FVector2D Dir(FMath::Cos(Angle), FMath::Sin(Angle));

		int32 Current = StartCell;
		for (int32 Step = 0; Step < Length; ++Step)
		{
			AddBlob(Ctx, Current, Power, Radius, Sharpness);

			// Anda para o vizinho mais alinhado com Dir.
			int32 Next = INDEX_NONE;
			float BestDot = -TNumericLimits<float>::Max();
			for (int32 N : Cells[Current].Neighbors)
			{
				const FVector2D Rel = (Cells[N].Position - Cells[Current].Position).GetSafeNormal();
				const float Dot = FVector2D::DotProduct(Rel, Dir);
				if (Dot > BestDot)
				{
					BestDot = Dot;
					Next = N;
				}
			}
			if (Next == INDEX_NONE)
			{
				break;
			}
			Current = Next;
		}
	}

	void PerturbWithPerlin(FWorldGenContext& Ctx, float Amplitude)
	{
		if (Amplitude <= 0.0f)
		{
			return;
		}

		// PerlinNoise2D do UE nao recebe seed: desloca o dominio por seed.
		const float Offset = (Ctx.Result.Seed % 1000) * 13.37f;

		for (FWorldCell& Cell : Ctx.Result.Cells)
		{
			const FVector2D Sample = Cell.Position * 0.005f + FVector2D(Offset, Offset);
			const float Noise = FMath::PerlinNoise2D(Sample);
			Cell.Height = FMath::Clamp(Cell.Height + Noise * Amplitude, 0.0f, 1.0f);
		}
	}

	void ForceBorderWater(FWorldGenContext& Ctx)
	{
		for (FWorldCell& Cell : Ctx.Result.Cells)
		{
			if (Cell.bIsBorder)
			{
				Cell.Height = 0.0f;
			}
		}
	}

	namespace
	{
		int32 RandomCell(FWorldGenContext& Ctx)
		{
			return Ctx.Stream.RandRange(0, Ctx.Result.Cells.Num() - 1);
		}

		void ApplyContinent(FWorldGenContext& Ctx)
		{
			const int32 CenterCell = StrategosWorldGen::Math::FindNearestCell(
				Ctx.Result.Cells, FVector2D(Ctx.Result.MapSize.X * 0.5f, Ctx.Result.MapSize.Y * 0.5f));

			AddBlob(Ctx, CenterCell, 0.9f, 0.85f, 0.15f);          // montanha central

			for (int32 i = 0; i < 8; ++i)
			{
				AddBlob(Ctx, RandomCell(Ctx), 0.4f, 0.95f, 0.10f); // colinas
			}
			for (int32 i = 0; i < 3; ++i)
			{
				AddRange(Ctx, RandomCell(Ctx), 0.5f, 0.96f, 0.10f, 6); // cordilheiras
			}
			for (int32 i = 0; i < 3; ++i)
			{
				AddBlob(Ctx, RandomCell(Ctx), -0.3f, 0.95f, 0.15f); // pits
			}

			MultiplyHeights(Ctx, 0.9f);
		}

		void ApplyArchipelago(FWorldGenContext& Ctx)
		{
			const int32 IslandCount = 12 + Ctx.Stream.RandRange(0, 8);
			for (int32 i = 0; i < IslandCount; ++i)
			{
				AddBlob(Ctx, RandomCell(Ctx), Ctx.Stream.FRandRange(0.3f, 0.55f), 0.90f, 0.20f);
			}
			for (int32 i = 0; i < 5; ++i)
			{
				AddBlob(Ctx, RandomCell(Ctx), -0.3f, 0.95f, 0.15f);
			}
			MultiplyHeights(Ctx, 0.85f);
		}

		void ApplyPangaea(FWorldGenContext& Ctx)
		{
			const int32 CenterCell = StrategosWorldGen::Math::FindNearestCell(
				Ctx.Result.Cells, FVector2D(Ctx.Result.MapSize.X * 0.5f, Ctx.Result.MapSize.Y * 0.5f));

			AddBlob(Ctx, CenterCell, 0.95f, 0.96f, 0.08f);         // massa central larga
			for (int32 i = 0; i < 14; ++i)
			{
				AddBlob(Ctx, RandomCell(Ctx), 0.45f, 0.96f, 0.10f);
			}
			for (int32 i = 0; i < 4; ++i)
			{
				AddRange(Ctx, RandomCell(Ctx), 0.6f, 0.97f, 0.10f, 8);
			}
			MultiplyHeights(Ctx, 0.95f);
		}
	}

	void ApplyTemplate(FWorldGenContext& Ctx)
	{
		switch (Ctx.Params.Template)
		{
		case EWorldTemplate::Archipelago: ApplyArchipelago(Ctx); break;
		case EWorldTemplate::Pangaea:     ApplyPangaea(Ctx);     break;
		case EWorldTemplate::Continent:
		default:                          ApplyContinent(Ctx);   break;
		}

		PerturbWithPerlin(Ctx, Ctx.Params.PerlinAmplitude);
		ForceBorderWater(Ctx);
	}
}
