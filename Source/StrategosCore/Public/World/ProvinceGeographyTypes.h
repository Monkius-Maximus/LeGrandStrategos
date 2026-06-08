#pragma once

#include "CoreMinimal.h"
#include "Economy/WaterAccessType.h"
#include "ProvinceGeographyTypes.generated.h"

/**
 * Tipos da geografia NATURAL da província (Camada 1 de Recursos e Produção).
 *
 * Quatro eixos independentes, decompostos de propósito (relevo != vegetação
 * != clima != água) para evitar combinações impossíveis e dar granularidade à
 * elegibilidade de recursos. Tudo aqui é a camada "natural": vem da geração de
 * mundo (StrategosWorldGen) ou do bootstrap handcrafted, e é estática. As ações
 * "humanas" do jogador (desmatar, drenar, irrigar) entram como delta numa
 * camada separada, em sessão futura.
 *
 * Agrupados num header só porque pertencem ao mesmo conceito; antes os enums de
 * província viviam dispersos (World/TerrainType, Economy/ClimateType, ...).
 */

/** Relevo. Derivado de altura + relevo local na geração; editável no bootstrap. */
UENUM(BlueprintType)
enum class ETerrainTopography : uint8
{
	Flatland	UMETA(DisplayName = "Planície"),
	Hills		UMETA(DisplayName = "Colinas"),
	Mountains	UMETA(DisplayName = "Montanhas"),
	Plateau		UMETA(DisplayName = "Planalto"),
	Wetlands	UMETA(DisplayName = "Pântano"),
	Coastal		UMETA(DisplayName = "Litoral")
};

/** Zona climática (taxonomia tipo Köppen simplificada, era 1820-1880). */
UENUM(BlueprintType)
enum class EClimateZone : uint8
{
	Tropical		UMETA(DisplayName = "Tropical"),
	Mediterranean	UMETA(DisplayName = "Mediterrâneo"),
	Continental		UMETA(DisplayName = "Continental"),
	Arid			UMETA(DisplayName = "Árido"),
	Arctic			UMETA(DisplayName = "Ártico"),
	Oceanic			UMETA(DisplayName = "Oceânico")
};

/** Cobertura vegetal NATURAL (clímax). A cobertura corrente pós-ação do player
 *  é uma camada humana futura; aqui é só o que o clima/precipitação entregam. */
UENUM(BlueprintType)
enum class EVegetationCover : uint8
{
	DenseForest		UMETA(DisplayName = "Floresta Densa"),
	LightForest		UMETA(DisplayName = "Floresta Esparsa"),
	Grassland		UMETA(DisplayName = "Campo"),
	Wetland			UMETA(DisplayName = "Brejo"),
	Desert			UMETA(DisplayName = "Deserto"),
	Tundra			UMETA(DisplayName = "Tundra")
};

/**
 * FHydrography — Features hídricas que podem COEXISTIR (por isso flags, não
 * enum). Fonte única da água da província: o antigo EWaterAccessType (enum de
 * "melhor acesso") passa a ser DERIVADO daqui via GetWaterAccessTier(), evitando
 * guardar duas representações do mesmo dado.
 */
USTRUCT(BlueprintType)
struct STRATEGOSCORE_API FHydrography
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hydrography")
	bool bHasMinorRiver = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hydrography")
	bool bHasNavigableRiver = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hydrography")
	bool bHasLake = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hydrography")
	bool bHasAquifer = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hydrography")
	bool bIsCoastal = false;

	/**
	 * Tier de acesso hídrico derivado das flags, para logística/portos/RGO que
	 * raciocinam em "melhor acesso" (compatibilidade com EWaterAccessType).
	 * Costeiro > rio navegável > rio menor/lago > nenhum.
	 */
	EWaterAccessType GetWaterAccessTier() const
	{
		if (bIsCoastal)
		{
			return EWaterAccessType::Coastal;
		}
		if (bHasNavigableRiver)
		{
			return EWaterAccessType::MajorRiver;
		}
		if (bHasMinorRiver || bHasLake)
		{
			return EWaterAccessType::MinorRiver;
		}
		return EWaterAccessType::None;
	}
};
