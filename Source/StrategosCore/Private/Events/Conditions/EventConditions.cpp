#include "Events/Conditions/EventConditions.h"
#include "World/WorldState.h"
#include "World/Nation.h"
#include "World/Province.h"
#include "Economy/PopGroup.h"

namespace
{
	FName ResolveNation(FName Explicit, const FEventContext& Ctx)
	{
		return Explicit.IsNone() ? Ctx.SourceNationId : Explicit;
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
