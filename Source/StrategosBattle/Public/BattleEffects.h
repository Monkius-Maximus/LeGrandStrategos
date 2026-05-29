#pragma once

#include "CoreMinimal.h"
#include "BattleCardAsset.h"
#include "BattleEffects.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// UEffect_DamageRegiment
// Causa dano extra a regimentos do alvo (porcentagem adicional do tick base).
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(DisplayName = "Dano em Regimento")
class STRATEGOSBATTLE_API UEffect_DamageRegiment : public UBattleEffect
{
	GENERATED_BODY()
public:
	/** Multiplicador adicional sobre o FightingPower da fonte (0.5 = +50% de dano). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "-1.0", ClampMax = "3.0"))
	float DamageMultiplier = 0.5f;

	/** Se true, afeta o lado inimigo; se false, afeta o próprio lado (auto-dano raro). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bTargetEnemy = true;

	virtual void Apply(FBattleContext& Ctx, FBattleSide& Source, FBattleSide& Target) override;
	virtual FText GetDescriptionText() const override;
};

// ─────────────────────────────────────────────────────────────────────────────
// UEffect_MoraleShift
// Altera a moral de um lado (positivo = boost, negativo = redução).
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(DisplayName = "Mudança de Moral")
class STRATEGOSBATTLE_API UEffect_MoraleShift : public UBattleEffect
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "-50.0", ClampMax = "50.0"))
	float MoraleDelta = 10.f;

	/** Se true, aplica no inimigo; se false, aplica na própria side. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bTargetEnemy = false;

	virtual void Apply(FBattleContext& Ctx, FBattleSide& Source, FBattleSide& Target) override;
	virtual FText GetDescriptionText() const override;
};

// ─────────────────────────────────────────────────────────────────────────────
// UEffect_AddPersistent
// Adiciona um FActiveBattleEffect ao contexto por N rounds.
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(DisplayName = "Efeito Persistente")
class STRATEGOSBATTLE_API UEffect_AddPersistent : public UBattleEffect
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FName EffectId;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) FText EffectLabel;

	/** Valor do modificador (ex.: 0.2 = +20% ATK, -0.15 = -15% DEF). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float Value = 0.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "1", ClampMax = "6"))
	int32 Duration = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EActiveEffectType EffectType = EActiveEffectType::AttackModifier;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bTargetEnemy = false;

	virtual void Apply(FBattleContext& Ctx, FBattleSide& Source, FBattleSide& Target) override;
	virtual FText GetDescriptionText() const override;
};

// ─────────────────────────────────────────────────────────────────────────────
// UEffect_RepositionSide
// Muda a EBattlePosition de um lado.
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(DisplayName = "Reposicionamento")
class STRATEGOSBATTLE_API UEffect_RepositionSide : public UBattleEffect
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EBattlePosition NewPosition = EBattlePosition::Flank;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bTargetEnemy = false;

	virtual bool CanApply(const FBattleContext& Ctx,
	                      const FBattleSide& Source,
	                      const FBattleSide& Target) const override;
	virtual void Apply(FBattleContext& Ctx, FBattleSide& Source, FBattleSide& Target) override;
	virtual FText GetDescriptionText() const override;
};

// ─────────────────────────────────────────────────────────────────────────────
// UEffect_DrawCards
// Compra cartas extras do DrawPile para a mão da side de origem.
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(DisplayName = "Comprar Cartas")
class STRATEGOSBATTLE_API UEffect_DrawCards : public UBattleEffect
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = "1", ClampMax = "4"))
	int32 CardsToDraw = 1;

	virtual void Apply(FBattleContext& Ctx, FBattleSide& Source, FBattleSide& Target) override;
	virtual FText GetDescriptionText() const override;
};
