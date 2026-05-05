#include "StrategosUI.h"

DEFINE_LOG_CATEGORY(LogStrategosUI);

void FStrategosUIModule::StartupModule()
{
	UE_LOG(LogStrategosUI, Log, TEXT("StrategosUI module started."));
}

void FStrategosUIModule::ShutdownModule()
{
	UE_LOG(LogStrategosUI, Log, TEXT("StrategosUI module shutdown."));
}

IMPLEMENT_MODULE(FStrategosUIModule, StrategosUI);
