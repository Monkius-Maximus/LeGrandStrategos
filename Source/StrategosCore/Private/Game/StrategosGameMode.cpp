#include "Game/StrategosGameMode.h"
#include "StrategosCore.h"
#include "Game/StrategosGameState.h"
#include "World/WorldState.h"
#include "Bootstrap/WorldBootstrapper.h"
#include "Bootstrap/WorldBootstrapAsset.h"
#include "Foundation/Time/TimeSubsystem.h"

AStrategosGameMode::AStrategosGameMode()
{
	GameStateClass = AStrategosGameState::StaticClass();
	bUseSeamlessTravel = true;
}

void AStrategosGameMode::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogStrategosCore, Log, TEXT("StrategosGameMode BeginPlay."));
	RunBootstrap();
}

void AStrategosGameMode::RunBootstrap()
{
	AStrategosGameState* GS = GetGameState<AStrategosGameState>();
	if (!GS)
	{
		UE_LOG(LogStrategosCore, Error, TEXT("RunBootstrap: GameState not ready."));
		return;
	}

	UWorldState* WorldState = GS->GetOrCreateWorldState();

	UWorldBootstrapAsset* Asset = nullptr;
	if (!bForceDefaultSandbox && !BootstrapAsset.IsNull())
	{
		Asset = BootstrapAsset.LoadSynchronous();
	}

	bool bApplied = false;
	int32 StartYear = 1836;
	int32 StartMonth = 1;
	int32 StartDay = 1;

	if (Asset)
	{
		bApplied = UWorldBootstrapper::ApplyBootstrap(WorldState, Asset);
		StartYear = Asset->StartYear;
		StartMonth = Asset->StartMonth;
		StartDay = Asset->StartDay;
	}

	if (!bApplied)
	{
		UE_LOG(LogStrategosCore, Log, TEXT("RunBootstrap: falling back to default sandbox."));
		UWorldBootstrapper::ApplyDefaultSandbox(WorldState);
	}

	if (UTimeSubsystem* Time = GetGameInstance()->GetSubsystem<UTimeSubsystem>())
	{
		Time->SetStartDate(StartYear, StartMonth, StartDay);
	}
}
