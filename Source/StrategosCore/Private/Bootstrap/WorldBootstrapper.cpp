#include "Bootstrap/WorldBootstrapper.h"
#include "Bootstrap/WorldBootstrapAsset.h"
#include "Bootstrap/WorldBootstrapRows.h"
#include "World/WorldState.h"
#include "World/Province.h"
#include "World/Nation.h"
#include "World/Army.h"
#include "World/Leader.h"
#include "Economy/PopGroup.h"
#include "Economy/PopStratum.h"
#include "Economy/TaxLevel.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "Diplomacy/DiplomacySubsystem.h"
#include "StrategosCore.h"

namespace
{
	// Mapeia ETerrainType → potencial de extração natural por bem.
	void ApplyTerrainPotential(UProvince& Prov)
	{
		Prov.RawResourcePotential.Empty();

		switch (Prov.Terrain)
		{
			case ETerrainType::Mountains:
				Prov.RawResourcePotential.Add(TEXT("IronOre"), 1.0f);
				Prov.RawResourcePotential.Add(TEXT("Coal"),    0.7f);
				Prov.BuildingSlots = 2;
				break;
			case ETerrainType::Hills:
				Prov.RawResourcePotential.Add(TEXT("IronOre"), 0.6f);
				Prov.RawResourcePotential.Add(TEXT("Coal"),    0.4f);
				Prov.RawResourcePotential.Add(TEXT("Wood"),    0.3f);
				Prov.BuildingSlots = 3;
				break;
			case ETerrainType::Forest:
				Prov.RawResourcePotential.Add(TEXT("Wood"),    1.0f);
				Prov.RawResourcePotential.Add(TEXT("Grain"),   0.4f);
				Prov.BuildingSlots = 3;
				break;
			case ETerrainType::Plains:
				Prov.RawResourcePotential.Add(TEXT("Grain"),   1.0f);
				Prov.RawResourcePotential.Add(TEXT("Cotton"),  0.5f);
				Prov.RawResourcePotential.Add(TEXT("Wood"),    0.3f);
				Prov.BuildingSlots = 4;
				break;
			case ETerrainType::Coast:
				Prov.RawResourcePotential.Add(TEXT("Grain"),   0.5f);
				Prov.RawResourcePotential.Add(TEXT("Wood"),    0.4f);
				Prov.RawResourcePotential.Add(TEXT("Cotton"),  0.4f);
				Prov.BuildingSlots = 4;
				break;
			case ETerrainType::Marsh:
				Prov.RawResourcePotential.Add(TEXT("Wood"),    0.3f);
				Prov.BuildingSlots = 2;
				break;
			case ETerrainType::Tundra:
				Prov.RawResourcePotential.Add(TEXT("Wood"),    0.2f);
				Prov.BuildingSlots = 2;
				break;
			case ETerrainType::Desert:
				Prov.BuildingSlots = 1;
				break;
			case ETerrainType::Water:
			default:
				Prov.BuildingSlots = 0;
				break;
		}
	}

	// Distribui POPs default por província conforme terreno e dono.
	void SeedPopsForProvince(UProvince& Prov, bool bIsCapital)
	{
		Prov.Pops.Empty();

		const int32 BaseLab     = bIsCapital ? 8000 : 5000;
		const int32 BaseArt     = bIsCapital ? 1500 : 800;
		const int32 BaseFactory = bIsCapital ? 600  : 200;
		const int32 BaseBourg   = bIsCapital ? 200  : 80;

		auto Add = [&](EPopStratum S, int32 Pop, float Wealth)
		{
			FPopGroup G;
			G.Stratum = S;
			G.Population = Pop;
			G.Wealth = Wealth;
			G.Loyalty = 1.0f;
			Prov.Pops.Add(S, G);
		};

		Add(EPopStratum::Laborer,       BaseLab,     50.f);
		Add(EPopStratum::Artisan,       BaseArt,     150.f);
		Add(EPopStratum::FactoryWorker, BaseFactory, 200.f);
		Add(EPopStratum::Bourgeoisie,   BaseBourg,   1000.f);
	}

