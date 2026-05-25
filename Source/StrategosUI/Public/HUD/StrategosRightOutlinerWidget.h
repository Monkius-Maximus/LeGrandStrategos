#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Diplomacy/DiplomaticRelation.h"
#include "StrategosRightOutlinerWidget.generated.h"

class UMapSubsystem;
class UDiplomacySubsystem;

UENUM(BlueprintType)
enum class EOutlinerTab : uint8
{
	Outline UMETA(DisplayName = "Outliner"),
	Diary   UMETA(DisplayName = "Diário"),
};

// ── Row structs ───────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct STRATEGOSUI_API FOutlinerArmyRow
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FName  ArmyId;
	UPROPERTY(BlueprintReadOnly) FText  ArmyName;
	UPROPERTY(BlueprintReadOnly) FText  ProvinceName;
	UPROPERTY(BlueprintReadOnly) int32  Manpower      = 0;
	UPROPERTY(BlueprintReadOnly) bool   bMoving        = false;
	UPROPERTY(BlueprintReadOnly) int32  MoveDaysLeft   = 0;
	UPROPERTY(BlueprintReadOnly) FLinearColor NationColor = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct STRATEGOSUI_API FOutlinerDiploRow
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FName           CounterpartId;
	UPROPERTY(BlueprintReadOnly) FText           CounterpartName;
	UPROPERTY(BlueprintReadOnly) EDiplomaticStatus Status = EDiplomaticStatus::Peace;
	UPROPERTY(BlueprintReadOnly) float            Opinion  = 0.f;
	UPROPERTY(BlueprintReadOnly) FLinearColor     NationColor = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct STRATEGOSUI_API FOutlinerResearchRow
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FName  TechId;
	UPROPERTY(BlueprintReadOnly) FText  TechName;
	UPROPERTY(BlueprintReadOnly) FText  TrackName;
	UPROPERTY(BlueprintReadOnly) float  Progress   = 0.f;  // [0..1]
	UPROPERTY(BlueprintReadOnly) int32  DaysLeft   = 0;
};

USTRUCT(BlueprintType)
struct STRATEGOSUI_API FOutlinerConstructionRow
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FName  BuildingId;
	UPROPERTY(BlueprintReadOnly) FText  BuildingTypeName;
	UPROPERTY(BlueprintReadOnly) FText  ProvinceName;
	UPROPERTY(BlueprintReadOnly) float  Progress   = 0.f;
	UPROPERTY(BlueprintReadOnly) int32  DaysLeft   = 0;
};

USTRUCT(BlueprintType)
struct STRATEGOSUI_API FOutlinerNotificationRow
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FName  EventId;
	UPROPERTY(BlueprintReadOnly) FText  Title;
	UPROPERTY(BlueprintReadOnly) FText  Body;
	UPROPERTY(BlueprintReadOnly) FText  DateLabel;
	/** "alert" | "diplo" | "econ" | "pol" */
	UPROPERTY(BlueprintReadOnly) FName  Category;
};

/**
 * UStrategosRightOutlinerWidget — painel lateral direito estilo Vic2/EU4.
 *
 * Duas abas: "Outliner" (exércitos, diplomacia, pesquisa, construção)
 * e "Diário" (log de notificações filtrável).
 *
 * Largura padrão: 320px. Colapsável para 40px liberando o mapa.
 */
UCLASS(Abstract)
class STRATEGOSUI_API UStrategosRightOutlinerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Strategos|Outliner")
	void SetActiveTab(EOutlinerTab NewTab);

	UFUNCTION(BlueprintPure, Category = "Strategos|Outliner")
	EOutlinerTab GetActiveTab() const { return ActiveTab; }

	// ── Seção: Exércitos & Frotas ─────────────────────────────────────────────
	UFUNCTION(BlueprintPure, Category = "Strategos|Outliner|Armies")
	TArray<FOutlinerArmyRow> GetArmyRows() const;

	/** Seleciona a província do exército no mapa. */
	UFUNCTION(BlueprintCallable, Category = "Strategos|Outliner|Armies")
	void FocusArmy(FName ArmyId);

	// ── Seção: Diplomacia ─────────────────────────────────────────────────────
	UFUNCTION(BlueprintPure, Category = "Strategos|Outliner|Diplomacy")
	TArray<FOutlinerDiploRow> GetDiploRows() const;

	// ── Seção: Pesquisa ───────────────────────────────────────────────────────
	UFUNCTION(BlueprintPure, Category = "Strategos|Outliner|Research")
	TArray<FOutlinerResearchRow> GetResearchRows() const;

	// ── Seção: Construção ────────────────────────────────────────────────────
	UFUNCTION(BlueprintPure, Category = "Strategos|Outliner|Construction")
	TArray<FOutlinerConstructionRow> GetConstructionRows() const;

	// ── Seção: Diário ─────────────────────────────────────────────────────────
	UFUNCTION(BlueprintPure, Category = "Strategos|Outliner|Diary")
	TArray<FOutlinerNotificationRow> GetNotificationRows() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|Outliner|Diary")
	int32 GetUnreadNotificationCount() const;

	UFUNCTION(BlueprintCallable, Category = "Strategos|Outliner|Diary")
	void MarkAllNotificationsRead();

	// ── BP overrides ──────────────────────────────────────────────────────────
	UFUNCTION(BlueprintImplementableEvent, Category = "Strategos|Outliner")
	void OnRelationChanged(FName CounterpartId);

	UFUNCTION(BlueprintImplementableEvent, Category = "Strategos|Outliner")
	void OnNewNotification(const FOutlinerNotificationRow& Row);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void HandleRelationChanged(const FDiplomaticRelation& Relation);

	UFUNCTION()
	void HandleEventFired(const struct FEventContext& Context);

	UDiplomacySubsystem* ResolveDiplo() const;
	const UWorldState*   ResolveWorldState() const;

	UPROPERTY() EOutlinerTab ActiveTab = EOutlinerTab::Outline;

	UPROPERTY()
	TArray<FOutlinerNotificationRow> NotificationLog;

	UPROPERTY()
	int32 UnreadCount = 0;
};
