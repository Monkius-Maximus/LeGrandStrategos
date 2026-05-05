#pragma once

#include "CoreMinimal.h"
#include "GameFlowState.generated.h"

UENUM(BlueprintType)
enum class EGameFlowState : uint8
{
	MainMenu	UMETA(DisplayName = "Main Menu"),
	Loading		UMETA(DisplayName = "Loading"),
	Running		UMETA(DisplayName = "Running"),
	Paused		UMETA(DisplayName = "Paused"),
	Battle		UMETA(DisplayName = "Battle"),
	Event		UMETA(DisplayName = "Event"),
	GameOver	UMETA(DisplayName = "Game Over")
};
