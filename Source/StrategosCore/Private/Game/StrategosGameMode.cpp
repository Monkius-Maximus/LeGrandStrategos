#include "Game/StrategosGameMode.h"
#include "StrategosCore.h"
#include "Game/StrategosGameState.h"

AStrategosGameMode::AStrategosGameMode()
{
	GameStateClass = AStrategosGameState::StaticClass();
	bUseSeamlessTravel = true;
}

void AStrategosGameMode::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogStrategosCore, Log, TEXT("StrategosGameMode BeginPlay."));
}
