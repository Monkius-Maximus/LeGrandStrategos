#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BattleTypes.h"
#include "BattleCardAsset.generated.h"

class UBattleCardAsset;

// ─────────────────────────────────────────────────────────────────────────────
// UBattleEffect — base abstrata para efeitos de carta
// ─────────────────────────────────────────────────────────────────────────────

/**
 * UBattleEffect — plugável. Cada tipo de efeito = 1 subclasse.
 * Use EditInlineNew + Instanced no array de UBattleCardAsset::Effects para
 * montar combinações no editor sem mudar o core.
 *
 * Stage 4: DamageRegiment, MoraleShift, AddPersistent, RepositionSide, DrawCards.
 */
UCLASS(Abstract, EditInlineNew, Blueprintable, CollapseCategories)
class STRATEGOSBATTLE_API UBattleEffect : public UObject
{
	GENERATED_BODY()
public:
	/** Retorna false para bloquear a aplicação sem consumir a carta. */
	virtual bool CanApply(const FBattleContext& Ctx,
	                      const FBattleSide& Source,
	                      const FBattleSide& Target) const;

	/** Aplica o efeito. Modifica Ctx, Source ou Target conforme o tipo. */
	virtual void Apply(FBattleContext& Ctx,
	                   FBattleSide& Source,
	                   FBattleSide& Target);

	/** Texto curto para tooltip / log. */
	virtual FText GetDescriptionText() const;
};

// ─────────────────────────────────────────────────────────────────────────────
// UBattleCardAsset — DataAsset editável no editor
// ─────────────────────────────────────────────────────────────────────────────

/**
 * UBattleCardAsset — carta tática. Designer cria instâncias no editor e as
 * referencia em registries ou decks de regimentos/doutrinas.
 *
 * Placeholder de arte: atribuir CardArt no editor quando texturas estiverem prontas.
 *
 * Naming: prefixo BC_ (ex.: BC_Charge, BC_HoldLine, BC_Flank, BC_Rally, BC_Ambush).
 */
UCLASS(BlueprintType)
class STRATEGOSBATTLE_API UBattleCardAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	// ── Metadados ──────────────────────────────────────────────────────────

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identidade")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identidade")
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identidade")
	ECardCategory Category = ECardCategory::Assault;

	/** Arte da carta — deixar null até o asset estar pronto. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identidade")
	TSoftObjectPtr<UTexture2D> CardArt;

	// ── Custos e timing ────────────────────────────────────────────────────

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jogo",
		meta = (ClampMin = "0", ClampMax = "6"))
	int32 CommandCost = 1;

	/** Maior priority resolve primeiro. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jogo",
		meta = (ClampMin = "0", ClampMax = "20"))
	int32 Priority = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jogo")
	ECardTiming Timing = ECardTiming::OnPlay;

	// ── Restrições ─────────────────────────────────────────────────────────

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Condições")
	FCardConditions Conditions;

	/** Vazio = válida em qualquer fase. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Condições")
	TArray<EBattlePhase> ValidPhases;

	// ── Efeitos ────────────────────────────────────────────────────────────

	/** Cada efeito é aplicado em sequência quando a carta resolve. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Efeitos",
		Instanced)
	TArray<TObjectPtr<UBattleEffect>> Effects;

	/** Se true, vai para ExhaustPile ao ser jogada (não volta ao deck). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Efeitos")
	bool bExhaustOnPlay = false;

	// ── Helpers ────────────────────────────────────────────────────────────

	/** Verifica se a carta pode ser jogada no contexto atual. */
	bool IsValidInContext(const FBattleContext& Ctx, const FBattleSide& Side) const;
};
