#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "WorldState.generated.h"

class UNation;
class UProvince;
class UArmy;

/**
 * UWorldState — Container raiz da simulação.
 *
 * Aglutina todas as nações, províncias e exércitos e expõe getters
 * por id. É deliberadamente um UObject puro (não Actor) porque a maioria
 * dos subsistemas precisa iterar sobre estes containers sem o overhead
 * do tick de Actor. O AStrategosGameState mantém uma referência canônica.
 */
UCLASS(BlueprintType)
class STRATEGOSCORE_API UWorldState : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "World")
	FName PlayerNationId;

	UPROPERTY()
	TMap<FName, TObjectPtr<UNation>> Nations;

	UPROPERTY()
	TMap<FName, TObjectPtr<UProvince>> Provinces;

	UPROPERTY()
	TMap<FName, TObjectPtr<UArmy>> Armies;

	UFUNCTION(BlueprintPure, Category = "World")
	UNation* GetNation(FName NationId) const;

	UFUNCTION(BlueprintPure, Category = "World")
	UProvince* GetProvince(FName ProvinceId) const;

	UFUNCTION(BlueprintPure, Category = "World")
	UArmy* GetArmy(FName ArmyId) const;

	UNation* AddNation(FName Id);
	UProvince* AddProvince(FName Id);
	UArmy* AddArmy(FName Id);

	UFUNCTION(BlueprintPure, Category = "World")
	int32 GetNationCount() const { return Nations.Num(); }

	UFUNCTION(BlueprintPure, Category = "World")
	int32 GetProvinceCount() const { return Provinces.Num(); }

	UFUNCTION(BlueprintPure, Category = "World")
	int32 GetArmyCount() const { return Armies.Num(); }
};
