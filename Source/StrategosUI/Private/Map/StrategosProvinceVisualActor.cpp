#include "Map/StrategosProvinceVisualActor.h"
#include "StrategosUI.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Map/MapSubsystem.h"
#include "Engine/World.h"

AStrategosProvinceVisualActor::AStrategosProvinceVisualActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	Mesh->SetGenerateOverlapEvents(false);
}

void AStrategosProvinceVisualActor::BeginPlay()
{
	Super::BeginPlay();

	if (UMaterialInterface* Mat = ProvinceMaterial.LoadSynchronous())
	{
		DynamicMaterial = UMaterialInstanceDynamic::Create(Mat, this);
		Mesh->SetMaterial(0, DynamicMaterial);
	}
	RefreshAppliedColor();
}

void AStrategosProvinceVisualActor::InitializeFromProvince(FName InProvinceId, FVector2D MapPosition)
{
	ProvinceId = InProvinceId;
	const FVector World = FVector(
		MapPosition.X * WorldUnitsPerMapCell,
		MapPosition.Y * WorldUnitsPerMapCell,
		0.f);
	SetActorLocation(World);
}

void AStrategosProvinceVisualActor::SetOwnerColor(const FLinearColor& Color)
{
	BaseColor = Color;
	RefreshAppliedColor();
}

void AStrategosProvinceVisualActor::SetSelected(bool bSelected)
{
	if (bIsSelected == bSelected) return;
	bIsSelected = bSelected;
	RefreshAppliedColor();
}

void AStrategosProvinceVisualActor::SetHovered(bool bHovered)
{
	if (bIsHovered == bHovered) return;
	bIsHovered = bHovered;
	RefreshAppliedColor();
}

void AStrategosProvinceVisualActor::RefreshAppliedColor()
{
	if (!DynamicMaterial) return;

	float Mult = 1.f;
	if (bIsSelected) Mult = SelectedBrightnessMultiplier;
	else if (bIsHovered) Mult = HoverBrightnessMultiplier;

	const FLinearColor Final = BaseColor * Mult;
	DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), Final);
}

void AStrategosProvinceVisualActor::NotifyActorOnClicked(FKey)
{
	if (UMapSubsystem* Map = GetWorld()->GetSubsystem<UMapSubsystem>())
	{
		Map->SelectProvince(ProvinceId);
	}
}

void AStrategosProvinceVisualActor::NotifyActorBeginCursorOver()
{
	if (UMapSubsystem* Map = GetWorld()->GetSubsystem<UMapSubsystem>())
	{
		Map->HoverProvince(ProvinceId);
	}
}

void AStrategosProvinceVisualActor::NotifyActorEndCursorOver()
{
	if (UMapSubsystem* Map = GetWorld()->GetSubsystem<UMapSubsystem>())
	{
		if (Map->GetHoveredProvinceId() == ProvinceId)
		{
			Map->HoverProvince(NAME_None);
		}
	}
}
