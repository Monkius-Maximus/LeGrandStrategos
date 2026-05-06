#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StrategosArmyVisualActor.generated.h"

class UStaticMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UTexture2D;

/**
 * AStrategosArmyVisualActor — Representação visual de um UArmy.
 *
 * Stage 1 (MVP): mesh placeholder + material com vector parameter
 * "BaseColor" (cor do dono) e texture parameter "BaseTexture"
 * (placeholder opcional, ex.: ícone de pelotão). Designer/teste
 * aponta uma UTexture2D em BP child sem alterar C++.
 *
 * O AStrategosMapActor reposiciona esses atores quando o exército
 * arriva numa nova província. Interpolação durante movimento entra
 * quando o tactical ficar mais maduro.
 */
UCLASS()
class STRATEGOSUI_API AStrategosArmyVisualActor : public AActor
{
	GENERATED_BODY()

public:
	AStrategosArmyVisualActor();

	UFUNCTION(BlueprintCallable, Category = "Strategos|Army")
	void InitializeFromArmy(FName InArmyId, FName InOwnerNationId);

	UFUNCTION(BlueprintCallable, Category = "Strategos|Army")
	void SetOwnerColor(const FLinearColor& Color);

	UFUNCTION(BlueprintPure, Category = "Strategos|Army")
	FName GetArmyId() const { return ArmyId; }

	UFUNCTION(BlueprintPure, Category = "Strategos|Army")
	FName GetOwnerNationId() const { return OwnerNationId; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Strategos|Army|Visual")
	TSoftObjectPtr<UMaterialInterface> ArmyMaterial;

	/** Placeholder de imagem opcional. Se setado, é aplicado ao MID em BeginPlay. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Strategos|Army|Visual")
	TSoftObjectPtr<UTexture2D> PlaceholderTexture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Strategos|Army|Visual")
	float HoverHeight = 50.f;

private:
	UPROPERTY() FName ArmyId;
	UPROPERTY() FName OwnerNationId;
	UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;
	UPROPERTY() FLinearColor BaseColor = FLinearColor::White;
};
