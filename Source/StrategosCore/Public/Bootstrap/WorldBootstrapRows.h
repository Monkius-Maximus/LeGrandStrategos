#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "World/TerrainType.h"
#include "World/LeaderArchetype.h"
#include "WorldBootstrapRows.generated.h"

/**
 * Row structs consumidos pelo UWorldBootstrapper para popular o UWorldState.
 *
 * Cada struct herda de FTableRowBase para que designers/modders editem
 * os cenários iniciais via UDataTable (CSV ou JSON).
 */

USTRUCT(BlueprintType)
struct STRATEGOSCORE_API FProvinceRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Province")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Province")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Province")
	FName OwnerNationId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Province")
	TArray<FName> AdjacentProvinceIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Province")
	FVector2D MapPosition = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Province")
	ETerrainType Terrain = ETerrainType::Plains;
};

USTRUCT(BlueprintType)
struct STRATEGOSCORE_API FNationRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nation")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nation")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nation")
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nation")
	FName CapitalProvinceId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nation")
	bool bIsPlayerControlled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nation")
	TArray<FName> NationalIdeas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nation")
	FName StartingLeaderName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nation")
	ELeaderArchetype StartingLeaderArchetype = ELeaderArchetype::Pragmatist;
};

USTRUCT(BlueprintType)
struct STRATEGOSCORE_API FArmyRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Army")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Army")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Army")
	FName OwnerNationId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Army")
	FName StartingProvinceId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Army")
	int32 ManpowerCount = 1000;
};

/**
 * Cada FNationalIdeaRow define quanto uma tag (NationalIdea) inclina a
 * sucessão para cada arquétipo. Um país com NationalIdea "Martial" e
 * "Mercantile" terá +Militarist e +Merchant na distribuição de líderes.
 */
USTRUCT(BlueprintType)
struct STRATEGOSCORE_API FNationalIdeaRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NationalIdea")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NationalIdea")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NationalIdea")
	TMap<ELeaderArchetype, float> ArchetypeBonus;
};
