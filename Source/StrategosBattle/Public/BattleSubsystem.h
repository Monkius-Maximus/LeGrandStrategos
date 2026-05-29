#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "BattleTypes.h"
#include "BattleProposal.h"
#include "BattleSubsystem.generated.h"

class UWorldState;
class UBattleAIController;
class UBattleAIProfile;
class UBattleCardAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBattleStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBattlePhaseChanged, EBattlePhase, OldPhase, EBattlePhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBattleRoundEnded, int32, Round);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBattleSideRouted, int32, SideIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBattleCardPlayed, int32, SideIndex, FName, CardId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBattleFinished, FBattleResult, Result);

/**
 * UBattleSubsystem — orquestra o loop tático de batalha.
 *
 * Etapa 1-2: fases + CombatTick + moral.
 * Etapa 4-5 (atual): deck/mão/CP + resolução de cartas + efeitos.
 * Etapa 6 (atual): UBattleAIController v1 (utility, sem lookahead).
 * Etapa 9: visualizadores táticos e input do jogador.
 *
 * O subsistema fica idle até InitBattle() ser chamado.
 * OnBattleFinished entrega FBattleResult para MilitarySubsystem::ApplyBattleResult().
 *
 * Ver docs/architecture/10-battle.md.
 */
UCLASS()
class STRATEGOSBATTLE_API UBattleSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;

	// ── API pública ──────────────────────────────────────────────────────────

	/**
	 * Inicializa o contexto. Retorna false se inválido ou já ativo.
	 * Emite OnBattleStarted em sucesso.
	 */
	UFUNCTION(BlueprintCallable, Category = "Strategos|Battle")
	bool InitBattle(const FBattleProposal& Proposal);

	/**
	 * Avança um round completo: Draw → CP → Declare → Resolve → CombatTick
	 * → MoraleCheck → RoundEnd.
	 * Retorna false se a batalha não estiver ativa.
	 * Emite OnBattleFinished no último round automaticamente.
	 */
	UFUNCTION(BlueprintCallable, Category = "Strategos|Battle")
	bool ProcessRound();

	UFUNCTION(BlueprintPure, Category = "Strategos|Battle")
	const FBattleContext& GetContext() const { return Context; }

	UFUNCTION(BlueprintPure, Category = "Strategos|Battle")
	bool IsBattleActive() const { return bActive; }

	// ── Eventos ──────────────────────────────────────────────────────────────

	UPROPERTY(BlueprintAssignable, Category = "Strategos|Battle")
	FOnBattleStarted OnBattleStarted;

	UPROPERTY(BlueprintAssignable, Category = "Strategos|Battle")
	FOnBattlePhaseChanged OnPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Strategos|Battle")
	FOnBattleRoundEnded OnRoundEnded;

	UPROPERTY(BlueprintAssignable, Category = "Strategos|Battle")
	FOnBattleSideRouted OnSideRouted;

	/** Disparado para cada carta jogada. SideIndex: 0=atacante, 1=defensor. */
	UPROPERTY(BlueprintAssignable, Category = "Strategos|Battle")
	FOnBattleCardPlayed OnCardPlayed;

	UPROPERTY(BlueprintAssignable, Category = "Strategos|Battle")
	FOnBattleFinished OnBattleFinished;

private:
	// ── Loop interno ─────────────────────────────────────────────────────────

	void DrawForBothSides();
	void RefreshCommandPoints();
	void GatherAndResolveDeclarations();
	void ApplyCombatTick(int32& OutAttStrLost, int32& OutDefStrLost);
	void CheckMoraleAndRout(int32 AttStrLost, int32 DefStrLost);
	void EndRound();

	bool ShouldAdvancePhase() const;
	void AdvancePhase();
	FBattleResult Finalize();

	// ── Gestão de deck ───────────────────────────────────────────────────────

	void DrawCards(FBattleSide& Side, int32 TargetHandSize);
	void ShuffleDeck(TArray<UBattleCardAsset*>& Deck);
	void ResolveDeclarations(const FBattleDeclaration& AttDecl,
	                         const FBattleDeclaration& DefDecl);
	int32 ComputeCardInitiative(const FBattleSide& Side,
	                            const UBattleCardAsset* Card) const;

	/** Cria 5 cartas placeholder em memória como fallback sem DataAssets. */
	TArray<UBattleCardAsset*> BuildFallbackDeck(UObject* Outer);

	// ── Cálculo de dano ──────────────────────────────────────────────────────

	FCombatTickResult ComputeDamage(const FBattleSide& Source,
	                                const FBattleSide& Target) const;
	void DistributeDamage(FBattleSide& Target, float TotalDamage);
	void UpdateSideMorale(FBattleSide& Side, int32 StrengthLost);

	float ActiveEffectsModifier(const FBattleSide& Side,
	                             EActiveEffectType Type) const;
	float TerrainCoeff(const FBattleSide& Side) const;
	float WeatherCoeff() const;
	float MoraleCoeff(float Morale) const;
	float SupplyCoeff(float Supply) const;
	float PositionCoeff(EBattlePosition Src, EBattlePosition Tgt) const;

	// ── Helpers ──────────────────────────────────────────────────────────────

	void LogEntry(int32 SideIdx, EBattleLogType Type,
	              const FString& Description, int32 NumericValue = 0,
	              FGuid TargetId = FGuid());

	FBattleSide BuildSide(int32 NationId, const TArray<FName>& ArmyIds,
	                      const UWorldState* WorldState) const;

	static void AssignInitialPositions(FBattleContext& Ctx);

	UWorldState* ResolveWorldState() const;

	// ── Estado ───────────────────────────────────────────────────────────────

	UPROPERTY()
	FBattleContext Context;

	UPROPERTY()
	TObjectPtr<UBattleAIController> AttackerAI;

	UPROPERTY()
	TObjectPtr<UBattleAIController> DefenderAI;

	bool bActive = false;
	int32 RoundsInCurrentPhase = 0;

	// Seed do FRandomStream para shuffle determinístico
	FRandomStream BattleRNG;

	static constexpr int32 MaxEngagementRounds    = 3;
	static constexpr float RoutThreshold          = 25.f;
	static constexpr float EarlyExitStrengthRatio = 0.5f;
	static constexpr float EarlyExitMoraleThresh  = 30.f;
	static constexpr float DamageScaleFactor      = 0.08f;
	static constexpr int32 HandSize               = 3;
};
