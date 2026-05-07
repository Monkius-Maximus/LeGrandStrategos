#include "Map/StrategosMapActor.h"
#include "StrategosUI.h"
#include "Map/StrategosProvinceVisualActor.h"
#include "Army/StrategosArmyVisualActor.h"
#include "Map/MapSubsystem.h"
#include "Strategy/MilitarySubsystem.h"
#include "Game/StrategosGameState.h"
#include "World/WorldState.h"
#include "World/Province.h"
#include "World/Nation.h"
#include "World/Army.h"
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

	if (UMilitarySubsystem* Military = GetWorld()->GetSubsystem<UMilitarySubsystem>())
	{
		Military->OnArmyArrived.AddDynamic(this, &AStrategosMapActor::HandleArmyArrived);
	}
}

void AStrategosMapActor::EndPlay(const EEndPlayReason::Type Reason)
{
	if (UMapSubsystem* Map = GetWorld()->GetSubsystem<UMapSubsystem>())
	{
		Map->OnProvinceSelected.RemoveDynamic(this, &AStrategosMapActor::HandleProvinceSelected);
		Map->OnProvinceHovered.RemoveDynamic(this, &AStrategosMapActor::HandleProvinceHovered);
	}
	if (UMilitarySubsystem* Military = GetWorld()->GetSubsystem<UMilitarySubsystem>())
	{
		Military->OnArmyArrived.RemoveDynamic(this, &AStrategosMapActor::HandleArmyArrived);
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

	// Spawn dos visuais de exército, se classe configurada.
	if (ArmyVisualClass)
	{
		for (const auto& Pair : WorldState->Armies)
		{
			const UArmy* Army = Pair.Value.Get();
			if (!Army) continue;

			FActorSpawnParameters Params;
			Params.Owner = this;
			AStrategosArmyVisualActor* Visual = World->SpawnActor<AStrategosArmyVisualActor>(
				ArmyVisualClass, FVector::ZeroVector, FRotator::ZeroRotator, Params);
			if (!Visual) continue;

			Visual->InitializeFromArmy(Army->Id, Army->OwnerNationId);
			if (const UNation* Owner = WorldState->GetNation(Army->OwnerNationId))
			{
				Visual->SetOwnerColor(Owner->Color);
			}
			if (const AStrategosProvinceVisualActor* Host = ProvinceVisuals.FindRef(Army->CurrentProvinceId))
			{
				Visual->SetActorLocation(Host->GetActorLocation() + FVector(0, 0, 50.f));
			}
			ArmyVisuals.Add(Army->Id, Visual);
		}
	}

	UE_LOG(LogStrategosUI, Log, TEXT("MapActor: spawned %d province visuals, %d army visuals"),
		ProvinceVisuals.Num(), ArmyVisuals.Num());
}

void AStrategosMapActor::HandleArmyArrived(FName ArmyId, FName ProvinceId)
{
	AStrategosArmyVisualActor* ArmyVisual = ArmyVisuals.FindRef(ArmyId);
	const AStrategosProvinceVisualActor* HostVisual = ProvinceVisuals.FindRef(ProvinceId);
	if (ArmyVisual && HostVisual)
	{
		ArmyVisual->SetActorLocation(HostVisual->GetActorLocation() + FVector(0, 0, 50.f));
	}
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
