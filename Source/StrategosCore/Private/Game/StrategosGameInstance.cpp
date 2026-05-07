#include "Game/StrategosGameInstance.h"
#include "StrategosCore.h"
#include "Foundation/GameFlow/GameFlowSubsystem.h"
#include "Foundation/Time/TimeSubsystem.h"
#include "Foundation/EventBus/EventBusSubsystem.h"

void UStrategosGameInstance::Init()
{
	Super::Init();
	UE_LOG(LogStrategosCore, Log, TEXT("StrategosGameInstance initialized."));
}

void UStrategosGameInstance::Shutdown()
{
	UE_LOG(LogStrategosCore, Log, TEXT("StrategosGameInstance shutdown."));
	Super::Shutdown();
}

UGameFlowSubsystem* UStrategosGameInstance::GetGameFlow() const
{
	return GetSubsystem<UGameFlowSubsystem>();
}

UTimeSubsystem* UStrategosGameInstance::GetTime() const
{
	return GetSubsystem<UTimeSubsystem>();
}

UEventBusSubsystem* UStrategosGameInstance::GetEventBus() const
{
	return GetSubsystem<UEventBusSubsystem>();
}
