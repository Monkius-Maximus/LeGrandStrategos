#include "HUD/StrategosHUDWidget.h"
#include "StrategosUI.h"
#include "Foundation/Time/TimeSubsystem.h"
#include "Map/MapSubsystem.h"
#include "Save/SaveSubsystem.h"
#include "Economy/EconomySubsystem.h"
#include "Economy/Building.h"
#include "Economy/BuildingTypeAsset.h"
#include "Events/EventSubsystem.h"
#include "Events/EventAsset.h"
#include "World/WorldState.h"
#include "World/Province.h"
#include "World/Nation.h"
#include "Game/StrategosGameState.h"
#include "Engine/World.h"
#include "Algo/Sort.h"

void UStrategosHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UMapSubsystem* Map = ResolveMap())
	{
		Map->OnProvinceSelected.AddDynamic(this, &UStrategosHUDWidget::HandleProvinceSelected);
	}
	if (UEventSubsystem* Events = ResolveEvents())
	{
		Events->OnEventFired.AddDynamic(this, &UStrategosHUDWidget::HandleEventFired);
		Events->OnDecisionEnqueued.AddDynamic(this, &UStrategosHUDWidget::HandleDecisionEnqueued);
	}
}

void UStrategosHUDWidget::NativeDestruct()
{
	if (UMapSubsystem* Map = ResolveMap())
	{
		Map->OnProvinceSelected.RemoveDynamic(this, &UStrategosHUDWidget::HandleProvinceSelected);
	}
	if (UEventSubsystem* Events = ResolveEvents())
	{
		Events->OnEventFired.RemoveDynamic(this, &UStrategosHUDWidget::HandleEventFired);
		Events->OnDecisionEnqueued.RemoveDynamic(this, &UStrategosHUDWidget::HandleDecisionEnqueued);
	}
	Super::NativeDestruct();
}

UEventSubsystem* UStrategosHUDWidget::ResolveEvents() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetSubsystem<UEventSubsystem>() : nullptr;
}

UTimeSubsystem* UStrategosHUDWidget::ResolveTime() const
{
	const UWorld* World = GetWorld();
	if (!World) return nullptr;
	const UGameInstance* GI = World->GetGameInstance();
	return GI ? GI->GetSubsystem<UTimeSubsystem>() : nullptr;
}

UMapSubsystem* UStrategosHUDWidget::ResolveMap() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetSubsystem<UMapSubsystem>() : nullptr;
}

USaveSubsystem* UStrategosHUDWidget::ResolveSave() const
{
	const UWorld* World = GetWorld();
	if (!World) return nullptr;
	const UGameInstance* GI = World->GetGameInstance();
	return GI ? GI->GetSubsystem<USaveSubsystem>() : nullptr;
}

void UStrategosHUDWidget::PauseGame()
{
	if (UTimeSubsystem* Time = ResolveTime())
	{
		Time->Pause();
	}
}

void UStrategosHUDWidget::ResumeGame()
{
	if (UTimeSubsystem* Time = ResolveTime())
	{
		Time->Resume();
	}
}

void UStrategosHUDWidget::SetTimeSpeed(ETimeSpeed NewSpeed)
{
	if (UTimeSubsystem* Time = ResolveTime())
	{
		Time->SetSpeed(NewSpeed);
	}
}

void UStrategosHUDWidget::RequestSave(const FString& SlotName)
{
	if (USaveSubsystem* Save = ResolveSave())
	{
		Save->SaveToSlot(SlotName);
	}
}

void UStrategosHUDWidget::RequestLoad(const FString& SlotName)
{
	if (USaveSubsystem* Save = ResolveSave())
	{
		Save->LoadFromSlot(SlotName);
	}
}

FText UStrategosHUDWidget::GetCurrentDateText() const
{
	if (const UTimeSubsystem* Time = ResolveTime())
	{
		const FDateTime D = Time->GetCurrentDate();
		return FText::FromString(FString::Printf(TEXT("%04d-%02d-%02d"),
			D.GetYear(), D.GetMonth(), D.GetDay()));
	}
	return FText::GetEmpty();
}

ETimeSpeed UStrategosHUDWidget::GetCurrentSpeed() const
{
	if (const UTimeSubsystem* Time = ResolveTime())
	{
		return Time->GetSpeed();
	}
	return ETimeSpeed::Paused;
}

FText UStrategosHUDWidget::GetSelectedProvinceName() const
{
	const UMapSubsystem* Map = ResolveMap();
	if (!Map) return FText::GetEmpty();
	const FName ProvId = Map->GetSelectedProvinceId();

	const UWorld* World = GetWorld();
	if (!World) return FText::GetEmpty();
	AStrategosGameState* GS = World->GetGameState<AStrategosGameState>();
	if (!GS || !GS->GetWorldState()) return FText::GetEmpty();

	if (const UProvince* P = GS->GetWorldState()->GetProvince(ProvId))
	{
		return P->DisplayName;
	}
	return FText::GetEmpty();
}

