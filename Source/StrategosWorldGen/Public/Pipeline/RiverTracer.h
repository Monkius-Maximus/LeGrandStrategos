#pragma once

#include "CoreMinimal.h"

class FWorldGenContext;

/**
 * Estagio 9 — Rios (versao rascunho).
 *
 * Para cada celula de terra acha o vizinho mais baixo (downhill graph),
 * acumula o fluxo de precipitacao ao longo dos caminhos e marca como rio
 * as celulas acima de um threshold. SEM lagos, pit-filling ou larguras
 * variaveis (isso e hidrologia de verdade, fora do escopo de estudo).
 */
namespace StrategosWorldGen::River
{
	STRATEGOSWORLDGEN_API void Trace(FWorldGenContext& Ctx);
}
