#include "HUD/StrategosRightOutlinerWidget.h"
#include "StrategosUI.h"
#include "Map/MapSubsystem.h"
#include "Diplomacy/DiplomacySubsystem.h"
#include "Events/EventSubsystem.h"
#include "Events/EventAsset.h"
#include "Economy/Building.h"
#include "Economy/BuildingTypeAsset.h"
#include "World/WorldState.h"
#include "World/Nation.h"
#include "World/Army.h"
#include "World/Province.h"
#include "Game/StrategosGameState.h"
#include "Engine/World.h"

// ── Lifecycle ─────────────────────────────────────────────────────────────────

void UStrategosRightOutlinerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UDiplomacySubsystem* Diplo = ResolveDiplo())
	{
		Diplo->OnRelationChanged.AddDynamic(this, &UStrategosRightOutlinerWidget::HandleRelationChanged);
	}
	if (const UWorld* W = GetWorld())
	{
		if (UEventSubsystem* Events = W->GetSubsystem<UEventSubsystem>())
		{
			Events->OnEventFired.AddDynamic(this, &UStrategosRightOutlinerWidget::HandleEventFired);
		}
	}
}

void UStrategosRightOutlinerWidget::NativeDestruct()
{
	if (UDiplomacySubsystem* Diplo = ResolveDiplo())
	{
		Diplo->OnRelationChanged.RemoveDynamic(this, &UStrategosRightOutlinerWidget::HandleRelationChanged);
	}
	if (const UWorld* W = GetWorld())
	{
		if (UEventSubsystem* Events = W->GetSubsystem<UEventSubsystem>())
		{
			Events->OnEventFired.RemoveDynamic(this, &UStrategosRightOutlinerWidget::HandleEventFired);
		}
	}
	Super::NativeDestruct();
}

// ── Tab ───────────────────────────────────────────────────────────────────────

void UStrategosRightOutlinerWidget::SetActiveTab(EOutlinerTab NewTab)
{
	ActiveTab = NewTab;
}

// ── Helpers ───────────────────────────────────────────────────────────────────

UDiplomacySubsystem* UStrategosRightOutlinerWidget::ResolveDiplo() const
{
	const UWorld* W = GetWorld();
	return W ? W->GetSubsystem<UDiplomacySubsystem>() : nullptr;
}

const UWorldState* UStrategosRightOutlinerWidget::ResolveWorldState() const
{
	const UWorld* W = GetWorld();
	if (!W) return nullptr;
	const AStrategosGameState* GS = W->GetGameState<AStrategosGameState>();
	return GS ? GS->GetWorldState() : nullptr;
}

static const UNation* GetPlayerNation(const UWorldState* WS)
{
	if (!WS) return nullptr;
	return WS->GetNation(WS->PlayerNationId);
}

// ── Armies ────────────────────────────────────────────────────────────────────

TArray<FOutlinerArmyRow> UStrategosRightOutlinerWidget::GetArmyRows() const
{
	TArray<FOutlinerArmyRow> Out;
	const UWorldState* WS = ResolveWorldState();
	const UNation* Player = GetPlayerNation(WS);
	if (!WS || !Player) return Out;

	TArray<TObjectPtr<UArmy>> PlayerArmies;
	for (const auto& Pair : WS->Armies)
	{
		if (Pair.Value && Pair.Value->OwnerNationId == Player->Id)
			PlayerArmies.Add(Pair.Value);
	}
	for (const TObjectPtr<UArmy>& APtr : PlayerArmies)
	{
		const UArmy* A = APtr.Get();
		if (!A) continue;

		FOutlinerArmyRow R;
		R.ArmyId    = A->Id;
		R.ArmyName  = A->DisplayName;
		R.Manpower  = A->ManpowerCount;
		R.bMoving   = A->IsMoving();
		R.MoveDaysLeft = A->MoveDaysRemaining;
		R.NationColor = FLinearColor(Player->Color);

		if (const UProvince* P = WS->GetProvince(A->CurrentProvinceId))
		{
			R.ProvinceName = P->DisplayName;
		}
		Out.Add(R);
	}
	return Out;
}

void UStrategosRightOutlinerWidget::FocusArmy(FName ArmyId)
{
	const UWorldState* WS = ResolveWorldState();
	if (!WS) return;
	const UNation* Player = GetPlayerNation(WS);
	if (!Player) return;

	TArray<TObjectPtr<UArmy>> PlayerArmies;
	for (const auto& Pair : WS->Armies)
	{
		if (Pair.Value && Pair.Value->OwnerNationId == Player->Id)
			PlayerArmies.Add(Pair.Value);
	}
	for (const TObjectPtr<UArmy>& APtr : PlayerArmies)
	{
		if (APtr && APtr->Id == ArmyId)
		{
			if (UMapSubsystem* Map = GetWorld()->GetSubsystem<UMapSubsystem>())
			{
				Map->SelectProvince(APtr->CurrentProvinceId);
			}
			return;
		}
	}
}

