#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "StrategosGameInstance.generated.h"

class UGameFlowSubsystem;
class UTimeSubsystem;
class UEventBusSubsystem;

/**
 * UStrategosGameInstance — host dos UGameInstanceSubsystem do jogo.
 *
 * Não detém lógica própria nesta etapa: apenas existe para que o engine
 * instancie GameFlow / Time / EventBus e expõe getters convenientes para
 * código que precisa pegá-los a partir de qualquer UWorld.
 */
UCLASS()
class STRATEGOSCORE_API UStrategosGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Strategos")
	UGameFlowSubsystem* GetGameFlow() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Strategos")
	UTimeSubsystem* GetTime() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Strategos")
	UEventBusSubsystem* GetEventBus() const;
};
