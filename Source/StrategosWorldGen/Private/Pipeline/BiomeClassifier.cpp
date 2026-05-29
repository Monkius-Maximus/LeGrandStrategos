#include "Pipeline/BiomeClassifier.h"
#include "WorldGenContext.h"

namespace StrategosWorldGen::Biome
{
	EBiomeType Classify(float TempC, float PrecipMm, bool bIsWater)
	{
		if (bIsWater)
		{
			return EBiomeType::Ocean;
		}

		if (TempC < -5.0f)
		{
			return PrecipMm > 100.0f ? EBiomeType::Glacier : EBiomeType::PolarDesert;
		}

		if (TempC < 5.0f)
		{
			return PrecipMm > 200.0f ? EBiomeType::Tundra : EBiomeType::ColdDesert;
		}

		if (TempC < 15.0f)
		{
			if (PrecipMm < 200.0f)  return EBiomeType::Steppe;
			if (PrecipMm < 600.0f)  return EBiomeType::TemperateGrassland;
			if (PrecipMm < 1200.0f) return EBiomeType::TemperateForest;
			return EBiomeType::TemperateRainforest;
		}

		if (TempC < 25.0f)
		{
			if (PrecipMm < 250.0f)  return EBiomeType::Desert;
			if (PrecipMm < 600.0f)  return EBiomeType::Savanna;
			if (PrecipMm < 1500.0f) return EBiomeType::TropicalSeasonalForest;
			return EBiomeType::TropicalRainforest;
		}

		// > 25 C
		if (PrecipMm < 300.0f) return EBiomeType::HotDesert;
		if (PrecipMm < 800.0f) return EBiomeType::Savanna;
		return EBiomeType::TropicalRainforest;
	}

	void ClassifyAll(FWorldGenContext& Ctx)
	{
		for (FWorldCell& Cell : Ctx.Result.Cells)
		{
			Cell.Biome = Classify(Cell.Temperature, Cell.Precipitation, Cell.bIsWater);
		}
	}
}
