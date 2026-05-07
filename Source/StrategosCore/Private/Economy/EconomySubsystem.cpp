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
#include "World/Army.h"
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

	RunBourgeoisieAutoInvestment(Nation, CurrentDate);
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
// Helpers de income/expense.

namespace
{
	float GetBaseTaxPerPopPerMonth(EPopStratum Stratum)
	{
		switch (Stratum)
		{
			case EPopStratum::Laborer:       return 0.05f;
			case EPopStratum::Artisan:       return 0.15f;
			case EPopStratum::FactoryWorker: return 0.20f;
			case EPopStratum::Bourgeoisie:   return 1.00f;
			default:                          return 0.f; // estratos stub
		}
	}
}

// ----------------------------------------------------------------------------
// Fase 6: CollectTaxes.
//
// Tax = Σ_strata Σ_pops [Population × BaseTax(stratum) × TaxMultiplier(level)]
// Aplicada loyalty delta correspondente ao nível selecionado.

void UEconomySubsystem::Phase_CollectTaxes(UNation& Nation)
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

			const float Base = GetBaseTaxPerPopPerMonth(G.Stratum);
			if (Base <= 0.f) continue;

			const ETaxLevel* LevelPtr = Nation.Treasury.TaxLevelByStratum.Find(G.Stratum);
			const ETaxLevel Level = LevelPtr ? *LevelPtr : ETaxLevel::Medium;

			const float Collected = static_cast<float>(G.Population) * Base * StrategosTax::Multiplier(Level);
			Nation.Treasury.Income_Taxes += Collected;

			G.Loyalty = FMath::Clamp(G.Loyalty + StrategosTax::LoyaltyDelta(Level), 0.f, 1.f);
		}
	}
}

// ----------------------------------------------------------------------------
// Fase 7: PayExpenses.
//
// ArmyUpkeep : Σ Manpower × 0.001 / mês (balance placeholder)
// AdminCost  : 10 + 0.5 × Province count (overhead administrativo)
// DebtInterest: AnnualInterestRate / 12 × DebtBalance

void UEconomySubsystem::Phase_PayExpenses(UNation& Nation)
{
	UWorldState* World = ResolveWorldState();
	if (!World) return;

	// Army upkeep.
	for (const auto& Pair : World->Armies)
	{
		const UArmy* A = Pair.Value.Get();
		if (!A || A->OwnerNationId != Nation.Id) continue;
		Nation.Treasury.Expense_ArmyUpkeep += static_cast<float>(A->ManpowerCount) * 0.001f;
	}

	// Admin overhead.
	const float ProvinceCount = static_cast<float>(Nation.OwnedProvinceIds.Num());
	Nation.Treasury.Expense_AdminCost += 10.f + 0.5f * ProvinceCount;

	// Juros sobre dívida.
	const float MonthlyRate = Nation.Treasury.AnnualInterestRate / 12.f;
	Nation.Treasury.Expense_DebtInterest += Nation.Treasury.DebtBalance * MonthlyRate;
}

// ----------------------------------------------------------------------------
// Fase 8: SettleTreasury.
//
// Balance += Income - Expense.
// Se Balance < 0: empréstimo automático (DebtBalance += -Balance; Balance = 0).
// Se DebtBalance > 5 × MonthlyIncome: emite OnBankruptcyImminent (sinaliza para
// HUD/IA, não para o jogo — a entidade que vai aplicar consequências é o
// UPoliticsSubsystem na Etapa 3).

void UEconomySubsystem::Phase_SettleTreasury(UNation& Nation)
{
	const float Income = Nation.Treasury.GetMonthlyIncome();
	const float Expense = Nation.Treasury.GetMonthlyExpenses();
	const float Net = Income - Expense;

	Nation.Treasury.Balance += Net;

	if (Nation.Treasury.Balance < 0.f)
	{
		const float LoanTaken = -Nation.Treasury.Balance;
		Nation.Treasury.DebtBalance += LoanTaken;
		Nation.Treasury.Balance = 0.f;

		UE_LOG(LogStrategosCore, Verbose, TEXT("Treasury: %s took loan of %.1f (debt=%.1f)"),
			*Nation.Id.ToString(), LoanTaken, Nation.Treasury.DebtBalance);
	}

	const float DebtCeiling = FMath::Max(50.f, Income * 5.f);
	if (Nation.Treasury.DebtBalance > DebtCeiling)
	{
		OnBankruptcyImminent.Broadcast(Nation.Id);
	}
}

