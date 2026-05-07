#include "Map/MapSubsystem.h"
#include "StrategosCore.h"
#include "World/WorldState.h"
#include "World/Province.h"
#include "Game/StrategosGameState.h"
#include "Engine/World.h"

void UMapSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogStrategosCore, Log, TEXT("MapSubsystem initialized."));
}

void UMapSubsystem::Deinitialize()
{
	SpatialIndex.Empty();
	Super::Deinitialize();
}

bool UMapSubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

UWorldState* UMapSubsystem::ResolveWorldState() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}
	AStrategosGameState* GS = World->GetGameState<AStrategosGameState>();
	return GS ? GS->GetWorldState() : nullptr;
}

void UMapSubsystem::RebuildSpatialIndex()
{
	SpatialIndex.Empty();

	const UWorldState* WorldState = ResolveWorldState();
	if (!WorldState)
	{
		return;
	}

	SpatialIndex.Reserve(WorldState->Provinces.Num());
	for (const auto& Pair : WorldState->Provinces)
	{
		if (const UProvince* Province = Pair.Value.Get())
		{
			SpatialIndex.Add(Pair.Key, Province->MapPosition);
		}
	}

	UE_LOG(LogStrategosCore, Log, TEXT("MapSubsystem indexed %d provinces."),
		SpatialIndex.Num());
}

FName UMapSubsystem::FindNearestProvinceTo(FVector2D MapPosition) const
{
	FName Best = NAME_None;
	double BestDistSq = TNumericLimits<double>::Max();

	for (const auto& Pair : SpatialIndex)
	{
		const double DistSq = FVector2D::DistSquared(Pair.Value, MapPosition);
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Best = Pair.Key;
		}
	}

	return Best;
}

void UMapSubsystem::SelectProvince(FName ProvinceId)
{
	if (SelectedProvinceId == ProvinceId)
	{
		return;
	}
	SelectedProvinceId = ProvinceId;
	OnProvinceSelected.Broadcast(ProvinceId);
}

void UMapSubsystem::HoverProvince(FName ProvinceId)
{
	if (HoveredProvinceId == ProvinceId)
	{
		return;
	}
	HoveredProvinceId = ProvinceId;
	OnProvinceHovered.Broadcast(ProvinceId);
}
