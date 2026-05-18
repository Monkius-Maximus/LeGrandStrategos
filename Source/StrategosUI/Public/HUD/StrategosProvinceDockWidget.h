#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StrategosProvinceDockWidget.generated.h"

class UProvince;
class UNation;
class UBuilding;
class UBuildingTypeAsset;
class UMapSubsystem;

// ── Enums ─────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EProvinceDockTab : uint8
{
	Overview    UMETA(DisplayName = "Visão Geral"),
	Buildings   UMETA(DisplayName = "Edifícios"),
	Population  UMETA(DisplayName = "População"),
	Information UMETA(DisplayName = "Informação"),
};

// ── Row structs (Blueprint-friendly) ─────────────────────────────────────────

USTRUCT(BlueprintType)
struct STRATEGOSUI_API FProvinceBuildingRow
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FName   BuildingId;
	UPROPERTY(BlueprintReadOnly) FText   TypeName;
	UPROPERTY(BlueprintReadOnly) int32   Level         = 1;
	UPROPERTY(BlueprintReadOnly) bool    bConstructing = false;
	UPROPERTY(BlueprintReadOnly) int32   DaysRemaining = 0;
	UPROPERTY(BlueprintReadOnly) float   LastProfit    = 0.f;
	UPROPERTY(BlueprintReadOnly) bool    bIsPrivate    = false;
	/** Cor de categoria do prédio (ex.: azul p/ têxtil, laranja p/ mineração). */
	UPROPERTY(BlueprintReadOnly) FLinearColor CategoryColor = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct STRATEGOSUI_API FProvincePopRow
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FText  StratumName;
	UPROPERTY(BlueprintReadOnly) int32  PopCount   = 0;
	UPROPERTY(BlueprintReadOnly) float  Wage       = 0.f;
	/** Satisfação [0..1] usada para desenhar a barra de humor. */
	UPROPERTY(BlueprintReadOnly) float  Satisfaction = 0.5f;
	UPROPERTY(BlueprintReadOnly) FLinearColor StratumColor = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct STRATEGOSUI_API FProvinceSummary
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FText  ProvinceName;
	UPROPERTY(BlueprintReadOnly) FText  OwnerName;
	UPROPERTY(BlueprintReadOnly) FLinearColor OwnerColor = FLinearColor::White;
	UPROPERTY(BlueprintReadOnly) FText  TerrainName;
	UPROPERTY(BlueprintReadOnly) int32  TotalPop       = 0;
	UPROPERTY(BlueprintReadOnly) int32  BuildingSlots  = 3;
	UPROPERTY(BlueprintReadOnly) int32  UsedSlots      = 0;
	UPROPERTY(BlueprintReadOnly) float  MonthlyIncome  = 0.f;
	UPROPERTY(BlueprintReadOnly) float  Militancy      = 0.f;
};

/**
 * UStrategosProvinceDockWidget — dock colapsável no canto inferior-direito do HUD.
 *
 * Inspirado na tela de Província do Victoria 3: 4 abas independentes
 * (Visão Geral, Edifícios, População, Informação) servidas por getters C++.
 * O BP child define o layout visual; o C++ entrega os dados e responde aos
 * eventos de seleção de província via MapSubsystem.
 *
 * Tamanho colapsado: 44px de altura (strip com nome + nação).
 * Tamanho expandido: 760×560 (3 colunas — definido no BP).
 */
UCLASS(Abstract)
class STRATEGOSUI_API UStrategosProvinceDockWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ── Controle de aba ───────────────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "Strategos|ProvinceDock")
	void SetActiveTab(EProvinceDockTab NewTab);

	UFUNCTION(BlueprintPure, Category = "Strategos|ProvinceDock")
	EProvinceDockTab GetActiveTab() const { return ActiveTab; }

	// ── Dados da província selecionada ────────────────────────────────────────

	/** Resumo para a strip colapsada e header expandido. */
	UFUNCTION(BlueprintPure, Category = "Strategos|ProvinceDock")
	FProvinceSummary GetProvinceSummary() const;

	// ── Aba: Visão Geral ──────────────────────────────────────────────────────

	UFUNCTION(BlueprintPure, Category = "Strategos|ProvinceDock|Overview")
	float GetProvinceMonthlyIncome() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|ProvinceDock|Overview")
	int32 GetProvinceTotalPop() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|ProvinceDock|Overview")
	float GetProvinceMilitancy() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|ProvinceDock|Overview")
	float GetProvinceStability() const;

	// ── Aba: Edifícios ────────────────────────────────────────────────────────

	UFUNCTION(BlueprintPure, Category = "Strategos|ProvinceDock|Buildings")
	TArray<FProvinceBuildingRow> GetBuildingRows() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|ProvinceDock|Buildings")
	int32 GetFreeBuildingSlots() const;

	/** Sugere o próximo prédio a construir com base no terreno/recursos. */
	UFUNCTION(BlueprintPure, Category = "Strategos|ProvinceDock|Buildings")
	FText GetNextBuildingSuggestion() const;

	/** Inicia construção do prédio indicado na província selecionada. */
	UFUNCTION(BlueprintCallable, Category = "Strategos|ProvinceDock|Buildings")
	void RequestBuildConstruction(FName BuildingTypeId);

	// ── Aba: População ────────────────────────────────────────────────────────

	UFUNCTION(BlueprintPure, Category = "Strategos|ProvinceDock|Population")
	TArray<FProvincePopRow> GetPopRows() const;

	// ── Aba: Informação ───────────────────────────────────────────────────────

	UFUNCTION(BlueprintPure, Category = "Strategos|ProvinceDock|Information")
	FText GetTerrainDescription() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|ProvinceDock|Information")
	TArray<FName> GetRawResourceIds() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|ProvinceDock|Information")
	float GetRawResourcePotential(FName GoodId) const;

	// ── Ações ─────────────────────────────────────────────────────────────────

	/** Recruta exército básico na província. Válido se jogador for dono. */
	UFUNCTION(BlueprintCallable, Category = "Strategos|ProvinceDock")
	void RequestRecruitArmy();

	// ── BP overrides ──────────────────────────────────────────────────────────

	/** Chamado quando a seleção de província muda — atualiza a dock. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Strategos|ProvinceDock")
	void OnProvinceChanged(FName NewProvinceId);

	/** Chamado quando uma construção termina nesta província. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Strategos|ProvinceDock")
	void OnBuildingCompleted(FName BuildingId);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void HandleProvinceSelected(FName ProvinceId);

	const UProvince* ResolveProvince() const;
	const UNation*   ResolveOwnerNation() const;
	UMapSubsystem*   ResolveMap() const;

	UPROPERTY() EProvinceDockTab ActiveTab = EProvinceDockTab::Overview;
};
