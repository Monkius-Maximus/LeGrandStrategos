#pragma once

#include "CoreMinimal.h"
#include "TechSphere.generated.h"

/**
 * ETechSphere — Área temática de uma tecnologia.
 */
UENUM(BlueprintType)
enum class ETechSphere : uint8
{
	PoliticalSocial		UMETA(DisplayName = "Political & Social"),
	ProductionEnergy	UMETA(DisplayName = "Production & Energy"),
	Economy				UMETA(DisplayName = "Economy"),
	Military			UMETA(DisplayName = "Military"),
	Communication		UMETA(DisplayName = "Communication"),
	CultureWorldview	UMETA(DisplayName = "Culture & Worldview"),
	NaturalScience		UMETA(DisplayName = "Natural Science")
};
