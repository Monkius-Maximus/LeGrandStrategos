#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "World/TerrainType.h"
#include "Province.generated.h"

/**
 * UProvince — célula territorial mínima do mapa.
 *
 * Stage 1 (MVP): identificação, dono, adjacências, terreno, posição 2D
 * para placement no mapa. Population, recursos, infraestrutura entram
 * nas etapas 2+ junto com a economia.
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

	UFUNCTION(BlueprintPure, Category = "Province")
	bool IsAdjacentTo(FName OtherProvinceId) const
	{
		return AdjacentProvinceIds.Contains(OtherProvinceId);
	}
};
