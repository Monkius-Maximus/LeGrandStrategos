#pragma once

#include "CoreMinimal.h"
#include "EventHistory.generated.h"

/**
 * FFiredEventRecord — Uma entrada no log de eventos já disparados.
 *
 * ChoiceIndex fica INDEX_NONE entre o disparo de uma Decision e sua
 * resolução; é preenchido quando o player (ou a IA) escolhe.
 */
USTRUCT(BlueprintType)
struct STRATEGOSCORE_API FFiredEventRecord
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Event|History")
	FName EventId;

	UPROPERTY(BlueprintReadOnly, Category = "Event|History")
	FName NationId;

	UPROPERTY(BlueprintReadOnly, Category = "Event|History")
	FDateTime FireDate;

	/** INDEX_NONE enquanto a Decision não foi resolvida; Notification/Silent nunca preenchem. */
	UPROPERTY(BlueprintReadOnly, Category = "Event|History")
	int32 ChoiceIndex = INDEX_NONE;
};

/** Par (EventId, data em que o cooldown expira). Forma serializável do TMap runtime. */
USTRUCT()
struct STRATEGOSCORE_API FEventCooldownRecord
{
	GENERATED_BODY()

	UPROPERTY() FName EventId;
	UPROPERTY() FDateTime Until;
};

/**
 * FNationEventState — Memória de disparos de uma nação.
 *
 * EverFired é intencionalmente separado do log de histórico: o log tem teto
 * de tamanho e descarta entradas antigas, então não serve para responder
 * "esse evento já aconteceu alguma vez?". EverFired é limitado pelo número
 * de eventos distintos, então cabe na memória sem teto.
 */
USTRUCT()
struct STRATEGOSCORE_API FNationEventState
{
	GENERATED_BODY()

	UPROPERTY() TSet<FName> EverFired;

	UPROPERTY() TMap<FName, FDateTime> CooldownUntil;
};

/** Forma flat de FNationEventState para o snapshot de save. */
USTRUCT()
struct STRATEGOSCORE_API FNationEventStateRecord
{
	GENERATED_BODY()

	UPROPERTY() FName NationId;
	UPROPERTY() TArray<FName> EverFiredEventIds;
	UPROPERTY() TArray<FEventCooldownRecord> Cooldowns;
};
