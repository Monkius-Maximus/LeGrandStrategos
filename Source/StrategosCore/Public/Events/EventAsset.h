#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Events/EventType.h"
#include "Events/EventCondition.h"
#include "Events/EventEffect.h"
#include "Events/EventChoice.h"
#include "EventAsset.generated.h"

class UTexture2D;

/**
 * UEventAsset — Definição completa de um evento como DataAsset.
 *
 * Disparo: o EventSubsystem indexa todos os UEventAsset por TriggerTag.
 * Quando o trigger correspondente acontece (Time.Month, Economy.Bankruptcy,
 * Military.ArmyArrived, etc), avalia Conditions; se todas passam:
 *  - Decision     → enfileira para resolução do player; AI auto-resolve
 *  - Notification → aplica AutoEffects e gera popup
 *  - Silent       → aplica AutoEffects sem UI
 *
 * MeanTimeToHappen (em meses) implementa probabilistic gate: cada vez
 * que o trigger dispara e conditions passam, há chance ≈ 1/MTTH de
 * efetivar. MTTH=0 → sempre. MTTH=12 → ~1/12 por trigger.
 */
UCLASS(BlueprintType)
class STRATEGOSCORE_API UEventAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event", meta = (MultiLine = "true"))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event")
	TSoftObjectPtr<UTexture2D> Image;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event")
	EEventType Type = EEventType::Notification;

	/**
	 * Categoria narrativa do evento para cor e filtro no modal.
	 * Valores canônicos: "economic" | "political" | "military" | "diplomatic" | "notification".
	 * Se NAME_None, o modal deriva da Type como fallback.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event")
	FName Category;

	/** Tag canônica do trigger. Convenção "Subsystem.EventName". */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event|Trigger")
	FName TriggerTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event|Trigger", meta = (ClampMin = "0"))
	int32 MeanTimeToHappenMonths = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Event")
	TArray<TObjectPtr<UEventCondition>> Conditions;

	/** Apenas para Notification/Silent. Decision usa as Effects de cada Choice. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Event|Effects")
	TArray<TObjectPtr<UEventEffect>> AutoEffects;

	/** Apenas para Decision. Pelo menos 1 Choice. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event|Choices")
	TArray<FEventChoice> Choices;
};
