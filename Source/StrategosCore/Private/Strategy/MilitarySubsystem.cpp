#include "Strategy/MilitarySubsystem.h"
#include "StrategosCore.h"
#include "World/WorldState.h"
#include "World/Province.h"
#include "World/Army.h"
#include "Foundation/Time/TimeSubsystem.h"
#include "Game/StrategosGameState.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"

void UMilitarySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (UTimeSubsystem* Time = ResolveTime())
	{
		Time->OnDayTick.AddDynamic(this, &UMilitarySubsystem::HandleDayTick);
	}

	UE_LOG(LogStrategosCore, Log, TEXT("MilitarySubsystem initialized."));
}

void UMilitarySubsystem::Deinitialize()
{
	if (UTimeSubsystem* Time = ResolveTime())
	{
		Time->OnDayTick.RemoveDynamic(this, &UMilitarySubsystem::HandleDayTick);
	}
	Super::Deinitialize();
}

bool UMilitarySubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

UWorldState* UMilitarySubsystem::ResolveWorldState() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	AStrategosGameState* GS = World->GetGameState<AStrategosGameState>();
	return GS ? GS->GetWorldState() : nullptr;
}

UTimeSubsystem* UMilitarySubsystem::ResolveTime() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	const UGameInstance* GI = World->GetGameInstance();
	return GI ? GI->GetSubsystem<UTimeSubsystem>() : nullptr;
}

int32 UMilitarySubsystem::GetMovementCostDays(FName FromProvinceId, FName ToProvinceId) const
{
	const UWorldState* World = ResolveWorldState();
	if (!World)
	{
		return 0;
	}

	const UProvince* From = World->GetProvince(FromProvinceId);
	const UProvince* To = World->GetProvince(ToProvinceId);
	if (!From || !To)
	{
		return 0;
	}

	switch (To->Terrain)
	{
		case ETerrainType::Plains:    return 3;
		case ETerrainType::Coast:     return 3;
		case ETerrainType::Forest:    return 5;
		case ETerrainType::Hills:     return 5;
		case ETerrainType::Desert:    return 6;
		case ETerrainType::Tundra:    return 6;
		case ETerrainType::Marsh:     return 7;
		case ETerrainType::Mountains: return 8;
		case ETerrainType::Water:     return 99;
		default:                       return 5;
	}
}

EMoveOrderResult UMilitarySubsystem::IssueMoveOrder(FName ArmyId, FName TargetProvinceId)
{
	UWorldState* World = ResolveWorldState();
	if (!World)
	{
		return EMoveOrderResult::Rejected_NoArmy;
	}

	UArmy* Army = World->GetArmy(ArmyId);
	if (!Army)
	{
		return EMoveOrderResult::Rejected_NoArmy;
	}

	if (Army->CurrentProvinceId == TargetProvinceId)
	{
		return EMoveOrderResult::Rejected_AlreadyThere;
	}

	UProvince* CurrentProv = World->GetProvince(Army->CurrentProvinceId);
	UProvince* TargetProv = World->GetProvince(TargetProvinceId);
	if (!CurrentProv || !TargetProv)
	{
		return EMoveOrderResult::Rejected_NoTarget;
	}

	if (!CurrentProv->IsAdjacentTo(TargetProvinceId))
	{
		return EMoveOrderResult::Rejected_NotAdjacent;
	}

	Army->MoveTargetProvinceId = TargetProvinceId;
	Army->MoveDaysRemaining = GetMovementCostDays(Army->CurrentProvinceId, TargetProvinceId);

	UE_LOG(LogStrategosCore, Log, TEXT("Military: %s -> %s (%d days)"),
		*ArmyId.ToString(), *TargetProvinceId.ToString(), Army->MoveDaysRemaining);

	OnArmyMoveIssued.Broadcast(ArmyId);
	return EMoveOrderResult::Issued;
}

void UMilitarySubsystem::CancelMoveOrder(FName ArmyId)
{
	UWorldState* World = ResolveWorldState();
	if (!World)
	{
		return;
	}

	if (UArmy* Army = World->GetArmy(ArmyId))
	{
		Army->MoveTargetProvinceId = NAME_None;
		Army->MoveDaysRemaining = 0;
	}
}

void UMilitarySubsystem::HandleDayTick(FDateTime CurrentDate)
{
	UWorldState* World = ResolveWorldState();
	if (!World)
	{
		return;
	}

	for (auto& Pair : World->Armies)
	{
		if (UArmy* Army = Pair.Value.Get())
		{
			TickArmyMovement(*Army);
		}
	}
}

void UMilitarySubsystem::TickArmyMovement(UArmy& Army)
{
	if (!Army.IsMoving())
	{
		return;
	}

	--Army.MoveDaysRemaining;
	if (Army.MoveDaysRemaining > 0)
	{
		return;
	}

	const FName ArrivedAt = Army.MoveTargetProvinceId;
	Army.CurrentProvinceId = ArrivedAt;
	Army.MoveTargetProvinceId = NAME_None;
	Army.MoveDaysRemaining = 0;

	UE_LOG(LogStrategosCore, Log, TEXT("Military: army %s arrived at %s"),
		*Army.Id.ToString(), *ArrivedAt.ToString());

	OnArmyArrived.Broadcast(Army.Id, ArrivedAt);
}
