#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "World/TerrainType.h"
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

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Province")
	ETerrainType Terrain = ETerrainType::Plains;

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
