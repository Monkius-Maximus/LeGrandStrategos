#include "Save/SaveSubsystem.h"
#include "StrategosCore.h"
#include "Save/StrategosSaveData.h"
#include "World/WorldState.h"
#include "World/Nation.h"
#include "World/Province.h"
#include "World/Army.h"
#include "Foundation/Time/TimeSubsystem.h"
#include "Game/StrategosGameState.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void USaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogStrategosCore, Log, TEXT("SaveSubsystem initialized."));
}

void USaveSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

UWorldState* USaveSubsystem::ResolveWorldState() const
{
	const UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return nullptr;
	}

	const UWorld* World = GI->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	AStrategosGameState* GS = World->GetGameState<AStrategosGameState>();
	return GS ? GS->GetOrCreateWorldState() : nullptr;
}

UStrategosSaveData* USaveSubsystem::CaptureSnapshot() const
{
	const UWorldState* WorldState = ResolveWorldState();
	if (!WorldState)
	{
		return nullptr;
	}

	UStrategosSaveData* Snapshot = Cast<UStrategosSaveData>(
		UGameplayStatics::CreateSaveGameObject(UStrategosSaveData::StaticClass()));
	if (!Snapshot)
	{
		return nullptr;
	}

	Snapshot->PlayerNationId = WorldState->PlayerNationId;

	if (const UTimeSubsystem* Time = GetGameInstance()->GetSubsystem<UTimeSubsystem>())
	{
		Snapshot->CurrentDate = Time->GetCurrentDate();
	}

	Snapshot->Nations.Reserve(WorldState->Nations.Num());
	for (const auto& Pair : WorldState->Nations)
	{
		const UNation* Nation = Pair.Value.Get();
		if (!Nation) continue;
		FNationRecord R;
		R.Id = Nation->Id;
		R.DisplayName = Nation->DisplayName;
		R.Color = Nation->Color;
		R.CapitalProvinceId = Nation->CapitalProvinceId;
		R.OwnedProvinceIds = Nation->OwnedProvinceIds;
		R.bIsPlayerControlled = Nation->bIsPlayerControlled;
		Snapshot->Nations.Add(R);
	}

	Snapshot->Provinces.Reserve(WorldState->Provinces.Num());
	for (const auto& Pair : WorldState->Provinces)
	{
		const UProvince* Province = Pair.Value.Get();
		if (!Province) continue;
		FProvinceRecord R;
		R.Id = Province->Id;
		R.DisplayName = Province->DisplayName;
		R.OwnerNationId = Province->OwnerNationId;
		R.AdjacentProvinceIds = Province->AdjacentProvinceIds;
		R.MapPosition = Province->MapPosition;
		R.Terrain = Province->Terrain;
		Snapshot->Provinces.Add(R);
	}

	Snapshot->Armies.Reserve(WorldState->Armies.Num());
	for (const auto& Pair : WorldState->Armies)
	{
		const UArmy* Army = Pair.Value.Get();
		if (!Army) continue;
		FArmyRecord R;
		R.Id = Army->Id;
		R.DisplayName = Army->DisplayName;
		R.OwnerNationId = Army->OwnerNationId;
		R.CurrentProvinceId = Army->CurrentProvinceId;
		R.MoveTargetProvinceId = Army->MoveTargetProvinceId;
		R.MoveDaysRemaining = Army->MoveDaysRemaining;
		R.ManpowerCount = Army->ManpowerCount;
		Snapshot->Armies.Add(R);
	}

	return Snapshot;
}

bool USaveSubsystem::ApplySnapshot(const UStrategosSaveData& Snapshot)
{
	UWorldState* WorldState = ResolveWorldState();
	if (!WorldState)
	{
		return false;
	}

	WorldState->Nations.Empty();
	WorldState->Provinces.Empty();
	WorldState->Armies.Empty();
	WorldState->PlayerNationId = Snapshot.PlayerNationId;

	for (const FProvinceRecord& R : Snapshot.Provinces)
	{
		UProvince* P = WorldState->AddProvince(R.Id);
		P->DisplayName = R.DisplayName;
		P->OwnerNationId = R.OwnerNationId;
		P->AdjacentProvinceIds = R.AdjacentProvinceIds;
		P->MapPosition = R.MapPosition;
		P->Terrain = R.Terrain;
	}

	for (const FNationRecord& R : Snapshot.Nations)
	{
		UNation* N = WorldState->AddNation(R.Id);
		N->DisplayName = R.DisplayName;
		N->Color = R.Color;
		N->CapitalProvinceId = R.CapitalProvinceId;
		N->OwnedProvinceIds = R.OwnedProvinceIds;
		N->bIsPlayerControlled = R.bIsPlayerControlled;
	}

	for (const FArmyRecord& R : Snapshot.Armies)
	{
		UArmy* A = WorldState->AddArmy(R.Id);
		A->DisplayName = R.DisplayName;
		A->OwnerNationId = R.OwnerNationId;
		A->CurrentProvinceId = R.CurrentProvinceId;
		A->MoveTargetProvinceId = R.MoveTargetProvinceId;
		A->MoveDaysRemaining = R.MoveDaysRemaining;
		A->ManpowerCount = R.ManpowerCount;
	}

	if (UTimeSubsystem* Time = GetGameInstance()->GetSubsystem<UTimeSubsystem>())
	{
		const FDateTime& D = Snapshot.CurrentDate;
		Time->SetStartDate(D.GetYear(), D.GetMonth(), D.GetDay());
	}

	return true;
}

bool USaveSubsystem::SaveToSlot(const FString& SlotName)
{
	UStrategosSaveData* Snapshot = CaptureSnapshot();
	if (!Snapshot)
	{
		OnSaveCompleted.Broadcast(false);
		return false;
	}

	const bool bOk = UGameplayStatics::SaveGameToSlot(Snapshot, SlotName, 0);
	UE_LOG(LogStrategosCore, Log, TEXT("Save '%s': %s"),
		*SlotName, bOk ? TEXT("ok") : TEXT("failed"));
	OnSaveCompleted.Broadcast(bOk);
	return bOk;
}

bool USaveSubsystem::LoadFromSlot(const FString& SlotName)
{
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		OnLoadCompleted.Broadcast(false);
		return false;
	}

	UStrategosSaveData* Loaded = Cast<UStrategosSaveData>(
		UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (!Loaded)
	{
		OnLoadCompleted.Broadcast(false);
		return false;
	}

	const bool bOk = ApplySnapshot(*Loaded);
	UE_LOG(LogStrategosCore, Log, TEXT("Load '%s': %s"),
		*SlotName, bOk ? TEXT("ok") : TEXT("failed"));
	OnLoadCompleted.Broadcast(bOk);
	return bOk;
}

bool USaveSubsystem::DoesSaveExist(const FString& SlotName) const
{
	return UGameplayStatics::DoesSaveGameExist(SlotName, 0);
}
