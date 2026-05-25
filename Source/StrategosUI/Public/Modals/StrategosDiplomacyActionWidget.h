#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StrategosDiplomacyActionWidget.generated.h"

class UDiplomacySubsystem;
class UWorldState;

USTRUCT(BlueprintType)
struct STRATEGOSUI_API FDiploModifierRow
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FText  Label;
	UPROPERTY(BlueprintReadOnly) float  Value      = 0.f;  // + ou -
	UPROPERTY(BlueprintReadOnly) bool   bPositive  = true;
	UPROPERTY(BlueprintReadOnly) FLinearColor RowColor = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct STRATEGOSUI_API FDiploActionCost
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) int32 PoliticalCapital = 0;
	UPROPERTY(BlueprintReadOnly) float Treasury          = 0.f;
	UPROPERTY(BlueprintReadOnly) int32 CooldownDays      = 0;
	/** Infâmia gerada (0 se não aplicável). */
	UPROPERTY(BlueprintReadOnly) float Infamy             = 0.f;
};

/**
 * UStrategosDiplomacyActionWidget — modal de ação diplomática estilo Vic3.
 *
 * Carregado com (ActionId, TargetNationId). Exibe:
 *   - Header: categoria, título, duração
 *   - Linha player → alvo (bandeiras)
 *   - Custo em Capital Político / Tesouro / Cooldown
 *   - Breakdown de aceitação da IA (modificadores somando ao total)
 *   - Variações especiais: Fabricar CB (4 tipos), Declarar Guerra (aviso vermelho)
 *
 * Ações configuradas: "propose_alliance", "fabricate_cb", "declare_war",
 *   "embargo", "offer_gift", "non_aggression_pact", "request_passage".
 */
UCLASS(Abstract)
class STRATEGOSUI_API UStrategosDiplomacyActionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Strategos|DiploAction")
	void OpenAction(FName ActionId, FName TargetNationId);

	// ── Header ────────────────────────────────────────────────────────────────
	UFUNCTION(BlueprintPure, Category = "Strategos|DiploAction")
	FText GetActionTitle() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|DiploAction")
	FText GetActionSummary() const;

	/** "alliance" | "hostile" | "economic" | "warning" */
	UFUNCTION(BlueprintPure, Category = "Strategos|DiploAction")
	FName GetActionCategory() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|DiploAction")
	FLinearColor GetCategoryColor() const;

	// ── Alvo ──────────────────────────────────────────────────────────────────
	UFUNCTION(BlueprintPure, Category = "Strategos|DiploAction")
	FText GetTargetNationName() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|DiploAction")
	FLinearColor GetTargetNationColor() const;

	// ── Custo ────────────────────────────────────────────────────────────────
	UFUNCTION(BlueprintPure, Category = "Strategos|DiploAction")
	FDiploActionCost GetActionCost() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|DiploAction")
	bool CanAffordAction() const;

	// ── Breakdown de aceitação ────────────────────────────────────────────────
	UFUNCTION(BlueprintPure, Category = "Strategos|DiploAction")
	TArray<FDiploModifierRow> GetAcceptanceModifiers() const;

	/** Total [0..1] — soma dos modificadores clampada. */
	UFUNCTION(BlueprintPure, Category = "Strategos|DiploAction")
	float GetAcceptanceProbability() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|DiploAction")
	FText GetAcceptanceVerdict() const;

	// ── Variações ────────────────────────────────────────────────────────────
	/** True se esta ação é "declare_war" sem CB — exibe painel de aviso. */
	UFUNCTION(BlueprintPure, Category = "Strategos|DiploAction")
	bool IsHighRiskAction() const;

	/** True se esta ação é "fabricate_cb" — exibe seleção de tipo de CB. */
	UFUNCTION(BlueprintPure, Category = "Strategos|DiploAction")
	bool IsCBFabrication() const;

	// ── Execução ─────────────────────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "Strategos|DiploAction")
	void ExecuteAction();

	// ── BP overrides ──────────────────────────────────────────────────────────
	UFUNCTION(BlueprintImplementableEvent, Category = "Strategos|DiploAction")
	void OnActionLoaded(FName ActionId, FName TargetId);

	UFUNCTION(BlueprintImplementableEvent, Category = "Strategos|DiploAction")
	void OnActionExecuted(bool bSucceeded);

private:
	struct FActionDef
	{
		FText  Title;
		FText  Summary;
		FName  Category;
		int32  PolitCapCost;
		float  TreasuryCost;
		int32  CooldownDays;
		float  InfamyCost;
		bool   bHighRisk;
		bool   bCBFab;
	};

	static TMap<FName, FActionDef> BuildActionDefs();
	TArray<FDiploModifierRow> ComputeModifiers() const;
	const UWorldState* ResolveWorldState() const;

	UPROPERTY() FName CurrentActionId;
	UPROPERTY() FName CurrentTargetId;
};
