#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MilitarySubsystem.generated.h"

class UArmy;
class UProvince;
class UWorldState;
class UTimeSubsystem;

UENUM(BlueprintType)
enum class EMoveOrderResult : uint8
{
	Issued				UMETA(DisplayName = "Issued"),
	Rejected_NoArmy		UMETA(DisplayName = "Rejected: army not found"),
	Rejected_NoTarget	UMETA(DisplayName = "Rejected: target not found"),
	Rejected_NotAdjacent UMETA(DisplayName = "Rejected: target not adjacent"),
	Rejected_AlreadyThere UMETA(DisplayName = "Rejected: army already there")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnArmyArrived, FName, ArmyId, FName, ProvinceId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnArmyMoveIssued, FName, ArmyId);

/**
 * UMilitarySubsystem (v1) — Movimento estratégico por adjacência.
 *
 * Stage 1 (MVP): cada exército pode receber uma ordem de mover-se para
 * uma província adjacente. O subsistema consome OnDayTick do
 * UTimeSubsystem e decrementa MoveDaysRemaining. Quando chega em zero,
 * o exército é teleportado para o destino e OnArmyArrived é disparado.
 *
 * Não há suprimento, combate, frente, naval — tudo isso entra nas
 * etapas 2-3 do roadmap. O custo de movimento por terreno é uma tabela
 * fixa por agora; passa para DataAsset quando UProvinceAsset existir.
 *
 * Ver docs/architecture/41-military.md para o design alvo.
 */
UCLASS()
class STRATEGOSCORE_API UMilitarySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;

	UFUNCTION(BlueprintCallable, Category = "Strategos|Military")
	EMoveOrderResult IssueMoveOrder(FName ArmyId, FName TargetProvinceId);

	UFUNCTION(BlueprintCallable, Category = "Strategos|Military")
	void CancelMoveOrder(FName ArmyId);

	UFUNCTION(BlueprintPure, Category = "Strategos|Military")
	int32 GetMovementCostDays(FName FromProvinceId, FName ToProvinceId) const;

	UPROPERTY(BlueprintAssignable, Category = "Strategos|Military")
	FOnArmyMoveIssued OnArmyMoveIssued;

	UPROPERTY(BlueprintAssignable, Category = "Strategos|Military")
	FOnArmyArrived OnArmyArrived;

private:
	UFUNCTION()
	void HandleDayTick(FDateTime CurrentDate);

	void TickArmyMovement(UArmy& Army);

	UWorldState* ResolveWorldState() const;
	UTimeSubsystem* ResolveTime() const;
};
