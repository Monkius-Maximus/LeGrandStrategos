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
	UWorldState* World = ResolveWorldState();

	// Reset contadores mensais antes das fases que escrevem neles.
	Nation.Stockpile.ResetMonthlyCounters();
	Nation.Treasury.Income_Taxes = 0.f;
	Nation.Treasury.Income_Tariffs = 0.f;
	Nation.Treasury.Income_StateProfits = 0.f;
	Nation.Treasury.Expense_Maintenance = 0.f;
	Nation.Treasury.Expense_ArmyUpkeep = 0.f;
	Nation.Treasury.Expense_AdminCost = 0.f;
	Nation.Treasury.Expense_DebtInterest = 0.f;

	if (World)
	{
		for (const FName& ProvId : Nation.OwnedProvinceIds)
		{
			if (UProvince* Prov = World->GetProvince(ProvId))
			{
				for (auto& Pair : Prov->Pops)
				{
					Pair.Value.WageEarnedLastMonth = 0.f;
				}
			}
		}
	}

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

// ----------------------------------------------------------------------------
// Helpers internos.

namespace
{
	struct FAggregatedModifiers
	{
		float Throughput = 1.f;
		float Input = 1.f;
		float Wage = 1.f;
		float Maintenance = 1.f;
		float MonthlyLoyaltyDelta = 0.f;
	};

	FAggregatedModifiers ComputeModifiers(const UBuilding& Building)
	{
		FAggregatedModifiers Acc;
		for (const TSoftObjectPtr<UProductionModifierAsset>& Soft : Building.ActiveProductionModifiers)
		{
			const UProductionModifierAsset* Mod = Soft.LoadSynchronous();
			if (!Mod) continue;
			Acc.Throughput          *= Mod->ThroughputMultiplier;
			Acc.Input               *= Mod->InputCostMultiplier;
			Acc.Wage                *= Mod->WageMultiplier;
			Acc.Maintenance         *= Mod->MaintenanceMultiplier;
			Acc.MonthlyLoyaltyDelta += Mod->MonthlyLoyaltyDelta;
		}
		return Acc;
	}
}

// ----------------------------------------------------------------------------
// Fase 1: PopGrowth (placeholder — crescimento natural lento, sem migração).

void UEconomySubsystem::Phase_PopGrowth(UNation& Nation)
{
	UWorldState* World = ResolveWorldState();
	if (!World) return;

	// 0.1% ao mês (~1.2% ao ano), aplicado igualmente nos estratos ativos.
	constexpr float MonthlyGrowthRate = 0.001f;

	for (const FName& ProvId : Nation.OwnedProvinceIds)
	{
		UProvince* Prov = World->GetProvince(ProvId);
		if (!Prov) continue;
		for (auto& Pair : Prov->Pops)
		{
			FPopGroup& G = Pair.Value;
			const int32 Growth = FMath::FloorToInt(G.Population * MonthlyGrowthRate);
			G.Population += Growth;
		}
	}
}

// ----------------------------------------------------------------------------
// Fase 2: AssignEmployment.
//
// Para cada prédio ativo, percorre EmploymentPerSlot e aloca POPs do estrato
// correspondente. Ordem de iteração estável (Province.Buildings na ordem que
// foram inseridos) garante determinismo. Buildings posteriores no mesmo
// município podem ficar com menos labor — comportamento esperado.

void UEconomySubsystem::Phase_AssignEmployment(UNation& Nation)
{
	UWorldState* World = ResolveWorldState();
	if (!World) return;

	// Reset.
	for (const FName& ProvId : Nation.OwnedProvinceIds)
	{
		UProvince* Prov = World->GetProvince(ProvId);
		if (!Prov) continue;
		for (auto& Pair : Prov->Pops)
		{
			Pair.Value.EmployedThisMonth = 0;
		}
	}

	// Aloca.
	for (const FName& ProvId : Nation.OwnedProvinceIds)
	{
		UProvince* Prov = World->GetProvince(ProvId);
		if (!Prov) continue;

		for (TObjectPtr<UBuilding>& BPtr : Prov->Buildings)
		{
			UBuilding* B = BPtr.Get();
			if (!B || !B->IsActive()) continue;

			UProductionMethodAsset* PM = B->CurrentProductionMethod.LoadSynchronous();
			if (!PM) continue;

			B->LastTickEmployment.Empty();
			for (const FStratumEmployment& Need : PM->EmploymentPerSlot)
			{
				const int32 Required = Need.Headcount * B->Level;
				FPopGroup* Pop = Prov->Pops.Find(Need.Stratum);
				const int32 Available = Pop
					? FMath::Max(0, Pop->Population - Pop->EmployedThisMonth)
					: 0;
				const int32 Effective = FMath::Min(Required, Available);
				if (Pop)
				{
					Pop->EmployedThisMonth += Effective;
				}
				B->LastTickEmployment.Add(Need.Stratum, Effective);
			}
		}
	}
}

