#pragma once

#include "CoreMinimal.h"
#include "EventChoice.generated.h"

class UEventEffect;

/**
 * FEventChoice — Uma das opções apresentadas ao player num evento Decision.
 *
 * Effects são aplicados em ordem quando esta escolha é selecionada.
 * Tooltip serve para HUD explicar o trade-off antes da seleção.
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
};
