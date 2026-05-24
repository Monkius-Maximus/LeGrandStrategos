#pragma once

#include "CoreMinimal.h"
#include "GoodConsumerProfile.generated.h"

/**
 * FGoodLogisticsProfile — Peso de um bem no sistema de Largura de Banda.
 *
 * "Tamanho do lote" do doc §3. Bens de baixa densidade de valor (carvão,
 * trigo) ocupam mais banda por unidade que bens densos (ouro, especiarias).
 * Carregado por UGoodAsset como composição.
 */
USTRUCT(BlueprintType)
struct STRATEGOSCORE_API FGoodLogisticsProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Logistics", meta = (ClampMin = "0.01"))
	float BandwidthCostPerUnit = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Logistics", meta = (ClampMin = "0.01"))
	float StockpileSizePerUnit = 1.0f;
};

/**
 * FGoodPrestigeProfile — Bônus diplomático calculado anualmente para os
 * 5 maiores produtores globais do bem (doc §3).
 *
 * Aplicado por UDiplomacySubsystem; valores em pontos de Prestígio
 * absolutos. Ranking 1 = TopProducerPrestige, 5 = FifthProducerPrestige,
 * lerp linear pelos intermediários.
 */
USTRUCT(BlueprintType)
struct STRATEGOSCORE_API FGoodPrestigeProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prestige", meta = (ClampMin = "0.0"))
	float TopProducerPrestige = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prestige", meta = (ClampMin = "0.0"))
	float FifthProducerPrestige = 0.f;

	/** Custo em Influência (capital político) cobrado de suserano ao taxar marionetes neste bem. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prestige", meta = (ClampMin = "0.0"))
	float InfluenceTaxCost = 0.f;
};

/**
 * FGoodLuxuryProfile — Buff passivo quando o acesso da população excede
 * AccessThreshold (doc §3, ex: Chocolate/Café).
 *
 * SatisfactionBonus aplicado a todos os POPs do estrato consumidor (-
 * insatisfação). 0 = sem efeito de luxo.
 */
USTRUCT(BlueprintType)
struct STRATEGOSCORE_API FGoodLuxuryProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Luxury", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AccessThreshold = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Luxury", meta = (ClampMin = "0.0"))
	float SatisfactionBonus = 0.f;
};

/**
 * FGoodAddictionProfile — Configuração do "efeito ópio" (doc §3).
 *
 * Curva: nos primeiros meses de consumo, SatisfactionBonusEarly reduz
 * insatisfação. Após RampUpMonths, MortalityRatePerMonth e
 * WorkEfficiencyPenalty começam a atuar. Corte abrupto (queda > X% do
 * baseline) dispara WithdrawalRadicalization (acréscimo de militância).
 *
 * Bens sem vício: AddictionChancePerMonth = 0 (default).
 */
USTRUCT(BlueprintType)
struct STRATEGOSCORE_API FGoodAddictionProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Addiction", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AddictionChancePerMonth = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Addiction", meta = (ClampMin = "0"))
	int32 RampUpMonths = 6;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Addiction|Honeymoon", meta = (ClampMin = "0.0"))
	float SatisfactionBonusEarly = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Addiction|Hangover", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MortalityRatePerMonth = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Addiction|Hangover", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WorkEfficiencyPenalty = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Addiction|Withdrawal", meta = (ClampMin = "0.0"))
	float WithdrawalRadicalization = 0.f;
};

/**
 * FPopAddictionState — Estado runtime de vício de um FPopGroup em um bem.
 *
 * Vive em UProvince (TMap<FName GoodId, FPopAddictionState> por estrato)
 * para que save/load preserve o ciclo do efeito ópio.
 */
USTRUCT(BlueprintType)
struct STRATEGOSCORE_API FPopAddictionState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Addiction")
	FName GoodId;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Addiction", meta = (ClampMin = "0"))
	int32 MonthsAddicted = 0;

	/** Baseline de consumo (unidades/mês) que esse pop "espera" no momento. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Addiction", meta = (ClampMin = "0.0"))
	float BaselineConsumption = 0.f;
};
