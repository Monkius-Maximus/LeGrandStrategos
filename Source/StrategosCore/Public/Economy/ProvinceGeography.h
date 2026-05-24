#pragma once

#include "CoreMinimal.h"
#include "World/TerrainType.h"
#include "Economy/ClimateType.h"
#include "Economy/WaterAccessType.h"
#include "ProvinceGeography.generated.h"

/**
 * FProvinceGeography — Bundle das variáveis geográficas que multiplicam
 * o potencial econômico da província.
 *
 * Esta struct é o **input** consumido por URGOTemplateAsset para resolver,
 * em bootstrap, o conjunto de bens viáveis e seus multiplicadores em
 * UProvince::RawResourcePotential. Não é mutável em tempo de execução
 * (mudanças climáticas / desertificação são fora de escopo no MVP).
 *
 * Fertility e ConstructionSlotCount são derivados primariamente de Terrain
 * + Climate na geração do mapa, mas ficam editáveis para overrides manuais
 * nas províncias canônicas (Albion/Galia/Norden).
 */
USTRUCT(BlueprintType)
struct STRATEGOSCORE_API FProvinceGeography
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Geography")
	ETerrainType Terrain = ETerrainType::Plains;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Geography")
	EClimateType Climate = EClimateType::Temperate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Geography")
	EWaterAccessType WaterAccess = EWaterAccessType::None;

	/** Fertilidade do solo em [0..1]. 0 = estéril, 1 = excepcional. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Geography", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Fertility = 0.5f;

	/**
	 * Quantos slots de cada âmbito a província oferece. A soma define o teto
	 * total de prédios; o split entre Vegetal/Mineral/Aquifer determina
	 * quais âmbitos o jogador pode diversificar.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Geography|Slots", meta = (ClampMin = "0"))
	int32 VegetalSlots = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Geography|Slots", meta = (ClampMin = "0"))
	int32 MineralSlots = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Geography|Slots", meta = (ClampMin = "0"))
	int32 AquiferSlots = 0;

	/** Identificador do bem que recebe bônus de Principal (+20%). None = nenhum. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Geography|Resources")
	FName PrincipalResourceId;
};
