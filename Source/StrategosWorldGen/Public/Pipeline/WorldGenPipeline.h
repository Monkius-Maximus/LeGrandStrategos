#pragma once

#include "CoreMinimal.h"
#include "WorldGenParams.h"
#include "WorldGenTypes.h"

/**
 * Orquestrador do pipeline de worldgen.
 *
 * Encadeia os estagios na ordem da Secao 16 do documento de referencia,
 * todos compartilhando o mesmo FRandomStream (determinismo). Cada etapa
 * e independente e testavel isoladamente; aqui so se define a ordem.
 */
namespace StrategosWorldGen::Pipeline
{
	/**
	 * Roda o pipeline completo. Retorna false se algum estagio critico
	 * (Voronoi) falhar; nesse caso OutResult fica em estado parcial.
	 */
	STRATEGOSWORLDGEN_API bool Generate(const FWorldGenParams& Params, FWorldGenResult& OutResult);
}
