#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "World/TerrainType.h"
#include "Economy/ClimateType.h"
#include "Economy/WaterAccessType.h"
#include "Economy/ResourceScope.h"
#include "RGOTemplateAsset.generated.h"

class UGoodAsset;

/**
 * FRGOYieldRule — Regra única "se condição geográfica → este bem com este peso".
 *
 * Avaliação: o EconomySubsystem soma TerrainMultiplier * ClimateMultiplier *
 * FertilityCurve(F) * WaterMultiplier (cada um default 1.0 quando não
 * aplicável). Resultado é o potencial bruto que vira RawResourcePotential
 * na província.
 *
 * GoodId é resolvido contra UEconomyContentRegistry no bootstrap.
 */
USTRUCT(BlueprintType)
struct STRATEGOSCORE_API FRGOYieldRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rule")
	FName GoodId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rule")
	EResourceScope Scope = EResourceScope::Vegetal;

	/** Vazio = qualquer terreno; senão, terreno tem que pertencer ao set. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rule|Conditions")
	TSet<ETerrainType> EligibleTerrain;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rule|Conditions")
	TSet<EClimateType> EligibleClimate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rule|Conditions")
	TSet<EWaterAccessType> EligibleWaterAccess;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rule|Conditions", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinFertility = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rule|Conditions", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxFertility = 1.0f;

	/** Peso base do bem se todas as condições passam. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rule|Yield", meta = (ClampMin = "0.0"))
	float BaseYield = 1.0f;

	/** Bens com mesma flag mas yield diferente — o de maior virou Principal. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rule|Yield")
	bool bEligibleAsPrincipal = true;
};

/**
 * URGOTemplateAsset — Tabela de regras de RGO de toda a campanha.
 *
 * Centraliza a matriz Clima x Terreno x Fertilidade x Água → Bens viáveis
 * num único DataAsset editável. O EconomySubsystem aplica suas regras a
 * cada FProvinceGeography no bootstrap para popular RawResourcePotential
 * e PrincipalResourceId.
 *
 * Múltiplos templates podem coexistir (ex.: mod de campanha colonial).
 * O registry default é resolvido via UEconomyContentRegistry.
 */
UCLASS(BlueprintType)
class STRATEGOSCORE_API URGOTemplateAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RGO")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RGO")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RGO")
	TArray<FRGOYieldRule> Rules;
};
