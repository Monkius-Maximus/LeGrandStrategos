#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "EconomySubsystem.generated.h"

class UWorldState;
class UNation;
class UProvince;
class UBuilding;
class UTimeSubsystem;
class UGoodAsset;
class UProductionMethodAsset;
class UProductionModifierAsset;
class UBuildingTypeAsset;
class UEconomyContentRegistry;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEconomyTickComplete, FName, NationId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuildingCompleted, FName, BuildingId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBankruptcyImminent, FName, NationId);

UENUM(BlueprintType)
enum class EBuildResult : uint8
{
	Issued					UMETA(DisplayName = "Issued"),
	Rejected_NoNation		UMETA(DisplayName = "Rejected: nation not found"),
	Rejected_NoProvince		UMETA(DisplayName = "Rejected: province not found"),
	Rejected_WrongOwner		UMETA(DisplayName = "Rejected: province not owned by nation"),
	Rejected_NoBuildingType	UMETA(DisplayName = "Rejected: building type not registered"),
	Rejected_NoSlot			UMETA(DisplayName = "Rejected: no free building slot"),
	Rejected_NoRawResource	UMETA(DisplayName = "Rejected: province lacks required raw resource"),
	Rejected_InsufficientFunds UMETA(DisplayName = "Rejected: insufficient treasury funds"),
	Rejected_InsufficientGoods UMETA(DisplayName = "Rejected: insufficient goods in stockpile")
};

UENUM(BlueprintType)
enum class EEconomyActionResult : uint8
{
	Ok				UMETA(DisplayName = "Ok"),
	NotFound		UMETA(DisplayName = "Not found"),
	Invalid			UMETA(DisplayName = "Invalid"),
	NotPermitted	UMETA(DisplayName = "Not permitted")
};

/**
 * UEconomySubsystem — Coração da simulação econômica.
 *
 * Consome OnDayTick (apenas para construction queue) e OnMonthTick (todas
 * as fases pesadas). Por nação, executa o tick em 9 fases ordenadas
 * deterministicamente:
 *
 *   1. PopGrowth                : crescimento natural
 *   2. AssignEmployment         : preenche prédios com POPs disponíveis
 *   3. RunProduction            : tier 0 -> 1 -> 2 -> 3 (DAG sem ciclos)
 *   4. PopConsumption           : POPs comem; deficit derruba loyalty
 *   5. PaywagesAndProfits       : POPs ganham wage; Bourgeoisie pega profit
 *   6. CollectTaxes             : Treasury.Income via tax level por estrato
 *   7. PayExpenses              : Maintenance + ArmyUpkeep + Admin + Interest
 *   8. SettleTreasury           : aplica balance, loan se negativo
 *   9. ComputeStrategicIndices  : forward hook para Battle/Politics
 *
 * Construction:
 * - Day tick decrementa ConstructionDaysRemaining nos prédios em obra
 * - Quando chega a 0, dispara OnBuildingCompleted; o prédio entra no
 *   ciclo de produção a partir do próximo month tick
 *
 * O conteúdo (UGoodAsset, UProductionMethodAsset etc) vem de um
 * UEconomyContentRegistry indicado pelo GameMode em Initialize.
 */
