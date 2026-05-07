#include "Events/EventSubsystem.h"
#include "Events/EventAsset.h"
#include "Events/EventContentRegistry.h"
#include "Events/EventCondition.h"
#include "Events/EventEffect.h"
#include "Events/EventChoice.h"
#include "World/WorldState.h"
#include "World/Nation.h"
#include "World/Province.h"
#include "Game/StrategosGameState.h"
#include "Foundation/Time/TimeSubsystem.h"
#include "Strategy/MilitarySubsystem.h"
#include "Economy/EconomySubsystem.h"
#include "StrategosCore.h"
#include "Engine/World.h"

namespace EventTriggers
{
	const FName TimeMonth                = TEXT("Time.Month");
	const FName TimeYear                 = TEXT("Time.Year");
	const FName MilitaryArmyArrived      = TEXT("Military.ArmyArrived");
	const FName EconomyBuildingCompleted = TEXT("Economy.BuildingCompleted");
	const FName EconomyBankruptcyImminent = TEXT("Economy.BankruptcyImminent");
}

void UEventSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	SubscribeToTriggers();
	RebuildIndex();
	UE_LOG(LogStrategosCore, Log, TEXT("EventSubsystem initialized."));
}

void UEventSubsystem::Deinitialize()
{
	UnsubscribeFromTriggers();
	EventById.Empty();
	EventsByTrigger.Empty();
	FallbackEvents.Empty();
	Super::Deinitialize();
}

bool UEventSubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

UWorldState* UEventSubsystem::ResolveWorldState() const
{
	const UWorld* World = GetWorld();
	if (!World) return nullptr;
	AStrategosGameState* GS = World->GetGameState<AStrategosGameState>();
	return GS ? GS->GetWorldState() : nullptr;
}

UTimeSubsystem* UEventSubsystem::ResolveTime() const
{
	const UWorld* World = GetWorld();
	if (!World) return nullptr;
	const UGameInstance* GI = World->GetGameInstance();
	return GI ? GI->GetSubsystem<UTimeSubsystem>() : nullptr;
}

UMilitarySubsystem* UEventSubsystem::ResolveMilitary() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetSubsystem<UMilitarySubsystem>() : nullptr;
}

UEconomySubsystem* UEventSubsystem::ResolveEconomy() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetSubsystem<UEconomySubsystem>() : nullptr;
}

void UEventSubsystem::SubscribeToTriggers()
{
	if (UTimeSubsystem* Time = ResolveTime())
	{
		Time->OnMonthTick.AddDynamic(this, &UEventSubsystem::HandleMonthTick);
		Time->OnYearTick.AddDynamic(this, &UEventSubsystem::HandleYearTick);
	}
	if (UMilitarySubsystem* Military = ResolveMilitary())
	{
		Military->OnArmyArrived.AddDynamic(this, &UEventSubsystem::HandleArmyArrived);
	}
	if (UEconomySubsystem* Economy = ResolveEconomy())
	{
		Economy->OnBuildingCompleted.AddDynamic(this, &UEventSubsystem::HandleBuildingCompleted);
		Economy->OnBankruptcyImminent.AddDynamic(this, &UEventSubsystem::HandleBankruptcyImminent);
	}
}

void UEventSubsystem::UnsubscribeFromTriggers()
{
	if (UTimeSubsystem* Time = ResolveTime())
	{
		Time->OnMonthTick.RemoveDynamic(this, &UEventSubsystem::HandleMonthTick);
		Time->OnYearTick.RemoveDynamic(this, &UEventSubsystem::HandleYearTick);
	}
	if (UMilitarySubsystem* Military = ResolveMilitary())
	{
		Military->OnArmyArrived.RemoveDynamic(this, &UEventSubsystem::HandleArmyArrived);
	}
	if (UEconomySubsystem* Economy = ResolveEconomy())
	{
		Economy->OnBuildingCompleted.RemoveDynamic(this, &UEventSubsystem::HandleBuildingCompleted);
		Economy->OnBankruptcyImminent.RemoveDynamic(this, &UEventSubsystem::HandleBankruptcyImminent);
	}
}

