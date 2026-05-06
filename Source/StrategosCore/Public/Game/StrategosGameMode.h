#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "StrategosGameMode.generated.h"

class UWorldBootstrapAsset;

UCLASS()
class STRATEGOSCORE_API AStrategosGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AStrategosGameMode();

	/**
	 * Cenário a aplicar em BeginPlay. Se vazio, o GameMode chama
	 * UWorldBootstrapper::ApplyDefaultSandbox como fallback.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Strategos|Bootstrap")
	TSoftObjectPtr<UWorldBootstrapAsset> BootstrapAsset;

	/** Se true, ignora BootstrapAsset e força o sandbox programático. Útil em smoke tests. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Strategos|Bootstrap")
	bool bForceDefaultSandbox = false;

protected:
	virtual void BeginPlay() override;

	void RunBootstrap();
};