FText UStrategosHUDWidget::GetSelectedProvinceOwnerName() const
{
	const UMapSubsystem* Map = ResolveMap();
	if (!Map) return FText::GetEmpty();
	const FName ProvId = Map->GetSelectedProvinceId();

	const UWorld* World = GetWorld();
	if (!World) return FText::GetEmpty();
	AStrategosGameState* GS = World->GetGameState<AStrategosGameState>();
	if (!GS || !GS->GetWorldState()) return FText::GetEmpty();

	const UProvince* P = GS->GetWorldState()->GetProvince(ProvId);
	if (!P) return FText::GetEmpty();
	if (const UNation* N = GS->GetWorldState()->GetNation(P->OwnerNationId))
	{
		return N->DisplayName;
	}
	return FText::GetEmpty();
}

void UStrategosHUDWidget::HandleProvinceSelected(FName ProvinceId)
{
	OnSelectionChanged(ProvinceId);
}

// ============================================================================
// Economia.
// ============================================================================

UEconomySubsystem* UStrategosHUDWidget::ResolveEconomy() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetSubsystem<UEconomySubsystem>() : nullptr;
}

UNation* UStrategosHUDWidget::ResolvePlayerNation() const
{
	const UWorld* World = GetWorld();
	if (!World) return nullptr;
	AStrategosGameState* GS = World->GetGameState<AStrategosGameState>();
	if (!GS || !GS->GetWorldState()) return nullptr;
	return GS->GetWorldState()->GetNation(GS->GetWorldState()->PlayerNationId);
}

float UStrategosHUDWidget::GetTreasuryBalance() const
{
	const UNation* N = ResolvePlayerNation();
	return N ? N->Treasury.Balance : 0.f;
}

float UStrategosHUDWidget::GetMonthlyIncome() const
{
	const UNation* N = ResolvePlayerNation();
	return N ? N->Treasury.GetMonthlyIncome() : 0.f;
}

float UStrategosHUDWidget::GetMonthlyExpenses() const
{
	const UNation* N = ResolvePlayerNation();
	return N ? N->Treasury.GetMonthlyExpenses() : 0.f;
}

float UStrategosHUDWidget::GetDebtBalance() const
{
	const UNation* N = ResolvePlayerNation();
	return N ? N->Treasury.DebtBalance : 0.f;
}

float UStrategosHUDWidget::GetMilitaryReadinessIndex() const
{
	const UNation* N = ResolvePlayerNation();
	return N ? N->StrategicIndices.MilitaryReadinessIndex : 1.f;
}

float UStrategosHUDWidget::GetCivilianMoraleIndex() const
{
	const UNation* N = ResolvePlayerNation();
	return N ? N->StrategicIndices.CivilianMoraleIndex : 1.f;
}

float UStrategosHUDWidget::GetIndustrialCapacityIndex() const
{
	const UNation* N = ResolvePlayerNation();
	return N ? N->StrategicIndices.IndustrialCapacityIndex : 1.f;
}

float UStrategosHUDWidget::GetGoodStock(FName GoodId) const
{
	const UNation* N = ResolvePlayerNation();
	return N ? N->Stockpile.GetStock(GoodId) : 0.f;
}

float UStrategosHUDWidget::GetGoodPrice(FName GoodId) const
{
	const UEconomySubsystem* Econ = ResolveEconomy();
	const UNation* N = ResolvePlayerNation();
	return Econ ? Econ->GetDynamicPrice(N, GoodId) : 0.f;
}

TArray<FShortfallEntry> UStrategosHUDWidget::GetTopShortfalls(int32 MaxEntries) const
{
	TArray<FShortfallEntry> Out;
	const UNation* N = ResolvePlayerNation();
	if (!N) return Out;

	for (const auto& Pair : N->Stockpile.Demand)
	{
		const float Demand = Pair.Value;
		const float* SupplyPtr = N->Stockpile.Supply.Find(Pair.Key);
		const float Supply = SupplyPtr ? *SupplyPtr : 0.f;
		const float Diff = Demand - Supply;
		if (Diff <= 0.f) continue;

		FShortfallEntry E;
		E.GoodId = Pair.Key;
		E.Demand = Demand;
		E.Supply = Supply;
		E.ShortfallAmount = Diff;
		Out.Add(E);
	}

	Algo::Sort(Out, [](const FShortfallEntry& A, const FShortfallEntry& B)
	{
		return A.ShortfallAmount > B.ShortfallAmount;
	});

	if (Out.Num() > MaxEntries)
	{
		Out.SetNum(MaxEntries);
	}
	return Out;
}

