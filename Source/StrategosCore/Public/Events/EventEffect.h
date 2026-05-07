#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Events/EventContext.h"
#include "EventEffect.generated.h"

class UWorldState;

/**
 * UEventEffect — Mutação plugável aplicada quando um evento resolve.
 *
 * Notification/Silent: aplicam todos UEventAsset.AutoEffects.
 * Decision: aplicam apenas FEventChoice.Effects da escolha selecionada.
 *
 * Effects podem encadear outros eventos (Effect_FireEvent), permitindo
 * narrativas multi-passo sem hardcoding em C++.
 */
UCLASS(Abstract, EditInlineNew, BlueprintType, DefaultToInstanced, CollapseCategories)
class STRATEGOSCORE_API UEventEffect : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Event|Effect")
	void Apply(UWorldState* WorldState, const FEventContext& Context);

	virtual void Apply_Implementation(UWorldState* WorldState, const FEventContext& Context) {}
};
