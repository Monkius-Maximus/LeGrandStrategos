#pragma once

#include "CoreMinimal.h"
#include "StrategosEvent.generated.h"

/**
 * FStrategosEvent — payload base de qualquer evento publicado pelo EventBus.
 *
 * Convenções:
 *  - EventTag identifica o tipo de evento (ex.: "Diplomacy.WarDeclared").
 *  - SourceId / TargetId são identificadores opcionais usados pela maioria
 *    dos eventos do jogo (nações, províncias, exércitos, etc).
 *  - Para payloads tipados, herde de FStrategosEvent e adicione campos.
 */
USTRUCT(BlueprintType)
struct STRATEGOSCORE_API FStrategosEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Strategos|Event")
	FName EventTag;

	UPROPERTY(BlueprintReadWrite, Category = "Strategos|Event")
	FName SourceId;

	UPROPERTY(BlueprintReadWrite, Category = "Strategos|Event")
	FName TargetId;

	FStrategosEvent() = default;

	explicit FStrategosEvent(FName InTag)
		: EventTag(InTag) {}

	FStrategosEvent(FName InTag, FName InSource, FName InTarget)
		: EventTag(InTag), SourceId(InSource), TargetId(InTarget) {}
};
