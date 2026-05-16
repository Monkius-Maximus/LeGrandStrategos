#include "Diplomacy/DiplomacySubsystem.h"
#include "StrategosCore.h"
#include "Foundation/Time/TimeSubsystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

void UDiplomacySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogStrategosCore, Log, TEXT("DiplomacySubsystem initialized."));
}

void UDiplomacySubsystem::Deinitialize()
{
	Relations.Empty();
	Super::Deinitialize();
}

bool UDiplomacySubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game
		|| WorldType == EWorldType::PIE
		|| WorldType == EWorldType::Editor
		|| WorldType == EWorldType::EditorPreview;
}

void UDiplomacySubsystem::SetRelation(FName A, FName B, EDiplomaticStatus Status, float Opinion)
{
	const FNationPair Key = FNationPair::Make(A, B);
	if (!Key.IsValid())
	{
		UE_LOG(LogStrategosCore, Warning, TEXT("Diplomacy::SetRelation invalid pair (%s, %s)"),
			*A.ToString(), *B.ToString());
		return;
	}

	FDiplomaticRelation& Rel = Relations.FindOrAdd(Key);
	const bool bStatusChanged = (Rel.Pair != Key) || (Rel.Status != Status);
	Rel.Pair = Key;
	Rel.Status = Status;
	Rel.Opinion = FMath::Clamp(Opinion, -100.f, 100.f);
	if (bStatusChanged)
	{
		if (const UWorld* World = GetWorld())
		{
			if (const UGameInstance* GI = World->GetGameInstance())
			{
				if (const UTimeSubsystem* Time = GI->GetSubsystem<UTimeSubsystem>())
				{
					Rel.LastStatusChange = Time->GetCurrentDate();
				}
			}
		}
	}
	BroadcastChange(Rel);
}

void UDiplomacySubsystem::SetStatus(FName A, FName B, EDiplomaticStatus NewStatus)
{
	const FNationPair Key = FNationPair::Make(A, B);
	if (!Key.IsValid()) return;

	FDiplomaticRelation& Rel = Relations.FindOrAdd(Key);
	if (Rel.Pair != Key) Rel.Pair = Key;
	if (Rel.Status == NewStatus) return;

	Rel.Status = NewStatus;
	if (const UGameInstance* GI = GetGameInstance())
	{
		if (const UTimeSubsystem* Time = GI->GetSubsystem<UTimeSubsystem>())
		{
			Rel.LastStatusChange = Time->GetCurrentDate();
		}
	}
	BroadcastChange(Rel);
}

void UDiplomacySubsystem::SetOpinion(FName A, FName B, float NewOpinion)
{
	const FNationPair Key = FNationPair::Make(A, B);
	if (!Key.IsValid()) return;

	FDiplomaticRelation& Rel = Relations.FindOrAdd(Key);
	if (Rel.Pair != Key) Rel.Pair = Key;
	Rel.Opinion = FMath::Clamp(NewOpinion, -100.f, 100.f);
	BroadcastChange(Rel);
}

void UDiplomacySubsystem::AdjustOpinion(FName A, FName B, float Delta)
{
	SetOpinion(A, B, GetOpinion(A, B) + Delta);
}

bool UDiplomacySubsystem::HasRelation(FName A, FName B) const
{
	return Relations.Contains(FNationPair::Make(A, B));
}

FDiplomaticRelation UDiplomacySubsystem::GetRelation(FName A, FName B) const
{
	const FNationPair Key = FNationPair::Make(A, B);
	if (const FDiplomaticRelation* Found = Relations.Find(Key))
	{
		return *Found;
	}
	FDiplomaticRelation Default;
	Default.Pair = Key;
	return Default;
}

EDiplomaticStatus UDiplomacySubsystem::GetStatus(FName A, FName B) const
{
	if (const FDiplomaticRelation* Found = Relations.Find(FNationPair::Make(A, B)))
	{
		return Found->Status;
	}
	return EDiplomaticStatus::Peace;
}

float UDiplomacySubsystem::GetOpinion(FName A, FName B) const
{
	if (const FDiplomaticRelation* Found = Relations.Find(FNationPair::Make(A, B)))
	{
		return Found->Opinion;
	}
	return 0.f;
}

bool UDiplomacySubsystem::AreAtWar(FName A, FName B) const
{
	return GetStatus(A, B) == EDiplomaticStatus::War;
}

bool UDiplomacySubsystem::AreAllied(FName A, FName B) const
{
	return GetStatus(A, B) == EDiplomaticStatus::Alliance;
}

TArray<FName> UDiplomacySubsystem::GetKnownCounterparts(FName Nation) const
{
	TArray<FName> Out;
	for (const auto& Pair : Relations)
	{
		if (Pair.Key.A == Nation) Out.AddUnique(Pair.Key.B);
		else if (Pair.Key.B == Nation) Out.AddUnique(Pair.Key.A);
	}
	return Out;
}

void UDiplomacySubsystem::RestoreRelations(const TArray<FDiplomaticRelation>& Snapshot)
{
	Relations.Empty();
	for (const FDiplomaticRelation& R : Snapshot)
	{
		if (!R.Pair.IsValid()) continue;
		Relations.Add(R.Pair, R);
	}
}

void UDiplomacySubsystem::BroadcastChange(const FDiplomaticRelation& Rel)
{
	OnRelationChanged.Broadcast(Rel);
}
