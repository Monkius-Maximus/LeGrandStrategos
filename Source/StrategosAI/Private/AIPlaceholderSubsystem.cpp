#include "AIPlaceholderSubsystem.h"
#include "StrategosAI.h"
#include "World/WorldState.h"
#include "World/Nation.h"
#include "World/Province.h"
#include "World/Army.h"
#include "World/Leader.h"
#include "World/LeaderArchetype.h"
#include "Foundation/Time/TimeSubsystem.h"
#include "Strategy/MilitarySubsystem.h"
#include "Game/StrategosGameState.h"
#include "Engine/World.h"

void UAIPlaceholderSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (UTimeSubsystem* Time = ResolveTime())
	{
		Time->OnMonthTick.AddDynamic(this, &UAIPlaceholderSubsystem::HandleMonthTick);
		Time->OnYearTick.AddDynamic(this, &UAIPlaceholderSubsystem::HandleYearTick);
	}

	UE_LOG(LogStrategosAI, Log, TEXT("AIPlaceholderSubsystem initialized."));
}

void UAIPlaceholderSubsystem::Deinitialize()
{
	if (UTimeSubsystem* Time = ResolveTime())
	{
		Time->OnMonthTick.RemoveDynamic(this, &UAIPlaceholderSubsystem::HandleMonthTick);
		Time->OnYearTick.RemoveDynamic(this, &UAIPlaceholderSubsystem::HandleYearTick);
	}
	Super::Deinitialize();
}

bool UAIPlaceholderSubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

UWorldState* UAIPlaceholderSubsystem::ResolveWorldState() const
{
	const UWorld* World = GetWorld();
	if (!World) return nullptr;
	AStrategosGameState* GS = World->GetGameState<AStrategosGameState>();
	return GS ? GS->GetWorldState() : nullptr;
}

UTimeSubsystem* UAIPlaceholderSubsystem::ResolveTime() const
{
	const UWorld* World = GetWorld();
	if (!World) return nullptr;
	const UGameInstance* GI = World->GetGameInstance();
	return GI ? GI->GetSubsystem<UTimeSubsystem>() : nullptr;
}

UMilitarySubsystem* UAIPlaceholderSubsystem::ResolveMilitary() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetSubsystem<UMilitarySubsystem>() : nullptr;
}

FRandomStream UAIPlaceholderSubsystem::MakeStream(FName NationId, int32 Year, int32 Month)
{
	const uint32 H1 = GetTypeHash(NationId);
	const uint32 H2 = HashCombine(static_cast<uint32>(Year), static_cast<uint32>(Month));
	return FRandomStream(static_cast<int32>(HashCombine(H1, H2)));
}

void UAIPlaceholderSubsystem::HandleYearTick(FDateTime CurrentDate)
{
	UWorldState* World = ResolveWorldState();
	if (!World) return;

	const int32 Year = CurrentDate.GetYear();
	for (auto& Pair : World->Nations)
	{
		UNation* Nation = Pair.Value.Get();
		if (!Nation || Nation->bIsPlayerControlled) continue;
		RollLeaderSuccession(*Nation, Year);
	}
}

void UAIPlaceholderSubsystem::RollLeaderSuccession(UNation& Nation, int32 Year)
{
	FRandomStream Stream = MakeStream(Nation.Id, Year, 0);
	if (Stream.FRand() > LeaderTurnoverChance)
	{
		return;
	}

	float TotalWeight = 0.f;
	for (const auto& Pair : Nation.ArchetypeAffinity)
	{
		TotalWeight += FMath::Max(0.f, Pair.Value);
	}
	if (TotalWeight <= 0.f)
	{
		return;
	}

	float Roll = Stream.FRandRange(0.f, TotalWeight);
	ELeaderArchetype Picked = ELeaderArchetype::Pragmatist;
	for (const auto& Pair : Nation.ArchetypeAffinity)
	{
		const float W = FMath::Max(0.f, Pair.Value);
		if (Roll <= W)
		{
			Picked = Pair.Key;
			break;
		}
		Roll -= W;
	}

	ULeader* NewLeader = NewObject<ULeader>(&Nation);
	NewLeader->Id = FName(*FString::Printf(TEXT("%s.Leader.%d"), *Nation.Id.ToString(), Year));
	NewLeader->DisplayName = FText::FromName(NewLeader->Id);
	NewLeader->Archetype = Picked;
	NewLeader->BirthYear = Year - 40;
	NewLeader->AscensionYear = Year;
	Nation.CurrentLeader = NewLeader;

	UE_LOG(LogStrategosAI, Log, TEXT("AI: %s succession in %d -> %s"),
		*Nation.Id.ToString(), Year, *UEnum::GetValueAsString(Picked));
}

void UAIPlaceholderSubsystem::HandleMonthTick(FDateTime CurrentDate)
{
	UWorldState* World = ResolveWorldState();
	if (!World) return;

	for (auto& Pair : World->Nations)
	{
		UNation* Nation = Pair.Value.Get();
		if (!Nation || Nation->bIsPlayerControlled) continue;
		RunArchetypeBehavior(*Nation, CurrentDate);
	}
}

