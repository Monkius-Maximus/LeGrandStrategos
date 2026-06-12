#include "Events/EventSubsystem.h"
#include "Events/EventAsset.h"
#include "Events/EventContentRegistry.h"
#include "Events/EventCondition.h"
#include "Events/EventEffect.h"
#include "Events/EventChoice.h"
#include "Events/Conditions/EventConditions.h"
#include "Events/Effects/EventEffects.h"
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
	FallbackEvents.Empty();

	auto MakeEffect_AddGold = [this](float Amount)
	{
		UEffect_AddGold* E = NewObject<UEffect_AddGold>(this);
		E->Amount = Amount;
		return E;
	};
	auto MakeEffect_AddLoyalty = [this](EPopStratum S, float Delta, bool bAll = false)
	{
		UEffect_AddPopLoyalty* E = NewObject<UEffect_AddPopLoyalty>(this);
		E->Stratum = S;
		E->Delta = Delta;
		E->bAllStrata = bAll;
		return E;
	};
	auto MakeEffect_AddGoods = [this](TArray<FGoodAmount> Items)
	{
		UEffect_AddGoodsToStockpile* E = NewObject<UEffect_AddGoodsToStockpile>(this);
		E->Goods = MoveTemp(Items);
		return E;
	};

	// 1. Bountiful Harvest — Notification anual, MTTH 6
	{
		UEventAsset* E = NewObject<UEventAsset>(this);
		E->Id = TEXT("BountifulHarvest");
		E->Title = NSLOCTEXT("Strategos", "BountifulHarvest_T", "Bountiful Harvest");
		E->Description = NSLOCTEXT("Strategos", "BountifulHarvest_D",
			"Favorable weather brings an exceptional harvest. Granaries swell.");
		E->Type = EEventType::Notification;
		E->Category = TEXT("economic");
		E->TriggerTag = TEXT("Time.Year");
		E->MeanTimeToHappenMonths = 6;
		E->AutoEffects.Add(MakeEffect_AddGoods({ { TEXT("Grain"), 200.f }, { TEXT("Bread"), 100.f } }));
		E->AutoEffects.Add(MakeEffect_AddLoyalty(EPopStratum::Laborer, 0.05f, false));
		FallbackEvents.Add(E);
	}

	// 2. Worker Strike — Notification mensal, gated por LoyaltyBelow(FactoryWorker, 0.6)
	{
		UEventAsset* E = NewObject<UEventAsset>(this);
		E->Id = TEXT("WorkerStrike");
		E->Title = NSLOCTEXT("Strategos", "WorkerStrike_T", "Worker Strike");
		E->Description = NSLOCTEXT("Strategos", "WorkerStrike_D",
			"Factory workers down their tools, demanding better conditions.");
		E->Type = EEventType::Notification;
		E->Category = TEXT("political");
		E->TriggerTag = TEXT("Time.Month");
		E->MeanTimeToHappenMonths = 4;
		UCondition_LoyaltyBelow* C = NewObject<UCondition_LoyaltyBelow>(this);
		C->Stratum = EPopStratum::FactoryWorker;
		C->Threshold = 0.6f;
		E->Conditions.Add(C);
		E->AutoEffects.Add(MakeEffect_AddLoyalty(EPopStratum::FactoryWorker, -0.05f, false));
		E->AutoEffects.Add(MakeEffect_AddGold(-50.f));
		FallbackEvents.Add(E);
	}

	// 3. Foreign Investor — Decision mensal, gated por TreasuryBelow(<10000) + Bourgeoisie loyalty
	{
		UEventAsset* E = NewObject<UEventAsset>(this);
		E->Id = TEXT("ForeignInvestor");
		E->Title = NSLOCTEXT("Strategos", "ForeignInvestor_T", "Foreign Investor Approaches");
		E->Description = NSLOCTEXT("Strategos", "ForeignInvestor_D",
			"A wealthy foreigner offers capital in exchange for trade concessions.");
		E->Type = EEventType::Decision;
		E->Category = TEXT("economic");
		E->TriggerTag = TEXT("Time.Month");
		E->MeanTimeToHappenMonths = 24;

		FEventChoice Accept;
		Accept.Label = NSLOCTEXT("Strategos", "FI_Accept", "Accept the offer (+500g, -loyalty Bourgeoisie)");
		Accept.Effects.Add(MakeEffect_AddGold(500.f));
		Accept.Effects.Add(MakeEffect_AddLoyalty(EPopStratum::Bourgeoisie, -0.10f, false));
		E->Choices.Add(Accept);

		FEventChoice Refuse;
		Refuse.Label = NSLOCTEXT("Strategos", "FI_Refuse", "Refuse (preserve sovereignty)");
		Refuse.Effects.Add(MakeEffect_AddLoyalty(EPopStratum::Bourgeoisie, 0.02f, false));
		E->Choices.Add(Refuse);

		FallbackEvents.Add(E);
	}

	// 4. Festival Petition — Decision mensal, gated por HasGoodInStockpile(Bread, 500)
	{
		UEventAsset* E = NewObject<UEventAsset>(this);
		E->Id = TEXT("FestivalPetition");
		E->Title = NSLOCTEXT("Strategos", "FP_T", "Festival Petition");
		E->Description = NSLOCTEXT("Strategos", "FP_D",
			"With granaries full, citizens petition for a public festival.");
		E->Type = EEventType::Decision;
		E->Category = TEXT("political");
		E->TriggerTag = TEXT("Time.Month");
		E->MeanTimeToHappenMonths = 12;

		UCondition_HasGoodInStockpile* C = NewObject<UCondition_HasGoodInStockpile>(this);
		C->GoodId = TEXT("Bread");
		C->MinAmount = 500.f;
		E->Conditions.Add(C);

		FEventChoice Hold;
		Hold.Label = NSLOCTEXT("Strategos", "FP_Hold", "Hold the festival (-100g, +loyalty all)");
		Hold.Effects.Add(MakeEffect_AddGold(-100.f));
		Hold.Effects.Add(MakeEffect_AddGoods({ { TEXT("Bread"), -100.f } }));
		Hold.Effects.Add(MakeEffect_AddLoyalty(EPopStratum::Laborer, 0.05f, true));
		E->Choices.Add(Hold);

		FEventChoice Skip;
		Skip.Label = NSLOCTEXT("Strategos", "FP_Skip", "Skip (no effect)");
		E->Choices.Add(Skip);

		FallbackEvents.Add(E);
	}

	// 5. Bandit Raid — Notification mensal raro
	{
		UEventAsset* E = NewObject<UEventAsset>(this);
		E->Id = TEXT("BanditRaid");
		E->Title = NSLOCTEXT("Strategos", "BR_T", "Bandit Raid");
		E->Description = NSLOCTEXT("Strategos", "BR_D",
			"Brigands raid a remote province, looting what they can carry.");
		E->Type = EEventType::Notification;
		E->Category = TEXT("military");
		E->TriggerTag = TEXT("Time.Month");
		E->MeanTimeToHappenMonths = 18;
		E->AutoEffects.Add(MakeEffect_AddGold(-30.f));
		E->AutoEffects.Add(MakeEffect_AddGoods({
			{ TEXT("Grain"), -20.f },
			{ TEXT("Iron"), -5.f }
		}));
		FallbackEvents.Add(E);
	}

	UE_LOG(LogStrategosCore, Log, TEXT("EventSubsystem: registered %d fallback events."),
		FallbackEvents.Num());
}

