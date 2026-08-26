#pragma once

#include "CoreMinimal.h"
#include "EventRepeatPolicy.generated.h"

/**
 * EEventRepeatPolicy — Quantas vezes um evento pode disparar.
 *
 * Sem política de repetição, todo evento com MTTH volta a rolar em cada
 * trigger para sempre. Isso quebra a garantia de terminação descrita em
 * docs/architecture/30-events.md §1 e faz eventos únicos ("a coroação")
 * virarem ocorrências mensais.
 *
 * O escopo é por nação, exceto OnceGlobal: Albion ter recebido um evento
 * OncePerNation não impede que Galia receba o mesmo evento.
 */
UENUM(BlueprintType)
enum class EEventRepeatPolicy : uint8
{
	/** Sem limite. Só o MTTH e as Conditions controlam a frequência. */
	Always			UMETA(DisplayName = "Always"),

	/** No máximo uma vez para cada nação. */
	OncePerNation	UMETA(DisplayName = "Once Per Nation"),

	/** No máximo uma vez na partida inteira, para qualquer nação. */
	OnceGlobal		UMETA(DisplayName = "Once Global"),

	/** Pode repetir, mas só depois de CooldownDays desde o último disparo. */
	Cooldown		UMETA(DisplayName = "Cooldown")
};
