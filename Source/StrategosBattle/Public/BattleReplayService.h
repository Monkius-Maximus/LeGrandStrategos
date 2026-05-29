#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "BattleTypes.h"
#include "BattleProposal.h"
#include "BattleReplayService.generated.h"

/**
 * UBattleReplayService — navega o log de uma batalha resolvida para replay pós-partida.
 * Carrega o TArray<FBattleLogEntry> de um FBattleResult e permite passo a passo.
 *
 * Etapa 10: serviço puro (sem visuais). ABattleVisualizer pode consumir as entradas
 * para reconstituir o estado a qualquer ponto da batalha.
 */
UCLASS(BlueprintType)
class STRATEGOSBATTLE_API UBattleReplayService : public UObject
{
	GENERATED_BODY()
public:
	/** Carrega entradas do resultado de batalha. Reseta o cursor para o início. */
	UFUNCTION(BlueprintCallable, Category = "Strategos|Battle|Replay")
	void LoadFromResult(const FBattleResult& Result);

	/** Avança para a próxima entrada. Retorna false se já no fim. */
	UFUNCTION(BlueprintCallable, Category = "Strategos|Battle|Replay")
	bool StepForward();

	/** Volta para a entrada anterior. Retorna false se já no início. */
	UFUNCTION(BlueprintCallable, Category = "Strategos|Battle|Replay")
	bool StepBackward();

	/** Salta para o índice especificado. Retorna false se fora do intervalo. */
	UFUNCTION(BlueprintCallable, Category = "Strategos|Battle|Replay")
	bool JumpToEntry(int32 Index);

	/** Entrada no cursor atual. Retorna entrada vazia se log não carregado. */
	UFUNCTION(BlueprintPure, Category = "Strategos|Battle|Replay")
	const FBattleLogEntry& GetCurrentEntry() const;

	UFUNCTION(BlueprintPure, Category = "Strategos|Battle|Replay")
	int32 GetCurrentIndex() const { return CurrentIndex; }

	UFUNCTION(BlueprintPure, Category = "Strategos|Battle|Replay")
	int32 GetTotalEntries() const { return Entries.Num(); }

	UFUNCTION(BlueprintPure, Category = "Strategos|Battle|Replay")
	bool IsLoaded() const { return !Entries.IsEmpty(); }

	UFUNCTION(BlueprintPure, Category = "Strategos|Battle|Replay")
	bool IsAtEnd() const { return CurrentIndex >= Entries.Num() - 1; }

	UFUNCTION(BlueprintPure, Category = "Strategos|Battle|Replay")
	bool IsAtStart() const { return CurrentIndex <= 0; }

private:
	TArray<FBattleLogEntry> Entries;
	int32 CurrentIndex = -1;
	FBattleLogEntry EmptyEntry;
};