void UEventSubsystem::RegisterEphemeralEvent(UEventAsset* Event)
{
	if (!Event || Event->Id.IsNone()) return;
	EventById.Add(Event->Id, Event);
	TArray<TObjectPtr<UEventAsset>>& Bucket = EventsByTrigger.FindOrAdd(Event->TriggerTag);
	if (!Bucket.Contains(Event))
	{
		Bucket.Add(Event);
	}
	Bucket.Sort([](const TObjectPtr<UEventAsset>& A, const TObjectPtr<UEventAsset>& B)
	{
		return A && B ? A->Id.LexicalLess(B->Id) : false;
	});
	UE_LOG(LogStrategosCore, Log, TEXT("EventSubsystem: evento efêmero '%s' registrado."), *Event->Id.ToString());
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

	FEventContext Ctx = Context;
	Ctx.EventId = E->Id;

	if (!EvaluateConditions(*E, Ctx)) return;

	if (E->Type == EEventType::Decision)
	{
		EnqueueOrAutoResolve(*E, Ctx);
	}
	else
	{
		ApplyAutoEffects(*E, Ctx);
		OnEventFired.Broadcast(Ctx);
	}
}

// ----------------------------------------------------------------------------
// Decision queue + resolve.

namespace
{
	FName ResolveQueryNation(FName NationId, const UWorldState* World)
	{
		if (!NationId.IsNone()) return NationId;
		return World ? World->PlayerNationId : NAME_None;
	}
}

