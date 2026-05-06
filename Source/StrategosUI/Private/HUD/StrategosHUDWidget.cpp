#include "HUD/StrategosHUDWidget.h"
#include "StrategosUI.h"
#include "Foundation/Time/TimeSubsystem.h"
#include "Map/MapSubsystem.h"
#include "Save/SaveSubsystem.h"
#include "World/WorldState.h"
#include "World/Province.h"
#include "World/Nation.h"
#include "Game/StrategosGameState.h"
#include "Engine/World.h"

void UStrategosHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UMapSubsystem* Map = ResolveMap())
	{
		Map->OnProvinceSelected.AddDynamic(this, &UStrategosHUDWidget::HandleProvinceSelected);
	}
}

void UStrategosHUDWidget::NativeDestruct()
{
	if (UMapSubsystem* Map = ResolveMap())
	{
		Map->OnProvinceSelected.RemoveDynamic(this, &UStrategosHUDWidget::HandleProvinceSelected);
	}
	Super::NativeDestruct();
}

UTimeSubsystem* UStrategosHUDWidget::ResolveTime() const
{
	const UWorld* World = GetWorld();
	if (!World) return nullptr;
	const UGameInstance* GI = World->GetGameInstance();
	return GI ? GI->GetSubsystem<UTimeSubsystem>() : nullptr;
}

UMapSubsystem* UStrategosHUDWidget::ResolveMap() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetSubsystem<UMapSubsystem>() : nullptr;
}

USaveSubsystem* UStrategosHUDWidget::ResolveSave() const
{
	const UWorld* World = GetWorld();
	if (!World) return nullptr;
	const UGameInstance* GI = World->GetGameInstance();
	return GI ? GI->GetSubsystem<USaveSubsystem>() : nullptr;
}

void UStrategosHUDWidget::PauseGame()
{
	if (UTimeSubsystem* Time = ResolveTime())
	{
		Time->Pause();
	}
}

void UStrategosHUDWidget::ResumeGame()
{
	if (UTimeSubsystem* Time = ResolveTime())
	{
		Time->Resume();
	}
}

void UStrategosHUDWidget::SetTimeSpeed(ETimeSpeed NewSpeed)
{
	if (UTimeSubsystem* Time = ResolveTime())
	{
		Time->SetSpeed(NewSpeed);
	}
}

void UStrategosHUDWidget::RequestSave(const FString& SlotName)
{
	if (USaveSubsystem* Save = ResolveSave())
	{
		Save->SaveToSlot(SlotName);
	}
}

void UStrategosHUDWidget::RequestLoad(const FString& SlotName)
{
	if (USaveSubsystem* Save = ResolveSave())
	{
		Save->LoadFromSlot(SlotName);
	}
}

FText UStrategosHUDWidget::GetCurrentDateText() const
{
	if (const UTimeSubsystem* Time = ResolveTime())
	{
		const FDateTime D = Time->GetCurrentDate();
		return FText::FromString(FString::Printf(TEXT("%04d-%02d-%02d"),
			D.GetYear(), D.GetMonth(), D.GetDay()));
	}
	return FText::GetEmpty();
}

ETimeSpeed UStrategosHUDWidget::GetCurrentSpeed() const
{
	if (const UTimeSubsystem* Time = ResolveTime())
	{
		return Time->GetSpeed();
	}
	return ETimeSpeed::Paused;
}

FText UStrategosHUDWidget::GetSelectedProvinceName() const
{
	const UMapSubsystem* Map = ResolveMap();
	if (!Map) return FText::GetEmpty();
	const FName ProvId = Map->GetSelectedProvinceId();

	const UWorld* World = GetWorld();
	if (!World) return FText::GetEmpty();
	AStrategosGameState* GS = World->GetGameState<AStrategosGameState>();
	if (!GS || !GS->GetWorldState()) return FText::GetEmpty();

	if (const UProvince* P = GS->GetWorldState()->GetProvince(ProvId))
	{
		return P->DisplayName;
	}
	return FText::GetEmpty();
}

FText UStrategosHUDWidget::GetSelectedProvinceOwnerName() const
{
	const UMapSubsystem* Map = ResolveMap();
	if (!Map) return FText::GetEmpty();
	const FName ProvId = Map->GetSelectedProvinceId();

	const UWorld* World = GetWorld();
	if (!World) return FText::GetEmpty();
	AStrategosGameState* GS = World->GetGameState<AStrategosGameState>();
	if (!GS || !GS->GetWorldState()) return FText::GetEmpty();

	const UProvince* P = GS->GetWorldState()->GetProvince(ProvId);
	if (!P) return FText::GetEmpty();
	if (const UNation* N = GS->GetWorldState()->GetNation(P->OwnerNationId))
	{
		return N->DisplayName;
	}
	return FText::GetEmpty();
}

void UStrategosHUDWidget::HandleProvinceSelected(FName ProvinceId)
{
	OnSelectionChanged(ProvinceId);
}