// ----------------------------------------------------------------------------
// Fase 9: ComputeStrategicIndices.
//
// Forward hook: Battle/Politics/Events lerão estes números na Etapa 3 sem
// conhecer detalhes do tick. Convencionalmente 1.0 = neutro, [0.5, 1.5] após
// clamp final. Usa média de supply ratios dos bens-chave para suavizar.

void UEconomySubsystem::Phase_ComputeStrategicIndices(UNation& Nation)
{
	const float ToolsSR    = Nation.Stockpile.GetSupplyRatio(TEXT("Tools"));
	const float IronSR     = Nation.Stockpile.GetSupplyRatio(TEXT("Iron"));
	const float BreadSR    = Nation.Stockpile.GetSupplyRatio(TEXT("Bread"));
	const float GarmentsSR = Nation.Stockpile.GetSupplyRatio(TEXT("Garments"));
	const float CoalSR     = Nation.Stockpile.GetSupplyRatio(TEXT("Coal"));

	Nation.StrategicIndices.MilitaryReadinessIndex   = FMath::Clamp((ToolsSR + IronSR) * 0.5f,    0.5f, 1.5f);
	Nation.StrategicIndices.CivilianMoraleIndex      = FMath::Clamp((BreadSR + GarmentsSR) * 0.5f, 0.5f, 1.5f);
	Nation.StrategicIndices.IndustrialCapacityIndex  = FMath::Clamp(CoalSR,                       0.5f, 1.5f);
}

// ============================================================================
// Player API.
// ============================================================================

UBuilding* UEconomySubsystem::FindBuildingById(FName BuildingId) const
{
	const UWorldState* World = ResolveWorldState();
	if (!World) return nullptr;

	for (const auto& Pair : World->Provinces)
	{
		const UProvince* Prov = Pair.Value.Get();
		if (!Prov) continue;
		for (const TObjectPtr<UBuilding>& BPtr : Prov->Buildings)
		{
			if (UBuilding* B = BPtr.Get())
			{
				if (B->Id == BuildingId)
				{
					return B;
				}
			}
		}
	}
	return nullptr;
}

EBuildResult UEconomySubsystem::BuildBuilding(FName NationId, FName ProvinceId, FName BuildingTypeId, EBuildingOwnerKind OwnerKind)
{
	UWorldState* World = ResolveWorldState();
	if (!World) return EBuildResult::Rejected_NoNation;

	UNation* Nation = World->GetNation(NationId);
	if (!Nation) return EBuildResult::Rejected_NoNation;

	UProvince* Prov = World->GetProvince(ProvinceId);
	if (!Prov) return EBuildResult::Rejected_NoProvince;

	if (Prov->OwnerNationId != NationId) return EBuildResult::Rejected_WrongOwner;

	UBuildingTypeAsset* BT = GetBuildingType(BuildingTypeId);
	if (!BT) return EBuildResult::Rejected_NoBuildingType;

	if (Prov->GetFreeBuildingSlots() <= 0) return EBuildResult::Rejected_NoSlot;

	if (BT->bRequiresRawResource)
	{
		const float* P = Prov->RawResourcePotential.Find(BT->RequiredResourceId);
		if (!P || *P <= 0.f) return EBuildResult::Rejected_NoRawResource;
	}

	// Verifica caixa.
	if (Nation->Treasury.Balance < BT->ConstructionMonetaryCost)
	{
		return EBuildResult::Rejected_InsufficientFunds;
	}

	// Verifica goods (sem subtrair ainda).
	for (const FGoodAmount& Cost : BT->ConstructionCost)
	{
		if (Nation->Stockpile.GetStock(Cost.GoodId) < Cost.Amount)
		{
			return EBuildResult::Rejected_InsufficientGoods;
		}
	}

	// Debita.
	Nation->Treasury.Balance -= BT->ConstructionMonetaryCost;
	for (const FGoodAmount& Cost : BT->ConstructionCost)
	{
		Nation->Stockpile.TryConsume(Cost.GoodId, Cost.Amount);
	}

	// Cria.
	UBuilding* B = NewObject<UBuilding>(Prov);
	B->Id = FName(*FString::Printf(TEXT("%s.%s.%d"),
		*ProvinceId.ToString(), *BuildingTypeId.ToString(), Prov->Buildings.Num()));
	B->BuildingType = BT;
	B->ProvinceId = ProvinceId;
	B->Level = 1;
	B->OwnerKind = OwnerKind;
	B->OwnerProvinceId = (OwnerKind == EBuildingOwnerKind::Private) ? ProvinceId : NAME_None;
	B->CurrentProductionMethod = BT->DefaultMethod;
	B->ConstructionDaysRemaining = BT->ConstructionDays;
	Prov->Buildings.Add(B);

	UE_LOG(LogStrategosCore, Log, TEXT("Build issued: %s (%s) in %s by %s, %d days"),
		*B->Id.ToString(), *UEnum::GetValueAsString(OwnerKind),
		*ProvinceId.ToString(), *NationId.ToString(), BT->ConstructionDays);

	return EBuildResult::Issued;
}