	// Define tax levels default e sementes de stockpile/treasury.
	void SeedNationEconomy(UNation& Nation)
	{
		Nation.Treasury.Balance = 500.f;
		Nation.Treasury.AnnualInterestRate = 0.05f;

		Nation.Treasury.TaxLevelByStratum.Add(EPopStratum::Laborer,       ETaxLevel::Medium);
		Nation.Treasury.TaxLevelByStratum.Add(EPopStratum::Artisan,       ETaxLevel::Medium);
		Nation.Treasury.TaxLevelByStratum.Add(EPopStratum::FactoryWorker, ETaxLevel::Medium);
		Nation.Treasury.TaxLevelByStratum.Add(EPopStratum::Bourgeoisie,   ETaxLevel::Low);

		// Sementes de bens essenciais para o jogo não travar logo no primeiro tick.
		Nation.Stockpile.Stocks.Add(TEXT("Bread"),     500.f);
		Nation.Stockpile.Stocks.Add(TEXT("Grain"),     300.f);
		Nation.Stockpile.Stocks.Add(TEXT("Garments"),  200.f);
		Nation.Stockpile.Stocks.Add(TEXT("Iron"),      100.f);
		Nation.Stockpile.Stocks.Add(TEXT("Coal"),      200.f);
		Nation.Stockpile.Stocks.Add(TEXT("Lumber"),    150.f);
	}
}

namespace
{
	void ResetWorld(UWorldState& WorldState)
	{
		WorldState.Nations.Empty();
		WorldState.Provinces.Empty();
		WorldState.Armies.Empty();
		WorldState.PlayerNationId = NAME_None;
	}

	void ApplyNationalIdeasToAffinity(
		UNation& Nation,
		const TMap<FName, TMap<ELeaderArchetype, float>>& IdeasIndex)
	{
		// Inicia todos arquétipos com base 1.0 — pluralismo "neutro".
		const TArray<ELeaderArchetype> AllArchetypes = {
			ELeaderArchetype::Militarist,
			ELeaderArchetype::Diplomat,
			ELeaderArchetype::Merchant,
			ELeaderArchetype::Religious,
			ELeaderArchetype::Intellectual,
			ELeaderArchetype::Pragmatist,
		};

		Nation.ArchetypeAffinity.Empty();
		for (ELeaderArchetype A : AllArchetypes)
		{
			Nation.ArchetypeAffinity.Add(A, 1.0f);
		}

		for (const FName& IdeaTag : Nation.NationalIdeas)
		{
			if (const TMap<ELeaderArchetype, float>* Bonus = IdeasIndex.Find(IdeaTag))
			{
				for (const auto& Pair : *Bonus)
				{
					float& Existing = Nation.ArchetypeAffinity.FindOrAdd(Pair.Key, 0.f);
					Existing += Pair.Value;
				}
			}
		}
	}

	ULeader* CreateLeader(UNation& Nation, FName LeaderName, ELeaderArchetype Archetype, int32 Year)
	{
		ULeader* Leader = NewObject<ULeader>(&Nation);
		Leader->Id = LeaderName.IsNone()
			? FName(*FString::Printf(TEXT("%s.Leader.%d"), *Nation.Id.ToString(), Year))
			: LeaderName;
		Leader->DisplayName = FText::FromName(Leader->Id);
		Leader->Archetype = Archetype;
		Leader->BirthYear = Year - 40;
		Leader->AscensionYear = Year;
		return Leader;
	}
}

