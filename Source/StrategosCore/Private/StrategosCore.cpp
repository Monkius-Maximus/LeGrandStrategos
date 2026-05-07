#include "StrategosCore.h"

DEFINE_LOG_CATEGORY(LogStrategosCore);

void FStrategosCoreModule::StartupModule()
{
	UE_LOG(LogStrategosCore, Log, TEXT("StrategosCore module started."));
}

void FStrategosCoreModule::ShutdownModule()
{
	UE_LOG(LogStrategosCore, Log, TEXT("StrategosCore module shutdown."));
}

IMPLEMENT_PRIMARY_GAME_MODULE(FStrategosCoreModule, StrategosCore, "StrategosCore");
