#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BattleTypes.h"
#include "BattleProposal.h"
#include "BattleHUDWidget.generated.h"

class UBattleSubsystem;

/**
 * UBattleHUDWidget — HUD principal da batalha tática.
 * Etapa 9: skeleton com delegates vinculados e BlueprintImplementableEvents.
 * Layout UMG (slots de cartas, barras de moral/força, log) entra quando
 * os assets de UI estiverem prontos no editor.
 *
 * Nomenclatura BP: WBP_BattleHUD.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class STRATEGOSBATTLE_API UBattleHUDWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	/**
	 * Conecta o HUD ao BattleSubsystem e assina os delegates.
	 * Chamar logo após AddToViewport(), antes de InitBattle().
	 */
	UFUNCTION(BlueprintCallable, Category = "Strategos|Battle")
	void InitializeHUD(UBattleSubsystem* Subsystem);

	// ── Eventos expostos ao Blueprint ─────────────────────────────────────────

	/** Disparado a cada mudança de fase — atualizar texto de fase e animações. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Strategos|Battle")
	void OnPhaseChangedUI(EBattlePhase OldPhase, EBattlePhase NewPhase);

	/** Disparado ao fim de cada round — atualizar barras de moral e força. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Strategos|Battle")
	void OnRoundEndedUI(int32 Round, const FBattleContext& BattleCtx);

	/** Disparado quando uma carta é jogada — animar slot de carta. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Strategos|Battle")
	void OnCardPlayedUI(int32 SideIndex, FName CardId);

	/** Disparado quando um lado foge — mostrar ícone de debandada. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Strategos|Battle")
	void OnSideRoutedUI(int32 SideIndex);

	/** Disparado ao fim da batalha — exibir tela de resultado. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Strategos|Battle")
	void OnBattleFinishedUI(const FBattleResult& Result);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION() void HandlePhaseChanged(EBattlePhase OldPhase, EBattlePhase NewPhase);
	UFUNCTION() void HandleRoundEnded(int32 Round);
	UFUNCTION() void HandleCardPlayed(int32 SideIndex, FName CardId);
	UFUNCTION() void HandleSideRouted(int32 SideIndex);
	UFUNCTION() void HandleBattleFinished(FBattleResult Result);

private:
	UPROPERTY()
	TWeakObjectPtr<UBattleSubsystem> BoundSubsystem;
};
