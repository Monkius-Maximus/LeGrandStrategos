#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StrategosProvinceVisualActor.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;

/**
 * AStrategosProvinceVisualActor — Representação visual de uma UProvince.
 *
 * Spawnado pelo AStrategosMapActor, um por província. Tem um
 * UStaticMeshComponent (plane por default) com Material Instance
 * Dynamic; o parâmetro vector "BaseColor" reflete a cor do dono
 * (atualizado via SetOwnerColor).
 *
 * Repassa hover/click para UMapSubsystem (HoverProvince/SelectProvince).
 */
UCLASS()
class STRATEGOSUI_API AStrategosProvinceVisualActor : public AActor
{
	GENERATED_BODY()

public:
	AStrategosProvinceVisualActor();

	UFUNCTION(BlueprintCallable, Category = "Strategos|Map")
	void InitializeFromProvince(FName InProvinceId, FVector2D MapPosition);

	UFUNCTION(BlueprintCallable, Category = "Strategos|Map")
	void SetOwnerColor(const FLinearColor& Color);

	UFUNCTION(BlueprintCallable, Category = "Strategos|Map")
	void SetSelected(bool bSelected);

	UFUNCTION(BlueprintCallable, Category = "Strategos|Map")
	void SetHovered(bool bHovered);

	UFUNCTION(BlueprintPure, Category = "Strategos|Map")
	FName GetProvinceId() const { return ProvinceId; }

protected:
	virtual void BeginPlay() override;
	virtual void NotifyActorOnClicked(FKey ButtonPressed = EKeys::LeftMouseButton) override;
	virtual void NotifyActorBeginCursorOver() override;
	virtual void NotifyActorEndCursorOver() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Strategos|Visual")
	TSoftObjectPtr<UMaterialInterface> ProvinceMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Strategos|Visual")
	float WorldUnitsPerMapCell = 1000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Strategos|Visual")
	float HoverBrightnessMultiplier = 1.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Strategos|Visual")
	float SelectedBrightnessMultiplier = 1.5f;

private:
	void RefreshAppliedColor();

	UPROPERTY()
	FName ProvinceId;

	UPROPERTY()
	FLinearColor BaseColor = FLinearColor::Gray;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

	bool bIsSelected = false;
	bool bIsHovered = false;
};