void UAIPlaceholderSubsystem::RunArchetypeBehavior(UNation& Nation, const FDateTime& CurrentDate)
{
	const ELeaderArchetype Archetype = Nation.CurrentLeader
		? Nation.CurrentLeader->Archetype
		: ELeaderArchetype::Pragmatist;

	FRandomStream RNG = MakeStream(Nation.Id, CurrentDate.GetYear(), CurrentDate.GetMonth());

	switch (Archetype)
	{
		case ELeaderArchetype::Militarist:    Behavior_Militarist(Nation, RNG); break;
		case ELeaderArchetype::Diplomat:      Behavior_Diplomat(Nation, RNG); break;
		case ELeaderArchetype::Pragmatist:    Behavior_Pragmatist(Nation, RNG); break;
		case ELeaderArchetype::Merchant:
		case ELeaderArchetype::Religious:
		case ELeaderArchetype::Intellectual:
			// No-op placeholder. Comportamentos econômico/político/tech entram
			// nas Etapas 2-3 do roadmap, quando os subsistemas relevantes existirem.
			break;
	}
}

void UAIPlaceholderSubsystem::Behavior_Militarist(UNation& Nation, FRandomStream& RNG)
{
	UWorldState* World = ResolveWorldState();
	UMilitarySubsystem* Military = ResolveMilitary();
	if (!World || !Military) return;

	// Procura uma fronteira hostil: prov. própria adjacente a prov. de outra nação.
	TArray<TPair<FName, FName>> HostileBorders; // (FromOwn, ToHostile)
	for (const FName& OwnedId : Nation.OwnedProvinceIds)
	{
		const UProvince* Owned = World->GetProvince(OwnedId);
		if (!Owned) continue;
		for (const FName& AdjId : Owned->AdjacentProvinceIds)
		{
			const UProvince* Adj = World->GetProvince(AdjId);
			if (Adj && Adj->OwnerNationId != Nation.Id && !Adj->OwnerNationId.IsNone())
			{
				HostileBorders.Emplace(OwnedId, AdjId);
			}
		}
	}

	if (HostileBorders.Num() == 0)
	{
		Behavior_Pragmatist(Nation, RNG);
		return;
	}

	// Procura um exército ocioso da nação e move-o para a fronteira mais próxima.
	for (const auto& ArmyPair : World->Armies)
	{
		UArmy* Army = ArmyPair.Value.Get();
		if (!Army || Army->OwnerNationId != Nation.Id || Army->IsMoving()) continue;

		// Selecionar borda: se exército já está em prov. fronteiriça, atacar; senão
		// mover em direção à primeira borda listada.
		for (const auto& Border : HostileBorders)
		{
			if (Army->CurrentProvinceId == Border.Key)
			{
				Military->IssueMoveOrder(Army->Id, Border.Value);
				return;
			}
		}
		const FName Target = HostileBorders[RNG.RandRange(0, HostileBorders.Num() - 1)].Key;
		const UProvince* Current = World->GetProvince(Army->CurrentProvinceId);
		if (Current)
		{
			// Se há um vizinho da posição atual em direção ao target, vai pra ele.
			for (const FName& Adj : Current->AdjacentProvinceIds)
			{
				if (const UProvince* AdjP = World->GetProvince(Adj))
				{
					if (AdjP->OwnerNationId == Nation.Id)
					{
						Military->IssueMoveOrder(Army->Id, Adj);
						return;
					}
				}
			}
		}
	}
}

void UAIPlaceholderSubsystem::Behavior_Diplomat(UNation& Nation, FRandomStream& RNG)
{
	UWorldState* World = ResolveWorldState();
	UMilitarySubsystem* Military = ResolveMilitary();
	if (!World || !Military) return;

	// Mantém exércitos na capital. Se algum estiver fora, manda voltar.
	for (const auto& ArmyPair : World->Armies)
	{
		UArmy* Army = ArmyPair.Value.Get();
		if (!Army || Army->OwnerNationId != Nation.Id || Army->IsMoving()) continue;
		if (Army->CurrentProvinceId == Nation.CapitalProvinceId) continue;

		const UProvince* Current = World->GetProvince(Army->CurrentProvinceId);
		if (!Current) continue;
		// Move um passo em direção à capital pelo primeiro vizinho próprio.
		for (const FName& Adj : Current->AdjacentProvinceIds)
		{
			if (const UProvince* AdjP = World->GetProvince(Adj))
			{
				if (AdjP->OwnerNationId == Nation.Id)
				{
					Military->IssueMoveOrder(Army->Id, Adj);
					break;
				}
			}
		}
	}
}

void UAIPlaceholderSubsystem::Behavior_Pragmatist(UNation& Nation, FRandomStream& RNG)
{
	if (RNG.FRand() > MonthlyMoveChance) return;

	UWorldState* World = ResolveWorldState();
	UMilitarySubsystem* Military = ResolveMilitary();
	if (!World || !Military) return;

	for (const auto& ArmyPair : World->Armies)
	{
		UArmy* Army = ArmyPair.Value.Get();
		if (!Army || Army->OwnerNationId != Nation.Id || Army->IsMoving()) continue;

		const UProvince* Current = World->GetProvince(Army->CurrentProvinceId);
		if (!Current || Current->AdjacentProvinceIds.Num() == 0) continue;

		const int32 Idx = RNG.RandRange(0, Current->AdjacentProvinceIds.Num() - 1);
		Military->IssueMoveOrder(Army->Id, Current->AdjacentProvinceIds[Idx]);
		return;
	}
}
