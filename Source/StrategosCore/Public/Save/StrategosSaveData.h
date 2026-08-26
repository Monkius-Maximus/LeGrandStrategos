#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Economy/ProvinceGeography.h"
#include "Economy/PopStratum.h"
#include "Economy/Treasury.h"
#include "Economy/BuildingOwnerKind.h"
#include "Economy/StrategicIndices.h"
#include "Events/EventContext.h"
#include "Events/EventHistory.h"
#include "World/ArmyStats.h"
#include "Diplomacy/DiplomaticRelation.h"
#include "StrategosSaveData.generated.h"

USTRUCT()
struct FPopRecord
{
	GENERATED_BODY()
	UPROPERTY() EPopStratum Stratum = EPopStratum::Laborer;
	UPROPERTY() int32 Population = 0;
	UPROPERTY() float Wealth = 0.f;
	UPROPERTY() float Loyalty = 1.f;
};

USTRUCT()
struct FBuildingRecord
{
	GENERATED_BODY()
	UPROPERTY() FName Id;
	UPROPERTY() FName ProvinceId;
	UPROPERTY() FName BuildingTypeAssetPath; // path do UBuildingTypeAsset
	UPROPERTY() FName CurrentMethodAssetPath;
	UPROPERTY() TArray<FName> ActiveModifierAssetPaths;
	UPROPERTY() int32 Level = 1;
	UPROPERTY() EBuildingOwnerKind OwnerKind = EBuildingOwnerKind::Government;
	UPROPERTY() FName OwnerProvinceId;
	UPROPERTY() int32 ConstructionDaysRemaining = 0;
};

USTRUCT()
struct FNationRecord
{
	GENERATED_BODY()
	UPROPERTY() FName Id;
	UPROPERTY() FText DisplayName;
	UPROPERTY() FLinearColor Color = FLinearColor::White;
	UPROPERTY() FName CapitalProvinceId;
	UPROPERTY() TArray<FName> OwnedProvinceIds;
	UPROPERTY() bool bIsPlayerControlled = false;

	// Etapa 2: economia.
	UPROPERTY() FTreasury Treasury;
	UPROPERTY() TMap<FName, float> StockpileStocks;
	UPROPERTY() FStrategicIndices StrategicIndices;

	// Etapa 2 (UI grid v4): identidade visual.
	UPROPERTY() FLinearColor SecondaryColor = FLinearColor::Gray;
	UPROPERTY() FName FlagTexturePath;
	UPROPERTY() FName CoatOfArmsIconPath;
};

USTRUCT()
struct FProvinceRecord
{
	GENERATED_BODY()
	UPROPERTY() FName Id;
	UPROPERTY() FText DisplayName;
	UPROPERTY() FName OwnerNationId;
	UPROPERTY() TArray<FName> AdjacentProvinceIds;
	UPROPERTY() FVector2D MapPosition = FVector2D::ZeroVector;

	// Recursos e Produção (Sessão 1): geografia natural em 4 eixos.
	// Substitui o antigo ETerrainType Terrain (agora derivado via GetTerrain()).
	UPROPERTY() FProvinceGeography Geography;

	// Etapa 2: economia.
	UPROPERTY() int32 BuildingSlots = 3;
	UPROPERTY() TMap<FName, float> RawResourcePotential;
	UPROPERTY() TArray<FPopRecord> Pops;
	UPROPERTY() TArray<FBuildingRecord> Buildings;
};

USTRUCT()
struct FArmyRecord
{
	GENERATED_BODY()
	UPROPERTY() FName Id;
	UPROPERTY() FText DisplayName;
	UPROPERTY() FName OwnerNationId;
	UPROPERTY() FName CurrentProvinceId;
	UPROPERTY() FName MoveTargetProvinceId;
	UPROPERTY() int32 MoveDaysRemaining = 0;
	UPROPERTY() int32 ManpowerCount = 0;

	// Etapa 2 (UI grid v4): tipo, stats, modificadores, experiência, estado.
	UPROPERTY() FName UnitTypeAssetPath;
	UPROPERTY() FArmyStats BaseStats;
	UPROPERTY() TArray<FArmyModifier> ActiveModifiers;
	UPROPERTY() EUnitState State = EUnitState::Ready;
	UPROPERTY() int32 ExperienceXP = 0;
	UPROPERTY() int32 ExperienceLevel = 0;
};

/**
 * UStrategosSaveData — snapshot serializável do estado do mundo.
 *
 * SaveVersion = 7 (Events: memória de disparo):
 *  - EventHistory: log de eventos disparados (com a escolha feita, quando houver)
 *  - NationEventStates: por nação, quais eventos já dispararam e cooldowns ativos
 *  - GlobalFiredEventIds: eventos OnceGlobal que já ocorreram na partida
 *  Sem esses três, recarregar um save reabria eventos únicos e zerava cooldowns.
 *
 * Versões anteriores:
 *  - 6: FProvinceRecord.Geography (geografia natural em 4 eixos, substituindo
 *       o antigo campo Terrain, hoje derivado via UProvince::GetTerrain())
 *  - 5: DiplomaticRelations (matriz esparsa N×N de FDiplomaticRelation)
 *  - 4: identidade visual de Nation + expansão de Army (stats, modifiers, state, XP)
 *  - 3: pending decisions de Events
 *  - 2: economia (Treasury, Stockpile, POPs, Buildings)
 *  - 1: base (Nations, Provinces, Armies)
 *
 * Versionamento real (com migração 4 → 5 etc) virá quando o jogo entrar em alpha
 * pública. Por agora, abrir um save antigo em código novo inicializa campos novos
 * com defaults.
 */
USTRUCT()
struct FPendingDecisionRecord
{
	GENERATED_BODY()
	UPROPERTY() FName NationId;
	UPROPERTY() FEventContext Context;
};

UCLASS()
class STRATEGOSCORE_API UStrategosSaveData : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY() int32 SaveVersion = 7;
	UPROPERTY() FDateTime CurrentDate;
	UPROPERTY() FName PlayerNationId;
	UPROPERTY() TArray<FNationRecord> Nations;
	UPROPERTY() TArray<FProvinceRecord> Provinces;
	UPROPERTY() TArray<FArmyRecord> Armies;
	UPROPERTY() TArray<FPendingDecisionRecord> PendingDecisions;
	UPROPERTY() TArray<FDiplomaticRelation> DiplomaticRelations;

	// SaveVersion 7: memória de disparo de eventos.
	UPROPERTY() TArray<FFiredEventRecord> EventHistory;
	UPROPERTY() TArray<FNationEventStateRecord> NationEventStates;
	UPROPERTY() TArray<FName> GlobalFiredEventIds;
};