void UEventSubsystem::SetContentRegistry(UEventContentRegistry* Registry)
{
	ContentRegistry = Registry;
	RebuildIndex();
}

void UEventSubsystem::RebuildIndex()
{
	EventById.Empty();
	EventsByTrigger.Empty();

	auto AddEvent = [this](UEventAsset* E)
	{
		if (!E || E->Id.IsNone()) return;
		EventById.Add(E->Id, E);
		TArray<TObjectPtr<UEventAsset>>& Bucket = EventsByTrigger.FindOrAdd(E->TriggerTag);
		Bucket.Add(E);
	};

	if (ContentRegistry)
	{
		for (const TSoftObjectPtr<UEventAsset>& Soft : ContentRegistry->Events)
		{
			AddEvent(Soft.LoadSynchronous());
		}
	}
	else
	{
		// Fallback hardcoded — preenchido no commit 7.
		RegisterFallbackEvents();
		for (const TObjectPtr<UEventAsset>& E : FallbackEvents)
		{
			AddEvent(E);
		}
	}

	// Ordena cada bucket por Id para determinismo.
	for (auto& Pair : EventsByTrigger)
	{
		Pair.Value.Sort([](const TObjectPtr<UEventAsset>& A, const TObjectPtr<UEventAsset>& B)
		{
			return A && B ? (A->Id.LexicalLess(B->Id)) : false;
		});
	}

	UE_LOG(LogStrategosCore, Log, TEXT("EventSubsystem indexed %d events across %d trigger tags."),
		EventById.Num(), EventsByTrigger.Num());
}

void UEventSubsystem::RegisterFallbackEvents()
{
	// Implementação real no commit 7. Por enquanto vazio.
}

UEventAsset* UEventSubsystem::GetEventById(FName EventId) const
{
	const TObjectPtr<UEventAsset>* P = EventById.Find(EventId);
	return P ? P->Get() : nullptr;
}

void UEventSubsystem::FireEventById(FName EventId, const FEventContext& Context)
{
	UEventAsset* E = GetEventById(EventId);
	if (!E) return;

	UWorldState* World = ResolveWorldState();
	FEventContext Ctx = Context;
	Ctx.EventId = E->Id;

	if (!EvaluateConditions(*E, Ctx)) return;

	if (E->Type == EEventType::Decision)
	{
		// Decision queueing implementado no commit 6.
		OnDecisionEnqueued.Broadcast(Ctx);
	}
	else
	{
		ApplyAutoEffects(*E, Ctx);
		OnEventFired.Broadcast(Ctx);
	}
}

bool UEventSubsystem::EvaluateConditions(const UEventAsset& Event, const FEventContext& Context) const
{
	UWorldState* World = ResolveWorldState();
	for (const TObjectPtr<UEventCondition>& C : Event.Conditions)
	{
		UEventCondition* Cond = C.Get();
		if (!Cond) continue;
		if (!Cond->Evaluate(World, Context))
		{
			return false;
		}
	}
	return true;
}

bool UEventSubsystem::RollMTTH(FName EventId, const FEventContext& Context, int32 MTTHMonths) const
{
	if (MTTHMonths <= 0) return true;

	const uint32 H1 = GetTypeHash(Context.SourceNationId);
	const uint32 H2 = HashCombine(GetTypeHash(EventId), static_cast<uint32>(Context.FireDate.GetTicks()));
	FRandomStream Stream(static_cast<int32>(HashCombine(H1, H2)));
	return Stream.FRand() < (1.0f / static_cast<float>(MTTHMonths));
}

