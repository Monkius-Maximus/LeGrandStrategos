#include "Pipeline/RiverTracer.h"
#include "WorldGenContext.h"

namespace StrategosWorldGen::River
{
	void Trace(FWorldGenContext& Ctx)
	{
		TArray<FWorldCell>& Cells = Ctx.Result.Cells;
		const int32 N = Cells.Num();
		const float FlowThreshold = Ctx.Params.RiverFlowThreshold;

		for (FWorldCell& Cell : Cells)
		{
			Cell.bIsRiver = false;
		}

		TArray<int32> Downhill;
		Downhill.Init(INDEX_NONE, N);

		TArray<float> Flow;
		Flow.Init(0.0f, N);

		// Ordena celulas por altura decrescente (montante antes de jusante).
		TArray<int32> Sorted;
		Sorted.SetNum(N);
		for (int32 i = 0; i < N; ++i)
		{
			Sorted[i] = i;
		}
		Sorted.Sort([&](int32 A, int32 B) { return Cells[A].Height > Cells[B].Height; });

		// Downhill: vizinho de menor altura (so terra).
		for (int32 i : Sorted)
		{
			if (Cells[i].bIsWater)
			{
				continue;
			}
			int32 Lowest = INDEX_NONE;
			float LowestH = Cells[i].Height;
			for (int32 NIdx : Cells[i].Neighbors)
			{
				if (Cells[NIdx].Height < LowestH)
				{
					LowestH = Cells[NIdx].Height;
					Lowest = NIdx;
				}
			}
			Downhill[i] = Lowest;
		}

		// Acumula fluxo: cada celula soma sua precipitacao e despeja no downhill.
		for (int32 i : Sorted)
		{
			Flow[i] += Cells[i].Precipitation;
			if (Downhill[i] != INDEX_NONE)
			{
				Flow[Downhill[i]] += Flow[i];
			}
		}

		for (int32 i = 0; i < N; ++i)
		{
			if (!Cells[i].bIsWater && Flow[i] > FlowThreshold)
			{
				Cells[i].bIsRiver = true;
			}
		}
	}
}
