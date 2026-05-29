#include "Pipeline/ClimateGenerator.h"
#include "WorldGenContext.h"

namespace StrategosWorldGen::Climate
{
	void DetectWaterAndCoast(FWorldGenContext& Ctx)
	{
		TArray<FWorldCell>& Cells = Ctx.Result.Cells;
		const float Sea = Ctx.Result.SeaLevel;

		for (FWorldCell& Cell : Cells)
		{
			Cell.bIsWater = Cell.Height < Sea;
			Cell.bIsCoast = false;
		}

		for (FWorldCell& Cell : Cells)
		{
			if (Cell.bIsWater)
			{
				continue;
			}
			for (int32 N : Cell.Neighbors)
			{
				if (Cells[N].bIsWater)
				{
					Cell.bIsCoast = true;
					break;
				}
			}
		}
	}

	void ComputeTemperature(FWorldGenContext& Ctx)
	{
		TArray<FWorldCell>& Cells = Ctx.Result.Cells;
		const float EquatorY = Ctx.Result.EquatorY;
		const float HalfH = Ctx.Result.MapSize.Y * 0.5f;
		const float LapseRate = 6.5f / 1000.0f; // graus C por metro

		const float EquatorTemp = Ctx.Params.EquatorTemp;
		const float PoleTemp = Ctx.Params.PoleTemp;
		const float MaxAltitudeM = Ctx.Params.MaxAltitudeM;

		for (FWorldCell& Cell : Cells)
		{
			const float LatRatio = FMath::Clamp(FMath::Abs(Cell.Position.Y - EquatorY) / HalfH, 0.0f, 1.0f);
			const float TempByLat = FMath::Lerp(EquatorTemp, PoleTemp, LatRatio);

			const float Alt = FMath::Max(0.0f, Cell.Height - Ctx.Result.SeaLevel);
			const float AltMeters = Alt * MaxAltitudeM;

			Cell.Temperature = TempByLat - AltMeters * LapseRate;
		}
	}

	FVector2D GetWindForCell(const FVector2D& Position, float EquatorY, float HalfHeight)
	{
		// -1 (polo sul) .. +1 (polo norte). Y cresce para baixo => sul positivo
		// no espaco de tela, mas o sinal so define hemisferio, nao muda biomas.
		const float LatRatioSigned = FMath::Clamp((Position.Y - EquatorY) / FMath::Max(1.0f, HalfHeight), -1.0f, 1.0f);
		const float Abs = FMath::Abs(LatRatioSigned);

		if (Abs < 0.33f)
		{
			return FVector2D(-1.0f, 0.0f); // tropical: easterlies
		}
		if (Abs < 0.66f)
		{
			return FVector2D(1.0f, 0.0f);  // temperado: westerlies
		}
		return FVector2D(-1.0f, 0.0f);     // polar: easterlies
	}

	namespace
	{
		bool IsUpwindBorder(const FVector2D& Pos, const FVector2D& Wind, const FIntPoint& Size, float Margin)
		{
			// A nuvem entra pela borda de onde o vento sopra.
			if (Wind.X > 0.0f && Pos.X <= Margin)
			{
				return true;
			}
			if (Wind.X < 0.0f && Pos.X >= Size.X - Margin)
			{
				return true;
			}
			return false;
		}

		int32 FindNeighborInDirection(const FWorldCell& Cell, const FVector2D& Wind, const TArray<FWorldCell>& Cells)
		{
			int32 Best = INDEX_NONE;
			float BestDot = 0.0f; // exige alinhamento positivo
			for (int32 N : Cell.Neighbors)
			{
				const FVector2D Rel = (Cells[N].Position - Cell.Position).GetSafeNormal();
				const float Dot = FVector2D::DotProduct(Rel, Wind);
				if (Dot > BestDot)
				{
					BestDot = Dot;
					Best = N;
				}
			}
			return Best;
		}
	}

	void ComputePrecipitation(FWorldGenContext& Ctx)
	{
		TArray<FWorldCell>& Cells = Ctx.Result.Cells;
		const FIntPoint Size = Ctx.Result.MapSize;
		const float EquatorY = Ctx.Result.EquatorY;
		const float HalfH = Ctx.Result.MapSize.Y * 0.5f;
		const float Margin = FMath::Max(8.0f, Ctx.Params.PoissonMinDist * 1.5f);
		const float InitialMoisture = Ctx.Params.InitialMoisture;

		for (FWorldCell& C : Cells)
		{
			C.Precipitation = 0.0f;
		}

		for (int32 i = 0; i < Cells.Num(); ++i)
		{
			const FVector2D Wind = GetWindForCell(Cells[i].Position, EquatorY, HalfH);
			if (!IsUpwindBorder(Cells[i].Position, Wind, Size, Margin))
			{
				continue;
			}

			float Moisture = InitialMoisture;
			int32 Current = i;
			TSet<int32> Visited;

			while (Moisture > 0.01f && !Visited.Contains(Current))
			{
				Visited.Add(Current);
				FWorldCell& Cell = Cells[Current];

				if (Cell.bIsWater)
				{
					Moisture = FMath::Min(Moisture + 5.0f, InitialMoisture);
				}
				else
				{
					const float AltFactor = 1.0f + Cell.Height * 4.0f; // orografico
					const float Rain = FMath::Min(Moisture, 10.0f * AltFactor);
					Cell.Precipitation += Rain;
					Moisture -= Rain;
				}

				const FVector2D CellWind = GetWindForCell(Cell.Position, EquatorY, HalfH);
				const int32 Next = FindNeighborInDirection(Cell, CellWind, Cells);
				if (Next == INDEX_NONE)
				{
					break;
				}
				Current = Next;
			}
		}
	}
}
