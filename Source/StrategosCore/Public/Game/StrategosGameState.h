#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "StrategosGameState.generated.h"

/**
 * AStrategosGameState — container do estado replicável do jogo.
 *
 * Stage 0: stub. Será expandido na Etapa 1 do roadmap quando entrarem
 * UWorldState, UNation, UProvince e demais containers de simulação.
 */
UCLASS()
class STRATEGOSCORE_API AStrategosGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AStrategosGameState();
};
