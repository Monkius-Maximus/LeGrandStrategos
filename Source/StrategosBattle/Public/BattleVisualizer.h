#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleTypes.h"
#include "BattleProposal.h"
#include "BattleVisualizer.generated.h"

class UBattleSubsystem;

/**
 * ABattleVisualizer — ator que espelha o estado do BattleSubsystem em visuais 2D.
 * Etapa 9: skeleton. Sprites e layout de regimentos entram quando assets estiverem prontos.
 * Instanciar no nível de batalha e chamar BindToSubsystem() no BeginPlay do GameMode.
 */
UCLASS()
class STRATEGOSBATTLE_API ABattleVisualizer : public AActor
{
	GENERATED_BODY()
public:
	ABattleVisualizer();

	/** Conecta ao BattleSubsystem e assina todos os delegates. */
	UFUNCTION(BlueprintCallable, Category = "Strategos|Battle")
	void BindToSubsystem(UBattleSubsystem* Subsystem);

	/** Desconecta delegates (chamado em Destroyed automaticamente). */
	UFUNCTION(BlueprintCallable, Category = "Strategos|Battle")
	void UnbindFromSubsystem();

protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	UFUNCTION() void OnBattleStarted();
	UFUNCTION() void OnPhaseChanged(EBattlePhase OldPhase, EBattlePhase NewPhase);
	UFUNCTION() void OnRoundEnded(int32 Round);
	UFUNCTION() void OnSideRouted(int32 SideIndex);
	UFUNCTION() void OnCardPlayed(int32 SideIndex, FName CardId);
	UFUNCTION() void OnBattleFinished(FBattleResult Result);

	/** Chamado sempre que o estado de batalha muda — atualizar posições visuais. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Strategos|Battle")
	void BP_RefreshVisuals();

private:
	UPROPERTY()
	TWeakObjectPtr<UBattleSubsystem> BoundSubsystem;

	void RefreshVisuals();
};
