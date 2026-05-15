#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "World/ArmyStats.h"
#include "Army.generated.h"

class UUnitTypeAsset;

/**
 * UArmy — unidade estratégica que ocupa províncias e se move entre elas.
 *
 * MVP de Etapa 1: dono, província, ordem de movimento, contagem de soldados.
 * Etapa 2 (cartas): + UnitType, BaseStats, ActiveModifiers, EUnitState, XP/Level.
 * Loadout customizável (Armament/Equipment/Doctrine/...) entra com o painel
 * de customização pós-MVP, junto com BattleResolver.
 */
UCLASS(BlueprintType)
class STRATEGOSCORE_API UArmy : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Army")
	FName Id;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Army")
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Army")
	FName OwnerNationId;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Army")
	FName CurrentProvinceId;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Army")
	FName MoveTargetProvinceId;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Army")
	int32 MoveDaysRemaining = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Army")
	int32 ManpowerCount = 1000;

	/** Template estático desta unidade. Carta UI lê daqui (nome, portrait, traços). */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Army|Type")
	TSoftObjectPtr<UUnitTypeAsset> UnitType;

	/** Stats efetivas. Inicializadas a partir de UnitType.BaseStats; modificadas em runtime. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Army|Stats")
	FArmyStats BaseStats;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Army|Stats")
	TArray<FArmyModifier> ActiveModifiers;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Army|Stats")
	EUnitState State = EUnitState::Ready;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Army|Experience")
	int32 ExperienceXP = 0;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Army|Experience")
	int32 ExperienceLevel = 0;

	UFUNCTION(BlueprintPure, Category = "Army")
	bool IsMoving() const { return !MoveTargetProvinceId.IsNone() && MoveDaysRemaining > 0; }
};
