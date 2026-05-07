#include "Army/StrategosArmyVisualActor.h"
#include "StrategosUI.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Texture2D.h"

AStrategosArmyVisualActor::AStrategosArmyVisualActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AStrategosArmyVisualActor::BeginPlay()
{
	Super::BeginPlay();

	if (UMaterialInterface* Mat = ArmyMaterial.LoadSynchronous())
	{
		DynamicMaterial = UMaterialInstanceDynamic::Create(Mat, this);
		Mesh->SetMaterial(0, DynamicMaterial);

		if (UTexture2D* Tex = PlaceholderTexture.LoadSynchronous())
		{
			DynamicMaterial->SetTextureParameterValue(TEXT("BaseTexture"), Tex);
		}
		DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), BaseColor);
	}
}

void AStrategosArmyVisualActor::InitializeFromArmy(FName InArmyId, FName InOwnerNationId)
{
	ArmyId = InArmyId;
	OwnerNationId = InOwnerNationId;
}

void AStrategosArmyVisualActor::SetOwnerColor(const FLinearColor& Color)
{
	BaseColor = Color;
	if (DynamicMaterial)
	{
		DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), BaseColor);
	}
}
