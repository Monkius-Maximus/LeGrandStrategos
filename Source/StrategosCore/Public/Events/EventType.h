#pragma once

#include "CoreMinimal.h"
#include "EventType.generated.h"

/**
 * EEventType — Como o evento é apresentado/aplicado.
 *
 * Decision     : abre painel para o player escolher; AI auto-resolve com
 *                a escolha 0 (ou regra futura por personalidade)
 * Notification : aplica AutoEffects e mostra popup informativo
 * Silent       : aplica AutoEffects sem UI (eventos de "background")
 *
 * Cinematic e Background mais sofisticados aparecem na Etapa 3 conforme
 * docs/architecture/30-events.md.
 */
UENUM(BlueprintType)
enum class EEventType : uint8
{
	Decision		UMETA(DisplayName = "Decision (player choice)"),
	Notification	UMETA(DisplayName = "Notification (auto + popup)"),
	Silent			UMETA(DisplayName = "Silent (auto, no UI)")
};
