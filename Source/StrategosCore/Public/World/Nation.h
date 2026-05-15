#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "World/LeaderArchetype.h"
#include "Economy/Treasury.h"
#include "Economy/NationalStockpile.h"
#include "Economy/StrategicIndices.h"
#include "Nation.generated.h"

class ULeader;
class UTexture2D;

/**
 * UNation — entidade política. No MVP detém id, nome, cor, províncias,
 * o líder atual e suas NationalIdeas (que inclinam a sucessão).
 *
 * Economia, política interna, diplomacia detalhada chegam nas etapas
 * seguintes do roadmap.
 */
UCLASS(BlueprintType)
class STRATEGOSCORE_API UNation : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Nation")
	FName Id;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Nation")
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Nation")
	FLinearColor Color = FLinearColor::White;

	/** Cor de trim/detalhe — usada em gradientes e ornamentos da UI. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Nation")
	FLinearColor SecondaryColor = FLinearColor::Gray;

	/** Bandeira nacional. Arte 256×192 (proporção 4:3 real). */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Nation|Identity")
	TSoftObjectPtr<UTexture2D> FlagTexture;

	/** Brasão circular. Arte 128×128. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Nation|Identity")
	TSoftObjectPtr<UTexture2D> CoatOfArmsIcon;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Nation")
	FName CapitalProvinceId;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Nation")
	TArray<FName> OwnedProvinceIds;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Nation")
	bool bIsPlayerControlled = false;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Nation|Leadership")
	TObjectPtr<ULeader> CurrentLeader;

	/** Tags como "Martial", "Mercantile", "Diplomatic" — pesam a sucessão de líderes. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Nation|Leadership")
	TArray<FName> NationalIdeas;

	/**
	 * Afinidade base de cada arquétipo. As NationalIdeas somam bônus a esse mapa
	 * via UNationalIdeaRegistry; o resultado é a distribuição de probabilidade
	 * usada pelo UAIPlaceholderSubsystem na sucessão.
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Nation|Leadership")
	TMap<ELeaderArchetype, float> ArchetypeAffinity;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Nation|Economy")
	FTreasury Treasury;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Nation|Economy")
	FNationalStockpile Stockpile;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Nation|Economy")
	FStrategicIndices StrategicIndices;
};
