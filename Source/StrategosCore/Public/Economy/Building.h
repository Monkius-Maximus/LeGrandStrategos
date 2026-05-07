#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Economy/BuildingOwnerKind.h"
#include "Economy/PopStratum.h"
#include "Building.generated.h"

class UBuildingTypeAsset;
class UProductionMethodAsset;
class UProductionModifierAsset;

/**
 * UBuilding — Estado runtime de uma instância de prédio.
 *
 * Aponta para o UBuildingTypeAsset estático (definição) e mantém o que
 * varia: Level (multiplicador de slots), CurrentProductionMethod
 * (qual receita está rodando), ActiveProductionModifiers (toggles),
 * ConstructionDaysRemaining (>0 enquanto em obra), Owner kind.
 *
 * Caches LastTick* alimentam a UI (cards estilo Vic3) sem precisar
 * recalcular o tick passado.
 */
UCLASS(BlueprintType)
class STRATEGOSCORE_API UBuilding : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Building")
	FName Id;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Building")
	TSoftObjectPtr<UBuildingTypeAsset> BuildingType;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Building")
	FName ProvinceId;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Building", meta = (ClampMin = "1"))
	int32 Level = 1;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Building|Ownership")
	EBuildingOwnerKind OwnerKind = EBuildingOwnerKind::Government;

	/** Se OwnerKind == Private, a Bourgeoisie desta província é o dono coletivo. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Building|Ownership")
	FName OwnerProvinceId;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Building|Production")
	TSoftObjectPtr<UProductionMethodAsset> CurrentProductionMethod;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Building|Production")
	TArray<TSoftObjectPtr<UProductionModifierAsset>> ActiveProductionModifiers;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Building|Construction", meta = (ClampMin = "0"))
	int32 ConstructionDaysRemaining = 0;

	// Cache do último tick (para HUD).
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Building|Cache")
	TMap<FName, float> LastTickInputs;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Building|Cache")
	TMap<FName, float> LastTickOutputs;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Building|Cache")
	TMap<EPopStratum, int32> LastTickEmployment;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Building|Cache")
	float LastTickWagesPaid = 0.f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Building|Cache")
	float LastTickProfit = 0.f;

	UFUNCTION(BlueprintPure, Category = "Building")
	bool IsUnderConstruction() const { return ConstructionDaysRemaining > 0; }

	UFUNCTION(BlueprintPure, Category = "Building")
	bool IsActive() const { return ConstructionDaysRemaining == 0 && Level > 0; }
};
