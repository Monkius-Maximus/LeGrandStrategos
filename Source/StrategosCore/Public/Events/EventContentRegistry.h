#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EventContentRegistry.generated.h"

class UEventAsset;

/**
 * UEventContentRegistry — Catálogo de UEventAsset disponíveis no jogo.
 *
 * O EventSubsystem consulta este asset em Initialize para popular seu
 * índice por TriggerTag. Designers/modders editam um único asset
 * (`DA_EventRegistry`) para ativar/desativar eventos.
 *
 * Se nenhum registry é apontado, o EventSubsystem cai num fallback de
 * 5 eventos hardcoded — o jogo funciona sem assets.
 */
UCLASS(BlueprintType)
class STRATEGOSCORE_API UEventContentRegistry : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Registry")
	TArray<TSoftObjectPtr<UEventAsset>> Events;
};
