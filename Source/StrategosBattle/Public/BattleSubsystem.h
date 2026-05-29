#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "BattleTypes.h"
#include "BattleProposal.h"
#include "BattleSubsystem.generated.h"

class UWorldState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBattleStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBattlePhaseChanged, EBattlePhase, OldPhase, EBattlePhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBattleRoundEnded, int32, Round);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBattleSideRouted, int32, SideIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBattleFinished, FBattleResult, Result);

/**
 * UBattleSubsystem — orquestra o loop tático de batalha.
 *
 * Etapa 1-2 (atual): fases de batalha + CombatTick sem cartas + moral + rota.
 * Etapa 4: cartas, deck, Command Points, UBattleEffect.
 * Etapa 6: UBattleAIController.
 * Etapa 9: visualizadores táticos.
 *
 * O subsistema fica idle até InitBattle() ser chamado. Subscribers
 * ouvem OnBattleFinished e chamam UMilitarySubsystem::ApplyBattleResult().
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
	 * Inicializa o contexto a partir do proposal.
	 * Retorna false se já houver uma batalha ativa ou se o proposal for inválido.
	 * Emite OnBattleStarted em caso de sucesso.
	 */
	UFUNCTION(BlueprintCallable, Category = "Strategos|Battle")
	bool InitBattle(const FBattleProposal& Proposal);

	/**
	 * Avança a batalha por um round completo (DrawPhase → CombatTick → MoraleCheck → RoundEnd).
	 * Retorna false se a batalha não estiver ativa ou já tiver terminado.
	 * Emite OnBattleFinished automaticamente quando o último round é processado.
	 */
	UFUNCTION(BlueprintCallable, Category = "Strategos|Battle")
	bool ProcessRound();

	UFUNCTION(BlueprintPure, Category = "Strategos|Battle")
	const FBattleContext& GetContext() const { return Context; }

	UFUNCTION(BlueprintPure, Category = "Strategos|Battle")
	bool IsBattleActive() const { return bActive; }

	// ── Eventos ──────────────────────────────────────────────────────────────

	/** Disparado quando InitBattle() é bem-sucedido. Leia contexto via GetContext(). */
	UPROPERTY(BlueprintAssignable, Category = "Strategos|Battle")
	FOnBattleStarted OnBattleStarted;

	/** Disparado a cada transição de fase. */
	UPROPERTY(BlueprintAssignable, Category = "Strategos|Battle")
	FOnBattlePhaseChanged OnPhaseChanged;

	/** Disparado ao final de cada round. */
	UPROPERTY(BlueprintAssignable, Category = "Strategos|Battle")
	FOnBattleRoundEnded OnRoundEnded;

	/** SideIndex: 0 = atacante, 1 = defensor. */
	UPROPERTY(BlueprintAssignable, Category = "Strategos|Battle")
	FOnBattleSideRouted OnSideRouted;

	/** Disparado quando a batalha termina. Subscriber chama ApplyBattleResult(). */
	UPROPERTY(BlueprintAssignable, Category = "Strategos|Battle")
	FOnBattleFinished OnBattleFinished;

private:
	// ── Loop interno por round ───────────────────────────────────────────────

	void DrawForBothSides();       // Stage 4: no-op
	void RefreshCommandPoints();   // Stage 4: no-op

	void ApplyCombatTick(int32& OutAttStrLost, int32& OutDefStrLost);
	void CheckMoraleAndRout(int32 AttStrLost, int32 DefStrLost);
	void EndRound();

	bool ShouldAdvancePhase() const;
	void AdvancePhase();

	FBattleResult Finalize();

	// ── Cálculo de dano ──────────────────────────────────────────────────────

	FCombatTickResult ComputeDamage(const FBattleSide& Source,
	                                const FBattleSide& Target) const;
	void DistributeDamage(FBattleSide& Target, float TotalDamage);

	void UpdateSideMorale(FBattleSide& Side, int32 StrengthLost);

	// ── Coeficientes de modificadores ────────────────────────────────────────

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

	bool bActive = false;
	int32 RoundsInCurrentPhase = 0;

	static constexpr int32 MaxEngagementRounds    = 3;
	static constexpr float RoutThreshold          = 25.f;
	static constexpr float EarlyExitStrengthRatio = 0.5f;
	static constexpr float EarlyExitMoraleThresh  = 30.f;
	static constexpr float DamageScaleFactor      = 0.08f;
};
