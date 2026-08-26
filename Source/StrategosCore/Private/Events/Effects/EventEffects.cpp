#include "Events/Effects/EventEffects.h"
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
	if (EventId.IsNone()) return;

	// WorldState primeiro: seu outer é o AStrategosGameState (um Actor), então
	// GetWorld() resolve. Effects carregados de um UEventAsset do editor têm o
	// próprio DataAsset como outer, que não tem mundo — a cadeia morreria em
	// silêncio se dependêssemos só do outer.
	UWorld* World = WorldState ? WorldState->GetWorld() : nullptr;
	if (!World)
	{
		if (UObject* Outer = GetOuter())
		{
			World = Outer->GetWorld();
		}
	}
	if (!World) return;

	UEventSubsystem* Events = World->GetSubsystem<UEventSubsystem>();
	if (!Events) return;

	FEventContext NewCtx = Context;
	NewCtx.EventId = EventId;
	NewCtx.SourceNationId = TargetNationId.IsNone() ? Context.SourceNationId : TargetNationId;
	NewCtx.TriggerTag = TEXT("Chain");
	Events->FireEventById(EventId, NewCtx);
}

// ---------------------------------------------------------------------------
// GetDescription — resumos legíveis para o modal de decisão.

FText UEffect_AddGold::GetDescription_Implementation() const
{
	const FString Sign = Amount >= 0.f ? TEXT("+") : TEXT("-");
	return FText::FromString(FString::Printf(TEXT("%s%.0f Ouro"), *Sign, FMath::Abs(Amount)));
}

FText UEffect_AddPopLoyalty::GetDescription_Implementation() const
{
	const FString Sign = Delta >= 0.f ? TEXT("+") : TEXT("-");
	const int32 Pct = FMath::RoundToInt(FMath::Abs(Delta) * 100.f);

	if (bAllStrata)
	{
		return FText::FromString(FString::Printf(TEXT("%s%d%% Lealdade (todos estratos)"), *Sign, Pct));
	}

	FString StratumLabel;
	switch (Stratum)
	{
		case EPopStratum::Laborer:       StratumLabel = TEXT("Trabalhadores");  break;
		case EPopStratum::Artisan:       StratumLabel = TEXT("Artesãos");       break;
		case EPopStratum::FactoryWorker: StratumLabel = TEXT("Operários");      break;
		case EPopStratum::Bourgeoisie:   StratumLabel = TEXT("Burguesia");      break;
		case EPopStratum::Aristocracy:   StratumLabel = TEXT("Aristocracia");   break;
		case EPopStratum::Soldier:       StratumLabel = TEXT("Soldados");       break;
		case EPopStratum::Clergy:        StratumLabel = TEXT("Clero");          break;
		default:                         StratumLabel = TEXT("?");              break;
	}
	return FText::FromString(FString::Printf(TEXT("%s%d%% Lealdade (%s)"), *Sign, Pct, *StratumLabel));
}

FText UEffect_AddGoodsToStockpile::GetDescription_Implementation() const
{
	TArray<FString> Parts;
	for (const FGoodAmount& G : Goods)
	{
		const FString Sign = G.Amount >= 0.f ? TEXT("+") : TEXT("-");
		Parts.Add(FString::Printf(TEXT("%s%.0f %s"), *Sign, FMath::Abs(G.Amount), *G.GoodId.ToString()));
	}
	return Parts.Num() > 0
		? FText::FromString(FString::Join(Parts, TEXT(", ")))
		: FText::GetEmpty();
}

FText UEffect_FireEvent::GetDescription_Implementation() const
{
	return FText::FromString(FString::Printf(TEXT("-> Evento: %s"), *EventId.ToString()));
}
