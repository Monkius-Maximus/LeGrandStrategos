#pragma once

#include "CoreMinimal.h"

class FWorldGenContext;

/**
 * Estagio 3 — Heightmap por blobs (mecanica central do FMG).
 *
 * Um blob escolhe uma celula, soma Power e propaga por BFS aos vizinhos com
 * decaimento (Radius) e jitter (Sharpness). Templates combinam varios blobs
 * numa receita (Continente, Arquipelago, Pangeia).
 */
namespace StrategosWorldGen::Heightmap
{
	/** Propaga um blob aditivo a partir de StartCell. Power<0 cria depressao. */
	STRATEGOSWORLDGEN_API void AddBlob(FWorldGenContext& Ctx, int32 StartCell, float Power, float Radius, float Sharpness);

	/** Multiplica a altura de todas as celulas (ex.: 0.9 rebaixa bordas). */
	STRATEGOSWORLDGEN_API void MultiplyHeights(FWorldGenContext& Ctx, float Factor);

	/** Cordilheira/depressao alongada: cadeia de blobs ao longo de uma direcao. */
	STRATEGOSWORLDGEN_API void AddRange(FWorldGenContext& Ctx, int32 StartCell, float Power, float Radius, float Sharpness, int32 Length);

	/** Camada de Perlin de baixa amplitude para quebrar a redondeza dos blobs. */
	STRATEGOSWORLDGEN_API void PerturbWithPerlin(FWorldGenContext& Ctx, float Amplitude);

	/** Forca celulas de borda do mapa para baixo do nivel do mar. */
	STRATEGOSWORLDGEN_API void ForceBorderWater(FWorldGenContext& Ctx);

	/** Aplica a receita escolhida em Ctx.Params.Template. */
	STRATEGOSWORLDGEN_API void ApplyTemplate(FWorldGenContext& Ctx);
}
