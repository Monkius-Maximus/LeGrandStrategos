#pragma once

#include "CoreMinimal.h"
#include "ResourceScope.generated.h"

/**
 * EResourceScope — Âmbito de exploração de RGO (Resource Gathering Operation).
 *
 * Cada slot de construção da província pertence a um âmbito; o que pode ser
 * extraído em cada âmbito depende das variáveis geográficas (ver
 * URGOTemplateAsset):
 *
 * Vegetal : depende de Clima + Fertilidade. Ex: Trigo, Gado, Algodão, Madeira.
 * Mineral : depende de Relevo. Ignora clima/fertilidade. Ex: Ferro, Carvão.
 * Aquifer : exige EWaterAccessType > None. Ex: Peixe, Sal, Pérolas.
 */
UENUM(BlueprintType)
enum class EResourceScope : uint8
{
	Vegetal		UMETA(DisplayName = "Vegetal"),
	Mineral		UMETA(DisplayName = "Mineral"),
	Aquifer		UMETA(DisplayName = "Aquifer")
};

/**
 * EResourceRole — Marca o papel econômico do recurso na província.
 *
 * Principal recebe +20% de eficiência em construções base (ver doc §2).
 * Alternative ocupa slots restantes sem bônus.
 */
UENUM(BlueprintType)
enum class EResourceRole : uint8
{
	Principal	UMETA(DisplayName = "Principal Resource"),
	Alternative	UMETA(DisplayName = "Alternative Resource")
};
