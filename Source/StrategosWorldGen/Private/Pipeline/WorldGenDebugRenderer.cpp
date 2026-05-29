#include "Pipeline/WorldGenDebugRenderer.h"
#include "Pipeline/WorldGenMath.h"
#include "StrategosWorldGen.h"
#include "Engine/Texture2D.h"
#include "TextureResource.h"

namespace StrategosWorldGen::DebugRender
{
	FColor BiomeColor(EBiomeType Biome)
	{
		switch (Biome)
		{
		case EBiomeType::Ocean:                  return FColor(38, 78, 140);
		case EBiomeType::Glacier:                return FColor(235, 245, 250);
		case EBiomeType::PolarDesert:            return FColor(200, 205, 210);
		case EBiomeType::Tundra:                 return FColor(150, 165, 150);
		case EBiomeType::ColdDesert:             return FColor(190, 180, 150);
		case EBiomeType::Steppe:                 return FColor(190, 185, 110);
		case EBiomeType::TemperateGrassland:     return FColor(150, 180, 90);
		case EBiomeType::TemperateForest:        return FColor(70, 130, 70);
		case EBiomeType::TemperateRainforest:    return FColor(40, 100, 60);
		case EBiomeType::Desert:                 return FColor(220, 200, 130);
		case EBiomeType::Savanna:                return FColor(190, 185, 90);
		case EBiomeType::TropicalSeasonalForest: return FColor(90, 150, 60);
		case EBiomeType::TropicalRainforest:     return FColor(30, 110, 40);
		case EBiomeType::HotDesert:              return FColor(235, 215, 140);
		case EBiomeType::River:                  return FColor(60, 120, 200);
		default:                                 return FColor(255, 0, 255);
		}
	}

	namespace
	{
		FColor Lerp(const FColor& A, const FColor& B, float T)
		{
			T = FMath::Clamp(T, 0.0f, 1.0f);
			return FColor(
				FMath::RoundToInt(FMath::Lerp((float)A.R, (float)B.R, T)),
				FMath::RoundToInt(FMath::Lerp((float)A.G, (float)B.G, T)),
				FMath::RoundToInt(FMath::Lerp((float)A.B, (float)B.B, T)),
				255);
		}

		float MapToUnit(float Value, float Min, float Max)
		{
			return FMath::Clamp((Value - Min) / FMath::Max(KINDA_SMALL_NUMBER, Max - Min), 0.0f, 1.0f);
		}

		FColor ColorForCell(const FWorldCell& Cell, EWorldGenRenderMode Mode, int32 Index, float SeaLevel)
		{
			switch (Mode)
			{
			case EWorldGenRenderMode::RandomCells:
			{
				const uint32 H = static_cast<uint32>(Index) * 2654435761u;
				return FColor((H & 0xFF), ((H >> 8) & 0xFF), ((H >> 16) & 0xFF), 255);
			}

			case EWorldGenRenderMode::Height:
			{
				if (Cell.bIsWater)
				{
					return Lerp(FColor(10, 20, 60), FColor(60, 110, 170), Cell.Height / FMath::Max(0.01f, SeaLevel));
				}
				const uint8 G = (uint8)FMath::Clamp(FMath::RoundToInt(Cell.Height * 255.0f), 0, 255);
				return FColor(G, G, G, 255);
			}

			case EWorldGenRenderMode::Coast:
			{
				if (Cell.bIsWater)    return FColor(45, 85, 150);
				if (Cell.bIsCoast)    return FColor(210, 195, 140);
				return Lerp(FColor(110, 150, 80), FColor(120, 95, 70), Cell.Height);
			}

			case EWorldGenRenderMode::Temperature:
			{
				const float T = MapToUnit(Cell.Temperature, -30.0f, 35.0f);
				return Lerp(FColor(40, 60, 200), FColor(220, 60, 40), T);
			}

			case EWorldGenRenderMode::Precipitation:
			{
				if (Cell.bIsWater) return FColor(45, 85, 150);
				const float P = MapToUnit(Cell.Precipitation, 0.0f, 1500.0f);
				return Lerp(FColor(200, 180, 120), FColor(30, 90, 50), P);
			}

			case EWorldGenRenderMode::Biomes:
			default:
			{
				if (Cell.bIsRiver) return BiomeColor(EBiomeType::River);
				return BiomeColor(Cell.Biome);
			}
			}
		}
	}

	UTexture2D* RenderToTexture(const FWorldGenResult& Result, EWorldGenRenderMode Mode)
	{
		const int32 W = Result.MapSize.X;
		const int32 H = Result.MapSize.Y;
		if (W <= 0 || H <= 0 || Result.Cells.Num() == 0)
		{
			UE_LOG(LogStrategosWorldGen, Error, TEXT("RenderToTexture: resultado vazio."));
			return nullptr;
		}

		// Pre-computa cor por celula.
		TArray<FColor> CellColors;
		CellColors.SetNum(Result.Cells.Num());
		for (int32 i = 0; i < Result.Cells.Num(); ++i)
		{
			CellColors[i] = ColorForCell(Result.Cells[i], Mode, i, Result.SeaLevel);
		}

		// Hash espacial para o nearest-cell.
		StrategosWorldGen::Math::FSpatialGrid Grid;
		Grid.Build(Result.Cells, Result.MapSize, FMath::Max(8.0f, (float)W / 128.0f));

		UTexture2D* Tex = UTexture2D::CreateTransient(W, H, PF_B8G8R8A8);
		if (!Tex)
		{
			return nullptr;
		}
		Tex->SRGB = true;
		Tex->NeverStream = true;

		FTexturePlatformData* PlatformData = Tex->GetPlatformData();
		if (!PlatformData || PlatformData->Mips.Num() == 0)
		{
			return nullptr;
		}

		FColor* Mip = static_cast<FColor*>(PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE));
		for (int32 Y = 0; Y < H; ++Y)
		{
			for (int32 X = 0; X < W; ++X)
			{
				const int32 Cell = Grid.FindNearest(Result.Cells, FVector2D(X + 0.5f, Y + 0.5f));
				Mip[Y * W + X] = CellColors.IsValidIndex(Cell) ? CellColors[Cell] : FColor::Magenta;
			}
		}
		PlatformData->Mips[0].BulkData.Unlock();
		Tex->UpdateResource();

		return Tex;
	}
}
