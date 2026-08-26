#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HAL/IConsoleManager.h"
#include "EventDebugSubsystem.generated.h"

class UEventSubsystem;
class USaveSubsystem;
class SWidget;

/**
 * UEventDebugSubsystem — Comandos de console para inspecionar e testar o sistema de eventos.
 *
 * Não instanciado em builds shipping (DoesSupportWorldType retorna false).
 * Registra os comandos Strategos.Event.* em Initialize.
 *
 * Comandos disponíveis:
 *   Strategos.Event.Fire <EventId> <NationId>
 *   Strategos.Event.ListPending
 *   Strategos.Event.CreateTestDecision <CustomId> <NationId>
 *   Strategos.Event.TestPersistence <CustomId> <NationId>
 */
UCLASS()
class STRATEGOSCORE_API UEventDebugSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;

	/** Dispara qualquer evento registrado para a nação informada. */
	UFUNCTION(BlueprintCallable, Category = "Strategos|Debug")
	void FireEventDebug(FName EventId, FName NationId);

	/**
	 * Cria um evento Decision efêmero com o Id informado, registra no EventSubsystem
	 * e o dispara para a nação (enfileira se for o jogador, auto-resolve caso contrário).
	 */
	UFUNCTION(BlueprintCallable, Category = "Strategos|Debug")
	void CreateAndFireTestDecision(FName CustomEventId, FName NationId);

	/** Loga todas as decisões pendentes para todas as nações. */
	UFUNCTION(BlueprintCallable, Category = "Strategos|Debug")
	FString DumpPendingDecisions() const;

	/**
	 * Ciclo completo de teste de persistência:
	 * cria decisão efêmera → SaveToSlot("_debug_test") → LoadFromSlot → verifica se
	 * a decisão ainda existe → loga PASS/FAIL no log e na tela.
	 */
	UFUNCTION(BlueprintCallable, Category = "Strategos|Debug")
	bool TestSaveLoadPersistence(FName CustomEventId, FName NationId);

	/** Mostra/esconde o painel de debug visual (Slate overlay). Atalho: Strategos.Event.ToggleDebugUI */
	UFUNCTION(BlueprintCallable, Category = "Strategos|Debug")
	void ToggleDebugUI();

	/** Repopula a lista de decisions do overlay, se ele estiver visível. */
	UFUNCTION(BlueprintCallable, Category = "Strategos|Debug")
	void RefreshDebugUI();

private:
	void RegisterConsoleCommands();
	void UnregisterConsoleCommands();

	void Cmd_FireEvent(const TArray<FString>& Args);
	void Cmd_ListPending(const TArray<FString>& Args);
	void Cmd_CreateTestDecision(const TArray<FString>& Args);
	void Cmd_TestPersistence(const TArray<FString>& Args);
	void Cmd_ToggleUI(const TArray<FString>& Args);

	UEventSubsystem* ResolveEventSubsystem() const;
	USaveSubsystem*  ResolveSaveSubsystem()  const;

	TArray<IConsoleObject*> ConsoleObjects;

	/** Overlay ativo. Não é UPROPERTY: SWidget não é UObject. */
	TSharedPtr<SWidget> DebugOverlay;
};
