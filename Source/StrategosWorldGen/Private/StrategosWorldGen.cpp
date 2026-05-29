#include "StrategosWorldGen.h"

DEFINE_LOG_CATEGORY(LogStrategosWorldGen);

void FStrategosWorldGenModule::StartupModule()
{
	UE_LOG(LogStrategosWorldGen, Log, TEXT("StrategosWorldGen module started."));
}

void FStrategosWorldGenModule::ShutdownModule()
{
	UE_LOG(LogStrategosWorldGen, Log, TEXT("StrategosWorldGen module shutdown."));
}

IMPLEMENT_MODULE(FStrategosWorldGenModule, StrategosWorldGen);
