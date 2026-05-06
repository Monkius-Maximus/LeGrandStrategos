#pragma once

#include "CoreMinimal.h"
#include "LeaderArchetype.generated.h"

/**
 * ELeaderArchetype — personalidade dominante de um líder nacional.
 *
 * Para a Etapa 1 (placeholder de IA), só Militarist e Diplomat têm
 * comportamento observável. Os demais ficam como hooks documentados
 * que serão usados quando os subsistemas correspondentes existirem:
 *
 *  - Merchant     → Etapa 2 (UEconomySubsystem)
 *  - Religious    → Etapa 3 (UPoliticsSubsystem)
 *  - Intellectual → Etapa 3 (UProgressSubsystem)
 *  - Pragmatist   → fallback neutro (move random adjacente)
 */
UENUM(BlueprintType)
enum class ELeaderArchetype : uint8
{
	Militarist		UMETA(DisplayName = "Militarist"),
	Diplomat		UMETA(DisplayName = "Diplomat"),
	Merchant		UMETA(DisplayName = "Merchant"),
	Religious		UMETA(DisplayName = "Religious"),
	Intellectual	UMETA(DisplayName = "Intellectual"),
	Pragmatist		UMETA(DisplayName = "Pragmatist")
};
