#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Foundation/Time/TimeSpeed.h"
#include "StrategosHUDWidget.generated.h"

class UTimeSubsystem;
class UMapSubsystem;
class USaveSubsystem;

/**
 * UStrategosHUDWidget — Base C++ do HUD estratégico.
 *
 * Expõe propriedades bindables para o BP child montar o layout livremente
 * (texto de data, seleção, botões de velocidade). Funções de comando
 * (PauseGame, ResumeGame, SetSpeed*) são chamáveis pelos OnClick dos
 * botões no BP.
 *
 * O subsistema de tempo é consultado em ReceiveBoundUpdate (chamado por
 * NativeTick) e os eventos de seleção em UMapSubsystem disparam refresh
 * imediato.
 */
UCLASS(Abstract)
class STRATEGOSUI_API UStrategosHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Strategos|HUD")
	void PauseGame();

	UFUNCTION(BlueprintCallable, Category = "Strategos|HUD")
	void ResumeGame();

	UFUNCTION(BlueprintCallable, Category = "Strategos|HUD")
	void SetTimeSpeed(ETimeSpeed NewSpeed);

	UFUNCTION(BlueprintCallable, Category = "Strategos|HUD")
	void RequestSave(const FString& SlotName);

	UFUNCTION(BlueprintCallable, Category = "Strategos|HUD")
	void RequestLoad(const FString& SlotName);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Strategos|HUD")
	FText GetCurrentDateText() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Strategos|HUD")
	ETimeSpeed GetCurrentSpeed() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Strategos|HUD")
	FText GetSelectedProvinceName() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Strategos|HUD")
	FText GetSelectedProvinceOwnerName() const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** Hook para o BP atualizar binds não-disponíveis em PURE getters (ex.: cores). */
	UFUNCTION(BlueprintImplementableEvent, Category = "Strategos|HUD")
	void OnSelectionChanged(FName ProvinceId);

private:
	UFUNCTION()
	void HandleProvinceSelected(FName ProvinceId);

	UTimeSubsystem* ResolveTime() const;
	UMapSubsystem* ResolveMap() const;
	USaveSubsystem* ResolveSave() const;
};
