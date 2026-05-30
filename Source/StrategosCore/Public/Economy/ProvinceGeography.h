#pragma once

#include "CoreMinimal.h"
#include "World/ProvinceGeographyTypes.h"
#include "ProvinceGeography.generated.h"

/**
 * FProvinceGeography — Geografia NATURAL e estática da província (Camada 1).
 *
 * Quatro eixos independentes (relevo / clima / vegetação / hidrografia) +
 * fertilidade e recurso principal derivados. É a fonte única da geografia da
 * província: ETerrainType (legado) passou a ser DERIVADO disto via
 * UProvince::GetTerrain(); EWaterAccessType vem de Hydrography.GetWaterAccessTier().
 *
 * Populada pela geração de mundo (StrategosWorldGen::GeographyClassifier) ou
 * pelo bootstrap handcrafted. Imutável em runtime nesta fase — mudanças por ação
 * do jogador (desmatar/drenar/irrigar) são camada humana de sessão futura.
 *
 * Slots foram removidos daqui (eram código morto e duplicavam
 * UProvince::BuildingSlots, que é o pool único do Alpha — ver Sessão 2).
 */
USTRUCT(BlueprintType)
struct STRATEGOSCORE_API FProvinceGeography
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Geography")
	ETerrainTopography Topography = ETerrainTopography::Flatland;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Geography")
	EClimateZone Climate = EClimateZone::Continental;

	/** Cobertura vegetal natural (clímax). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Geography")
	EVegetationCover Vegetation = EVegetationCover::Grassland;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Geography")
	FHydrography Hydrography;

	/** Fertilidade do solo em [0..1]. 0 = estéril, 1 = excepcional. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Geography", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Fertility = 0.5f;

	/** Bem que recebe bônus de Principal (+20%). None = nenhum. Resolvido na Sessão 4. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Geography|Resources")
	FName PrincipalResourceId;
};
