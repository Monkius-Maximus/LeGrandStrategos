#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "World/ArmyStats.h"
#include "UnitTypeAsset.generated.h"

class UTexture2D;

/**
 * UUnitTypeAsset — "carta-template" de um tipo de unidade (Infantaria de Linha,
 * Cavalaria de Linha, Artilharia de Campanha, etc).
 *
 * Cada UArmy aponta para um UUnitTypeAsset via TSoftObjectPtr. A UI da carta
 * (compact/micro/detalhada) lê dados estáticos daqui (nome, portrait, stats
 * base) e mescla com o estado runtime da UArmy (XP, modificadores, estado).
 *
 * Variantes (Guarda Imperial, Caçadores à Pé, Zouaves) ficam em AvailableVariants
 * — cada variante é outro UUnitTypeAsset. Forma uma árvore rasa de especialização.
 */
UCLASS(BlueprintType)
class STRATEGOSCORE_API UUnitTypeAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Identificador único. Convenção: U_<Tipo>_<Variante>, ex.: U_Infantry_Line. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity", meta = (MultiLine = true))
	FText Description;

	/** Função tática curta (ex.: "Linha de Frente", "Cavalaria Pesada"). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText Role;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	TSoftObjectPtr<UTexture2D> Portrait;

	/** Ícone da classe (espada, cavalo, canhão). 48×48. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	TSoftObjectPtr<UTexture2D> ClassIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	FArmyStats BaseStats;

	/** Traço passivo principal mostrado na carta Compact. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traits")
	FText PrimaryTrait;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Traits", meta = (MultiLine = true))
	FText PrimaryTraitDescription;

	/** Variantes desta unidade (Guarda Imperial, Reserva, etc). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Variants")
	TArray<TSoftObjectPtr<UUnitTypeAsset>> AvailableVariants;

	/** Tamanho típico (homens). Cosmético — usado só na exibição. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 NominalSize = 1000;

	/** Tempo de mobilização em dias (cosmético no MVP, funcional pós-Battle). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 MobilizationDays = 3;
};
