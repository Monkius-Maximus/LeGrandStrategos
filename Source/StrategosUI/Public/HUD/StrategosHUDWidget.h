#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Foundation/Time/TimeSpeed.h"
#include "Events/EventContext.h"
#include "StrategosHUDWidget.generated.h"

class UTimeSubsystem;
class UMapSubsystem;
class USaveSubsystem;
class UEconomySubsystem;
class UEventSubsystem;
class UNation;

USTRUCT(BlueprintType)
struct STRATEGOSUI_API FShortfallEntry
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly) FName GoodId;
	UPROPERTY(BlueprintReadOnly) float Demand = 0.f;
	UPROPERTY(BlueprintReadOnly) float Supply = 0.f;
	UPROPERTY(BlueprintReadOnly) float ShortfallAmount = 0.f;
};

USTRUCT(BlueprintType)
struct STRATEGOSUI_API FBuildingHUDRow
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly) FName BuildingId;
	UPROPERTY(BlueprintReadOnly) FName ProvinceId;
	UPROPERTY(BlueprintReadOnly) FText BuildingTypeName;
	UPROPERTY(BlueprintReadOnly) int32 Level = 1;
	UPROPERTY(BlueprintReadOnly) bool bUnderConstruction = false;
	UPROPERTY(BlueprintReadOnly) int32 ConstructionDaysRemaining = 0;
	UPROPERTY(BlueprintReadOnly) float LastTickProfit = 0.f;
	UPROPERTY(BlueprintReadOnly) bool bIsPrivate = false;
};

USTRUCT(BlueprintType)
struct STRATEGOSUI_API FPendingDecisionHUDRow
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly) FName EventId;
	UPROPERTY(BlueprintReadOnly) FText Title;
	UPROPERTY(BlueprintReadOnly) FText Description;
	UPROPERTY(BlueprintReadOnly) TArray<FText> ChoiceLabels;
	UPROPERTY(BlueprintReadOnly) TArray<FText> ChoiceTooltips;
};

/**
 * UStrategosHUDWidget — Base C++ do HUD estratégico.
 *
 * Expõe propriedades bindables para o BP child montar o layout livremente
 * (texto de data, seleção, botões de velocidade). Funções de comando
 * (PauseGame, ResumeGame, SetSpeed*) são chamáveis pelos OnClick dos
 * botões no BP.
 *
 * O subsistema de tempo é consultado em ReceiveBoundUpdate (chamado por
 * NativeTick) e os eventos de seleção em UMapSubsystem disparam refresh
 * imediato.
 */
UCLASS(Abstract)
class STRATEGOSUI_API UStrategosHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Strategos|HUD")
	void PauseGame();

	UFUNCTION(BlueprintCallable, Category = "Strategos|HUD")
	void ResumeGame();

	UFUNCTION(BlueprintCallable, Category = "Strategos|HUD")
	void SetTimeSpeed(ETimeSpeed NewSpeed);

	UFUNCTION(BlueprintCallable, Category = "Strategos|HUD")
	void RequestSave(const FString& SlotName);

	UFUNCTION(BlueprintCallable, Category = "Strategos|HUD")
	void RequestLoad(const FString& SlotName);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Strategos|HUD")
	FText GetCurrentDateText() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Strategos|HUD")
	ETimeSpeed GetCurrentSpeed() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Strategos|HUD")
	FText GetSelectedProvinceName() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Strategos|HUD")
	FText GetSelectedProvinceOwnerName() const;

	// --- Economia ----------------------------------------------------------

	/** Resumo financeiro do jogador para mostrar na top bar. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Strategos|HUD|Economy")
	float GetTreasuryBalance() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Strategos|HUD|Economy")
	float GetMonthlyIncome() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Strategos|HUD|Economy")
	float GetMonthlyExpenses() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Strategos|HUD|Economy")
	float GetDebtBalance() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Strategos|HUD|Economy")
	float GetMilitaryReadinessIndex() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Strategos|HUD|Economy")
	float GetCivilianMoraleIndex() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Strategos|HUD|Economy")
	float GetIndustrialCapacityIndex() const;

	/** Lê quanto da nação tem do bem; útil para mostrar reservas estratégicas. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Strategos|HUD|Economy")
	float GetGoodStock(FName GoodId) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Strategos|HUD|Economy")
	float GetGoodPrice(FName GoodId) const;

	/** Top N bens com maior shortfall (Demand>Supply). Pega do tick anterior. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Strategos|HUD|Economy")
	TArray<FShortfallEntry> GetTopShortfalls(int32 MaxEntries = 3) const;

	/** Lista de prédios da nação do jogador para o painel da economia. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Strategos|HUD|Economy")
	TArray<FBuildingHUDRow> GetPlayerBuildings() const;

	// --- Eventos -----------------------------------------------------------

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Strategos|HUD|Events")
	bool HasPendingDecisions() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Strategos|HUD|Events")
	int32 GetPendingDecisionCount() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Strategos|HUD|Events")
	bool GetTopPendingDecision(FPendingDecisionHUDRow& OutDecision) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Strategos|HUD|Events")
	TArray<FPendingDecisionHUDRow> GetAllPendingDecisions() const;

	/** Resolve a decisão dada com a escolha indicada. Retorna true se OK. */
	UFUNCTION(BlueprintCallable, Category = "Strategos|HUD|Events")
	bool ResolvePendingDecision(FName EventId, int32 ChoiceIndex);

	/** BP override para reagir quando um evento Notification dispara. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Strategos|HUD|Events")
	void OnNotificationFired(const FName& EventId, const FText& Title, const FText& Description);

	/** BP override para reagir quando uma decisão entra na fila. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Strategos|HUD|Events")
	void OnDecisionEnqueued(const FName& EventId);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** Hook para o BP atualizar binds não-disponíveis em PURE getters (ex.: cores). */
	UFUNCTION(BlueprintImplementableEvent, Category = "Strategos|HUD")
	void OnSelectionChanged(FName ProvinceId);

private:
	UFUNCTION()
	void HandleProvinceSelected(FName ProvinceId);

	UFUNCTION()
	void HandleEventFired(const FEventContext& Context);

	UFUNCTION()
	void HandleDecisionEnqueued(const FEventContext& Context);

	UTimeSubsystem* ResolveTime() const;
	UMapSubsystem* ResolveMap() const;
	USaveSubsystem* ResolveSave() const;
	UEconomySubsystem* ResolveEconomy() const;
	UEventSubsystem* ResolveEvents() const;
	UNation* ResolvePlayerNation() const;
};