// ── Diplomacy ────────────────────────────────────────────────────────────────

TArray<FOutlinerDiploRow> UStrategosRightOutlinerWidget::GetDiploRows() const
{
	TArray<FOutlinerDiploRow> Out;
	const UWorldState* WS = ResolveWorldState();
	const UNation* Player = GetPlayerNation(WS);
	const UDiplomacySubsystem* Diplo = ResolveDiplo();
	if (!WS || !Player || !Diplo) return Out;

	for (const FName& Other : Diplo->GetKnownCounterparts(Player->Id))
	{
		const FDiplomaticRelation Rel = Diplo->GetRelation(Player->Id, Other);
		FOutlinerDiploRow R;
		R.CounterpartId   = Other;
		R.Status          = Rel.Status;
		R.Opinion         = Rel.Opinion;
		if (const UNation* N = WS->GetNation(Other))
		{
			R.CounterpartName = N->DisplayName;
			R.NationColor     = FLinearColor(N->Color);
		}
		Out.Add(R);
	}
	return Out;
}

// ── Research ──────────────────────────────────────────────────────────────────

TArray<FOutlinerResearchRow> UStrategosRightOutlinerWidget::GetResearchRows() const
{
	// Placeholder — research subsystem is a future iteration.
	return {};
}

// ── Construction ─────────────────────────────────────────────────────────────

TArray<FOutlinerConstructionRow> UStrategosRightOutlinerWidget::GetConstructionRows() const
{
	TArray<FOutlinerConstructionRow> Out;
	const UWorldState* WS = ResolveWorldState();
	const UNation* Player = GetPlayerNation(WS);
	if (!WS || !Player) return Out;

	for (const FName& ProvId : Player->OwnedProvinceIds)
	{
		const UProvince* Prov = WS->GetProvince(ProvId);
		if (!Prov) continue;
		for (const TObjectPtr<UBuilding>& BPtr : Prov->Buildings)
		{
			const UBuilding* B = BPtr.Get();
			if (!B || !B->IsUnderConstruction()) continue;

			FOutlinerConstructionRow R;
			R.BuildingId = B->Id;
			R.DaysLeft   = B->ConstructionDaysRemaining;
			R.ProvinceName = Prov->DisplayName;
			// Estimate progress — ConstructionDaysTotal not stored; use remaining vs assumed 365
		const int32 TotalDays = FMath::Max(B->ConstructionDaysRemaining, 1);
		R.Progress = 0.f; // accurate value requires total; shows indeterminate bar

			if (UBuildingTypeAsset* BT = B->BuildingType.LoadSynchronous())
			{
				R.BuildingTypeName = BT->DisplayName;
			}
			Out.Add(R);
		}
	}
	return Out;
}

// ── Diary ─────────────────────────────────────────────────────────────────────

TArray<FOutlinerNotificationRow> UStrategosRightOutlinerWidget::GetNotificationRows() const
{
	return NotificationLog;
}

int32 UStrategosRightOutlinerWidget::GetUnreadNotificationCount() const
{
	return UnreadCount;
}

void UStrategosRightOutlinerWidget::MarkAllNotificationsRead()
{
	UnreadCount = 0;
}

// ── Delegates ────────────────────────────────────────────────────────────────

void UStrategosRightOutlinerWidget::HandleRelationChanged(const FDiplomaticRelation& Relation)
{
	const UWorldState* WS = ResolveWorldState();
	const UNation* Player = GetPlayerNation(WS);
	if (!Player) return;

	const FName Other = (Relation.Pair.A == Player->Id) ? Relation.Pair.B : Relation.Pair.A;
	if (!Other.IsNone())
	{
		OnRelationChanged(Other);
	}
}

void UStrategosRightOutlinerWidget::HandleEventFired(const FEventContext& Context)
{
	const UWorldState* WS = ResolveWorldState();
	const UNation* Player = GetPlayerNation(WS);
	if (!Player || Context.SourceNationId != Player->Id) return;

	const UWorld* W = GetWorld();
	if (!W) return;
	const UEventSubsystem* Events = W->GetSubsystem<UEventSubsystem>();
	if (!Events) return;

	UEventAsset* Asset = Events->GetEventById(Context.EventId);
	if (!Asset) return;

	FOutlinerNotificationRow Row;
	Row.EventId  = Asset->Id;
	Row.Title    = Asset->Title;
	Row.Body     = Asset->Description;
	Row.Category = NAME_None; // to be set by event metadata in future

	NotificationLog.Insert(Row, 0);
	if (NotificationLog.Num() > 50) NotificationLog.SetNum(50);
	++UnreadCount;
	OnNewNotification(Row);
}
