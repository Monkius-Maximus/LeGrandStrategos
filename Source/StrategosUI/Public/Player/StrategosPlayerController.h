#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "StrategosPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
struct FInputActionValue;
class UMapSubsystem;
class UMilitarySubsystem;
class UWorldState;

/**
 * AStrategosPlayerController — Receptor de input do jogador no mapa estratégico.
 *
 * Mantém o cursor visível, escuta clique esquerdo (selecionar) e direito
 * (emitir ordem de movimento para o exército atualmente selecionado).
 *
 * SelectedArmyId é atualizado automaticamente quando o jogador clica numa
 * província que contém um exército da nação dele. RMB num destino válido
 * dispara IssueMoveOrder no MilitarySubsystem.
 */
UCLASS()
class STRATEGOSUI_API AStrategosPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AStrategosPlayerController();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Strategos|Selection")
	FName GetSelectedArmyId() const { return SelectedArmyId; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Strategos|Selection")
	FName GetSelectedProvinceId() const { return SelectedProvinceId; }

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Strategos|Input")
	TSoftObjectPtr<UInputMappingContext> SelectionMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Strategos|Input")
	TSoftObjectPtr<UInputAction> SelectClickAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Strategos|Input")
	TSoftObjectPtr<UInputAction> OrderMoveAction;

private:
	void OnSelectClicked(const FInputActionValue& Value);
	void OnOrderMoveClicked(const FInputActionValue& Value);

	FName GetProvinceUnderCursor() const;
	FName FindArmyInProvince(FName ProvinceId, FName OwnerNationId) const;

	UWorldState* ResolveWorldState() const;
	UMapSubsystem* ResolveMap() const;
	UMilitarySubsystem* ResolveMilitary() const;

	UPROPERTY()
	FName SelectedArmyId;

	UPROPERTY()
	FName SelectedProvinceId;
};
