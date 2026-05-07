#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Events/EventContext.h"
#include "EventCondition.generated.h"

class UWorldState;

/**
 * UEventCondition — Predicado plugável avaliado quando um evento dispara.
 *
 * EditInlineNew permite que UEventAsset segure instâncias inline editáveis
 * no editor. BlueprintNativeEvent permite que designers escrevam condições
 * em Blueprint sem tocar C++ (override do _Implementation).
 *
 * Convenção: condições NÃO mutam estado. Apenas leitura. Mutações vão para
 * UEventEffect.
 */
UCLASS(Abstract, EditInlineNew, BlueprintType, DefaultToInstanced, CollapseCategories)
class STRATEGOSCORE_API UEventCondition : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Event|Condition")
	bool Evaluate(UWorldState* WorldState, const FEventContext& Context);

	virtual bool Evaluate_Implementation(UWorldState* WorldState, const FEventContext& Context) { return true; }
};