EEconomyActionResult UEconomySubsystem::DemolishBuilding(FName BuildingId)
{
	UWorldState* World = ResolveWorldState();
	if (!World) return EEconomyActionResult::NotFound;

	for (auto& ProvPair : World->Provinces)
	{
		UProvince* Prov = ProvPair.Value.Get();
		if (!Prov) continue;

		for (int32 i = 0; i < Prov->Buildings.Num(); ++i)
		{
			UBuilding* B = Prov->Buildings[i].Get();
			if (!B || B->Id != BuildingId) continue;

			// Refund 50% do custo monetário para a nação dona da prov.
			if (UNation* OwnerNation = World->GetNation(Prov->OwnerNationId))
			{
				if (UBuildingTypeAsset* BT = B->BuildingType.LoadSynchronous())
				{
					const float Refund = BT->ConstructionMonetaryCost * 0.5f * static_cast<float>(B->Level);
					if (B->OwnerKind == EBuildingOwnerKind::Government)
					{
						OwnerNation->Treasury.Balance += Refund;
					}
					else if (UProvince* OwnerProv = World->GetProvince(B->OwnerProvinceId))
					{
						if (FPopGroup* Bourg = OwnerProv->Pops.Find(EPopStratum::Bourgeoisie))
						{
							Bourg->Wealth += Refund;
						}
					}
				}
			}

			Prov->Buildings.RemoveAt(i);
			UE_LOG(LogStrategosCore, Log, TEXT("Demolished %s"), *BuildingId.ToString());
			return EEconomyActionResult::Ok;
		}
	}
	return EEconomyActionResult::NotFound;
}

EEconomyActionResult UEconomySubsystem::UpgradeBuildingLevel(FName BuildingId)
{
	UBuilding* B = FindBuildingById(BuildingId);
	if (!B) return EEconomyActionResult::NotFound;
	if (B->IsUnderConstruction()) return EEconomyActionResult::Invalid;

	UBuildingTypeAsset* BT = B->BuildingType.LoadSynchronous();
	if (!BT) return EEconomyActionResult::Invalid;
	if (B->Level >= BT->MaxLevel) return EEconomyActionResult::Invalid;

	UWorldState* World = ResolveWorldState();
	if (!World) return EEconomyActionResult::NotFound;
	UProvince* Prov = World->GetProvince(B->ProvinceId);
	if (!Prov) return EEconomyActionResult::NotFound;
	UNation* Nation = World->GetNation(Prov->OwnerNationId);
	if (!Nation) return EEconomyActionResult::NotFound;

	if (Nation->Treasury.Balance < BT->ConstructionMonetaryCost) return EEconomyActionResult::NotPermitted;
	for (const FGoodAmount& Cost : BT->ConstructionCost)
	{
		if (Nation->Stockpile.GetStock(Cost.GoodId) < Cost.Amount) return EEconomyActionResult::NotPermitted;
	}

	Nation->Treasury.Balance -= BT->ConstructionMonetaryCost;
	for (const FGoodAmount& Cost : BT->ConstructionCost)
	{
		Nation->Stockpile.TryConsume(Cost.GoodId, Cost.Amount);
	}

	++B->Level;
	UE_LOG(LogStrategosCore, Log, TEXT("Upgraded %s to L%d"), *BuildingId.ToString(), B->Level);
	return EEconomyActionResult::Ok;
}

