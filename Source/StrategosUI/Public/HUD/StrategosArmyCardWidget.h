#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "World/ArmyStats.h"
#include "StrategosArmyCardWidget.generated.h"

class UArmy;
class UUnitTypeAsset;

UENUM(BlueprintType)
enum class EArmyCardVariant : uint8
{
	Compact  UMETA(DisplayName = "Compact 192x288"),
	Micro    UMETA(DisplayName = "Micro 144x320"),
};

/**
 * UStrategosArmyCardWidget — "carta" de unidade.
 *
 * Implementa as variantes Compact (192×288) e Micro (144×320) descritas no
 * design system (etapa-2-ui-grid.md). Recebe um ponteiro para UArmy no
 * NativeConstruct ou via SetArmy(); a partir daí expõe todos os dados
 * necessários via getters BlueprintPure.
 *
 * O BP child monta o layout; o C++ entrega dados e reage a mudanças de estado
 * (dano, modificadores, nível de XP).
 */
UCLASS(Abstract)
class STRATEGOSUI_API UStrategosArmyCardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Configura qual exército esta carta representa. */
	UFUNCTION(BlueprintCallable, Category = "Strategos|ArmyCard")
	void SetArmy(UArmy* InArmy);

	UFUNCTION(BlueprintPure, Category = "Strategos|ArmyCard")
	UArmy* GetArmy() const { return BoundArmy; }

	UFUNCTION(BlueprintCallable, Category = "Strategos|ArmyCard")
	void SetVariant(EArmyCardVariant InVariant);

	UFUNCTION(BlueprintPure, Category = "Strategos|ArmyCard")
	EArmyCardVariant GetVariant() const { return Variant; }

	// ── Header ────────────────────────────────────────────────────────────────
	UFUNCTION(BlueprintPure, Category = "Strategos|ArmyCard")
	FText GetUnitDisplayName() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|ArmyCard")
	FText GetUnitRole() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|ArmyCard")
	FLinearColor GetNationColor() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|ArmyCard")
	int32 GetMaintenanceCost() const;

	// ── Portrait ──────────────────────────────────────────────────────────────
	UFUNCTION(BlueprintPure, Category = "Strategos|ArmyCard")
	UTexture2D* GetPortrait() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|ArmyCard")
	EUnitState GetUnitState() const;

	// ── Stats (compact: ATQ/DEF/MOB/MOR; micro: + ORG/SUP) ─────────────────
	UFUNCTION(BlueprintPure, Category = "Strategos|ArmyCard")
	FArmyStats GetEffectiveStats() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|ArmyCard")
	int32 GetManpower() const;

	// ── Trait (Compact only) ──────────────────────────────────────────────────
	UFUNCTION(BlueprintPure, Category = "Strategos|ArmyCard")
	FText GetPrimaryTrait() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|ArmyCard")
	FText GetPrimaryTraitDescription() const;

	// ── XP / Level (Compact footer) ───────────────────────────────────────────
	UFUNCTION(BlueprintPure, Category = "Strategos|ArmyCard")
	int32 GetExperienceXP() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|ArmyCard")
	int32 GetExperienceLevel() const;

	/** [0..1] progresso da barra de XP dentro do nível atual. */
	UFUNCTION(BlueprintPure, Category = "Strategos|ArmyCard")
	float GetXPProgress() const;

	// ── Modifiers (Micro / Detailed) ─────────────────────────────────────────
	UFUNCTION(BlueprintPure, Category = "Strategos|ArmyCard")
	TArray<FArmyModifier> GetActiveModifiers() const;

	// ── BP overrides ──────────────────────────────────────────────────────────
	UFUNCTION(BlueprintImplementableEvent, Category = "Strategos|ArmyCard")
	void OnArmyBound(UArmy* Army);

	UFUNCTION(BlueprintImplementableEvent, Category = "Strategos|ArmyCard")
	void OnStateChanged(EUnitState NewState);

private:
	static constexpr int32 XPPerLevel = 100;

	UPROPERTY() TObjectPtr<UArmy> BoundArmy;
	UPROPERTY() EArmyCardVariant   Variant = EArmyCardVariant::Compact;
};
