#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StrategosMapActor.generated.h"

class AStrategosProvinceVisualActor;
class UWorldState;

/**
 * AStrategosMapActor — Orquestra a visualização do mapa estratégico.
 *
 * Em BeginPlay (após o WorldState estar bootstrapped), itera as
 * províncias e spawna um AStrategosProvinceVisualActor por uma; idem
 * para exércitos, spawnando AStrategosArmyVisualActor.
 *
 * Mantém maps id → actor para que mudanças em UMapSubsystem
 * (selected/hovered) sejam refletidas em todos os visuais sem precisar
 * varrer o nível.
 */
UCLASS()
class STRATEGOSUI_API AStrategosMapActor : public AActor
{
	GENERATED_BODY()

public:
	AStrategosMapActor();

	UFUNCTION(BlueprintCallable, Category = "Strategos|Map")
	void RefreshAllOwnerColors();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Strategos|Map|Spawn")
	TSubclassOf<AStrategosProvinceVisualActor> ProvinceVisualClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Strategos|Map|Spawn")
	float SpawnDelaySeconds = 0.1f;

private:
	void BuildVisuals();

	UFUNCTION()
	void HandleProvinceSelected(FName ProvinceId);

	UFUNCTION()
	void HandleProvinceHovered(FName ProvinceId);

	UWorldState* ResolveWorldState() const;

	UPROPERTY()
	TMap<FName, TObjectPtr<AStrategosProvinceVisualActor>> ProvinceVisuals;

	UPROPERTY()
	FName CurrentSelected;

	UPROPERTY()
	FName CurrentHovered;

	FTimerHandle BuildTimer;
};
