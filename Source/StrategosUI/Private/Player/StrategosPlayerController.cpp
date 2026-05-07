#include "Player/StrategosPlayerController.h"
#include "StrategosUI.h"
#include "Map/MapSubsystem.h"
#include "Strategy/MilitarySubsystem.h"
#include "World/WorldState.h"
#include "World/Province.h"
#include "World/Army.h"
#include "World/Nation.h"
#include "Game/StrategosGameState.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/World.h"

AStrategosPlayerController::AStrategosPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}

void AStrategosPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UEnhancedInputLocalPlayerSubsystem* EIS =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (UInputMappingContext* IMC = SelectionMappingContext.LoadSynchronous())
		{
			EIS->AddMappingContext(IMC, 1);
		}
	}
}

void AStrategosPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (UInputAction* Sel = SelectClickAction.LoadSynchronous())
		{
			EIC->BindAction(Sel, ETriggerEvent::Started, this, &AStrategosPlayerController::OnSelectClicked);
		}
		if (UInputAction* Ord = OrderMoveAction.LoadSynchronous())
		{
			EIC->BindAction(Ord, ETriggerEvent::Started, this, &AStrategosPlayerController::OnOrderMoveClicked);
		}
	}
}

UWorldState* AStrategosPlayerController::ResolveWorldState() const
{
	const UWorld* World = GetWorld();
	if (!World) return nullptr;
	AStrategosGameState* GS = World->GetGameState<AStrategosGameState>();
	return GS ? GS->GetWorldState() : nullptr;
}

UMapSubsystem* AStrategosPlayerController::ResolveMap() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetSubsystem<UMapSubsystem>() : nullptr;
}

UMilitarySubsystem* AStrategosPlayerController::ResolveMilitary() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetSubsystem<UMilitarySubsystem>() : nullptr;
}

FName AStrategosPlayerController::GetProvinceUnderCursor() const
{
	// Para o MVP, AStrategosProvinceVisualActor reporta seleção/hover diretamente
	// ao MapSubsystem via OnClicked. O PlayerController consulta o estado já lá.
	const UMapSubsystem* Map = ResolveMap();
	return Map ? Map->GetHoveredProvinceId() : NAME_None;
}

FName AStrategosPlayerController::FindArmyInProvince(FName ProvinceId, FName OwnerNationId) const
{
	UWorldState* World = ResolveWorldState();
	if (!World) return NAME_None;

	for (const auto& Pair : World->Armies)
	{
		const UArmy* A = Pair.Value.Get();
		if (!A) continue;
		if (A->CurrentProvinceId == ProvinceId && A->OwnerNationId == OwnerNationId)
		{
			return A->Id;
		}
	}
	return NAME_None;
}

void AStrategosPlayerController::OnSelectClicked(const FInputActionValue& Value)
{
	const FName ProvinceId = GetProvinceUnderCursor();
	if (ProvinceId.IsNone()) return;

	UWorldState* World = ResolveWorldState();
	if (!World) return;

	if (UMapSubsystem* Map = ResolveMap())
	{
		Map->SelectProvince(ProvinceId);
	}
	SelectedProvinceId = ProvinceId;

	// Se prov. tem exército do jogador, seleciona-o.
	const FName PlayerNation = World->PlayerNationId;
	const FName ArmyId = FindArmyInProvince(ProvinceId, PlayerNation);
	SelectedArmyId = ArmyId;

	UE_LOG(LogStrategosUI, Verbose, TEXT("Selected prov=%s army=%s"),
		*ProvinceId.ToString(), *ArmyId.ToString());
}

void AStrategosPlayerController::OnOrderMoveClicked(const FInputActionValue& Value)
{
	if (SelectedArmyId.IsNone()) return;

	const FName TargetProvinceId = GetProvinceUnderCursor();
	if (TargetProvinceId.IsNone()) return;

	if (UMilitarySubsystem* Military = ResolveMilitary())
	{
		const EMoveOrderResult R = Military->IssueMoveOrder(SelectedArmyId, TargetProvinceId);
		UE_LOG(LogStrategosUI, Log, TEXT("Order move %s -> %s: %s"),
			*SelectedArmyId.ToString(),
			*TargetProvinceId.ToString(),
			*UEnum::GetValueAsString(R));
	}
}
