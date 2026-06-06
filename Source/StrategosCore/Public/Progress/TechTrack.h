#pragma once

#include "CoreMinimal.h"
#include "TechTrack.generated.h"

/**
 * ETechTrack — Canal de progressão de uma tecnologia.
 *
 * CivilFlow   : emerge organicamente (faísca + difusão).
 * StateResearch: requer política ativa; ativado em versão futura.
 */
UENUM(BlueprintType)
enum class ETechTrack : uint8
{
	CivilFlow		UMETA(DisplayName = "Civil Flow"),
	StateResearch	UMETA(DisplayName = "State Research")
};