TArray<FPendingDecision> UEventSubsystem::GetPendingDecisions(FName NationId) const
{
	const FName Target = ResolveQueryNation(NationId, ResolveWorldState());
	if (const TArray<FPendingDecision>* P = PendingByNation.Find(Target))
	{
		return *P;
	}
	return {};
}

bool UEventSubsystem::HasPendingDecisions(FName NationId) const
{
	const FName Target = ResolveQueryNation(NationId, ResolveWorldState());
	const TArray<FPendingDecision>* P = PendingByNation.Find(Target);
	return P && P->Num() > 0;
}

EDecisionResolveResult UEventSubsystem::ResolveDecision(FName NationId, FName EventId, int32 ChoiceIndex)
{
	TArray<FPendingDecision>* Pending = PendingByNation.Find(NationId);
	if (!Pending) return EDecisionResolveResult::NoSuchDecision;

	const int32 Idx = Pending->IndexOfByPredicate([&](const FPendingDecision& P)
	{
		return P.Context.EventId == EventId;
	});
	if (Idx == INDEX_NONE) return EDecisionResolveResult::NoSuchDecision;

	UEventAsset* Event = GetEventById(EventId);
	if (!Event) return EDecisionResolveResult::NoSuchDecision;

	if (!Event->Choices.IsValidIndex(ChoiceIndex))
	{
		return EDecisionResolveResult::InvalidChoice;
	}

	const FEventContext Ctx = (*Pending)[Idx].Context;
	Pending->RemoveAt(Idx);

	ApplyChoiceEffects(Event->Choices[ChoiceIndex], Ctx);
	OnDecisionResolved.Broadcast(Ctx, ChoiceIndex);

	UE_LOG(LogStrategosCore, Log, TEXT("Decision %s resolved by %s with choice %d"),
		*EventId.ToString(), *NationId.ToString(), ChoiceIndex);
	return EDecisionResolveResult::Ok;
}

void UEventSubsystem::ApplyChoiceEffects(const FEventChoice& Choice, const FEventContext& Context)
{
	UWorldState* World = ResolveWorldState();
	for (const TObjectPtr<UEventEffect>& EffPtr : Choice.Effects)
	{
		if (UEventEffect* Eff = EffPtr.Get())
		{
			Eff->Apply(World, Context);
		}
	}
}

void UEventSubsystem::EnqueueOrAutoResolve(UEventAsset& Event, const FEventContext& Context)
{
	UWorldState* World = ResolveWorldState();
	if (!World) return;

	UNation* Nation = World->GetNation(Context.SourceNationId);
	const bool bIsPlayer = Nation && Nation->bIsPlayerControlled;

	if (Event.Choices.Num() == 0)
	{
		// Decision sem choices é mal-formada; trata como Notification.
		ApplyAutoEffects(Event, Context);
		OnEventFired.Broadcast(Context);
		return;
	}

	if (bIsPlayer)
	{
		FPendingDecision P;
		P.Context = Context;
		PendingByNation.FindOrAdd(Context.SourceNationId).Add(P);
		OnDecisionEnqueued.Broadcast(Context);
		UE_LOG(LogStrategosCore, Verbose, TEXT("Decision %s queued for player nation %s"),
			*Event.Id.ToString(), *Context.SourceNationId.ToString());
	}
	else
	{
		const int32 Pick = PickAIChoice(Event, Context);
		ApplyChoiceEffects(Event.Choices[Pick], Context);
		OnDecisionResolved.Broadcast(Context, Pick);
	}
}

int32 UEventSubsystem::PickAIChoice(const UEventAsset& Event, const FEventContext& Context) const
{
	// Determinístico: hash de (NationId, EventId, Date) modulo NumChoices.
	const uint32 H1 = GetTypeHash(Context.SourceNationId);
	const uint32 H2 = HashCombine(GetTypeHash(Event.Id), static_cast<uint32>(Context.FireDate.GetTicks()));
	return static_cast<int32>(HashCombine(H1, H2)) % FMath::Max(1, Event.Choices.Num());
}

void UEventSubsystem::RestorePendingDecisions(const TMap<FName, TArray<FPendingDecision>>& Pending)
{
	PendingByNation = Pending;
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
			EnqueueOrAutoResolve(*E, Ctx);
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