bool UWorldBootstrapper::ApplyBootstrap(UWorldState* WorldState, UWorldBootstrapAsset* Asset)
{
	if (!WorldState || !Asset)
	{
		return false;
	}

	UDataTable* ProvincesTable = Asset->ProvincesTable.LoadSynchronous();
	UDataTable* NationsTable   = Asset->NationsTable.LoadSynchronous();
	UDataTable* ArmiesTable    = Asset->ArmiesTable.LoadSynchronous();
	UDataTable* IdeasTable     = Asset->NationalIdeasTable.LoadSynchronous();

	if (!ProvincesTable || !NationsTable)
	{
		UE_LOG(LogStrategosCore, Error, TEXT("Bootstrap '%s': missing required tables."),
			*Asset->ScenarioName.ToString());
		return false;
	}

	ResetWorld(*WorldState);
	WorldState->PlayerNationId = Asset->PlayerNationId;

	// 1. Indexar bônus de NationalIdeas (pode estar vazia).
	TMap<FName, TMap<ELeaderArchetype, float>> IdeasIndex;
	if (IdeasTable)
	{
		IdeasTable->ForeachRow<FNationalIdeaRow>(TEXT("Bootstrap"),
			[&IdeasIndex](const FName&, const FNationalIdeaRow& Row)
			{
				IdeasIndex.Add(Row.Id, Row.ArchetypeBonus);
			});
	}

	// 2. Províncias primeiro (Nações apontam para CapitalProvinceId).
	ProvincesTable->ForeachRow<FProvinceRow>(TEXT("Bootstrap"),
		[WorldState](const FName&, const FProvinceRow& Row)
		{
			UProvince* P = WorldState->AddProvince(Row.Id);
			P->DisplayName = Row.DisplayName;
			P->OwnerNationId = Row.OwnerNationId;
			P->AdjacentProvinceIds = Row.AdjacentProvinceIds;
			P->MapPosition = Row.MapPosition;
			P->Terrain = Row.Terrain;
			ApplyTerrainPotential(*P);
			SeedPopsForProvince(*P, /*bIsCapital=*/false);
		});

	// 3. Nações + líderes iniciais.
	const int32 StartYear = Asset->StartYear;
	NationsTable->ForeachRow<FNationRow>(TEXT("Bootstrap"),
		[WorldState, &IdeasIndex, StartYear](const FName&, const FNationRow& Row)
		{
			UNation* N = WorldState->AddNation(Row.Id);
			N->DisplayName = Row.DisplayName;
			N->Color = Row.Color;
			N->CapitalProvinceId = Row.CapitalProvinceId;
			N->bIsPlayerControlled = Row.bIsPlayerControlled;
			N->NationalIdeas = Row.NationalIdeas;
			ApplyNationalIdeasToAffinity(*N, IdeasIndex);
			N->CurrentLeader = CreateLeader(*N, Row.StartingLeaderName, Row.StartingLeaderArchetype, StartYear);
			SeedNationEconomy(*N);
		});

	// 3b. Re-seed pops da capital com valores maiores.
	for (const auto& NationPair : WorldState->Nations)
	{
		const UNation* N = NationPair.Value.Get();
		if (!N) continue;
		if (UProvince* Cap = WorldState->GetProvince(N->CapitalProvinceId))
		{
			SeedPopsForProvince(*Cap, /*bIsCapital=*/true);
		}
	}

	// 4. Reconciliar OwnedProvinceIds em cada Nation a partir dos donos das províncias.
	for (auto& NationPair : WorldState->Nations)
	{
		if (UNation* N = NationPair.Value.Get())
		{
			N->OwnedProvinceIds.Empty();
		}
	}
	for (const auto& ProvincePair : WorldState->Provinces)
	{
		const UProvince* P = ProvincePair.Value.Get();
		if (!P) continue;
		if (UNation* Owner = WorldState->GetNation(P->OwnerNationId))
		{
			Owner->OwnedProvinceIds.AddUnique(P->Id);
		}
	}

	// 5. Exércitos.
	if (ArmiesTable)
	{
		ArmiesTable->ForeachRow<FArmyRow>(TEXT("Bootstrap"),
			[WorldState](const FName&, const FArmyRow& Row)
			{
				UArmy* A = WorldState->AddArmy(Row.Id);
				A->DisplayName = Row.DisplayName;
				A->OwnerNationId = Row.OwnerNationId;
				A->CurrentProvinceId = Row.StartingProvinceId;
				A->ManpowerCount = Row.ManpowerCount;
			});
	}

	UE_LOG(LogStrategosCore, Log, TEXT("Bootstrap '%s': %d nations, %d provinces, %d armies"),
		*Asset->ScenarioName.ToString(),
		WorldState->Nations.Num(),
		WorldState->Provinces.Num(),
		WorldState->Armies.Num());

	return true;
}

