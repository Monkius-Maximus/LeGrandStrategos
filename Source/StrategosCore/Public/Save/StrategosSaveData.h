#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "World/TerrainType.h"
#include "StrategosSaveData.generated.h"

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
 * Stage 1 (MVP): converte UWorldState em arrays de records flat. Quando
 * subsistemas com estado complexo (Brains de IA, Modifiers, Treaties)
 * entrarem, expandiremos com versionamento e migração — ver
 * docs/architecture/99-implementation-roadmap.md seção "USaveSubsystem".
 */
UCLASS()
class STRATEGOSCORE_API UStrategosSaveData : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY() int32 SaveVersion = 1;
	UPROPERTY() FDateTime CurrentDate;
	UPROPERTY() FName PlayerNationId;
	UPROPERTY() TArray<FNationRecord> Nations;
	UPROPERTY() TArray<FProvinceRecord> Provinces;
	UPROPERTY() TArray<FArmyRecord> Armies;
};
