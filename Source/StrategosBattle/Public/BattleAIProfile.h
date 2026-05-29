#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BattleTypes.h"
#include "BattleAIProfile.generated.h"

/**
 * UBattleAIProfile — personalidade tática de uma IA de batalha.
 *
 * Crie DataAssets no editor com prefixo BAI_ (ex.: BAI_Militarista, BAI_Diplomata).
 * Cada nação/arquétipo usa um perfil distinto; sem perfil, usa os defaults (Pragmatista).
 */
UCLASS(BlueprintType)
class STRATEGOSBATTLE_API UBattleAIProfile : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	/** 0 = defensivo, 1 = muito agressivo. Afeta preferência por Assault vs Support. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Comportamento",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Aggression = 0.5f;

	/** 0 = avesso ao risco, 1 = ignora baixa moral/suprimento. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Comportamento",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RiskTolerance = 0.5f;

	/** Quanto a IA considera bônus de terreno ao pontuar cartas de Maneuver. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Comportamento",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TerrainAwareness = 0.5f;

	/** Multiplicador de score por categoria de carta. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Comportamento")
	TMap<ECardCategory, float> CategoryBias;

	/** Temperatura do softmax — Stage 8 (lookahead). Maior = mais aleatório. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Avançado")
	float LookaheadTemperature = 1.0f;

	/** Retorna o bias da categoria (1.0 se não configurado). */
	float GetCategoryBias(ECardCategory Cat) const;
};
