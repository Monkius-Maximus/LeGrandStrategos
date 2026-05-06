#include "HUD/StrategosMainMenuWidget.h"
#include "StrategosUI.h"
#include "Foundation/GameFlow/GameFlowSubsystem.h"
#include "Save/SaveSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"

void UStrategosMainMenuWidget::NewGame(const FString& MapName)
{
	const UWorld* World = GetWorld();
	if (!World) return;

	if (UGameInstance* GI = World->GetGameInstance())
	{
		if (UGameFlowSubsystem* Flow = GI->GetSubsystem<UGameFlowSubsystem>())
		{
			Flow->TransitionTo(EGameFlowState::Loading);
		}
	}

	if (!MapName.IsEmpty())
	{
		UGameplayStatics::OpenLevel(World, FName(*MapName));
	}
}

void UStrategosMainMenuWidget::LoadGameFromSlot(const FString& SlotName)
{
	const UWorld* World = GetWorld();
	if (!World) return;

	UGameInstance* GI = World->GetGameInstance();
	if (!GI) return;

	if (USaveSubsystem* Save = GI->GetSubsystem<USaveSubsystem>())
	{
		Save->LoadFromSlot(SlotName);
	}

	if (UGameFlowSubsystem* Flow = GI->GetSubsystem<UGameFlowSubsystem>())
	{
		Flow->TransitionTo(EGameFlowState::Loading);
	}
}

void UStrategosMainMenuWidget::QuitGame()
{
	const UWorld* World = GetWorld();
	if (!World) return;

	if (APlayerController* PC = World->GetFirstPlayerController())
	{
		UKismetSystemLibrary::QuitGame(World, PC, EQuitPreference::Quit, false);
	}
}

bool UStrategosMainMenuWidget::DoesSlotExist(const FString& SlotName) const
{
	const UWorld* World = GetWorld();
	if (!World) return false;

	const UGameInstance* GI = World->GetGameInstance();
	if (!GI) return false;

	if (const USaveSubsystem* Save = GI->GetSubsystem<USaveSubsystem>())
	{
		return Save->DoesSaveExist(SlotName);
	}
	return false;
}
