#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Events/EventContext.h"
#include "StrategosEventModalWidget.generated.h"

class UEventAsset;
class UEventSubsystem;

USTRUCT(BlueprintType)
struct STRATEGOSUI_API FEventChoiceRow
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) int32 ChoiceIndex  = 0;
	UPROPERTY(BlueprintReadOnly) FText Label;
	UPROPERTY(BlueprintReadOnly) FText Tooltip;
	/** Descrição curta dos efeitos em texto (ex.: "+£500, -10 Estabilidade"). */
	UPROPERTY(BlueprintReadOnly) FText EffectsPreview;
	/** Se falso, o botão aparece desabilitado (condição não satisfeita). */
	UPROPERTY(BlueprintReadOnly) bool  bAvailable = true;
};

/**
 * UStrategosEventModalWidget — modal de evento com escolhas, estilo EU4/Vic3.
 *
 * Chamar OpenEvent(EventId) carrega o asset e disponibiliza os getters.
 * ResolveChoice() aplica o efeito via EventSubsystem e fecha o modal.
 *
 * Tamanho: 540px largura (definido no BP).
 * Categorias de cor mapeadas: "economic", "political", "military", "diplomatic".
 */
UCLASS(Abstract)
class STRATEGOSUI_API UStrategosEventModalWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Strategos|EventModal")
	void OpenEvent(FName EventId);

	UFUNCTION(BlueprintCallable, Category = "Strategos|EventModal")
	void ResolveChoice(int32 ChoiceIndex);

	// ── Dados do evento ───────────────────────────────────────────────────────
	UFUNCTION(BlueprintPure, Category = "Strategos|EventModal")
	FText GetEventTitle() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|EventModal")
	FText GetEventDescription() const;

	/** "economic" | "political" | "military" | "diplomatic" */
	UFUNCTION(BlueprintPure, Category = "Strategos|EventModal")
	FName GetEventCategory() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|EventModal")
	FLinearColor GetCategoryColor() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|EventModal")
	TArray<FEventChoiceRow> GetChoiceRows() const;

	// ── BP overrides ──────────────────────────────────────────────────────────
	UFUNCTION(BlueprintImplementableEvent, Category = "Strategos|EventModal")
	void OnEventLoaded(FName EventId);

	UFUNCTION(BlueprintImplementableEvent, Category = "Strategos|EventModal")
	void OnChoiceResolved(int32 ChoiceIndex);

private:
	UEventSubsystem* ResolveEvents() const;

	UPROPERTY() FName CurrentEventId;
	UPROPERTY() TObjectPtr<UEventAsset> CurrentAsset;
};
