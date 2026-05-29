#include "BattleReplayService.h"
#include "StrategosBattle.h"

void UBattleReplayService::LoadFromResult(const FBattleResult& Result)
{
	Entries      = Result.CombatLog;
	CurrentIndex = Entries.IsEmpty() ? -1 : 0;

	UE_LOG(LogStrategosBattle, Log,
		TEXT("BattleReplayService: %d entradas carregadas para replay (batalha %s)."),
		Entries.Num(), *Result.BattleId.ToString());
}

bool UBattleReplayService::StepForward()
{
	if (CurrentIndex >= Entries.Num() - 1) return false;
	++CurrentIndex;
	return true;
}

bool UBattleReplayService::StepBackward()
{
	if (CurrentIndex <= 0) return false;
	--CurrentIndex;
	return true;
}

bool UBattleReplayService::JumpToEntry(int32 Index)
{
	if (!Entries.IsValidIndex(Index)) return false;
	CurrentIndex = Index;
	return true;
}

const FBattleLogEntry& UBattleReplayService::GetCurrentEntry() const
{
	if (!Entries.IsValidIndex(CurrentIndex)) return EmptyEntry;
	return Entries[CurrentIndex];
}
