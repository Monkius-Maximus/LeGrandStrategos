#include "Events/Conditions/EventConditions.h"
#include "Events/EventSubsystem.h"
#include "World/WorldState.h"
#include "World/Nation.h"
#include "World/Province.h"
#include "Economy/PopGroup.h"
#include "Engine/World.h"

namespace
{
	FName ResolveNation(FName Explicit, const FEventContext& Ctx)
	{
		return Explicit.IsNone() ? Ctx.SourceNationId : Explicit;
	}

	/**
	 * O outer de uma condition carregada de um UEventAsset do editor é o próprio
	 * DataAsset, que não tem mundo. O WorldState tem: seu outer é o
	 * AStrategosGameState, um Actor. Por isso resolvemos por ele.
	 */
	UEventSubsystem* ResolveEvents(const UWorldState* WorldState)
	{
		const UWorld* World = WorldState ? WorldState->GetWorld() : nullptr;
		return World ? World->GetSubsystem<UEventSubsystem>() : nullptr;
	}
}

bool UCondition_TreasuryBelow::Evaluate_Implementation(UWorldState* WorldState, const FEventContext& Context)
{
	if (!WorldState) return false;
	const UNation* Nation = WorldState->GetNation(ResolveNation(NationId, Context));
	if (!Nation) return false;
	return Nation->Treasury.Balance < Threshold;
}

bool UCondition_LoyaltyBelow::Evaluate_Implementation(UWorldState* WorldState, const FEventContext& Context)
{
	if (!WorldState) return false;
	const UNation* Nation = WorldState->GetNation(ResolveNation(NationId, Context));
	if (!Nation) return false;

	for (const FName& ProvId : Nation->OwnedProvinceIds)
	{
		const UProvince* Prov = WorldState->GetProvince(ProvId);
		if (!Prov) continue;
		if (const FPopGroup* G = Prov->Pops.Find(Stratum))
		{
			if (G->Population > 0 && G->Loyalty < Threshold)
			{
				return true;
			}
		}
	}
	return false;
}

bool UCondition_HasGoodInStockpile::Evaluate_Implementation(UWorldState* WorldState, const FEventContext& Context)
{
	if (!WorldState) return false;
	const UNation* Nation = WorldState->GetNation(ResolveNation(NationId, Context));
	if (!Nation) return false;
	return Nation->Stockpile.GetStock(GoodId) >= MinAmount;
}

// ---------------------------------------------------------------------------
// Composição lógica.

bool UCondition_And::Evaluate_Implementation(UWorldState* WorldState, const FEventContext& Context)
{
	for (const TObjectPtr<UEventCondition>& C : SubConditions)
	{
		UEventCondition* Cond = C.Get();
		if (Cond && !Cond->Evaluate(WorldState, Context))
		{
			return false;
		}
	}
	return true;
}

bool UCondition_Or::Evaluate_Implementation(UWorldState* WorldState, const FEventContext& Context)
{
	for (const TObjectPtr<UEventCondition>& C : SubConditions)
	{
		UEventCondition* Cond = C.Get();
		if (Cond && Cond->Evaluate(WorldState, Context))
		{
			return true;
		}
	}
	return false;
}

bool UCondition_Not::Evaluate_Implementation(UWorldState* WorldState, const FEventContext& Context)
{
	UEventCondition* Cond = SubCondition.Get();
	if (!Cond) return false;
	return !Cond->Evaluate(WorldState, Context);
}

// ---------------------------------------------------------------------------
// Consulta ao histórico.

bool UCondition_PreviousEventFired::Evaluate_Implementation(UWorldState* WorldState, const FEventContext& Context)
{
	if (EventId.IsNone()) return false;

	const UEventSubsystem* Events = ResolveEvents(WorldState);
	if (!Events) return false;

	const bool bFired = Events->HasEventEverFired(EventId, ResolveNation(NationId, Context));
	return bInvert ? !bFired : bFired;
}

bool UCondition_PreviousChoiceWas::Evaluate_Implementation(UWorldState* WorldState, const FEventContext& Context)
{
	if (EventId.IsNone()) return false;

	const UEventSubsystem* Events = ResolveEvents(WorldState);
	if (!Events) return false;

	return Events->GetLastChoiceFor(EventId, ResolveNation(NationId, Context)) == ChoiceIndex;
}
