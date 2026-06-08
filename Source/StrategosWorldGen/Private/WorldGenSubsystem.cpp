#include "WorldGenSubsystem.h"
#include "WorldGenDataAsset.h"
#include "Pipeline/WorldGenPipeline.h"
#include "StrategosWorldGen.h"

bool UWorldGenSubsystem::GenerateWorld(const FWorldGenParams& Params)
{
	bHasResult = StrategosWorldGen::Pipeline::Generate(Params, LastResult);
	return bHasResult;
}

bool UWorldGenSubsystem::GenerateFromAsset(const UWorldGenDataAsset* Asset)
{
	if (!Asset)
	{
		UE_LOG(LogStrategosWorldGen, Warning, TEXT("GenerateFromAsset: asset nulo."));
		return false;
	}
	return GenerateWorld(Asset->Params);
}

UTexture2D* UWorldGenSubsystem::RenderDebugTexture(EWorldGenRenderMode Mode)
{
	if (!bHasResult)
	{
		UE_LOG(LogStrategosWorldGen, Warning, TEXT("RenderDebugTexture: nenhum resultado gerado ainda."));
		return nullptr;
	}
	return StrategosWorldGen::DebugRender::RenderToTexture(LastResult, Mode);
}