void UWorldBootstrapper::ApplyDefaultSandbox(UWorldState* WorldState)
{
	if (!WorldState)
	{
		return;
	}

	ResetWorld(*WorldState);
	WorldState->PlayerNationId = TEXT("Albion");

	// Mini-tabela de NationalIdeas in-memory.
	TMap<FName, TMap<ELeaderArchetype, float>> IdeasIndex;
	IdeasIndex.Add(TEXT("Martial"),     { { ELeaderArchetype::Militarist,    0.6f } });
	IdeasIndex.Add(TEXT("Diplomatic"),  { { ELeaderArchetype::Diplomat,      0.6f } });
	IdeasIndex.Add(TEXT("Mercantile"),  { { ELeaderArchetype::Merchant,      0.6f } });
	IdeasIndex.Add(TEXT("Devout"),      { { ELeaderArchetype::Religious,     0.6f } });
	IdeasIndex.Add(TEXT("Scholarly"),   { { ELeaderArchetype::Intellectual,  0.6f } });

	// Grid 4 colunas x 3 linhas (12 províncias). Coluna 0..3, Linha 0..2.
	struct FStartProv { FName Id; FText Name; FName Owner; FVector2D Pos; ETerrainType T; };
	const TArray<FStartProv> Provs = {
		{ TEXT("AlbionCenter"),	FText::FromString(TEXT("Albion Center")),	TEXT("Albion"),	{ 0, 1 }, ETerrainType::Plains    },
		{ TEXT("AlbionNorth"),	FText::FromString(TEXT("Albion North")),	TEXT("Albion"),	{ 0, 0 }, ETerrainType::Hills     },
		{ TEXT("AlbionSouth"),	FText::FromString(TEXT("Albion South")),	TEXT("Albion"),	{ 0, 2 }, ETerrainType::Coast     },
		{ TEXT("MidlandsWest"),	FText::FromString(TEXT("Midlands West")),	TEXT("Albion"),	{ 1, 1 }, ETerrainType::Forest    },

		{ TEXT("MidlandsEast"),	FText::FromString(TEXT("Midlands East")),	TEXT("Galia"),	{ 2, 1 }, ETerrainType::Plains    },
		{ TEXT("GaliaCenter"),	FText::FromString(TEXT("Galia Center")),	TEXT("Galia"),	{ 3, 1 }, ETerrainType::Plains    },
		{ TEXT("GaliaSouth"),	FText::FromString(TEXT("Galia South")),		TEXT("Galia"),	{ 3, 2 }, ETerrainType::Marsh     },
		{ TEXT("GaliaMountains"), FText::FromString(TEXT("Galia Highlands")), TEXT("Galia"),	{ 2, 2 }, ETerrainType::Mountains },

		{ TEXT("NordenSouth"),	FText::FromString(TEXT("Norden South")),	TEXT("Norden"),	{ 1, 0 }, ETerrainType::Tundra    },
		{ TEXT("NordenCenter"),	FText::FromString(TEXT("Norden Center")),	TEXT("Norden"),	{ 2, 0 }, ETerrainType::Tundra    },
		{ TEXT("NordenNorth"),	FText::FromString(TEXT("Norden North")),	TEXT("Norden"),	{ 3, 0 }, ETerrainType::Mountains },
	};

	for (const FStartProv& SP : Provs)
	{
		UProvince* P = WorldState->AddProvince(SP.Id);
		P->DisplayName = SP.Name;
		P->OwnerNationId = SP.Owner;
		P->MapPosition = SP.Pos;
		P->Terrain = SP.T;
		ApplyTerrainPotential(*P);
		SeedPopsForProvince(*P, /*bIsCapital=*/false);
	}

	// Adjacências em grid (vizinhos cardeais).
	auto AddAdj = [WorldState](FName A, FName B)
	{
		if (UProvince* PA = WorldState->GetProvince(A)) PA->AdjacentProvinceIds.AddUnique(B);
		if (UProvince* PB = WorldState->GetProvince(B)) PB->AdjacentProvinceIds.AddUnique(A);
	};
	for (int32 Col = 0; Col < 4; ++Col)
	{
		for (int32 Row = 0; Row < 3; ++Row)
		{
			FName Self = NAME_None;
			for (const FStartProv& SP : Provs)
			{
				if (FMath::IsNearlyEqual(SP.Pos.X, (double)Col) && FMath::IsNearlyEqual(SP.Pos.Y, (double)Row))
				{
					Self = SP.Id; break;
				}
			}
			if (Self.IsNone()) continue;

			for (const FStartProv& Other : Provs)
			{
				const bool bRight = FMath::IsNearlyEqual(Other.Pos.X, (double)(Col + 1)) && FMath::IsNearlyEqual(Other.Pos.Y, (double)Row);
				const bool bDown  = FMath::IsNearlyEqual(Other.Pos.X, (double)Col) && FMath::IsNearlyEqual(Other.Pos.Y, (double)(Row + 1));
				if (bRight || bDown)
				{
					AddAdj(Self, Other.Id);
				}
			}
		}
	}

	// Nações.
	struct FStartNation { FName Id; FText Name; FLinearColor Color; FName Capital; bool bPlayer; TArray<FName> Ideas; ELeaderArchetype StartArch; };
	const TArray<FStartNation> Nations = {
		{ TEXT("Albion"), FText::FromString(TEXT("Republic of Albion")), FLinearColor(0.2f, 0.4f, 0.9f), TEXT("AlbionCenter"), true,  { TEXT("Mercantile"), TEXT("Diplomatic") }, ELeaderArchetype::Diplomat   },
		{ TEXT("Galia"),  FText::FromString(TEXT("Kingdom of Galia")),    FLinearColor(0.85f, 0.2f, 0.2f), TEXT("GaliaCenter"),  false, { TEXT("Martial"), TEXT("Devout") },         ELeaderArchetype::Militarist },
		{ TEXT("Norden"), FText::FromString(TEXT("Confederacy of Norden")), FLinearColor(0.95f, 0.85f, 0.4f), TEXT("NordenCenter"), false, { TEXT("Martial"), TEXT("Scholarly") },    ELeaderArchetype::Militarist },
	};

	for (const FStartNation& SN : Nations)
	{
		UNation* N = WorldState->AddNation(SN.Id);
		N->DisplayName = SN.Name;
		N->Color = SN.Color;
		N->CapitalProvinceId = SN.Capital;
		N->bIsPlayerControlled = SN.bPlayer;
		N->NationalIdeas = SN.Ideas;
		ApplyNationalIdeasToAffinity(*N, IdeasIndex);
		N->CurrentLeader = CreateLeader(*N, NAME_None, SN.StartArch, 1836);
		SeedNationEconomy(*N);

		if (UProvince* Cap = WorldState->GetProvince(SN.Capital))
		{
			SeedPopsForProvince(*Cap, /*bIsCapital=*/true);
		}
	}

	// Reconciliar OwnedProvinceIds.
	for (const auto& Pair : WorldState->Provinces)
	{
		const UProvince* P = Pair.Value.Get();
		if (!P) continue;
		if (UNation* Owner = WorldState->GetNation(P->OwnerNationId))
		{
			Owner->OwnedProvinceIds.AddUnique(P->Id);
		}
	}

	// Exércitos: 1 por nação na capital.
	for (const FStartNation& SN : Nations)
	{
		const FName ArmyId = FName(*FString::Printf(TEXT("%s.1stArmy"), *SN.Id.ToString()));
		UArmy* A = WorldState->AddArmy(ArmyId);
		A->DisplayName = FText::Format(NSLOCTEXT("Strategos", "1stArmyName", "{0} 1st Army"), SN.Name);
		A->OwnerNationId = SN.Id;
		A->CurrentProvinceId = SN.Capital;
		A->ManpowerCount = 1500;
	}

	// Relações diplomáticas iniciais (Diplomacy v1).
	if (UWorld* GameWorld = WorldState->GetWorld())
	{
		if (UDiplomacySubsystem* Diplomacy = GameWorld->GetSubsystem<UDiplomacySubsystem>())
		{
			Diplomacy->SetRelation(TEXT("Albion"), TEXT("Galia"),  EDiplomaticStatus::NonAggressionPact, -40.f);
			Diplomacy->SetRelation(TEXT("Albion"), TEXT("Norden"), EDiplomaticStatus::Peace,             25.f);
			Diplomacy->SetRelation(TEXT("Galia"),  TEXT("Norden"), EDiplomaticStatus::Peace,            -10.f);
		}
	}

	UE_LOG(LogStrategosCore, Log, TEXT("Default sandbox applied: %d nations, %d provinces, %d armies"),
		WorldState->Nations.Num(),
		WorldState->Provinces.Num(),
		WorldState->Armies.Num());
}
