#include "Pipeline/ProvinceGeographyClassifier.h"
#include "WorldGenTypes.h"

namespace StrategosWorldGen::GeographyClassifier
{
	namespace
	{
		// Limiares de calibragem (moddáveis). Ajustar comparando o PNG de debug
		// com noções da Terra real. Nada de mágica espalhada: tudo aqui.
		constexpr float ARID_PRECIP        = 250.0f;  // abaixo disto: deserto/semiárido
		constexpr float MED_PRECIP         = 550.0f;  // divisor mediterrâneo/continental
		constexpr float TROPICAL_TEMP      = 20.0f;
		constexpr float ARCTIC_TEMP        = -2.0f;
		constexpr float MEDITERRANEAN_TEMP = 12.0f;

		constexpr float MOUNTAIN_RELIEF    = 0.18f;   // desnível local p/ montanha
		constexpr float HILL_RELIEF        = 0.07f;
		constexpr float PLATEAU_HEIGHT     = 0.55f;
		constexpr float PLATEAU_RELIEF     = 0.05f;

		constexpr float NAVIGABLE_MAX_H    = 0.45f;   // rio baixo = a jusante = maior
		constexpr float WETLAND_PRECIP     = 600.0f;
		constexpr float AQUIFER_PRECIP     = 600.0f;

		float LocalRelief(const FWorldGenResult& R, int32 i)
		{
			const FWorldCell& C = R.Cells[i];
			float MaxDelta = 0.0f;
			for (int32 N : C.Neighbors)
			{
				if (R.Cells.IsValidIndex(N))
				{
					MaxDelta = FMath::Max(MaxDelta, FMath::Abs(C.Height - R.Cells[N].Height));
				}
			}
			return MaxDelta;
		}

		bool NearCoast(const FWorldGenResult& R, int32 i)
		{
			const FWorldCell& C = R.Cells[i];
			if (C.bIsCoast)
			{
				return true;
			}
			for (int32 N : C.Neighbors)
			{
				if (R.Cells.IsValidIndex(N) && R.Cells[N].bIsCoast)
				{
					return true;
				}
			}
			return false;
		}

		EVegetationCover VegetationFromBiome(EBiomeType B)
		{
			switch (B)
			{
			case EBiomeType::Glacier:
			case EBiomeType::PolarDesert:
			case EBiomeType::Tundra:
				return EVegetationCover::Tundra;
			case EBiomeType::ColdDesert:
			case EBiomeType::Desert:
			case EBiomeType::HotDesert:
				return EVegetationCover::Desert;
			case EBiomeType::Steppe:
			case EBiomeType::TemperateGrassland:
			case EBiomeType::Savanna:
				return EVegetationCover::Grassland;
			case EBiomeType::TemperateForest:
			case EBiomeType::TropicalSeasonalForest:
				return EVegetationCover::LightForest;
			case EBiomeType::TemperateRainforest:
			case EBiomeType::TropicalRainforest:
				return EVegetationCover::DenseForest;
			default:
				return EVegetationCover::Grassland;
			}
		}

		float FertilityFromBiome(EBiomeType B)
		{
			switch (B)
			{
			case EBiomeType::TemperateGrassland:     return 0.80f;
			case EBiomeType::TemperateForest:        return 0.65f;
			case EBiomeType::TropicalSeasonalForest: return 0.55f;
			case EBiomeType::Savanna:
			case EBiomeType::Steppe:                 return 0.50f;
			case EBiomeType::TropicalRainforest:
			case EBiomeType::TemperateRainforest:    return 0.45f; // solos lixiviados
			case EBiomeType::Tundra:                 return 0.15f;
			case EBiomeType::ColdDesert:
			case EBiomeType::Desert:
			case EBiomeType::HotDesert:
			case EBiomeType::PolarDesert:
			case EBiomeType::Glacier:                return 0.05f;
			default:                                 return 0.40f;
			}
		}

		EClimateZone ClassifyClimate(const FWorldGenResult& R, int32 i)
		{
			const FWorldCell& C = R.Cells[i];
			const float T = C.Temperature;
			const float P = C.Precipitation;

			if (T < ARCTIC_TEMP)
			{
				return EClimateZone::Arctic;
			}
			if (P < ARID_PRECIP)
			{
				return EClimateZone::Arid;
			}
			if (T >= TROPICAL_TEMP)
			{
				return EClimateZone::Tropical;
			}
			// Banda temperada.
			if (NearCoast(R, i) && P >= MED_PRECIP)
			{
				return EClimateZone::Oceanic;
			}
			if (P < MED_PRECIP && T >= MEDITERRANEAN_TEMP)
			{
				return EClimateZone::Mediterranean;
			}
			return EClimateZone::Continental;
		}
	}

	FProvinceGeography Classify(const FWorldGenResult& Result, int32 CellIndex)
	{
		FProvinceGeography G;
		if (!Result.Cells.IsValidIndex(CellIndex))
		{
			return G;
		}
		const FWorldCell& C = Result.Cells[CellIndex];

		// --- Hidrografia (flags) ---
		G.Hydrography.bIsCoastal = C.bIsCoast;
		if (C.bIsRiver)
		{
			const bool bNavigable = C.Height < NAVIGABLE_MAX_H;
			G.Hydrography.bHasNavigableRiver = bNavigable;
			G.Hydrography.bHasMinorRiver = !bNavigable;
		}
		// bHasLake: worldgen v0 não gera lagos (rios são rascunho). Fica false.

		// --- Vegetação natural (do bioma Whittaker) ---
		G.Vegetation = VegetationFromBiome(C.Biome);

		// --- Topografia (altura + relevo local) ---
		const float Relief = LocalRelief(Result, CellIndex);
		const bool bWetland = !C.bIsWater
			&& C.Height < Result.SeaLevel + 0.06f
			&& C.Precipitation > WETLAND_PRECIP
			&& Relief < HILL_RELIEF;

		if (C.bIsWater || C.bIsCoast)
		{
			G.Topography = ETerrainTopography::Coastal;
		}
		else if (Relief >= MOUNTAIN_RELIEF)
		{
			G.Topography = ETerrainTopography::Mountains;
		}
		else if (C.Height >= PLATEAU_HEIGHT && Relief <= PLATEAU_RELIEF)
		{
			G.Topography = ETerrainTopography::Plateau;
		}
		else if (Relief >= HILL_RELIEF)
		{
			G.Topography = ETerrainTopography::Hills;
		}
		else if (bWetland)
		{
			G.Topography = ETerrainTopography::Wetlands;
			G.Vegetation = EVegetationCover::Wetland;
		}
		else
		{
			G.Topography = ETerrainTopography::Flatland;
		}

		// --- Clima ---
		G.Climate = ClassifyClimate(Result, CellIndex);

		// --- Aquífero: recarga em terreno plano, úmido e não-árido ---
		G.Hydrography.bHasAquifer =
			C.Precipitation > AQUIFER_PRECIP
			&& (G.Topography == ETerrainTopography::Flatland || G.Topography == ETerrainTopography::Plateau)
			&& G.Climate != EClimateZone::Arid;

		// --- Fertilidade ---
		G.Fertility = FertilityFromBiome(C.Biome);

		// PrincipalResourceId: Sessão 4 (determinístico). Fica None aqui.
		return G;
	}

	void ClassifyAll(const FWorldGenResult& Result, TArray<FProvinceGeography>& Out)
	{
		Out.Reset();
		Out.SetNum(Result.Cells.Num());
		for (int32 i = 0; i < Result.Cells.Num(); ++i)
		{
			Out[i] = Classify(Result, i);
		}
	}
}
