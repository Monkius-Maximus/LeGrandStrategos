#pragma once

#include "CoreMinimal.h"
#include "StrategicIndices.generated.h"

/**
 * FStrategicIndices — Snapshot mensal de "saúde" econômica de uma nação.
 *
 * Computado pelo UEconomySubsystem ao final de cada tick a partir das
 * razões Supply/Demand de bens-chave. Forward hook para futuros
 * subsistemas (Battle, Politics, Events) consumirem sem precisar
 * conhecer detalhes internos da Economy.
 *
 * Convenção: 1.0 = neutro; <1.0 = pressão negativa; >1.0 = abundância.
 * Range clampado [0.5, 1.5].
 */
USTRUCT(BlueprintType)
struct STRATEGOSCORE_API FStrategicIndices
{
	GENERATED_BODY()

	/** Tools × Iron supply ratios. Cartas militares e velocidade de exército lerão isto. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Indices")
	float MilitaryReadinessIndex = 1.0f;

	/** Bread × Garments supply ratios. Eventos e políticas lerão isto. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Indices")
	float CivilianMoraleIndex = 1.0f;

	/** Coal supply ratio. Modificadores mecanizados sofrem se este cai. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Indices")
	float IndustrialCapacityIndex = 1.0f;
};
