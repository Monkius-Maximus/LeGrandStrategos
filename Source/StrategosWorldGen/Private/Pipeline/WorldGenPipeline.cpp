#include "Pipeline/WorldGenPipeline.h"
#include "WorldGenContext.h"
#include "StrategosWorldGen.h"

#include "Pipeline/PoissonDiscSampler.h"
#include "Pipeline/VoronoiBuilder.h"
#include "Pipeline/HeightmapGenerator.h"
#include "Pipeline/ClimateGenerator.h"
#include "Pipeline/BiomeClassifier.h"
#include "Pipeline/RiverTracer.h"

namespace StrategosWorldGen::Pipeline
{
	bool Generate(const FWorldGenParams& Params, FWorldGenResult& OutResult)
	{
		const double StartTime = FPlatformTime::Seconds();
		FWorldGenContext Ctx(Params);

		// Estagio 1 — Poisson-disc.
		TArray<FVector2D> Points = PoissonDisc::Sample(Ctx.Result.MapSize, Params.PoissonMinDist, Ctx.Stream);
		UE_LOG(LogStrategosWorldGen, Log, TEXT("Worldgen: %d pontos amostrados."), Points.Num());

		// Estagio 2 — Lloyd + Voronoi.
		if (Params.LloydIterations > 0)
		{
			Points = Voronoi::LloydRelax(Points, Ctx.Result.MapSize, Params.LloydIterations);
		}
		if (!Voronoi::Build(Points, Ctx.Result))
		{
			UE_LOG(LogStrategosWorldGen, Error, TEXT("Worldgen abortado: Voronoi falhou."));
			OutResult = MoveTemp(Ctx.Result);
			return false;
		}

		// Estagio 3 — Heightmap por blobs + template.
		Heightmap::ApplyTemplate(Ctx);

		// Estagio 4 — Coastline.
		Climate::DetectWaterAndCoast(Ctx);

		// Estagios 5-7 — Temperatura, vento (implicito), precipitacao.
		Climate::ComputeTemperature(Ctx);
		Climate::ComputePrecipitation(Ctx);

		// Estagio 8 — Biomas.
		Biome::ClassifyAll(Ctx);

		// Estagio 9 — Rios (rascunho).
		if (Params.bGenerateRivers)
		{
			River::Trace(Ctx);
		}

		OutResult = MoveTemp(Ctx.Result);

		const double Elapsed = (FPlatformTime::Seconds() - StartTime) * 1000.0;
		UE_LOG(LogStrategosWorldGen, Log,
			TEXT("Worldgen concluido: %d celulas, seed %d, %.1f ms."),
			OutResult.Cells.Num(), OutResult.Seed, Elapsed);

		return true;
	}
}
