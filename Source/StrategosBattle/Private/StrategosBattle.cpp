#include "StrategosBattle.h"

DEFINE_LOG_CATEGORY(LogStrategosBattle);

void FStrategosBattleModule::StartupModule()
{
	UE_LOG(LogStrategosBattle, Log, TEXT("StrategosBattle module started."));
}

void FStrategosBattleModule::ShutdownModule()
{
	UE_LOG(LogStrategosBattle, Log, TEXT("StrategosBattle module shutdown."));
}

IMPLEMENT_MODULE(FStrategosBattleModule, StrategosBattle);
