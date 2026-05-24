#pragma once

#include "CoreMinimal.h"
#include "ClimateType.generated.h"

/**
 * EClimateType — Classificação climática da província.
 *
 * Junto com ETerrainType e Fertility define quais bens vegetais a
 * província pode produzir e quais penalidades de mortalidade/migração
 * são aplicadas a POPs sensíveis (ver UProvince + UEconomySubsystem).
 *
 * Freezing  : tundras e estepes do extremo norte/sul
 * Cold      : invernos rigorosos, verão curto
 * Temperate : default europeu, ampla faixa de cultivos
 * Arid      : desertos, semiárido; flora limitada
 * Tropical  : clima quente úmido; algodão, borracha, açúcar
 */
UENUM(BlueprintType)
enum class EClimateType : uint8
{
	Freezing	UMETA(DisplayName = "Freezing"),
	Cold		UMETA(DisplayName = "Cold"),
	Temperate	UMETA(DisplayName = "Temperate"),
	Arid		UMETA(DisplayName = "Arid"),
	Tropical	UMETA(DisplayName = "Tropical")
};
