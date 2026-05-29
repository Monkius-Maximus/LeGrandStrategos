#include "BattleCameraPawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/InputComponent.h"

ABattleCameraPawn::ABattleCameraPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength        = 800.f;
	SpringArm->bDoCollisionTest       = false;
	SpringArm->bInheritPitch          = false;
	SpringArm->bInheritRoll           = false;
	SpringArm->bInheritYaw            = false;
	SpringArm->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	CameraComponent->ProjectionMode = ECameraProjectionMode::Orthographic;
	CameraComponent->OrthoWidth     = 1920.f;
}

void ABattleCameraPawn::BeginPlay()
{
	Super::BeginPlay();
}

void ABattleCameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("CameraPanX", this, &ABattleCameraPawn::PanX);
	PlayerInputComponent->BindAxis("CameraPanY", this, &ABattleCameraPawn::PanY);
	PlayerInputComponent->BindAxis("CameraZoom", this, &ABattleCameraPawn::Zoom);
}

void ABattleCameraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!PanInput.IsZero())
	{
		const FVector Delta(PanInput.Y, PanInput.X, 0.f);
		AddActorWorldOffset(Delta * PanSpeed * DeltaTime);
		PanInput = FVector2D::ZeroVector;
	}

	if (FMath::Abs(ZoomInput) > KINDA_SMALL_NUMBER)
	{
		const float NewLength = FMath::Clamp(
			SpringArm->TargetArmLength + ZoomInput * ZoomSpeed * DeltaTime,
			MinArmLength, MaxArmLength);
		SpringArm->TargetArmLength = NewLength;
		ZoomInput = 0.f;
	}
}

void ABattleCameraPawn::PanX(float Value)
{
	PanInput.X = Value;
}

void ABattleCameraPawn::PanY(float Value)
{
	PanInput.Y = Value;
}

void ABattleCameraPawn::Zoom(float Value)
{
	ZoomInput = Value;
}
