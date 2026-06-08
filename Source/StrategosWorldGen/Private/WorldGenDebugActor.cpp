#include "WorldGenDebugActor.h"
#include "WorldGenTypes.h"
#include "Pipeline/WorldGenPipeline.h"
#include "StrategosWorldGen.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

AWorldGenDebugActor::AWorldGenDebugActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AWorldGenDebugActor::BeginPlay()
{
	Super::BeginPlay();
	if (bGenerateOnBeginPlay)
	{
		GenerateAndExport();
	}
}

FString AWorldGenDebugActor::ModeName(EWorldGenRenderMode InMode)
{
	switch (InMode)
	{
	case EWorldGenRenderMode::RandomCells:   return TEXT("randomcells");
	case EWorldGenRenderMode::Height:        return TEXT("height");
	case EWorldGenRenderMode::Coast:         return TEXT("coast");
	case EWorldGenRenderMode::Temperature:   return TEXT("temperature");
	case EWorldGenRenderMode::Precipitation: return TEXT("precipitation");
	case EWorldGenRenderMode::Biomes:
	default:                                 return TEXT("biomes");
	}
}

void AWorldGenDebugActor::GenerateAndExport()
{
	// Roda o pipeline localmente (independe de GameInstance: funciona no botao
	// CallInEditor e tambem em PIE).
	FWorldGenResult Result;
	if (!StrategosWorldGen::Pipeline::Generate(Params, Result))
	{
		UE_LOG(LogStrategosWorldGen, Error, TEXT("WorldGenDebugActor: geracao falhou."));
		return;
	}

	const FString Dir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("WorldGen"));
	IFileManager::Get().MakeDirectory(*Dir, /*Tree=*/true);

	auto ExportMode = [&](EWorldGenRenderMode InMode)
	{
		const FString File = FString::Printf(TEXT("worldgen_seed%d_%s.png"), Result.Seed, *ModeName(InMode));
		const FString Path = FPaths::Combine(Dir, File);
		StrategosWorldGen::DebugRender::SaveToPng(Path, Result, InMode);
	};

	if (bExportAllModes)
	{
		ExportMode(EWorldGenRenderMode::RandomCells);
		ExportMode(EWorldGenRenderMode::Height);
		ExportMode(EWorldGenRenderMode::Coast);
		ExportMode(EWorldGenRenderMode::Temperature);
		ExportMode(EWorldGenRenderMode::Precipitation);
		ExportMode(EWorldGenRenderMode::Biomes);
	}
	else
	{
		ExportMode(Mode);
	}

	LastTexture = StrategosWorldGen::DebugRender::RenderToTexture(Result, Mode);

	UE_LOG(LogStrategosWorldGen, Log,
		TEXT("WorldGenDebugActor: PNG(s) em %s"), *FPaths::ConvertRelativePathToFull(Dir));
}
