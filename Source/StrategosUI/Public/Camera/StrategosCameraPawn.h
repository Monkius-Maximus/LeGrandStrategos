#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "StrategosCameraPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

/**
 * AStrategosCameraPawn — Câmera estratégica top-down.
 *
 * Estrutura: Root (USceneComponent) → SpringArm (pitch fixo top-down,
 * length variável p/ zoom) → Camera. Movimento panela WASD em 2D no
 * plano XY do mundo, scroll modula SpringArmLength.
 *
 * Os Input Actions e o Mapping Context são apontados via UPROPERTY
 * (TSoftObjectPtr) e ligados em editor — ver docs/setup. Limites de
 * altura e velocidade ficam expostos para tuning por BP child.
 */
UCLASS()
class STRATEGOSUI_API AStrategosCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	AStrategosCameraPawn();

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> RootSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Strategos|Camera|Input")
	TSoftObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Strategos|Camera|Input")
	TSoftObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Strategos|Camera|Input")
	TSoftObjectPtr<UInputAction> ZoomAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Strategos|Camera|Tuning")
	float PanSpeed = 2000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Strategos|Camera|Tuning")
	float ZoomStep = 250.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Strategos|Camera|Tuning")
	float MinZoomDistance = 500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Strategos|Camera|Tuning")
	float MaxZoomDistance = 5000.f;

private:
	void OnMove(const FInputActionValue& Value);
	void OnZoom(const FInputActionValue& Value);
};