void UEventSubsystem::ApplyAutoEffects(const UEventAsset& Event, const FEventContext& Context)
{
	UWorldState* World = ResolveWorldState();
	for (const TObjectPtr<UEventEffect>& EffPtr : Event.AutoEffects)
	{
		if (UEventEffect* Eff = EffPtr.Get())
		{
			Eff->Apply(World, Context);
		}
	}
}

void UEventSubsystem::DispatchTrigger(FName TriggerTag, const FEventContext& BaseContext)
{
	const TArray<TObjectPtr<UEventAsset>>* Bucket = EventsByTrigger.Find(TriggerTag);
	if (!Bucket) return;

	for (const TObjectPtr<UEventAsset>& EPtr : *Bucket)
	{
		UEventAsset* E = EPtr.Get();
		if (!E) continue;

		FEventContext Ctx = BaseContext;
		Ctx.EventId = E->Id;
		Ctx.TriggerTag = TriggerTag;

		if (!EvaluateConditions(*E, Ctx)) continue;
		if (!RollMTTH(E->Id, Ctx, E->MeanTimeToHappenMonths)) continue;

		if (E->Type == EEventType::Decision)
		{
			OnDecisionEnqueued.Broadcast(Ctx);
		}
		else
		{
			ApplyAutoEffects(*E, Ctx);
			OnEventFired.Broadcast(Ctx);
			UE_LOG(LogStrategosCore, Verbose, TEXT("Event fired: %s on %s"),
				*E->Id.ToString(), *Ctx.SourceNationId.ToString());
		}
	}
}

// ----------------------------------------------------------------------------
// Trigger handlers — montam contexto e dispatch.

void UEventSubsystem::HandleMonthTick(FDateTime CurrentDate)
{
	UWorldState* World = ResolveWorldState();
	if (!World) return;

	for (const auto& Pair : World->Nations)
	{
		const UNation* Nation = Pair.Value.Get();
		if (!Nation) continue;
		FEventContext Ctx;
		Ctx.SourceNationId = Nation->Id;
		Ctx.FireDate = CurrentDate;
		DispatchTrigger(EventTriggers::TimeMonth, Ctx);
	}
}

void UEventSubsystem::HandleYearTick(FDateTime CurrentDate)
{
	UWorldState* World = ResolveWorldState();
	if (!World) return;

	for (const auto& Pair : World->Nations)
	{
		const UNation* Nation = Pair.Value.Get();
		if (!Nation) continue;
		FEventContext Ctx;
		Ctx.SourceNationId = Nation->Id;
		Ctx.FireDate = CurrentDate;
		DispatchTrigger(EventTriggers::TimeYear, Ctx);
	}
}

void UEventSubsystem::HandleArmyArrived(FName ArmyId, FName ProvinceId)
{
	UWorldState* World = ResolveWorldState();
	if (!World) return;

	const UProvince* Prov = World->GetProvince(ProvinceId);
	FEventContext Ctx;
	Ctx.SourceNationId = Prov ? Prov->OwnerNationId : NAME_None;
	Ctx.SourceProvinceId = ProvinceId;
	Ctx.SourceEntityId = ArmyId;
	if (UTimeSubsystem* Time = ResolveTime()) Ctx.FireDate = Time->GetCurrentDate();
	DispatchTrigger(EventTriggers::MilitaryArmyArrived, Ctx);
}

void UEventSubsystem::HandleBuildingCompleted(FName BuildingId)
{
	FEventContext Ctx;
	Ctx.SourceEntityId = BuildingId;
	if (UTimeSubsystem* Time = ResolveTime()) Ctx.FireDate = Time->GetCurrentDate();
	DispatchTrigger(EventTriggers::EconomyBuildingCompleted, Ctx);
}

void UEventSubsystem::HandleBankruptcyImminent(FName NationId)
{
	FEventContext Ctx;
	Ctx.SourceNationId = NationId;
	if (UTimeSubsystem* Time = ResolveTime()) Ctx.FireDate = Time->GetCurrentDate();
	DispatchTrigger(EventTriggers::EconomyBankruptcyImminent, Ctx);
}
