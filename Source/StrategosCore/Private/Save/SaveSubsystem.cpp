#include "Save/SaveSubsystem.h"
#include "StrategosCore.h"
#include "Save/StrategosSaveData.h"
#include "World/WorldState.h"
#include "World/Nation.h"
#include "World/Province.h"
#include "World/Army.h"
#include "World/UnitTypeAsset.h"
#include "Economy/Building.h"
#include "Economy/BuildingTypeAsset.h"
#include "Economy/ProductionMethodAsset.h"
#include "Economy/ProductionModifierAsset.h"
#include "Economy/EconomySubsystem.h"
#include "Events/EventSubsystem.h"
#include "Diplomacy/DiplomacySubsystem.h"
#include "Foundation/Time/TimeSubsystem.h"
#include "Game/StrategosGameState.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void USaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogStrategosCore, Log, TEXT("SaveSubsystem initialized."));
}

void USaveSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

UWorldState* USaveSubsystem::ResolveWorldState() const
{
	const UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return nullptr;
	}

	const UWorld* World = GI->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	AStrategosGameState* GS = World->GetGameState<AStrategosGameState>();
	return GS ? GS->GetOrCreateWorldState() : nullptr;
}

UStrategosSaveData* USaveSubsystem::CaptureSnapshot() const
{
	const UWorldState* WorldState = ResolveWorldState();
	if (!WorldState)
	{
		return nullptr;
	}

	UStrategosSaveData* Snapshot = Cast<UStrategosSaveData>(
		UGameplayStatics::CreateSaveGameObject(UStrategosSaveData::StaticClass()));
	if (!Snapshot)
	{
		return nullptr;
	}

	Snapshot->PlayerNationId = WorldState->PlayerNationId;

	if (const UTimeSubsystem* Time = GetGameInstance()->GetSubsystem<UTimeSubsystem>())
	{
		Snapshot->CurrentDate = Time->GetCurrentDate();
	}

	Snapshot->Nations.Reserve(WorldState->Nations.Num());
	for (const auto& Pair : WorldState->Nations)
	{
		const UNation* Nation = Pair.Value.Get();
		if (!Nation) continue;
		FNationRecord R;
		R.Id = Nation->Id;
		R.DisplayName = Nation->DisplayName;
		R.Color = Nation->Color;
		R.CapitalProvinceId = Nation->CapitalProvinceId;
		R.OwnedProvinceIds = Nation->OwnedProvinceIds;
		R.bIsPlayerControlled = Nation->bIsPlayerControlled;
		R.Treasury = Nation->Treasury;
		R.StockpileStocks = Nation->Stockpile.Stocks;
		R.StrategicIndices = Nation->StrategicIndices;
		R.SecondaryColor = Nation->SecondaryColor;
		R.FlagTexturePath = FName(*Nation->FlagTexture.ToSoftObjectPath().ToString());
		R.CoatOfArmsIconPath = FName(*Nation->CoatOfArmsIcon.ToSoftObjectPath().ToString());
		Snapshot->Nations.Add(R);
	}

	Snapshot->Provinces.Reserve(WorldState->Provinces.Num());
	for (const auto& Pair : WorldState->Provinces)
	{
		const UProvince* Province = Pair.Value.Get();
		if (!Province) continue;
		FProvinceRecord R;
		R.Id = Province->Id;
		R.DisplayName = Province->DisplayName;
		R.OwnerNationId = Province->OwnerNationId;
		R.AdjacentProvinceIds = Province->AdjacentProvinceIds;
		R.MapPosition = Province->MapPosition;
		R.Terrain = Province->Terrain;
		R.BuildingSlots = Province->BuildingSlots;
		R.RawResourcePotential = Province->RawResourcePotential;

		R.Pops.Reserve(Province->Pops.Num());
		for (const auto& PopPair : Province->Pops)
		{
			FPopRecord PR;
			PR.Stratum = PopPair.Value.Stratum;
			PR.Population = PopPair.Value.Population;
			PR.Wealth = PopPair.Value.Wealth;
			PR.Loyalty = PopPair.Value.Loyalty;
			R.Pops.Add(PR);
		}

		R.Buildings.Reserve(Province->Buildings.Num());
		for (const TObjectPtr<UBuilding>& BPtr : Province->Buildings)
		{
			const UBuilding* B = BPtr.Get();
			if (!B) continue;
			FBuildingRecord BR;
			BR.Id = B->Id;
			BR.ProvinceId = B->ProvinceId;
			BR.BuildingTypeAssetPath = FName(*B->BuildingType.ToSoftObjectPath().ToString());
			BR.CurrentMethodAssetPath = FName(*B->CurrentProductionMethod.ToSoftObjectPath().ToString());
			BR.Level = B->Level;
			BR.OwnerKind = B->OwnerKind;
			BR.OwnerProvinceId = B->OwnerProvinceId;
			BR.ConstructionDaysRemaining = B->ConstructionDaysRemaining;
			for (const TSoftObjectPtr<UProductionModifierAsset>& Mod : B->ActiveProductionModifiers)
			{
				BR.ActiveModifierAssetPaths.Add(FName(*Mod.ToSoftObjectPath().ToString()));
			}
			R.Buildings.Add(BR);
		}

		Snapshot->Provinces.Add(R);
	}

	Snapshot->Armies.Reserve(WorldState->Armies.Num());
	for (const auto& Pair : WorldState->Armies)
	{
		const UArmy* Army = Pair.Value.Get();
		if (!Army) continue;
		FArmyRecord R;
		R.Id = Army->Id;
		R.DisplayName = Army->DisplayName;
		R.OwnerNationId = Army->OwnerNationId;
		R.CurrentProvinceId = Army->CurrentProvinceId;
		R.MoveTargetProvinceId = Army->MoveTargetProvinceId;
		R.MoveDaysRemaining = Army->MoveDaysRemaining;
		R.ManpowerCount = Army->ManpowerCount;
		R.UnitTypeAssetPath = FName(*Army->UnitType.ToSoftObjectPath().ToString());
		R.BaseStats = Army->BaseStats;
		R.ActiveModifiers = Army->ActiveModifiers;
		R.State = Army->State;
		R.ExperienceXP = Army->ExperienceXP;
		R.ExperienceLevel = Army->ExperienceLevel;
		Snapshot->Armies.Add(R);
	}

	// Pending decisions.
	if (const UWorld* GameWorld = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr)
	{
		if (UEventSubsystem* Events = GameWorld->GetSubsystem<UEventSubsystem>())
		{
			for (const auto& Pair : Events->GetPendingDecisionsRaw())
			{
				for (const FPendingDecision& P : Pair.Value)
				{
					FPendingDecisionRecord R;
					R.NationId = Pair.Key;
					R.Context = P.Context;
					Snapshot->PendingDecisions.Add(R);
				}
			}
		}

		// Diplomatic relations.
		if (const UDiplomacySubsystem* Diplomacy = GameWorld->GetSubsystem<UDiplomacySubsystem>())
		{
			for (const auto& Pair : Diplomacy->GetRelationsRaw())
			{
				Snapshot->DiplomaticRelations.Add(Pair.Value);
			}
		}
	}

	return Snapshot;
}

