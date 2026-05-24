#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StrategosWarRoomWidget.generated.h"

class UDiplomacySubsystem;

UENUM(BlueprintType)
enum class EWarRoomTab : uint8
{
	Overview   UMETA(DisplayName = "Visão Geral"),
	Objectives UMETA(DisplayName = "Objetivos"),
	Battles    UMETA(DisplayName = "Batalhas"),
};

USTRUCT(BlueprintType)
struct STRATEGOSUI_API FWarGoalRow
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FText  GoalName;
	UPROPERTY(BlueprintReadOnly) FText  TargetName;
	UPROPERTY(BlueprintReadOnly) int32  ScoreCost  = 0;
	UPROPERTY(BlueprintReadOnly) bool   bAchieved  = false;
	UPROPERTY(BlueprintReadOnly) bool   bPlayerSide = true;
};

USTRUCT(BlueprintType)
struct STRATEGOSUI_API FWarBattleRecord
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FText  ProvinceName;
	UPROPERTY(BlueprintReadOnly) FText  DateLabel;
	UPROPERTY(BlueprintReadOnly) bool   bPlayerVictory = true;
	UPROPERTY(BlueprintReadOnly) int32  AttackerLosses = 0;
	UPROPERTY(BlueprintReadOnly) int32  DefenderLosses = 0;
	UPROPERTY(BlueprintReadOnly) int32  WarScoreDelta  = 0;
};

USTRUCT(BlueprintType)
struct STRATEGOSUI_API FWarSideSummary
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FText  NationName;
	UPROPERTY(BlueprintReadOnly) FLinearColor NationColor = FLinearColor::White;
	UPROPERTY(BlueprintReadOnly) TArray<FText> AllyNames;
	UPROPERTY(BlueprintReadOnly) float  Exhaustion = 0.f;  // [0..1]
	UPROPERTY(BlueprintReadOnly) int32  TotalLosses = 0;
	UPROPERTY(BlueprintReadOnly) float  MonthlyCost = 0.f;
};

/**
 * UStrategosWarRoomWidget — painel de guerra estilo Vic3.
 *
 * Exibe: banner do Casus Belli, resumo em 3 colunas (atacante / war score /
 * defensor), 3 abas (Visão Geral, Objetivos, Batalhas) e botões de ação
 * (Propor Paz, Render-se).
 *
 * Tamanho: 900×640 (definido no BP).
 */
UCLASS(Abstract)
class STRATEGOSUI_API UStrategosWarRoomWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Configura contra qual nação a guerra está sendo travada. */
	UFUNCTION(BlueprintCallable, Category = "Strategos|WarRoom")
	void OpenWarRoom(FName EnemyNationId);

	UFUNCTION(BlueprintCallable, Category = "Strategos|WarRoom")
	void SetActiveTab(EWarRoomTab NewTab);

	UFUNCTION(BlueprintPure, Category = "Strategos|WarRoom")
	EWarRoomTab GetActiveTab() const { return ActiveTab; }

	// ── Header ────────────────────────────────────────────────────────────────
	UFUNCTION(BlueprintPure, Category = "Strategos|WarRoom")
	FText GetWarName() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|WarRoom")
	FText GetCasusBelliName() const;

	// ── War score ────────────────────────────────────────────────────────────
	/** [-100..+100]. Positivo = player vantagem. */
	UFUNCTION(BlueprintPure, Category = "Strategos|WarRoom")
	int32 GetWarScore() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|WarRoom")
	FWarSideSummary GetPlayerSide() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|WarRoom")
	FWarSideSummary GetEnemySide() const;

	// ── Aba: Objetivos ────────────────────────────────────────────────────────
	UFUNCTION(BlueprintPure, Category = "Strategos|WarRoom")
	TArray<FWarGoalRow> GetWarGoals() const;

	// ── Aba: Batalhas ─────────────────────────────────────────────────────────
	UFUNCTION(BlueprintPure, Category = "Strategos|WarRoom")
	TArray<FWarBattleRecord> GetBattleHistory() const;

	// ── Ações ─────────────────────────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "Strategos|WarRoom")
	void RequestProposePeace();

	UFUNCTION(BlueprintCallable, Category = "Strategos|WarRoom")
	void RequestSurrender();

	// ── BP overrides ──────────────────────────────────────────────────────────
	UFUNCTION(BlueprintImplementableEvent, Category = "Strategos|WarRoom")
	void OnWarScoreChanged(int32 NewScore);

	UFUNCTION(BlueprintImplementableEvent, Category = "Strategos|WarRoom")
	void OnOpenPeaceNegotiation();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UPROPERTY() FName EnemyNationId;
	UPROPERTY() EWarRoomTab ActiveTab = EWarRoomTab::Overview;
};

// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct STRATEGOSUI_API FPeaceTermRow
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FName  TermId;
	UPROPERTY(BlueprintReadOnly) FText  Label;
	UPROPERTY(BlueprintReadOnly) FText  Description;
	UPROPERTY(BlueprintReadOnly) int32  ScoreCost    = 0;
	UPROPERTY(BlueprintReadOnly) bool   bSelected    = false;
	UPROPERTY(BlueprintReadOnly) bool   bAvailable   = true;
};

/**
 * UStrategosWarRoomWidget — mesa de paz estilo EU4.
 *
 * Lista de termos clicáveis; sidebar mostra saldo de war score e barra de
 * aceitação do oponente decrescendo conforme a demanda sobe.
 */
UCLASS(Abstract)
class STRATEGOSUI_API UStrategosPeaceNegotiationWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Strategos|Peace")
	void OpenPeaceTable(FName EnemyNationId);

	UFUNCTION(BlueprintCallable, Category = "Strategos|Peace")
	void ToggleTerm(FName TermId);

	UFUNCTION(BlueprintPure, Category = "Strategos|Peace")
	TArray<FPeaceTermRow> GetTermRows() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|Peace")
	int32 GetAvailableScore() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|Peace")
	int32 GetSelectedCost() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|Peace")
	int32 GetScoreBalance() const;

	/** [0..1] probabilidade da IA aceitar (cai conforme custo sobe). */
	UFUNCTION(BlueprintPure, Category = "Strategos|Peace")
	float GetAIAcceptanceProbability() const;

	UFUNCTION(BlueprintCallable, Category = "Strategos|Peace")
	void SendProposal();

	UFUNCTION(BlueprintImplementableEvent, Category = "Strategos|Peace")
	void OnProposalSent(bool bAccepted);

private:
	UPROPERTY() FName EnemyNationId;
	UPROPERTY() TArray<FName> SelectedTermIds;
};
