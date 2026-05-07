#include "Game/StrategosGameState.h"
#include "StrategosCore.h"
#include "World/WorldState.h"

AStrategosGameState::AStrategosGameState()
{
}

void AStrategosGameState::BeginPlay()
{
	Super::BeginPlay();
	GetOrCreateWorldState();
	UE_LOG(LogStrategosCore, Log, TEXT("StrategosGameState BeginPlay; WorldState ready."));
}

UWorldState* AStrategosGameState::GetOrCreateWorldState()
{
	if (!WorldState)
	{
		WorldState = NewObject<UWorldState>(this, TEXT("WorldState"));
	}
	return WorldState;
}
