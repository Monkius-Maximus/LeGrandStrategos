#include "Camera/StrategosCameraPawn.h"
#include "StrategosUI.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

AStrategosCameraPawn::AStrategosCameraPawn()
{
	PrimaryActorTick.bCanEverTick = false;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = RootSceneComponent;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 2000.f;
	SpringArm->SetRelativeRotation(FRotator(-70.f, 0.f, 0.f));
	SpringArm->bDoCollisionTest = false;
	SpringArm->bUsePawnControlRotation = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
}

void AStrategosCameraPawn::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* EIS =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (UInputMappingContext* IMC = DefaultMappingContext.LoadSynchronous())
			{
				EIS->AddMappingContext(IMC, 0);
			}
		}
	}
}

void AStrategosCameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (UInputAction* Move = MoveAction.LoadSynchronous())
		{
			EIC->BindAction(Move, ETriggerEvent::Triggered, this, &AStrategosCameraPawn::OnMove);
		}
		if (UInputAction* Zoom = ZoomAction.LoadSynchronous())
		{
			EIC->BindAction(Zoom, ETriggerEvent::Triggered, this, &AStrategosCameraPawn::OnZoom);
		}
	}
}

void AStrategosCameraPawn::OnMove(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (Axis.IsNearlyZero()) return;

	const float Dt = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f;
	const FVector Delta(Axis.Y * PanSpeed * Dt, Axis.X * PanSpeed * Dt, 0.f);
	AddActorWorldOffset(Delta);
}

void AStrategosCameraPawn::OnZoom(const FInputActionValue& Value)
{
	const float Axis = Value.Get<float>();
	if (FMath::IsNearlyZero(Axis)) return;

	const float NewLen = FMath::Clamp(
		SpringArm->TargetArmLength - Axis * ZoomStep,
		MinZoomDistance,
		MaxZoomDistance);
	SpringArm->TargetArmLength = NewLen;
}

void AStrategosCameraPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}
