#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "StrategosGameMode.generated.h"

UCLASS()
class STRATEGOSCORE_API AStrategosGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AStrategosGameMode();

protected:
	virtual void BeginPlay() override;
};
