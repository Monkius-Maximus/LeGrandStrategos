#pragma once

#include "CoreMinimal.h"
#include "WaterAccessType.generated.h"

/**
 * EWaterAccessType — Tipo de acesso hídrico da província.
 *
 * Gate para o âmbito Aquífero (peixe, sal, pérolas) e multiplicador de
 * Largura de Banda logística: ribeirinhas e costeiras movimentam carga
 * por água a custo muito inferior a estradas de terra (ver FLogisticsBandwidth).
 *
 * None       : sem corpo d'água navegável
 * MinorRiver : riacho ou lago pequeno; uso doméstico apenas
 * MajorRiver : rio navegável; bônus logístico significativo
 * Coastal    : litoral marítimo; gate para portos e comércio oceânico
 */
UENUM(BlueprintType)
enum class EWaterAccessType : uint8
{
	None		UMETA(DisplayName = "No Access"),
	MinorRiver	UMETA(DisplayName = "Minor River"),
	MajorRiver	UMETA(DisplayName = "Major River"),
	Coastal		UMETA(DisplayName = "Coastal")
};
