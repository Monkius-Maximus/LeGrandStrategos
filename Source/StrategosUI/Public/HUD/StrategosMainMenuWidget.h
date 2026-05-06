#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StrategosMainMenuWidget.generated.h"

/**
 * UStrategosMainMenuWidget — Base C++ do menu principal.
 *
 * Expõe ações de transição (NewGame/LoadGame/Quit) que o BP child
 * conecta aos OnClick dos botões. Coordena com UGameFlowSubsystem para
 * efetivar transições de estado declaradas (MainMenu → Loading → Running).
 */
UCLASS(Abstract)
class STRATEGOSUI_API UStrategosMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Strategos|MainMenu")
	void NewGame(const FString& MapName);

	UFUNCTION(BlueprintCallable, Category = "Strategos|MainMenu")
	void LoadGameFromSlot(const FString& SlotName);

	UFUNCTION(BlueprintCallable, Category = "Strategos|MainMenu")
	void QuitGame();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Strategos|MainMenu")
	bool DoesSlotExist(const FString& SlotName) const;
};
