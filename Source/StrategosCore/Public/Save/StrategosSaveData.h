#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "World/TerrainType.h"
#include "Economy/PopStratum.h"
#include "Economy/Treasury.h"
#include "Economy/BuildingOwnerKind.h"
#include "Economy/StrategicIndices.h"
#include "Events/EventContext.h"
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
	UPROPERTY() ETerrainType Terrain = ETerrainType::Plains;

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
};

/**
 * UStrategosSaveData — snapshot serializável do estado do mundo.
 *
 * SaveVersion = 2 (Etapa 2 v1):
 *  - Adiciona economy state em FNationRecord (Treasury + Stockpile + Indices)
 *  - Adiciona POPs e Buildings em FProvinceRecord
 *  - DataAssets (UBuildingTypeAsset etc) são referenciados por path/Id
 *
 * Versionamento real (com migração de SaveVersion 1 → 2) virá quando o
 * jogo entrar em alpha pública. Por agora, abrir um save antigo em código
 * novo apenas inicializa os campos novos com defaults.
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
	UPROPERTY() int32 SaveVersion = 3;
	UPROPERTY() FDateTime CurrentDate;
	UPROPERTY() FName PlayerNationId;
	UPROPERTY() TArray<FNationRecord> Nations;
	UPROPERTY() TArray<FProvinceRecord> Provinces;
	UPROPERTY() TArray<FArmyRecord> Armies;
	UPROPERTY() TArray<FPendingDecisionRecord> PendingDecisions;
};