bool USaveSubsystem::ApplySnapshot(const UStrategosSaveData& Snapshot)
{
	UWorldState* WorldState = ResolveWorldState();
	if (!WorldState)
	{
		return false;
	}

	WorldState->Nations.Empty();
	WorldState->Provinces.Empty();
	WorldState->Armies.Empty();
	WorldState->PlayerNationId = Snapshot.PlayerNationId;

	UEconomySubsystem* Economy = nullptr;
	if (const UWorld* GameWorld = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr)
	{
		Economy = GameWorld->GetSubsystem<UEconomySubsystem>();
	}

	for (const FProvinceRecord& R : Snapshot.Provinces)
	{
		UProvince* P = WorldState->AddProvince(R.Id);
		P->DisplayName = R.DisplayName;
		P->OwnerNationId = R.OwnerNationId;
		P->AdjacentProvinceIds = R.AdjacentProvinceIds;
		P->MapPosition = R.MapPosition;
		P->Terrain = R.Terrain;
		P->BuildingSlots = R.BuildingSlots;
		P->RawResourcePotential = R.RawResourcePotential;

		P->Pops.Empty();
		for (const FPopRecord& PR : R.Pops)
		{
			FPopGroup G;
			G.Stratum = PR.Stratum;
			G.Population = PR.Population;
			G.Wealth = PR.Wealth;
			G.Loyalty = PR.Loyalty;
			P->Pops.Add(PR.Stratum, G);
		}

		P->Buildings.Empty();
		for (const FBuildingRecord& BR : R.Buildings)
		{
			UBuilding* B = NewObject<UBuilding>(P);
			B->Id = BR.Id;
			B->ProvinceId = BR.ProvinceId;
			B->BuildingType = TSoftObjectPtr<UBuildingTypeAsset>(FSoftObjectPath(BR.BuildingTypeAssetPath.ToString()));
			B->CurrentProductionMethod = TSoftObjectPtr<UProductionMethodAsset>(FSoftObjectPath(BR.CurrentMethodAssetPath.ToString()));
			B->Level = BR.Level;
			B->OwnerKind = BR.OwnerKind;
			B->OwnerProvinceId = BR.OwnerProvinceId;
			B->ConstructionDaysRemaining = BR.ConstructionDaysRemaining;
			for (const FName& ModPath : BR.ActiveModifierAssetPaths)
			{
				B->ActiveProductionModifiers.Add(
					TSoftObjectPtr<UProductionModifierAsset>(FSoftObjectPath(ModPath.ToString())));
			}
			P->Buildings.Add(B);
		}
	}

	for (const FNationRecord& R : Snapshot.Nations)
	{
		UNation* N = WorldState->AddNation(R.Id);
		N->DisplayName = R.DisplayName;
		N->Color = R.Color;
		N->CapitalProvinceId = R.CapitalProvinceId;
		N->OwnedProvinceIds = R.OwnedProvinceIds;
		N->bIsPlayerControlled = R.bIsPlayerControlled;
		N->Treasury = R.Treasury;
		N->Stockpile.Stocks = R.StockpileStocks;
		N->StrategicIndices = R.StrategicIndices;
		N->SecondaryColor = R.SecondaryColor;
		if (!R.FlagTexturePath.IsNone())
		{
			N->FlagTexture = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(R.FlagTexturePath.ToString()));
		}
		if (!R.CoatOfArmsIconPath.IsNone())
		{
			N->CoatOfArmsIcon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(R.CoatOfArmsIconPath.ToString()));
		}
	}

	(void)Economy;

	for (const FArmyRecord& R : Snapshot.Armies)
	{
		UArmy* A = WorldState->AddArmy(R.Id);
		A->DisplayName = R.DisplayName;
		A->OwnerNationId = R.OwnerNationId;
		A->CurrentProvinceId = R.CurrentProvinceId;
		A->MoveTargetProvinceId = R.MoveTargetProvinceId;
		A->MoveDaysRemaining = R.MoveDaysRemaining;
		A->ManpowerCount = R.ManpowerCount;
		if (!R.UnitTypeAssetPath.IsNone())
		{
			A->UnitType = TSoftObjectPtr<UUnitTypeAsset>(FSoftObjectPath(R.UnitTypeAssetPath.ToString()));
		}
		A->BaseStats = R.BaseStats;
		A->ActiveModifiers = R.ActiveModifiers;
		A->State = R.State;
		A->ExperienceXP = R.ExperienceXP;
		A->ExperienceLevel = R.ExperienceLevel;
	}

	if (UTimeSubsystem* Time = GetGameInstance()->GetSubsystem<UTimeSubsystem>())
	{
		const FDateTime& D = Snapshot.CurrentDate;
		Time->SetStartDate(D.GetYear(), D.GetMonth(), D.GetDay());
	}

	// Pending decisions.
	if (const UWorld* GameWorld = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr)
	{
		if (UEventSubsystem* Events = GameWorld->GetSubsystem<UEventSubsystem>())
		{
			TMap<FName, TArray<FPendingDecision>> Restored;
			for (const FPendingDecisionRecord& R : Snapshot.PendingDecisions)
			{
				FPendingDecision P;
				P.Context = R.Context;
				Restored.FindOrAdd(R.NationId).Add(P);
			}
			Events->RestorePendingDecisions(Restored);
		}

		if (UDiplomacySubsystem* Diplomacy = GameWorld->GetSubsystem<UDiplomacySubsystem>())
		{
			Diplomacy->RestoreRelations(Snapshot.DiplomaticRelations);
		}
	}

	return true;
}

