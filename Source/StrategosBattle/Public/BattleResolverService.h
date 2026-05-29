#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "BattleProposal.h"
#include "BattleResolverService.generated.h"

class UWorldState;

/**
 * UBattleResolverService — resolve batalhas de forma síncrona, sem renderização.
 *
 * Uso típico: UMilitarySubsystem instancia este objeto e chama ResolveQuick()
 * quando dois exércitos de nações inimigas ocupam a mesma província e o
 * jogador não está envolvido (ou escolheu auto-resolve).
 *
 * Usa a mesma fórmula de combate do UBattleSubsystem para garantir
 * consistência entre resultado tático e auto-resolve.
 *
 * Stage 3 (atual): ResolveQuick síncrono.
 * Stage 4+: ResolveAsync via TaskGraph para múltiplas batalhas simultâneas.
 *
 * Ver docs/architecture/10-battle.md § 9.
 */
UCLASS()
class STRATEGOSBATTLE_API UBattleResolverService : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Resolve a batalha descrita pelo proposal de forma síncrona.
	 * WorldState pode ser null — nesse caso o serviço usa stats padrão (ATQ/DEF=50).
	 */
	FBattleResult ResolveQuick(const FBattleProposal& Proposal,
	                           UWorldState* WorldState);
};