EEconomyActionResult UEconomySubsystem::ChangeProductionMethod(FName BuildingId, FName NewMethodId)
{
	UBuilding* B = FindBuildingById(BuildingId);
	if (!B) return EEconomyActionResult::NotFound;

	UProductionMethodAsset* NewPM = GetProductionMethod(NewMethodId);
	if (!NewPM) return EEconomyActionResult::NotFound;

	// Verifica que o NewPM está entre os AvailableMethods do BuildingType.
	UBuildingTypeAsset* BT = B->BuildingType.LoadSynchronous();
	if (!BT) return EEconomyActionResult::Invalid;

	bool bValid = false;
	for (const TSoftObjectPtr<UProductionMethodAsset>& Soft : BT->AvailableMethods)
	{
		if (Soft.GetUniqueID() == NewPM->GetPrimaryAssetId().PrimaryAssetName ||
		    Soft.LoadSynchronous() == NewPM)
		{
			bValid = true;
			break;
		}
	}
	if (!bValid) return EEconomyActionResult::NotPermitted;

	B->CurrentProductionMethod = NewPM;
	UE_LOG(LogStrategosCore, Log, TEXT("Building %s -> PM %s"),
		*BuildingId.ToString(), *NewMethodId.ToString());
	return EEconomyActionResult::Ok;
}

EEconomyActionResult UEconomySubsystem::ToggleProductionModifier(FName BuildingId, FName ModifierId, bool bActivate)
{
	UBuilding* B = FindBuildingById(BuildingId);
	if (!B) return EEconomyActionResult::NotFound;

	UProductionModifierAsset* Mod = GetProductionModifier(ModifierId);
	if (!Mod) return EEconomyActionResult::NotFound;

	const int32 ExistingIdx = B->ActiveProductionModifiers.IndexOfByPredicate(
		[Mod](const TSoftObjectPtr<UProductionModifierAsset>& Soft)
		{
			return Soft.LoadSynchronous() == Mod;
		});

	if (!bActivate)
	{
		if (ExistingIdx != INDEX_NONE)
		{
			B->ActiveProductionModifiers.RemoveAt(ExistingIdx);
		}
		return EEconomyActionResult::Ok;
	}

	if (ExistingIdx != INDEX_NONE)
	{
		return EEconomyActionResult::Ok; // já ativo
	}

	// Mutex group: remove qualquer outro modifier do mesmo grupo.
	if (!Mod->MutexGroup.IsNone())
	{
		for (int32 i = B->ActiveProductionModifiers.Num() - 1; i >= 0; --i)
		{
			UProductionModifierAsset* Existing = B->ActiveProductionModifiers[i].LoadSynchronous();
			if (Existing && Existing->MutexGroup == Mod->MutexGroup)
			{
				B->ActiveProductionModifiers.RemoveAt(i);
			}
		}
	}

	B->ActiveProductionModifiers.Add(Mod);
	UE_LOG(LogStrategosCore, Log, TEXT("Building %s + modifier %s (mutex=%s)"),
		*BuildingId.ToString(), *ModifierId.ToString(), *Mod->MutexGroup.ToString());
	return EEconomyActionResult::Ok;
}

void UEconomySubsystem::SetTaxLevel(FName NationId, EPopStratum Stratum, ETaxLevel NewLevel)
{
	UWorldState* World = ResolveWorldState();
	if (!World) return;
	UNation* Nation = World->GetNation(NationId);
	if (!Nation) return;
	Nation->Treasury.TaxLevelByStratum.FindOrAdd(Stratum) = NewLevel;
}

// ============================================================================
// Ownership.
// ============================================================================

EEconomyActionResult UEconomySubsystem::Privatize(FName BuildingId)
{
	UBuilding* B = FindBuildingById(BuildingId);
	if (!B) return EEconomyActionResult::NotFound;
	if (B->OwnerKind != EBuildingOwnerKind::Government) return EEconomyActionResult::Invalid;
	if (B->IsUnderConstruction()) return EEconomyActionResult::Invalid;

	UWorldState* World = ResolveWorldState();
	if (!World) return EEconomyActionResult::NotFound;
	UProvince* Prov = World->GetProvince(B->ProvinceId);
	if (!Prov) return EEconomyActionResult::NotFound;
	UNation* Nation = World->GetNation(Prov->OwnerNationId);
	if (!Nation) return EEconomyActionResult::NotFound;

	UBuildingTypeAsset* BT = B->BuildingType.LoadSynchronous();
	if (!BT) return EEconomyActionResult::Invalid;

	// Treasury recebe 0.7× custo × Level; Bourgeoisie absorve a propriedade.
	const float SalePrice = BT->ConstructionMonetaryCost * static_cast<float>(B->Level) * 0.7f;
	Nation->Treasury.Balance += SalePrice;

	if (FPopGroup* Bourg = Prov->Pops.Find(EPopStratum::Bourgeoisie))
	{
		Bourg->Wealth = FMath::Max(0.f, Bourg->Wealth - SalePrice);
	}

	B->OwnerKind = EBuildingOwnerKind::Private;
	B->OwnerProvinceId = Prov->Id;

	UE_LOG(LogStrategosCore, Log, TEXT("Privatized %s for %.1f"), *BuildingId.ToString(), SalePrice);
	return EEconomyActionResult::Ok;
}

