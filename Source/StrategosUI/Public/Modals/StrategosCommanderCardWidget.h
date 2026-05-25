#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "World/LeaderArchetype.h"
#include "StrategosCommanderCardWidget.generated.h"

class ULeader;
class UNation;

USTRUCT(BlueprintType)
struct STRATEGOSUI_API FCommanderStatRow
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FText  Label;
	UPROPERTY(BlueprintReadOnly) int32  Value     = 0;
	UPROPERTY(BlueprintReadOnly) float  Normalized = 0.f;  // [0..1] para barra
	UPROPERTY(BlueprintReadOnly) FLinearColor BarColor = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct STRATEGOSUI_API FCommanderTraitRow
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FText Label;
	UPROPERTY(BlueprintReadOnly) FText Description;
	/** "positive" | "neutral" | "mixed" | "negative" */
	UPROPERTY(BlueprintReadOnly) FName Kind;
	UPROPERTY(BlueprintReadOnly) FLinearColor PillColor = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct STRATEGOSUI_API FCommanderBattleRecord
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FText ProvinceName;
	UPROPERTY(BlueprintReadOnly) FText DateLabel;
	UPROPERTY(BlueprintReadOnly) FText EnemyNationName;
	UPROPERTY(BlueprintReadOnly) bool  bVictory = true;
	UPROPERTY(BlueprintReadOnly) int32 OwnLosses   = 0;
	UPROPERTY(BlueprintReadOnly) int32 EnemyLosses = 0;
};

/**
 * UStrategosCommanderCardWidget — modal com retrato + stats + traços + histórico,
 * estilo Romance of the Three Kingdoms / EU4.
 *
 * Vinculado a um ULeader (líder de nação ou comandante militar).
 * Tamanho: 640×720 (definido no BP).
 */
UCLASS(Abstract)
class STRATEGOSUI_API UStrategosCommanderCardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Strategos|CommanderCard")
	void OpenCommander(FName LeaderId);

	// ── Identidade ────────────────────────────────────────────────────────────
	UFUNCTION(BlueprintPure, Category = "Strategos|CommanderCard")
	FText GetCommanderName() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|CommanderCard")
	FText GetCommanderArchetype() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|CommanderCard")
	FLinearColor GetNationColor() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|CommanderCard")
	UTexture2D* GetPortrait() const;

	// ── Stats (Tática/Carisma/Logística/Ousadia — derivados do Archetype) ─────
	UFUNCTION(BlueprintPure, Category = "Strategos|CommanderCard")
	TArray<FCommanderStatRow> GetStatRows() const;

	// ── Traços ────────────────────────────────────────────────────────────────
	UFUNCTION(BlueprintPure, Category = "Strategos|CommanderCard")
	TArray<FCommanderTraitRow> GetTraitRows() const;

	// ── Histórico de batalhas ─────────────────────────────────────────────────
	UFUNCTION(BlueprintPure, Category = "Strategos|CommanderCard")
	TArray<FCommanderBattleRecord> GetBattleRecords() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|CommanderCard")
	int32 GetTotalVictories() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|CommanderCard")
	int32 GetTotalDefeats() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|CommanderCard")
	int32 GetTotalCampaigns() const;

	// ── BP overrides ──────────────────────────────────────────────────────────
	UFUNCTION(BlueprintImplementableEvent, Category = "Strategos|CommanderCard")
	void OnCommanderLoaded(FName LeaderId);

private:
	const UNation* FindNationForLeader(FName LeaderId) const;

	UPROPERTY() FName CurrentLeaderId;
	UPROPERTY() TObjectPtr<ULeader> CurrentLeader;
};
