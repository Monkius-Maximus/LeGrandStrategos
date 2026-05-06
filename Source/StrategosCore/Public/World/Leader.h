#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "World/LeaderArchetype.h"
#include "Leader.generated.h"

/**
 * ULeader — chefe de governo de uma UNation.
 *
 * Detém id, nome, arquétipo (personalidade que dirige o comportamento da IA)
 * e datas. O AIPlaceholderSubsystem consulta o Archetype para decidir ações.
 * Sucessão acontece anualmente via mesmo subsistema, com weights modificados
 * pelas NationalIdeas da nação.
 */
UCLASS(BlueprintType)
class STRATEGOSCORE_API ULeader : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Leader")
	FName Id;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leader")
	FText DisplayName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leader")
	ELeaderArchetype Archetype = ELeaderArchetype::Pragmatist;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leader")
	int32 BirthYear = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Leader")
	int32 AscensionYear = 0;
};
