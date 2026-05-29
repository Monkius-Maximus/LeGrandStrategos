#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BattleCameraPawn.generated.h"

/**
 * ABattleCameraPawn — câmera tática ortográfica para a batalha.
 * Etapa 9: skeleton com componentes e parâmetros expostos.
 * Input mapping e zoom suave entram quando o Enhanced Input estiver configurado.
 */
UCLASS()
class STRATEGOSBATTLE_API ABattleCameraPawn : public APawn
{
	GENERATED_BODY()
public:
	ABattleCameraPawn();

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<class USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<class UCameraComponent> CameraComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera")
	float PanSpeed = 500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera")
	float ZoomSpeed = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera")
	float MinArmLength = 300.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera")
	float MaxArmLength = 2000.f;

	/** Mover horizontalmente. */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void PanX(float Value);

	/** Mover verticalmente. */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void PanY(float Value);

	/** Aproximar/afastar câmera. */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void Zoom(float Value);

private:
	FVector2D PanInput;
	float     ZoomInput = 0.f;
};
