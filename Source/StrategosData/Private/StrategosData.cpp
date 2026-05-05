#include "StrategosData.h"

DEFINE_LOG_CATEGORY(LogStrategosData);

void FStrategosDataModule::StartupModule()
{
	UE_LOG(LogStrategosData, Log, TEXT("StrategosData module started."));
}

void FStrategosDataModule::ShutdownModule()
{
	UE_LOG(LogStrategosData, Log, TEXT("StrategosData module shutdown."));
}

IMPLEMENT_MODULE(FStrategosDataModule, StrategosData);
