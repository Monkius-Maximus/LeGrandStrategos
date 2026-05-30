#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "World/TerrainType.h"
#include "Economy/ProvinceGeography.h"
#include "Economy/PopGroup.h"
#include "Province.generated.h"

class UBuilding;

/**
 * UProvince — célula territorial mínima do mapa.
 *
 * Stage 1 (MVP): identificação, dono, adjacências, terreno, posição 2D.
 * Stage 2 (Economy v1): adiciona POPs estratificados, slots de prédio
 * e potencial de recursos naturais derivado do terreno.
 */
UCLASS(BlueprintType)
class STRATEGOSCORE_API UProvince : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Province")
	FName Id;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Province")
	FText DisplayName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Province")
	FName OwnerNationId;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Province")
	TArray<FName> AdjacentProvinceIds;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Province")
	FVector2D MapPosition = FVector2D::ZeroVector;

	/**
	 * Geografia natural (Camada 1 — Recursos e Produção). Fonte única dos quatro
	 * eixos (relevo/clima/vegetação/hidrografia). ETerrainType é derivado via
	 * GetTerrain(); o tier de água via Geography.Hydrography.GetWaterAccessTier().
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Province")
	FProvinceGeography Geography;

	/** Estratos populacionais que vivem aqui. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Province|Economy")
	TMap<EPopStratum, FPopGroup> Pops;

	/** Quantos prédios cabem nesta província. Setado no bootstrap por terreno. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Province|Economy", meta = (ClampMin = "0"))
	int32 BuildingSlots = 3;

	/**
	 * Potencial de extração natural por bem (apenas Tier 0 raws).
	 * Map: GoodId → multiplicador [0..1+]; 0 significa indisponível.
	 * Ex.: Mountains → { "IronOre": 1.0, "Coal": 0.7 }.
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Province|Economy")
	TMap<FName, float> RawResourcePotential;

	/** Prédios (incluindo em construção). Vivem aqui mas operam sob a UNation dona. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Province|Economy")
	TArray<TObjectPtr<UBuilding>> Buildings;

	UFUNCTION(BlueprintPure, Category = "Province")
	bool IsAdjacentTo(FName OtherProvinceId) const
	{
		return AdjacentProvinceIds.Contains(OtherProvinceId);
	}

	/**
	 * ETerrainType derivado da geografia em 4 eixos, para os consumidores legados
	 * (Military, UI, RGO) que ainda raciocinam num enum único. A geografia é a
	 * verdade; este enum é uma projeção lossy. Quando esses consumidores migrarem
	 * para os eixos, este getter sai.
	 */
	UFUNCTION(BlueprintPure, Category = "Province")
	ETerrainType GetTerrain() const
	{
		if (Geography.Hydrography.bIsCoastal || Geography.Topography == ETerrainTopography::Coastal)
		{
			return ETerrainType::Coast;
		}
		if (Geography.Topography == ETerrainTopography::Mountains)
		{
			return ETerrainType::Mountains;
		}
		if (Geography.Topography == ETerrainTopography::Hills || Geography.Topography == ETerrainTopography::Plateau)
		{
			return ETerrainType::Hills;
		}
		if (Geography.Topography == ETerrainTopography::Wetlands || Geography.Vegetation == EVegetationCover::Wetland)
		{
			return ETerrainType::Marsh;
		}
		if (Geography.Vegetation == EVegetationCover::Desert)
		{
			return ETerrainType::Desert;
		}
		if (Geography.Vegetation == EVegetationCover::Tundra)
		{
			return ETerrainType::Tundra;
		}
		if (Geography.Vegetation == EVegetationCover::DenseForest || Geography.Vegetation == EVegetationCover::LightForest)
		{
			return ETerrainType::Forest;
		}
		return ETerrainType::Plains;
	}

	UFUNCTION(BlueprintPure, Category = "Province|Economy")
	int32 GetUsedBuildingSlots() const
	{
		return Buildings.Num();
	}

	UFUNCTION(BlueprintPure, Category = "Province|Economy")
	int32 GetFreeBuildingSlots() const
	{
		return FMath::Max(0, BuildingSlots - Buildings.Num());
	}
};
