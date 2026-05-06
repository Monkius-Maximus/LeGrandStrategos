#pragma once

#include "CoreMinimal.h"
#include "PopStratum.generated.h"

/**
 * EPopStratum — Estratos econômicos da população.
 *
 * Estratos ativos na Etapa 2 v1: Laborer (peão), Artisan, FactoryWorker,
 * Bourgeoisie. Aristocracy, Soldier e Clergy estão declarados mas
 * permanecem stub econômico até Politics/Military/Religion entrarem
 * (Etapa 3).
 */
UENUM(BlueprintType)
enum class EPopStratum : uint8
{
	Laborer			UMETA(DisplayName = "Laborer"),
	Artisan			UMETA(DisplayName = "Artisan"),
	FactoryWorker	UMETA(DisplayName = "Factory Worker"),
	Bourgeoisie		UMETA(DisplayName = "Bourgeoisie"),
	Aristocracy		UMETA(DisplayName = "Aristocracy"),
	Soldier			UMETA(DisplayName = "Soldier"),
	Clergy			UMETA(DisplayName = "Clergy")
};
