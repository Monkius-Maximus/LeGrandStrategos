#pragma once

#include "CoreMinimal.h"
#include "WorldGenTypes.h"
#include "WorldGenParams.h"

/**
 * FWorldGenContext — Estado mutavel compartilhado por todos os estagios.
 *
 * Carrega o unico FRandomStream do pipeline (determinismo: NENHUM estagio
 * pode chamar FMath::RandRange — sempre Stream.*), os parametros de entrada
 * e o FWorldGenResult em construcao.
 *
 * Classe C++ pura (nao UObject): vive na stack durante a geracao.
 */
class STRATEGOSWORLDGEN_API FWorldGenContext
{
public:
	explicit FWorldGenContext(const FWorldGenParams& InParams)
		: Params(InParams)
		, Stream(InParams.Seed)
	{
		Result.Seed = InParams.Seed;
		Result.MapSize = InParams.MapSize;
		Result.SeaLevel = InParams.SeaLevel;
		Result.EquatorY = InParams.MapSize.Y * 0.5f;
	}

	FWorldGenParams Params;
	FRandomStream Stream;
	FWorldGenResult Result;
};