EEconomyActionResult UEconomySubsystem::Nationalize(FName BuildingId)
{
	UBuilding* B = FindBuildingById(BuildingId);
	if (!B) return EEconomyActionResult::NotFound;
	if (B->OwnerKind != EBuildingOwnerKind::Private) return EEconomyActionResult::Invalid;
	if (B->IsUnderConstruction()) return EEconomyActionResult::Invalid;

	UWorldState* World = ResolveWorldState();
	if (!World) return EEconomyActionResult::NotFound;
	UProvince* Prov = World->GetProvince(B->ProvinceId);
	if (!Prov) return EEconomyActionResult::NotFound;
	UNation* Nation = World->GetNation(Prov->OwnerNationId);
	if (!Nation) return EEconomyActionResult::NotFound;
	UBuildingTypeAsset* BT = B->BuildingType.LoadSynchronous();
	if (!BT) return EEconomyActionResult::Invalid;

	// Premium de 1.2× custo × Level.
	const float Compensation = BT->ConstructionMonetaryCost * static_cast<float>(B->Level) * 1.2f;
	if (Nation->Treasury.Balance < Compensation)
	{
		return EEconomyActionResult::NotPermitted;
	}

	Nation->Treasury.Balance -= Compensation;

	if (UProvince* OwnerProv = World->GetProvince(B->OwnerProvinceId))
	{
		if (FPopGroup* Bourg = OwnerProv->Pops.Find(EPopStratum::Bourgeoisie))
		{
			Bourg->Wealth += Compensation;
			// Pequeno hit de loyalty pela intervenção.
			Bourg->Loyalty = FMath::Clamp(Bourg->Loyalty - 0.05f, 0.f, 1.f);
		}
	}

	B->OwnerKind = EBuildingOwnerKind::Government;
	B->OwnerProvinceId = NAME_None;

	UE_LOG(LogStrategosCore, Log, TEXT("Nationalized %s for %.1f"), *BuildingId.ToString(), Compensation);
	return EEconomyActionResult::Ok;
}

EBuildResult UEconomySubsystem::SponsorPrivateIndustry(FName NationId, FName ProvinceId, FName BuildingTypeId)
{
	// Treasury banca, mas o prédio sai como Private — atalho conveniente.
	return BuildBuilding(NationId, ProvinceId, BuildingTypeId, EBuildingOwnerKind::Private);
}

// ============================================================================
// Bourgeoisie auto-investment.
// ============================================================================

float UEconomySubsystem::ComputeProfitabilityScore(const UNation& Nation, const UProvince& Province,
	const UBuildingTypeAsset& BT, const UProductionMethodAsset& PM) const
{
	float Revenue = 0.f;
	for (const FGoodAmount& Out : PM.OutputsPerSlot)
	{
		const float Price = GetDynamicPrice(&Nation, Out.GoodId);
		float RawMult = 1.0f;
		if (PM.bRequiresRawResource)
		{
			const float* P = Province.RawResourcePotential.Find(PM.RawResourceGoodId);
			RawMult = P ? *P : 0.f;
		}
		Revenue += Out.Amount * Price * RawMult;
	}

	float InputCost = 0.f;
	for (const FGoodAmount& In : PM.InputsPerSlot)
	{
		InputCost += In.Amount * GetDynamicPrice(&Nation, In.GoodId);
	}

	float Wages = 0.f;
	for (const FStratumEmployment& Emp : PM.EmploymentPerSlot)
	{
		Wages += static_cast<float>(Emp.Headcount) * Emp.WagePerWorker;
	}

	const float Maintenance = PM.MaintenancePerSlot;

	return Revenue - InputCost - Wages - Maintenance;
}

