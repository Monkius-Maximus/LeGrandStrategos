#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WorldBootstrapper.generated.h"

class UWorldState;
class UWorldBootstrapAsset;

/**
 * UWorldBootstrapper — Aplica um cenário inicial em UWorldState.
 *
 * Duas entradas:
 *  - ApplyBootstrap(WorldState, Asset): lê DataTables apontadas pelo asset
 *    e materializa Provinces/Nations/Armies/NationalIdeas no WorldState.
 *  - ApplyDefaultSandbox(WorldState): cenário programático Albion/Galia/Norden
 *    (3 nações fictícias, ~10 províncias, 3 exércitos) usado como fallback
 *    quando nenhum asset é fornecido. Útil para teste sem editor.
 *
 * Idempotente: limpa WorldState antes de popular, então pode ser chamado
 * múltiplas vezes (ex.: novo jogo).
 */
UCLASS()
class STRATEGOSCORE_API UWorldBootstrapper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Strategos|Bootstrap")
	static bool ApplyBootstrap(UWorldState* WorldState, UWorldBootstrapAsset* Asset);

	UFUNCTION(BlueprintCallable, Category = "Strategos|Bootstrap")
	static void ApplyDefaultSandbox(UWorldState* WorldState);
};