// ----------------------------------------------------------------------------
// Fase 3: RunProduction (em ordem estrita de tier 0 -> 3).
//
// Para cada prédio ativo:
//   1. Computa EmploymentRatio (effective / required).
//   2. DesiredScale = Level × EmpRatio × RawPotential × ThroughputMult.
//   3. Verifica disponibilidade de inputs no Stockpile; reduz scale se preciso.
//   4. Consome inputs, produz outputs, registra Demand/Supply para preço
//      dinâmico e índices estratégicos.

void UEconomySubsystem::Phase_RunProduction(UNation& Nation)
{
	UWorldState* World = ResolveWorldState();
	if (!World) return;

	for (int32 Tier = 0; Tier <= 3; ++Tier)
	{
		for (const FName& ProvId : Nation.OwnedProvinceIds)
		{
			UProvince* Prov = World->GetProvince(ProvId);
			if (!Prov) continue;

			for (TObjectPtr<UBuilding>& BPtr : Prov->Buildings)
			{
				UBuilding* B = BPtr.Get();
				if (!B || !B->IsActive()) continue;

				UProductionMethodAsset* PM = B->CurrentProductionMethod.LoadSynchronous();
				if (!PM || PM->ProductionTier != Tier) continue;

				const FAggregatedModifiers Mods = ComputeModifiers(*B);

				// Employment ratio agregado.
				int32 RequiredTotal = 0;
				int32 EffectiveTotal = 0;
				for (const FStratumEmployment& Need : PM->EmploymentPerSlot)
				{
					RequiredTotal += Need.Headcount * B->Level;
					if (const int32* E = B->LastTickEmployment.Find(Need.Stratum))
					{
						EffectiveTotal += *E;
					}
				}
				const float EmpRatio = RequiredTotal > 0
					? static_cast<float>(EffectiveTotal) / static_cast<float>(RequiredTotal)
					: 1.0f;

				// Raw resource potential (mines/farms).
				float RawMult = 1.0f;
				if (PM->bRequiresRawResource)
				{
					const float* P = Prov->RawResourcePotential.Find(PM->RawResourceGoodId);
					RawMult = P ? FMath::Max(0.f, *P) : 0.f;
				}

				const float DesiredScale = static_cast<float>(B->Level) * EmpRatio * RawMult * Mods.Throughput;

				// Clear caches do tick anterior.
				B->LastTickInputs.Empty();
				B->LastTickOutputs.Empty();

				if (DesiredScale <= KINDA_SMALL_NUMBER)
				{
					continue;
				}

				// Demand registrada e gargalo de inputs.
				float RealizedScale = DesiredScale;
				for (const FGoodAmount& In : PM->InputsPerSlot)
				{
					if (In.Amount <= 0.f) continue;
					const float Desired = In.Amount * DesiredScale * Mods.Input;
					Nation.Stockpile.RecordDemand(In.GoodId, Desired);
					const float Available = Nation.Stockpile.GetStock(In.GoodId);
					if (Available < Desired)
					{
						const float MaxAllowed = (In.Amount * Mods.Input) > KINDA_SMALL_NUMBER
							? Available / (In.Amount * Mods.Input)
							: 0.f;
						RealizedScale = FMath::Min(RealizedScale, MaxAllowed);
					}
				}
				RealizedScale = FMath::Max(0.f, RealizedScale);

				// Consome inputs (pelo realizado).
				for (const FGoodAmount& In : PM->InputsPerSlot)
				{
					if (In.Amount <= 0.f) continue;
					const float Used = In.Amount * RealizedScale * Mods.Input;
					Nation.Stockpile.ConsumeUpTo(In.GoodId, Used);
					B->LastTickInputs.Add(In.GoodId, Used);
				}

				// Produz outputs.
				for (const FGoodAmount& Out : PM->OutputsPerSlot)
				{
					if (Out.Amount <= 0.f) continue;
					const float Produced = Out.Amount * RealizedScale;
					Nation.Stockpile.AddStock(Out.GoodId, Produced);
					Nation.Stockpile.RecordSupply(Out.GoodId, Produced);
					B->LastTickOutputs.Add(Out.GoodId, Produced);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------------
// Helper: cesta de consumo por estrato.
// TODO Etapa 3: extrair para UPopConsumptionBasketAsset (DataAsset) para que
// designers e modders ajustem sem recompilar.

namespace
{
	struct FBasketEntry
	{
		FName GoodId;
		float AmountPerPop;
	};

	const TArray<FBasketEntry>& GetConsumptionBasket(EPopStratum Stratum)
	{
		static const TArray<FBasketEntry> Empty;
		static const TArray<FBasketEntry> Laborer        = { { TEXT("Bread"), 1.0f } };
		static const TArray<FBasketEntry> Artisan        = { { TEXT("Bread"), 1.0f }, { TEXT("Garments"), 0.3f } };
		static const TArray<FBasketEntry> FactoryWorker  = { { TEXT("Bread"), 1.0f }, { TEXT("Garments"), 0.3f } };
		static const TArray<FBasketEntry> Bourgeoisie    = { { TEXT("Bread"), 1.0f }, { TEXT("Garments"), 0.8f } };

		switch (Stratum)
		{
			case EPopStratum::Laborer:       return Laborer;
			case EPopStratum::Artisan:       return Artisan;
			case EPopStratum::FactoryWorker: return FactoryWorker;
			case EPopStratum::Bourgeoisie:   return Bourgeoisie;
			default:                          return Empty;
		}
	}
}

// ----------------------------------------------------------------------------
// Fase 4: PopConsumption.
//
// Cada estrato tenta consumir sua cesta a partir do stockpile nacional. Cesta
// atendida → +0.5% loyalty/mês. Faltou algum item → -2% loyalty/mês. Demand é
// registrada mesmo quando o consumo é parcial — o preço sobe e o forward
// hook MilitaryReadiness cai.

void UEconomySubsystem::Phase_PopConsumption(UNation& Nation)
{
	UWorldState* World = ResolveWorldState();
	if (!World) return;

	for (const FName& ProvId : Nation.OwnedProvinceIds)
	{
		UProvince* Prov = World->GetProvince(ProvId);
		if (!Prov) continue;

		for (auto& Pair : Prov->Pops)
		{
			FPopGroup& G = Pair.Value;
			if (G.Population <= 0) continue;

			const TArray<FBasketEntry>& Basket = GetConsumptionBasket(G.Stratum);
			if (Basket.Num() == 0) continue;

			bool bAllSatisfied = true;
			for (const FBasketEntry& Item : Basket)
			{
				const float Wanted = Item.AmountPerPop * static_cast<float>(G.Population);
				if (Wanted <= 0.f) continue;

				Nation.Stockpile.RecordDemand(Item.GoodId, Wanted);
				const float Got = Nation.Stockpile.ConsumeUpTo(Item.GoodId, Wanted);
				if (Got < Wanted * 0.95f)
				{
					bAllSatisfied = false;
				}
			}

			G.Loyalty = FMath::Clamp(
				G.Loyalty + (bAllSatisfied ? 0.005f : -0.02f),
				0.f, 1.f);
		}
	}
}

// ----------------------------------------------------------------------------
// Fase 5: PaywagesAndProfits.
//
// Para cada prédio ativo:
//   Revenue   = Σ Output × DynamicPrice
//   InputCost = Σ Input  × DynamicPrice
//   Wages     = Σ Employment × WagePerWorker × WageMult   (creditado em POPs)
//   Maint     = MaintenancePerSlot × Level × MaintMult
//   Profit    = Revenue - InputCost - Wages - Maint
//
// Government → Profit vai para Treasury (decomposto em Income_StateProfits e
//              Expense_Maintenance para o HUD ver breakdown)
// Private    → Profit vai para Wealth da Bourgeoisie em OwnerProvinceId
//              (clamp >= 0; bankruptcy real entra na Etapa 3)

void UEconomySubsystem::Phase_PaywagesAndProfits(UNation& Nation)
{
	UWorldState* World = ResolveWorldState();
	if (!World) return;

	for (const FName& ProvId : Nation.OwnedProvinceIds)
	{
		UProvince* Prov = World->GetProvince(ProvId);
		if (!Prov) continue;

		for (TObjectPtr<UBuilding>& BPtr : Prov->Buildings)
		{
			UBuilding* B = BPtr.Get();
			if (!B || !B->IsActive()) continue;

			UProductionMethodAsset* PM = B->CurrentProductionMethod.LoadSynchronous();
			if (!PM) continue;

			const FAggregatedModifiers Mods = ComputeModifiers(*B);

			float Revenue = 0.f;
			for (const auto& Pair : B->LastTickOutputs)
			{
				Revenue += Pair.Value * GetDynamicPrice(&Nation, Pair.Key);
			}
			float InputCost = 0.f;
			for (const auto& Pair : B->LastTickInputs)
			{
				InputCost += Pair.Value * GetDynamicPrice(&Nation, Pair.Key);
			}

			// Pagar salários (e creditar em POPs).
			float TotalWages = 0.f;
			for (const FStratumEmployment& Need : PM->EmploymentPerSlot)
			{
				const int32* E = B->LastTickEmployment.Find(Need.Stratum);
				const int32 Eff = E ? *E : 0;
				if (Eff <= 0) continue;
				const float Wages = static_cast<float>(Eff) * Need.WagePerWorker * Mods.Wage;
				TotalWages += Wages;
				if (FPopGroup* Pop = Prov->Pops.Find(Need.Stratum))
				{
					Pop->WageEarnedLastMonth += Wages;
					Pop->Wealth += Wages;
				}
			}

			const float Maintenance = PM->MaintenancePerSlot * static_cast<float>(B->Level) * Mods.Maintenance;
			const float Profit = Revenue - InputCost - TotalWages - Maintenance;

			B->LastTickWagesPaid = TotalWages;
			B->LastTickProfit = Profit;

			if (B->OwnerKind == EBuildingOwnerKind::Government)
			{
				// Decompõe para que o HUD mostre o breakdown completo.
				Nation.Treasury.Income_StateProfits += (Revenue - InputCost - TotalWages);
				Nation.Treasury.Expense_Maintenance += Maintenance;
			}
			else
			{
				if (UProvince* OwnerProv = World->GetProvince(B->OwnerProvinceId))
				{
					if (FPopGroup* Bourg = OwnerProv->Pops.Find(EPopStratum::Bourgeoisie))
					{
						Bourg->Wealth = FMath::Max(0.f, Bourg->Wealth + Profit);
					}
				}
			}

			// Aplica MonthlyLoyaltyDelta dos modifiers nos workers da província.
			if (Mods.MonthlyLoyaltyDelta != 0.f)
			{
				const TArray<EPopStratum> Affected = {
					EPopStratum::Laborer, EPopStratum::Artisan, EPopStratum::FactoryWorker
				};
				for (EPopStratum S : Affected)
				{
					if (FPopGroup* Pop = Prov->Pops.Find(S))
					{
						Pop->Loyalty = FMath::Clamp(Pop->Loyalty + Mods.MonthlyLoyaltyDelta, 0.f, 1.f);
					}
				}
			}
		}

	}
}

// ----------------------------------------------------------------------------
// Fases 6-9: stubs — implementadas no commit 8.

void UEconomySubsystem::Phase_CollectTaxes(UNation& Nation) {}
void UEconomySubsystem::Phase_PayExpenses(UNation& Nation) {}
void UEconomySubsystem::Phase_SettleTreasury(UNation& Nation) {}
void UEconomySubsystem::Phase_ComputeStrategicIndices(UNation& Nation) {}