void UEconomySubsystem::RunBourgeoisieAutoInvestment(UNation& Nation, const FDateTime& CurrentDate)
{
	UWorldState* World = ResolveWorldState();
	if (!World) return;

	// Determinístico: seed por nação + ano + mês.
	const uint32 H = HashCombine(GetTypeHash(Nation.Id),
		HashCombine(static_cast<uint32>(CurrentDate.GetYear()),
		            static_cast<uint32>(CurrentDate.GetMonth())));
	FRandomStream Rng(static_cast<int32>(H));

	for (const FName& ProvId : Nation.OwnedProvinceIds)
	{
		UProvince* Prov = World->GetProvince(ProvId);
		if (!Prov) continue;
		if (Prov->GetFreeBuildingSlots() <= 0) continue;

		FPopGroup* Bourg = Prov->Pops.Find(EPopStratum::Bourgeoisie);
		if (!Bourg || Bourg->Population <= 0) continue;

		// Threshold de capital mínimo escala com tamanho da Bourgeoisie.
		const float MinWealth = 200.f;
		if (Bourg->Wealth < MinWealth) continue;

		// Ranqueia tipos de prédio por ProfitabilityScore × DemandSignal.
		FName BestBT = NAME_None;
		float BestScore = -KINDA_SMALL_NUMBER;

		for (const auto& Pair : BuildingTypeById)
		{
			UBuildingTypeAsset* BT = Pair.Value.Get();
			if (!BT) continue;
			if (BT->bRequiresRawResource)
			{
				const float* P = Prov->RawResourcePotential.Find(BT->RequiredResourceId);
				if (!P || *P <= 0.f) continue;
			}

			UProductionMethodAsset* PM = BT->DefaultMethod.LoadSynchronous();
			if (!PM) continue;

			const float Score = ComputeProfitabilityScore(Nation, *Prov, *BT, *PM);
			if (Score > BestScore)
			{
				BestScore = Score;
				BestBT = BT->Id;
			}
		}

		if (BestBT.IsNone() || BestScore <= 0.f) continue;

		UBuildingTypeAsset* PickedBT = GetBuildingType(BestBT);
		if (!PickedBT) continue;

		// Capital necessário: monetário + valor estimado dos goods de construção.
		float GoodsValue = 0.f;
		for (const FGoodAmount& Cost : PickedBT->ConstructionCost)
		{
			GoodsValue += Cost.Amount * GetDynamicPrice(&Nation, Cost.GoodId);
		}
		const float TotalNeeded = PickedBT->ConstructionMonetaryCost + GoodsValue;

		if (Bourg->Wealth < TotalNeeded) continue;

		// Verifica se a nação tem os goods em stock — Bourgeoisie "compra" do
		// stockpile pagando o equivalente em wealth para o Treasury.
		bool bHasGoods = true;
		for (const FGoodAmount& Cost : PickedBT->ConstructionCost)
		{
			if (Nation.Stockpile.GetStock(Cost.GoodId) < Cost.Amount) { bHasGoods = false; break; }
		}
		if (!bHasGoods) continue;

		// Transação.
		Bourg->Wealth -= TotalNeeded;
		Nation.Treasury.Balance += GoodsValue; // a nação fornece os goods e recebe pagamento
		for (const FGoodAmount& Cost : PickedBT->ConstructionCost)
		{
			Nation.Stockpile.TryConsume(Cost.GoodId, Cost.Amount);
		}

		UBuilding* B = NewObject<UBuilding>(Prov);
		B->Id = FName(*FString::Printf(TEXT("%s.%s.priv.%d"),
			*ProvId.ToString(), *BestBT.ToString(), Prov->Buildings.Num()));
		B->BuildingType = PickedBT;
		B->ProvinceId = ProvId;
		B->Level = 1;
		B->OwnerKind = EBuildingOwnerKind::Private;
		B->OwnerProvinceId = ProvId;
		B->CurrentProductionMethod = PickedBT->DefaultMethod;
		B->ConstructionDaysRemaining = PickedBT->ConstructionDays;
		Prov->Buildings.Add(B);

		UE_LOG(LogStrategosCore, Log,
			TEXT("Bourgeoisie of %s invested in %s (score=%.1f, wealth left=%.1f)"),
			*ProvId.ToString(), *BestBT.ToString(), BestScore, Bourg->Wealth);

		// Apenas 1 investimento por mês por província — evita explosão.
		(void)Rng; // seed reservada para tie-break em iterações futuras
	}
}
