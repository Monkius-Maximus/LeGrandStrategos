#pragma once

#include "CoreMinimal.h"
#include "EventContext.generated.h"

/**
 * FEventContext — Carga útil que viaja com um evento desde a fonte do
 * trigger até as conditions/effects.
 *
 * Convenções:
 *  - SourceNationId     : nação à qual o evento se refere (alvo principal)
 *  - SourceProvinceId   : se aplicável (ex.: trigger de prédio completou)
 *  - SourceEntityId     : id genérico — exército, prédio, líder etc
 *  - FireDate           : data do mundo no momento do disparo (não real-time)
 *
 * Conditions e Effects recebem este struct + um UWorldState para fazer
 * lookups. Não há referências de objetos vivos para evitar dangling refs
 * em saves/loads.
 */
USTRUCT(BlueprintType)
struct STRATEGOSCORE_API FEventContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Event")
	FName EventId;

	UPROPERTY(BlueprintReadWrite, Category = "Event")
	FName TriggerTag;

	UPROPERTY(BlueprintReadWrite, Category = "Event")
	FName SourceNationId;

	UPROPERTY(BlueprintReadWrite, Category = "Event")
	FName SourceProvinceId;

	UPROPERTY(BlueprintReadWrite, Category = "Event")
	FName SourceEntityId;

	UPROPERTY(BlueprintReadWrite, Category = "Event")
	FDateTime FireDate;
};