UCLASS()
class STRATEGOSCORE_API UEconomySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;

	UFUNCTION(BlueprintCallable, Category = "Strategos|Economy")
	void SetContentRegistry(UEconomyContentRegistry* Registry);

	UFUNCTION(BlueprintPure, Category = "Strategos|Economy")
	UGoodAsset* GetGood(FName GoodId) const;

	UFUNCTION(BlueprintPure, Category = "Strategos|Economy")
	UProductionMethodAsset* GetProductionMethod(FName Id) const;

	UFUNCTION(BlueprintPure, Category = "Strategos|Economy")
	UProductionModifierAsset* GetProductionModifier(FName Id) const;

	UFUNCTION(BlueprintPure, Category = "Strategos|Economy")
	UBuildingTypeAsset* GetBuildingType(FName Id) const;

	/** Preço dinâmico = BasePrice × clamp(1 + (Demand-Supply)/Demand, 0.5, 2.0). */
	UFUNCTION(BlueprintPure, Category = "Strategos|Economy")
	float GetDynamicPrice(const UNation* Nation, FName GoodId) const;

	// --- Player API: construção e gestão de prédios ----------------------------

	/**
	 * Tenta construir um prédio em ProvinceId. Custo (goods + cash) sai do
	 * Treasury e do Stockpile da nação dona. ConstructionDays count down via
	 * UTimeSubsystem.OnDayTick.
	 */
	UFUNCTION(BlueprintCallable, Category = "Strategos|Economy|Player")
	EBuildResult BuildBuilding(FName NationId, FName ProvinceId, FName BuildingTypeId,
		EBuildingOwnerKind OwnerKind = EBuildingOwnerKind::Government);

	UFUNCTION(BlueprintCallable, Category = "Strategos|Economy|Player")
	EEconomyActionResult DemolishBuilding(FName BuildingId);

	UFUNCTION(BlueprintCallable, Category = "Strategos|Economy|Player")
	EEconomyActionResult UpgradeBuildingLevel(FName BuildingId);

	UFUNCTION(BlueprintCallable, Category = "Strategos|Economy|Player")
	EEconomyActionResult ChangeProductionMethod(FName BuildingId, FName NewMethodId);

	UFUNCTION(BlueprintCallable, Category = "Strategos|Economy|Player")
	EEconomyActionResult ToggleProductionModifier(FName BuildingId, FName ModifierId, bool bActivate);

	UFUNCTION(BlueprintCallable, Category = "Strategos|Economy|Player")
	void SetTaxLevel(FName NationId, EPopStratum Stratum, ETaxLevel NewLevel);

	UFUNCTION(BlueprintPure, Category = "Strategos|Economy|Player")
	UBuilding* FindBuildingById(FName BuildingId) const;

	UPROPERTY(BlueprintAssignable, Category = "Strategos|Economy")
	FOnEconomyTickComplete OnEconomyTickComplete;

	UPROPERTY(BlueprintAssignable, Category = "Strategos|Economy")
	FOnBuildingCompleted OnBuildingCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Strategos|Economy")
	FOnBankruptcyImminent OnBankruptcyImminent;

protected:
	UFUNCTION()
	void HandleDayTick(FDateTime CurrentDate);

	UFUNCTION()
	void HandleMonthTick(FDateTime CurrentDate);

	void RunMonthlyTickForNation(UNation& Nation, const FDateTime& CurrentDate);

	// Fases — implementadas nos commits 6-8.
	void Phase_PopGrowth(UNation& Nation);
	void Phase_AssignEmployment(UNation& Nation);
	void Phase_RunProduction(UNation& Nation);
	void Phase_PopConsumption(UNation& Nation);
	void Phase_PaywagesAndProfits(UNation& Nation);
	void Phase_CollectTaxes(UNation& Nation);
	void Phase_PayExpenses(UNation& Nation);
	void Phase_SettleTreasury(UNation& Nation);
	void Phase_ComputeStrategicIndices(UNation& Nation);

	// Construction tick (diário).
	void TickConstruction(UProvince& Province);

	// Helpers.
	UWorldState* ResolveWorldState() const;
	UTimeSubsystem* ResolveTime() const;

private:
	void RebuildContentLookups();

	UPROPERTY()
	TObjectPtr<UEconomyContentRegistry> ContentRegistry;

	UPROPERTY()
	TMap<FName, TObjectPtr<UGoodAsset>> GoodById;

	UPROPERTY()
	TMap<FName, TObjectPtr<UProductionMethodAsset>> MethodById;

	UPROPERTY()
	TMap<FName, TObjectPtr<UProductionModifierAsset>> ModifierById;

	UPROPERTY()
	TMap<FName, TObjectPtr<UBuildingTypeAsset>> BuildingTypeById;
};
