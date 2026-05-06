#include "Economy/EconomySubsystem.h"
#include "StrategosCore.h"
#include "Economy/EconomyContentRegistry.h"
#include "Economy/GoodAsset.h"
#include "Economy/ProductionMethodAsset.h"
#include "Economy/ProductionModifierAsset.h"
#include "Economy/BuildingTypeAsset.h"
#include "Economy/Building.h"
#include "Foundation/Time/TimeSubsystem.h"
#include "Game/StrategosGameState.h"
#include "World/WorldState.h"
#include "World/Nation.h"
#include "World/Province.h"
#include "Engine/World.h"

void UEconomySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (UTimeSubsystem* Time = ResolveTime())
	{
		Time->OnDayTick.AddDynamic(this, &UEconomySubsystem::HandleDayTick);
		Time->OnMonthTick.AddDynamic(this, &UEconomySubsystem::HandleMonthTick);
	}

	UE_LOG(LogStrategosCore, Log, TEXT("EconomySubsystem initialized."));
}

void UEconomySubsystem::Deinitialize()
{
	if (UTimeSubsystem* Time = ResolveTime())
	{
		Time->OnDayTick.RemoveDynamic(this, &UEconomySubsystem::HandleDayTick);
		Time->OnMonthTick.RemoveDynamic(this, &UEconomySubsystem::HandleMonthTick);
	}
	Super::Deinitialize();
}

bool UEconomySubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

UWorldState* UEconomySubsystem::ResolveWorldState() const
{
	const UWorld* World = GetWorld();
	if (!World) return nullptr;
	AStrategosGameState* GS = World->GetGameState<AStrategosGameState>();
	return GS ? GS->GetWorldState() : nullptr;
}

UTimeSubsystem* UEconomySubsystem::ResolveTime() const
{
	const UWorld* World = GetWorld();
	if (!World) return nullptr;
	const UGameInstance* GI = World->GetGameInstance();
	return GI ? GI->GetSubsystem<UTimeSubsystem>() : nullptr;
}

void UEconomySubsystem::SetContentRegistry(UEconomyContentRegistry* Registry)
{
	ContentRegistry = Registry;
	RebuildContentLookups();
}

void UEconomySubsystem::RebuildContentLookups()
{
	GoodById.Empty();
	MethodById.Empty();
	ModifierById.Empty();
	BuildingTypeById.Empty();

	if (!ContentRegistry)
	{
		UE_LOG(LogStrategosCore, Warning, TEXT("EconomySubsystem: no ContentRegistry; lookups empty."));
		return;
	}

	for (const TSoftObjectPtr<UGoodAsset>& Soft : ContentRegistry->Goods)
	{
		if (UGoodAsset* G = Soft.LoadSynchronous())
		{
			GoodById.Add(G->Id, G);
		}
	}
	for (const TSoftObjectPtr<UProductionMethodAsset>& Soft : ContentRegistry->ProductionMethods)
	{
		if (UProductionMethodAsset* M = Soft.LoadSynchronous())
		{
			MethodById.Add(M->Id, M);
		}
	}
	for (const TSoftObjectPtr<UProductionModifierAsset>& Soft : ContentRegistry->ProductionModifiers)
	{
		if (UProductionModifierAsset* M = Soft.LoadSynchronous())
		{
			ModifierById.Add(M->Id, M);
		}
	}
	for (const TSoftObjectPtr<UBuildingTypeAsset>& Soft : ContentRegistry->BuildingTypes)
	{
		if (UBuildingTypeAsset* B = Soft.LoadSynchronous())
		{
			BuildingTypeById.Add(B->Id, B);
		}
	}

	UE_LOG(LogStrategosCore, Log,
		TEXT("EconomySubsystem content: %d goods, %d methods, %d modifiers, %d building types"),
		GoodById.Num(), MethodById.Num(), ModifierById.Num(), BuildingTypeById.Num());
}

UGoodAsset* UEconomySubsystem::GetGood(FName GoodId) const
{
	const TObjectPtr<UGoodAsset>* P = GoodById.Find(GoodId);
	return P ? P->Get() : nullptr;
}

UProductionMethodAsset* UEconomySubsystem::GetProductionMethod(FName Id) const
{
	const TObjectPtr<UProductionMethodAsset>* P = MethodById.Find(Id);
	return P ? P->Get() : nullptr;
}

UProductionModifierAsset* UEconomySubsystem::GetProductionModifier(FName Id) const
{
	const TObjectPtr<UProductionModifierAsset>* P = ModifierById.Find(Id);
	return P ? P->Get() : nullptr;
}

