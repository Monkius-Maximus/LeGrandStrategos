#include "Events/Effects/EventEffects.h"
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
}

void UEffect_AddGold::Apply_Implementation(UWorldState* WorldState, const FEventContext& Context)
{
	if (!WorldState) return;
	if (UNation* Nation = WorldState->GetNation(ResolveNation(NationId, Context)))
	{
		Nation->Treasury.Balance += Amount;
	}
}

void UEffect_AddPopLoyalty::Apply_Implementation(UWorldState* WorldState, const FEventContext& Context)
{
	if (!WorldState) return;
	UNation* Nation = WorldState->GetNation(ResolveNation(NationId, Context));
	if (!Nation) return;

	for (const FName& ProvId : Nation->OwnedProvinceIds)
	{
		UProvince* Prov = WorldState->GetProvince(ProvId);
		if (!Prov) continue;

		if (bAllStrata)
		{
			for (auto& Pair : Prov->Pops)
			{
				Pair.Value.Loyalty = FMath::Clamp(Pair.Value.Loyalty + Delta, 0.f, 1.f);
			}
		}
		else if (FPopGroup* G = Prov->Pops.Find(Stratum))
		{
			G->Loyalty = FMath::Clamp(G->Loyalty + Delta, 0.f, 1.f);
		}
	}
}

void UEffect_AddGoodsToStockpile::Apply_Implementation(UWorldState* WorldState, const FEventContext& Context)
{
	if (!WorldState) return;
	UNation* Nation = WorldState->GetNation(ResolveNation(NationId, Context));
	if (!Nation) return;

	for (const FGoodAmount& G : Goods)
	{
		Nation->Stockpile.AddStock(G.GoodId, G.Amount);
	}
}

void UEffect_FireEvent::Apply_Implementation(UWorldState* WorldState, const FEventContext& Context)
{
	// Wired no commit que adiciona UEventSubsystem.FireEventById (commit 6).
	// Stub seguro: registra log para detectar configuração premature.
	UE_LOG(LogTemp, Verbose, TEXT("Effect_FireEvent stub for '%s' (target=%s)"),
		*EventId.ToString(), *TargetNationId.ToString());
}
