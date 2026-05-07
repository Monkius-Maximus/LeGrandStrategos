#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Foundation/GameFlow/GameFlowState.h"
#include "GameFlowSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGameFlowStateChanged, EGameFlowState, FromState, EGameFlowState, ToState);

/**
 * UGameFlowSubsystem — FSM principal do jogo.
 *
 * Mantém o estado global (MainMenu, Loading, Running, Paused, Battle, Event, GameOver)
 * e enforce as transições permitidas. Quem quiser saber sobre mudança de estado
 * inscreve em OnStateChanged.
 *
 * Ver docs/architecture/01-game-flow.md para tabela completa de transições.
 */
UCLASS()
class STRATEGOSCORE_API UGameFlowSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Strategos|GameFlow")
	bool TransitionTo(EGameFlowState NewState);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Strategos|GameFlow")
	EGameFlowState GetCurrentState() const { return CurrentState; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Strategos|GameFlow")
	bool IsTransitionAllowed(EGameFlowState From, EGameFlowState To) const;

	UPROPERTY(BlueprintAssignable, Category = "Strategos|GameFlow")
	FOnGameFlowStateChanged OnStateChanged;

private:
	void BuildTransitionTable();

	UPROPERTY()
	EGameFlowState CurrentState = EGameFlowState::MainMenu;

	TMap<EGameFlowState, TSet<EGameFlowState>> AllowedTransitions;
};
