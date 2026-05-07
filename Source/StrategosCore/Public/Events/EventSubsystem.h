#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Events/EventContext.h"
#include "EventSubsystem.generated.h"

class UEventAsset;
class UEventContentRegistry;
class UWorldState;
class UTimeSubsystem;
class UMilitarySubsystem;
class UEconomySubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEventFired, const FEventContext&, Context);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDecisionEnqueued, const FEventContext&, Context);

/**
 * UEventSubsystem — Roteador central de eventos narrativos.
 *
 * Em Initialize:
 *  1. Carrega ContentRegistry (ou usa fallback hardcoded)
 *  2. Indexa UEventAsset por TriggerTag (TMap<FName, TArray<...>>)
 *  3. Subscreve em delegates dos outros subsistemas (Time/Military/Economy)
 *
 * Quando um trigger acontece (ex.: OnMonthTick):
 *  - Itera os eventos da tag em ordem alfabética por Id (determinismo)
 *  - Para cada, monta FEventContext, avalia Conditions, gate por MTTH
 *  - Notification/Silent → aplica AutoEffects, broadcast OnEventFired
 *  - Decision → broadcast OnDecisionEnqueued; player resolve depois
 *
 * MTTH usa FRandomStream seeded por (NationId, Year, Month, EventId)
 * para determinismo absoluto entre runs com mesmo save.
 */
UCLASS()
class STRATEGOSCORE_API UEventSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;

	UFUNCTION(BlueprintCallable, Category = "Strategos|Events")
	void SetContentRegistry(UEventContentRegistry* Registry);

	UFUNCTION(BlueprintPure, Category = "Strategos|Events")
	UEventAsset* GetEventById(FName EventId) const;

	/** Dispara manualmente um evento por Id (chaining ou debug). */
	UFUNCTION(BlueprintCallable, Category = "Strategos|Events")
	void FireEventById(FName EventId, const FEventContext& Context);

	UPROPERTY(BlueprintAssignable, Category = "Strategos|Events")
	FOnEventFired OnEventFired;

	UPROPERTY(BlueprintAssignable, Category = "Strategos|Events")
	FOnDecisionEnqueued OnDecisionEnqueued;

protected:
	UFUNCTION()
	void HandleMonthTick(FDateTime CurrentDate);

	UFUNCTION()
	void HandleYearTick(FDateTime CurrentDate);

	UFUNCTION()
	void HandleArmyArrived(FName ArmyId, FName ProvinceId);

	UFUNCTION()
	void HandleBuildingCompleted(FName BuildingId);

	UFUNCTION()
	void HandleBankruptcyImminent(FName NationId);

	void DispatchTrigger(FName TriggerTag, const FEventContext& BaseContext);

	bool EvaluateConditions(const UEventAsset& Event, const FEventContext& Context) const;

	bool RollMTTH(FName EventId, const FEventContext& Context, int32 MTTHMonths) const;

	void ApplyAutoEffects(const UEventAsset& Event, const FEventContext& Context);

private:
	void RebuildIndex();
	void RegisterFallbackEvents();
	void SubscribeToTriggers();
	void UnsubscribeFromTriggers();

	UWorldState* ResolveWorldState() const;
	UTimeSubsystem* ResolveTime() const;
	UMilitarySubsystem* ResolveMilitary() const;
	UEconomySubsystem* ResolveEconomy() const;

	UPROPERTY()
	TObjectPtr<UEventContentRegistry> ContentRegistry;

	/** Eventos hardcoded criados no Initialize quando não há registry. */
	UPROPERTY()
	TArray<TObjectPtr<UEventAsset>> FallbackEvents;

	/** Lookup por Id. */
	UPROPERTY()
	TMap<FName, TObjectPtr<UEventAsset>> EventById;

	/** Indexação primária: TriggerTag → TArray<EventAsset*>. */
	UPROPERTY()
	TMap<FName, TArray<TObjectPtr<UEventAsset>>> EventsByTrigger;
};
