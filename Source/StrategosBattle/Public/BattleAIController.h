#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "BattleTypes.h"
#include "BattleAIController.generated.h"

class UBattleSubsystem;
class UBattleAIProfile;
class UBattleCardAsset;

/**
 * UBattleAIController — decide quais cartas jogar em cada round para um lado.
 *
 * Não é um AAIController (que é para Pawns). É um UObject puro instanciado
 * pelo BattleSubsystem para o lado controlado por IA.
 *
 * Stage 6 (atual): Utility AI sem lookahead — pontua cada carta na mão e
 * escolhe a de maior score que caiba no CommandPoints.
 * Stage 8: adiciona lookahead 1-round simulado + temperatura por perfil.
 *
 * Ver docs/architecture/10-battle.md § 7.
 */
UCLASS()
class STRATEGOSBATTLE_API UBattleAIController : public UObject
{
	GENERATED_BODY()
public:
	void Initialize(UBattleSubsystem* InOwner, int32 InSideIndex,
	                UBattleAIProfile* InProfile);

	/**
	 * Escolhe as cartas a jogar neste round.
	 * Chamado por BattleSubsystem na DeclarePhase antes de ResolveDeclarations.
	 */
	FBattleDeclaration ChooseDeclaration(const FBattleContext& Ctx,
	                                      const FBattleSide& Self,
	                                      const FBattleSide& Enemy) const;

private:
	float ScoreCard(const UBattleCardAsset* Card,
	                const FBattleContext& Ctx,
	                const FBattleSide& Self,
	                const FBattleSide& Enemy) const;

	UPROPERTY()
	TWeakObjectPtr<UBattleSubsystem> Owner;

	UPROPERTY()
	TObjectPtr<UBattleAIProfile> Profile;

	int32 SideIndex = 0;
};
