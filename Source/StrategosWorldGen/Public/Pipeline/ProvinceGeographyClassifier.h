#pragma once

#include "CoreMinimal.h"
#include "Economy/ProvinceGeography.h"

struct FWorldGenResult;

/**
 * Classificador worldgen -> geografia de província (Camada 1).
 *
 * Ponte entre a saída procedural (Height/Temperature/Precipitation/EBiomeType)
 * e os quatro eixos naturais de FProvinceGeography (relevo/clima/vegetação/
 * hidrografia). Regras Terra-like com limiares editáveis no .cpp — você continua
 * modando a calibragem sem mexer na arquitetura.
 *
 * Produz APENAS a camada natural; recurso principal (PrincipalResourceId) é a
 * Sessão 4. Não há ligação célula->província ainda: quando a geração popular
 * províncias, cada uma amostra/agrega as células da sua área e chama isto.
 */
namespace StrategosWorldGen::GeographyClassifier
{
	/** Deriva a geografia natural de uma célula do worldgen. */
	STRATEGOSWORLDGEN_API FProvinceGeography Classify(const FWorldGenResult& Result, int32 CellIndex);

	/** Aplica Classify a todas as células; Out fica com Result.Cells.Num() itens. */
	STRATEGOSWORLDGEN_API void ClassifyAll(const FWorldGenResult& Result, TArray<FProvinceGeography>& Out);
}
