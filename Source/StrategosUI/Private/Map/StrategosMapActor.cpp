#include "Map/StrategosMapActor.h"
#include "StrategosUI.h"
#include "Map/StrategosProvinceVisualActor.h"
#include "Map/MapSubsystem.h"
#include "Game/StrategosGameState.h"
#include "World/WorldState.h"
#include "World/Province.h"
#include "World/Nation.h"
#include "Engine/World.h"
#include "TimerManager.h"

AStrategosMapActor::AStrategosMapActor()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AStrategosMapActor::BeginPlay()
{
	Super::BeginPlay();

	// Atrasa um pouco para garantir que o GameMode já rodou o bootstrap.
	GetWorldTimerManager().SetTimer(BuildTimer, this, &AStrategosMapActor::BuildVisuals,
		FMath::Max(SpawnDelaySeconds, 0.01f), false);

	if (UMapSubsystem* Map = GetWorld()->GetSubsystem<UMapSubsystem>())
	{
		Map->OnProvinceSelected.AddDynamic(this, &AStrategosMapActor::HandleProvinceSelected);
		Map->OnProvinceHovered.AddDynamic(this, &AStrategosMapActor::HandleProvinceHovered);
	}
}

void AStrategosMapActor::EndPlay(const EEndPlayReason::Type Reason)
{
	if (UMapSubsystem* Map = GetWorld()->GetSubsystem<UMapSubsystem>())
	{
		Map->OnProvinceSelected.RemoveDynamic(this, &AStrategosMapActor::HandleProvinceSelected);
		Map->OnProvinceHovered.RemoveDynamic(this, &AStrategosMapActor::HandleProvinceHovered);
	}
	Super::EndPlay(Reason);
}

UWorldState* AStrategosMapActor::ResolveWorldState() const
{
	const UWorld* World = GetWorld();
	if (!World) return nullptr;
	AStrategosGameState* GS = World->GetGameState<AStrategosGameState>();
	return GS ? GS->GetWorldState() : nullptr;
}

void AStrategosMapActor::BuildVisuals()
{
	UWorldState* WorldState = ResolveWorldState();
	if (!WorldState)
	{
		UE_LOG(LogStrategosUI, Warning, TEXT("MapActor: WorldState not available, retrying."));
		GetWorldTimerManager().SetTimer(BuildTimer, this, &AStrategosMapActor::BuildVisuals,
			SpawnDelaySeconds, false);
		return;
	}

	if (!ProvinceVisualClass)
	{
		UE_LOG(LogStrategosUI, Error, TEXT("MapActor: ProvinceVisualClass not set."));
		return;
	}

	UWorld* World = GetWorld();

	for (const auto& Pair : WorldState->Provinces)
	{
		const UProvince* Province = Pair.Value.Get();
		if (!Province) continue;

		FActorSpawnParameters Params;
		Params.Owner = this;
		AStrategosProvinceVisualActor* Visual = World->SpawnActor<AStrategosProvinceVisualActor>(
			ProvinceVisualClass, FVector::ZeroVector, FRotator::ZeroRotator, Params);
		if (!Visual) continue;

		Visual->InitializeFromProvince(Province->Id, Province->MapPosition);
		if (const UNation* Owner = WorldState->GetNation(Province->OwnerNationId))
		{
			Visual->SetOwnerColor(Owner->Color);
		}
		ProvinceVisuals.Add(Province->Id, Visual);
	}

	if (UMapSubsystem* Map = World->GetSubsystem<UMapSubsystem>())
	{
		Map->RebuildSpatialIndex();
	}

	UE_LOG(LogStrategosUI, Log, TEXT("MapActor: spawned %d province visuals"),
		ProvinceVisuals.Num());
}

void AStrategosMapActor::HandleProvinceSelected(FName ProvinceId)
{
	if (AStrategosProvinceVisualActor* Old = ProvinceVisuals.FindRef(CurrentSelected))
	{
		Old->SetSelected(false);
	}
	CurrentSelected = ProvinceId;
	if (AStrategosProvinceVisualActor* New = ProvinceVisuals.FindRef(ProvinceId))
	{
		New->SetSelected(true);
	}
}

void AStrategosMapActor::HandleProvinceHovered(FName ProvinceId)
{
	if (AStrategosProvinceVisualActor* Old = ProvinceVisuals.FindRef(CurrentHovered))
	{
		Old->SetHovered(false);
	}
	CurrentHovered = ProvinceId;
	if (AStrategosProvinceVisualActor* New = ProvinceVisuals.FindRef(ProvinceId))
	{
		New->SetHovered(true);
	}
}

void AStrategosMapActor::RefreshAllOwnerColors()
{
	UWorldState* WorldState = ResolveWorldState();
	if (!WorldState) return;

	for (auto& Pair : ProvinceVisuals)
	{
		AStrategosProvinceVisualActor* Visual = Pair.Value.Get();
		if (!Visual) continue;
		if (const UProvince* Province = WorldState->GetProvince(Pair.Key))
		{
			if (const UNation* Owner = WorldState->GetNation(Province->OwnerNationId))
			{
				Visual->SetOwnerColor(Owner->Color);
			}
		}
	}
}
