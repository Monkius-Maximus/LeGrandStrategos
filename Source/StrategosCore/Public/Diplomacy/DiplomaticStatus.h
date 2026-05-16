#pragma once

#include "CoreMinimal.h"
#include "DiplomaticStatus.generated.h"

/**
 * EDiplomaticStatus — estado bilateral entre duas nações.
 *
 * V1: 4 valores. Vassalagem, Trégua e Embargo entram em iterações futuras.
 */
UENUM(BlueprintType)
enum class EDiplomaticStatus : uint8
{
	Peace				UMETA(DisplayName = "Paz"),
	NonAggressionPact	UMETA(DisplayName = "Pacto de Não-Agressão"),
	Alliance			UMETA(DisplayName = "Aliança"),
	War					UMETA(DisplayName = "Guerra")
};