TArray<FBuildingHUDRow> UStrategosHUDWidget::GetPlayerBuildings() const
{
	TArray<FBuildingHUDRow> Out;
	const UNation* N = ResolvePlayerNation();
	if (!N) return Out;

	const UWorld* World = GetWorld();
	if (!World) return Out;
	AStrategosGameState* GS = World->GetGameState<AStrategosGameState>();
	if (!GS || !GS->GetWorldState()) return Out;
	UWorldState* WS = GS->GetWorldState();

	for (const FName& ProvId : N->OwnedProvinceIds)
	{
		const UProvince* Prov = WS->GetProvince(ProvId);
		if (!Prov) continue;

		for (const TObjectPtr<UBuilding>& BPtr : Prov->Buildings)
		{
			const UBuilding* B = BPtr.Get();
			if (!B) continue;

			FBuildingHUDRow R;
			R.BuildingId = B->Id;
			R.ProvinceId = B->ProvinceId;
			R.Level = B->Level;
			R.bUnderConstruction = B->IsUnderConstruction();
			R.ConstructionDaysRemaining = B->ConstructionDaysRemaining;
			R.LastTickProfit = B->LastTickProfit;
			R.bIsPrivate = (B->OwnerKind == EBuildingOwnerKind::Private);
			if (UBuildingTypeAsset* BT = B->BuildingType.LoadSynchronous())
			{
				R.BuildingTypeName = BT->DisplayName;
			}
			Out.Add(R);
		}
	}
	return Out;
}

// ============================================================================
// Eventos.

bool UStrategosHUDWidget::HasPendingDecisions() const
{
	const UEventSubsystem* Events = ResolveEvents();
	return Events && Events->HasPendingDecisions(NAME_None);
}

int32 UStrategosHUDWidget::GetPendingDecisionCount() const
{
	const UEventSubsystem* Events = ResolveEvents();
	if (!Events) return 0;
	return Events->GetPendingDecisions(NAME_None).Num();
}

bool UStrategosHUDWidget::GetTopPendingDecision(FPendingDecisionHUDRow& OutDecision) const
{
	const UEventSubsystem* Events = ResolveEvents();
	if (!Events) return false;
	const TArray<FPendingDecision> Pending = Events->GetPendingDecisions(NAME_None);
	if (Pending.Num() == 0) return false;

	const FEventContext& Ctx = Pending[0].Context;
	UEventAsset* Asset = Events->GetEventById(Ctx.EventId);
	if (!Asset) return false;

	OutDecision.EventId = Asset->Id;
	OutDecision.Title = Asset->Title;
	OutDecision.Description = Asset->Description;
	for (const FEventChoice& C : Asset->Choices)
	{
		OutDecision.ChoiceLabels.Add(C.Label);
		OutDecision.ChoiceTooltips.Add(C.Tooltip);
	}
	return true;
}

TArray<FPendingDecisionHUDRow> UStrategosHUDWidget::GetAllPendingDecisions() const
{
	TArray<FPendingDecisionHUDRow> Out;
	const UEventSubsystem* Events = ResolveEvents();
	if (!Events) return Out;

	for (const FPendingDecision& P : Events->GetPendingDecisions(NAME_None))
	{
		UEventAsset* Asset = Events->GetEventById(P.Context.EventId);
		if (!Asset) continue;

		FPendingDecisionHUDRow R;
		R.EventId = Asset->Id;
		R.Title = Asset->Title;
		R.Description = Asset->Description;
		for (const FEventChoice& C : Asset->Choices)
		{
			R.ChoiceLabels.Add(C.Label);
			R.ChoiceTooltips.Add(C.Tooltip);
		}
		Out.Add(R);
	}
	return Out;
}

bool UStrategosHUDWidget::ResolvePendingDecision(FName EventId, int32 ChoiceIndex)
{
	UEventSubsystem* Events = ResolveEvents();
	if (!Events) return false;

	const UNation* Nation = ResolvePlayerNation();
	if (!Nation) return false;

	const EDecisionResolveResult R = Events->ResolveDecision(Nation->Id, EventId, ChoiceIndex);
	return R == EDecisionResolveResult::Ok;
}

void UStrategosHUDWidget::HandleEventFired(const FEventContext& Context)
{
	const UNation* Player = ResolvePlayerNation();
	if (!Player || Context.SourceNationId != Player->Id) return;

	const UEventSubsystem* Events = ResolveEvents();
	if (!Events) return;

	if (UEventAsset* Asset = Events->GetEventById(Context.EventId))
	{
		if (Asset->Type == EEventType::Notification)
		{
			OnNotificationFired(Asset->Id, Asset->Title, Asset->Description);
		}
	}
}

void UStrategosHUDWidget::HandleDecisionEnqueued(const FEventContext& Context)
{
	const UNation* Player = ResolvePlayerNation();
	if (!Player || Context.SourceNationId != Player->Id) return;
	OnDecisionEnqueued(Context.EventId);
}
