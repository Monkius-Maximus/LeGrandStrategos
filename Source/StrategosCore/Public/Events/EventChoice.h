#pragma once

#include "CoreMinimal.h"
#include "Events/EventCondition.h"
#include "EventChoice.generated.h"

class UEventEffect;

/**
 * FEventChoice — Uma das opções apresentadas ao player num evento Decision.
 *
 * Effects são aplicados em ordem quando esta escolha é selecionada.
 * Tooltip serve para HUD explicar o trade-off antes da seleção.
 * AvailabilityConditions determinam se a choice aparece habilitada no modal
 * (estilo EU4/Victoria 3: botões bloqueados com tooltip explicativo).
 */
USTRUCT(BlueprintType)
struct STRATEGOSCORE_API FEventChoice
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
	FText Label;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice", meta = (MultiLine = "true"))
	FText Tooltip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Choice")
	TArray<TObjectPtr<UEventEffect>> Effects;

	/** Se qualquer condição falhar, a choice aparece bloqueada (bAvailable=false). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Choice|Disponibilidade")
	TArray<TObjectPtr<UEventCondition>> AvailabilityConditions;

	/** Substituição do Tooltip quando bAvailable=false. Ex.: "Requer: 500 Ouro". */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice|Disponibilidade",
		meta = (MultiLine = "true"))
	FText UnavailableTooltip;
};
