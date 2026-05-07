#include "StrategosAI.h"

DEFINE_LOG_CATEGORY(LogStrategosAI);

void FStrategosAIModule::StartupModule()
{
	UE_LOG(LogStrategosAI, Log, TEXT("StrategosAI module started."));
}

void FStrategosAIModule::ShutdownModule()
{
	UE_LOG(LogStrategosAI, Log, TEXT("StrategosAI module shutdown."));
}

IMPLEMENT_MODULE(FStrategosAIModule, StrategosAI);