UBuildingTypeAsset* UEconomySubsystem::GetBuildingType(FName Id) const
{
	const TObjectPtr<UBuildingTypeAsset>* P = BuildingTypeById.Find(Id);
	return P ? P->Get() : nullptr;
}

float UEconomySubsystem::GetDynamicPrice(const UNation* Nation, FName GoodId) const
{
	const UGoodAsset* Good = GetGood(GoodId);
	if (!Good) return 1.0f;

	float Modifier = 1.0f;
	if (Nation)
	{
		const float* D = Nation->Stockpile.Demand.Find(GoodId);
		const float* S = Nation->Stockpile.Supply.Find(GoodId);
		const float Dv = D ? *D : 0.f;
		const float Sv = S ? *S : 0.f;
		if (Dv > KINDA_SMALL_NUMBER)
		{
			Modifier = FMath::Clamp(1.f + (Dv - Sv) / Dv, 0.5f, 2.0f);
		}
	}
	return Good->BasePrice * Modifier;
}

void UEconomySubsystem::HandleDayTick(FDateTime CurrentDate)
{
	UWorldState* World = ResolveWorldState();
	if (!World) return;

	for (auto& Pair : World->Provinces)
	{
		if (UProvince* Prov = Pair.Value.Get())
		{
			TickConstruction(*Prov);
		}
	}
}

void UEconomySubsystem::HandleMonthTick(FDateTime CurrentDate)
{
	UWorldState* World = ResolveWorldState();
	if (!World) return;

	// Itera nações em ordem estável (TMap iteration sobre FName é estável
	// dada a mesma sequência de inserções — garantido pelo bootstrap).
	for (auto& Pair : World->Nations)
	{
		if (UNation* Nation = Pair.Value.Get())
		{
			RunMonthlyTickForNation(*Nation, CurrentDate);
			OnEconomyTickComplete.Broadcast(Nation->Id);
		}
	}
}

void UEconomySubsystem::RunMonthlyTickForNation(UNation& Nation, const FDateTime& CurrentDate)
{
	// Reset contadores mensais antes das fases que escrevem neles.
	Nation.Stockpile.ResetMonthlyCounters();
	Nation.Treasury.Income_Taxes = 0.f;
	Nation.Treasury.Income_Tariffs = 0.f;
	Nation.Treasury.Income_StateProfits = 0.f;
	Nation.Treasury.Expense_Maintenance = 0.f;
	Nation.Treasury.Expense_ArmyUpkeep = 0.f;
	Nation.Treasury.Expense_AdminCost = 0.f;
	Nation.Treasury.Expense_DebtInterest = 0.f;

	Phase_PopGrowth(Nation);
	Phase_AssignEmployment(Nation);
	Phase_RunProduction(Nation);
	Phase_PopConsumption(Nation);
	Phase_PaywagesAndProfits(Nation);
	Phase_CollectTaxes(Nation);
	Phase_PayExpenses(Nation);
	Phase_SettleTreasury(Nation);
	Phase_ComputeStrategicIndices(Nation);
}

void UEconomySubsystem::TickConstruction(UProvince& Province)
{
	for (TObjectPtr<UBuilding>& BPtr : Province.Buildings)
	{
		UBuilding* B = BPtr.Get();
		if (!B || !B->IsUnderConstruction()) continue;

		--B->ConstructionDaysRemaining;
		if (B->ConstructionDaysRemaining == 0)
		{
			UE_LOG(LogStrategosCore, Log, TEXT("Building %s finished in %s"),
				*B->Id.ToString(), *Province.Id.ToString());
			OnBuildingCompleted.Broadcast(B->Id);
		}
	}
}

// Stubs das fases — implementadas nos commits 6-8.
void UEconomySubsystem::Phase_PopGrowth(UNation& Nation) {}
void UEconomySubsystem::Phase_AssignEmployment(UNation& Nation) {}
void UEconomySubsystem::Phase_RunProduction(UNation& Nation) {}
void UEconomySubsystem::Phase_PopConsumption(UNation& Nation) {}
void UEconomySubsystem::Phase_PaywagesAndProfits(UNation& Nation) {}
void UEconomySubsystem::Phase_CollectTaxes(UNation& Nation) {}
void UEconomySubsystem::Phase_PayExpenses(UNation& Nation) {}
void UEconomySubsystem::Phase_SettleTreasury(UNation& Nation) {}
void UEconomySubsystem::Phase_ComputeStrategicIndices(UNation& Nation) {}
