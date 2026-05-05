#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MapSubsystem.generated.h"

class UProvince;
class UWorldState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProvinceSelected, FName, ProvinceId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProvinceHovered, FName, ProvinceId);

/**
 * UMapSubsystem (v1) — Registro espacial + helpers de picking.
 *
 * Stage 1 (MVP): mantém um cache id→posição e expõe lookup espacial
 * (FindNearestProvinceTo) para picking via raycast no editor virá. Os
 * métodos de seleção/hover propagam a intenção do usuário para que
 * UI e outros subsistemas (Military, etc.) possam reagir.
 *
 * Modos de mapa, fog of war, animações de fronteira: docs/architecture/00-overview.md
 * indica que esses entram quando os outros subsistemas estiverem maduros.
 */
UCLASS()
class STRATEGOSCORE_API UMapSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;

	UFUNCTION(BlueprintCallable, Category = "Strategos|Map")
	void RebuildSpatialIndex();

	UFUNCTION(BlueprintCallable, Category = "Strategos|Map")
	FName FindNearestProvinceTo(FVector2D MapPosition) const;

	UFUNCTION(BlueprintCallable, Category = "Strategos|Map")
	void SelectProvince(FName ProvinceId);

	UFUNCTION(BlueprintCallable, Category = "Strategos|Map")
	void HoverProvince(FName ProvinceId);

	UFUNCTION(BlueprintPure, Category = "Strategos|Map")
	FName GetSelectedProvinceId() const { return SelectedProvinceId; }

	UFUNCTION(BlueprintPure, Category = "Strategos|Map")
	FName GetHoveredProvinceId() const { return HoveredProvinceId; }

	UPROPERTY(BlueprintAssignable, Category = "Strategos|Map")
	FOnProvinceSelected OnProvinceSelected;

	UPROPERTY(BlueprintAssignable, Category = "Strategos|Map")
	FOnProvinceHovered OnProvinceHovered;

private:
	UWorldState* ResolveWorldState() const;

	UPROPERTY()
	FName SelectedProvinceId;

	UPROPERTY()
	FName HoveredProvinceId;

	TMap<FName, FVector2D> SpatialIndex;
};