bool USaveSubsystem::SaveToSlot(const FString& SlotName)
{
	UStrategosSaveData* Snapshot = CaptureSnapshot();
	if (!Snapshot)
	{
		OnSaveCompleted.Broadcast(false);
		return false;
	}

	const bool bOk = UGameplayStatics::SaveGameToSlot(Snapshot, SlotName, 0);
	UE_LOG(LogStrategosCore, Log, TEXT("Save '%s': %s"),
		*SlotName, bOk ? TEXT("ok") : TEXT("failed"));
	OnSaveCompleted.Broadcast(bOk);
	return bOk;
}

bool USaveSubsystem::LoadFromSlot(const FString& SlotName)
{
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		OnLoadCompleted.Broadcast(false);
		return false;
	}

	UStrategosSaveData* Loaded = Cast<UStrategosSaveData>(
		UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (!Loaded)
	{
		OnLoadCompleted.Broadcast(false);
		return false;
	}

	const bool bOk = ApplySnapshot(*Loaded);
	UE_LOG(LogStrategosCore, Log, TEXT("Load '%s': %s"),
		*SlotName, bOk ? TEXT("ok") : TEXT("failed"));
	OnLoadCompleted.Broadcast(bOk);
	return bOk;
}

bool USaveSubsystem::DoesSaveExist(const FString& SlotName) const
{
	return UGameplayStatics::DoesSaveGameExist(SlotName, 0);
}
