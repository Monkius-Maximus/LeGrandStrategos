#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "StrategosGameState.generated.h"

class UWorldState;

/**
 * AStrategosGameState — container do estado replicável do jogo.
 *
 * Detém o UWorldState (Nations, Provinces, Armies) como dono canônico
 * do estado da simulação. Subsistemas pegam o WorldState a partir daqui
 * em vez de criarem o próprio.
 */
UCLASS()
class STRATEGOSCORE_API AStrategosGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AStrategosGameState();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Strategos")
	UWorldState* GetWorldState() const { return WorldState; }

	UWorldState* GetOrCreateWorldState();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TObjectPtr<UWorldState> WorldState;
};
