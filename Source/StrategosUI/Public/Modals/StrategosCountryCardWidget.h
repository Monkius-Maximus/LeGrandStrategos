#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Diplomacy/DiplomaticRelation.h"
#include "StrategosCountryCardWidget.generated.h"

class UNation;
class ULeader;
class UProvince;
class UWorldState;

USTRUCT(BlueprintType)
struct STRATEGOSUI_API FCountryStatRow
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FText  Label;
	UPROPERTY(BlueprintReadOnly) FText  Value;
	UPROPERTY(BlueprintReadOnly) FLinearColor ValueColor = FLinearColor::White;
};

/**
 * UStrategosCountryCardWidget — modal de país estilo Vic3.
 *
 * Exibe: retrato-silhueta, governo, instituições, grid de stats,
 * distribuição ideológica e relação atual com o jogador.
 *
 * Tamanho: 480×640 (definido no BP).
 */
UCLASS(Abstract)
class STRATEGOSUI_API UStrategosCountryCardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Strategos|CountryCard")
	void OpenCountry(FName NationId);

	// ── Identidade ────────────────────────────────────────────────────────────
	UFUNCTION(BlueprintPure, Category = "Strategos|CountryCard")
	FText GetNationName() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|CountryCard")
	FText GetNationFullName() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|CountryCard")
	FLinearColor GetNationColor() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|CountryCard")
	UTexture2D* GetFlagTexture() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|CountryCard")
	UTexture2D* GetCoatOfArmsIcon() const;

	// ── Governo ───────────────────────────────────────────────────────────────
	UFUNCTION(BlueprintPure, Category = "Strategos|CountryCard")
	FText GetGovernmentType() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|CountryCard")
	FText GetRulerName() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|CountryCard")
	ELeaderArchetype GetRulerArchetype() const;

	// ── Stats (grid 2×3) ──────────────────────────────────────────────────────
	UFUNCTION(BlueprintPure, Category = "Strategos|CountryCard")
	TArray<FCountryStatRow> GetStatRows() const;

	// ── Índices estratégicos ──────────────────────────────────────────────────
	UFUNCTION(BlueprintPure, Category = "Strategos|CountryCard")
	float GetMilitaryIndex() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|CountryCard")
	float GetMoraleIndex() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|CountryCard")
	float GetIndustryIndex() const;

	// ── Relação com o jogador ─────────────────────────────────────────────────
	UFUNCTION(BlueprintPure, Category = "Strategos|CountryCard")
	EDiplomaticStatus GetRelationStatus() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|CountryCard")
	float GetOpinion() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|CountryCard")
	FText GetStatusLabel() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|CountryCard")
	FLinearColor GetStatusColor() const;

	// ── Ações rápidas ─────────────────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "Strategos|CountryCard")
	void OpenDiplomacyAction(FName ActionId);

	// ── BP overrides ──────────────────────────────────────────────────────────
	UFUNCTION(BlueprintImplementableEvent, Category = "Strategos|CountryCard")
	void OnCountryLoaded(FName NationId);

	UFUNCTION(BlueprintImplementableEvent, Category = "Strategos|CountryCard")
	void OnOpenDiploAction(FName ActionId, FName TargetNationId);

private:
	const UNation* ResolveNation() const;
	const UNation* ResolvePlayerNation() const;
	const UWorldState* ResolveWorldState() const;

	UPROPERTY() FName CurrentNationId;
};
