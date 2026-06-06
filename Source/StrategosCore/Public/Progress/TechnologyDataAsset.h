#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Progress/TechSphere.h"
#include "Progress/TechTrack.h"
#include "Economy/PopStratum.h"
#include "TechnologyDataAsset.generated.h"

/**
 * UTechnologyDataAsset — Definição de uma tecnologia emergente.
 *
 * Instâncias (conteúdo) são criadas no TE-3. Esta classe é apenas esquema.
 * RequiredGoods referencia bens pelo campo Id do UGoodAsset (FName simples, ex.: "Coal").
 * GroupSentiment: chaves = nomes de estratos como FName; valores modulam difusão (TE-3+).
 */
UCLASS(BlueprintType)
class STRATEGOSCORE_API UTechnologyDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Technology")
	FName TechId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Technology")
	ETechSphere Sphere = ETechSphere::NaturalScience;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Technology")
	ETechTrack Track = ETechTrack::CivilFlow;

	/** Bens consumidos no momento da adoção (pelo Id do UGoodAsset). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Technology")
	TArray<FName> RequiredGoods;

	/** Tecnologias que devem estar presentes antes desta poder emergir. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Technology")
	TArray<FName> RequiredPrereqTech;

	/** Estrato mínimo necessário para a faísca ocorrer nesta província. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Technology")
	EPopStratum RequiredStratum = EPopStratum::Laborer;

	/** Parcela mínima do estrato requerido na população provincial (0..1). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Technology", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RequiredStratumShare = 0.0f;

	/** Piso de alfabetização provincial para a faísca (0..1). Usado a partir do TE-2. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Technology", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LiteracyFloor = 0.0f;

	/** Probabilidade anual de faísca quando condições são atendidas. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Technology", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BaseSparkChance = 0.0f;

	/** Ganho mensal de presença durante difusão. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Technology", meta = (ClampMin = "0.0"))
	float DiffusionBase = 0.0f;

	/**
	 * Sentimento dos grupos em relação a esta tecnologia.
	 * Chaves = nomes de estratos (FName); valores positivos aceleram difusão, negativos freiam.
	 * Mapeamento completo entra no TE-3.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Technology")
	TMap<FName, float> GroupSentiment;
};
