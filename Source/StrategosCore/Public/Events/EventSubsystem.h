#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Events/EventContext.h"
#include "Events/EventChoice.h"
#include "Events/EventHistory.h"
#include "EventSubsystem.generated.h"

class UEventAsset;
class UEventContentRegistry;
class UWorldState;
class UTimeSubsystem;
class UMilitarySubsystem;
class UEconomySubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEventFired, const FEventContext&, Context);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDecisionEnqueued, const FEventContext&, Context);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDecisionResolved, const FEventContext&, Context, int32, ChoiceIndex);

UENUM(BlueprintType)
enum class EDecisionResolveResult : uint8
{
	Ok					UMETA(DisplayName = "Ok"),
	NoSuchDecision		UMETA(DisplayName = "No such pending decision"),
	InvalidChoice		UMETA(DisplayName = "Invalid choice index")
};

USTRUCT(BlueprintType)
struct STRATEGOSCORE_API FPendingDecision
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FEventContext Context;
};

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

	/**
	 * Registra um evento efêmero sem substituir o ContentRegistry.
	 * Usado por debug e encadeamentos em runtime. Não sobrevive a RebuildIndex().
	 */
	UFUNCTION(BlueprintCallable, Category = "Strategos|Events")
	void RegisterEphemeralEvent(UEventAsset* Event);

	/**
	 * Dispara manualmente um evento por Id (chaining ou debug).
	 * bBypassRepeatPolicy ignora Once/Cooldown — use apenas em debug.
	 */
	UFUNCTION(BlueprintCallable, Category = "Strategos|Events")
	void FireEventById(FName EventId, const FEventContext& Context, bool bBypassRepeatPolicy = false);

	/** False se RepeatPolicy (Once/Cooldown) bloqueia este evento para esta nação agora. */
	UFUNCTION(BlueprintPure, Category = "Strategos|Events")
	bool CanEventFire(const UEventAsset* Event, const FEventContext& Context) const;

	// --- Decision queue ----------------------------------------------------

	/** Pending decisions da nação dada (vazio = nação do player). */
	UFUNCTION(BlueprintPure, Category = "Strategos|Events")
	TArray<FPendingDecision> GetPendingDecisions(FName NationId) const;

	UFUNCTION(BlueprintPure, Category = "Strategos|Events")
	bool HasPendingDecisions(FName NationId) const;

	/** Resolve uma decisão aplicando os effects da choice. */
	UFUNCTION(BlueprintCallable, Category = "Strategos|Events")
	EDecisionResolveResult ResolveDecision(FName NationId, FName EventId, int32 ChoiceIndex);

	UPROPERTY(BlueprintAssignable, Category = "Strategos|Events")
	FOnEventFired OnEventFired;

	UPROPERTY(BlueprintAssignable, Category = "Strategos|Events")
	FOnDecisionEnqueued OnDecisionEnqueued;

	UPROPERTY(BlueprintAssignable, Category = "Strategos|Events")
	FOnDecisionResolved OnDecisionResolved;

	// --- História ----------------------------------------------------------

	/**
	 * Log de disparos, do mais antigo ao mais recente. Tem teto de tamanho.
	 * Sem UFUNCTION: Blueprint não aceita retorno por referência, e copiar
	 * centenas de records a cada chamada não vale o acesso em BP.
	 */
	const TArray<FFiredEventRecord>& GetHistory() const { return History; }

	/** True se o evento já disparou para essa nação em qualquer momento da partida. */
	UFUNCTION(BlueprintPure, Category = "Strategos|Events")
	bool HasEventEverFired(FName EventId, FName NationId) const;

	/**
	 * Índice da escolha feita no disparo mais recente desse evento para essa nação.
	 * INDEX_NONE se nunca disparou, se ainda não foi resolvido, ou se o registro
	 * já saiu do log por limite de tamanho.
	 */
	UFUNCTION(BlueprintPure, Category = "Strategos|Events")
	int32 GetLastChoiceFor(FName EventId, FName NationId) const;

	// --- Save/Load support -------------------------------------------------

	/** Snapshot da fila de decisões pendentes (serialização). */
	const TMap<FName, TArray<FPendingDecision>>& GetPendingDecisionsRaw() const { return PendingByNation; }

	/** Restaura a fila a partir de records carregados. Substitui estado atual. */
	void RestorePendingDecisions(const TMap<FName, TArray<FPendingDecision>>& Pending);

	const TMap<FName, FNationEventState>& GetNationStatesRaw() const { return StateByNation; }
	const TSet<FName>& GetGlobalFiredRaw() const { return GlobalEverFired; }

	/** Restaura história e memória de disparos. Substitui estado atual. */
	void RestoreHistory(const TArray<FFiredEventRecord>& InHistory,
		const TMap<FName, FNationEventState>& InStates,
		const TSet<FName>& InGlobalFired);

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

	void ApplyChoiceEffects(const FEventChoice& Choice, const FEventContext& Context);

	void EnqueueOrAutoResolve(UEventAsset& Event, const FEventContext& Context);

	int32 PickAIChoice(const UEventAsset& Event, const FEventContext& Context) const;

	/** Caminho único de disparo: registra a ocorrência e despacha por Type. */
	void DispatchResolved(UEventAsset& Event, const FEventContext& Context);

	/** Grava o disparo em EverFired/cooldown/history. */
	void RecordFired(const UEventAsset& Event, const FEventContext& Context);

	/** Preenche o ChoiceIndex do registro pendente mais recente desse evento. */
	void RecordChoice(FName EventId, FName NationId, int32 ChoiceIndex);

	void AppendHistory(const FFiredEventRecord& Record);

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

	/** Pending decisions por nação. Ordem: FIFO. */
	UPROPERTY()
	TMap<FName, TArray<FPendingDecision>> PendingByNation;

	/** Memória de disparos por nação (Once/Cooldown). */
	UPROPERTY()
	TMap<FName, FNationEventState> StateByNation;

	/** Eventos com RepeatPolicy OnceGlobal que já dispararam. */
	UPROPERTY()
	TSet<FName> GlobalEverFired;

	/** Log de disparos, mais antigo primeiro. Truncado em MaxHistoryEntries. */
	UPROPERTY()
	TArray<FFiredEventRecord> History;

	/**
	 * Teto do log. Uma campanha de 60 anos com eventos mensais em várias nações
	 * geraria milhares de entradas; o log serve para consulta recente e vai
	 * inteiro para o save. EverFired cobre as consultas de longo prazo.
	 */
	static constexpr int32 MaxHistoryEntries = 512;
};
