#include "BattleVisualizer.h"
#include "BattleSubsystem.h"
#include "StrategosBattle.h"
#include "Engine/World.h"

ABattleVisualizer::ABattleVisualizer()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ABattleVisualizer::BeginPlay()
{
	Super::BeginPlay();
}

void ABattleVisualizer::Destroyed()
{
	UnbindFromSubsystem();
	Super::Destroyed();
}

void ABattleVisualizer::BindToSubsystem(UBattleSubsystem* Subsystem)
{
	if (!Subsystem) return;

	UnbindFromSubsystem();
	BoundSubsystem = Subsystem;

	Subsystem->OnBattleStarted.AddDynamic(this, &ABattleVisualizer::OnBattleStarted);
	Subsystem->OnPhaseChanged .AddDynamic(this, &ABattleVisualizer::OnPhaseChanged);
	Subsystem->OnRoundEnded   .AddDynamic(this, &ABattleVisualizer::OnRoundEnded);
	Subsystem->OnSideRouted   .AddDynamic(this, &ABattleVisualizer::OnSideRouted);
	Subsystem->OnCardPlayed   .AddDynamic(this, &ABattleVisualizer::OnCardPlayed);
	Subsystem->OnBattleFinished.AddDynamic(this, &ABattleVisualizer::OnBattleFinished);

	UE_LOG(LogStrategosBattle, Log, TEXT("BattleVisualizer: conectado ao subsistema."));
}

void ABattleVisualizer::UnbindFromSubsystem()
{
	if (UBattleSubsystem* Sub = BoundSubsystem.Get())
	{
		Sub->OnBattleStarted .RemoveDynamic(this, &ABattleVisualizer::OnBattleStarted);
		Sub->OnPhaseChanged  .RemoveDynamic(this, &ABattleVisualizer::OnPhaseChanged);
		Sub->OnRoundEnded    .RemoveDynamic(this, &ABattleVisualizer::OnRoundEnded);
		Sub->OnSideRouted    .RemoveDynamic(this, &ABattleVisualizer::OnSideRouted);
		Sub->OnCardPlayed    .RemoveDynamic(this, &ABattleVisualizer::OnCardPlayed);
		Sub->OnBattleFinished.RemoveDynamic(this, &ABattleVisualizer::OnBattleFinished);
	}
	BoundSubsystem.Reset();
}

void ABattleVisualizer::OnBattleStarted()
{
	UE_LOG(LogStrategosBattle, Log, TEXT("BattleVisualizer: batalha iniciada — preparando visuais."));
	RefreshVisuals();
}

void ABattleVisualizer::OnPhaseChanged(EBattlePhase OldPhase, EBattlePhase NewPhase)
{
	UE_LOG(LogStrategosBattle, Log,
		TEXT("BattleVisualizer: fase %d → %d"),
		static_cast<int32>(OldPhase), static_cast<int32>(NewPhase));
	RefreshVisuals();
}

void ABattleVisualizer::OnRoundEnded(int32 Round)
{
	UE_LOG(LogStrategosBattle, Verbose,
		TEXT("BattleVisualizer: round %d encerrado."), Round);
	RefreshVisuals();
}

void ABattleVisualizer::OnSideRouted(int32 SideIndex)
{
	UE_LOG(LogStrategosBattle, Log,
		TEXT("BattleVisualizer: lado %d em fuga."), SideIndex);
	RefreshVisuals();
}

void ABattleVisualizer::OnCardPlayed(int32 SideIndex, FName CardId)
{
	UE_LOG(LogStrategosBattle, Verbose,
		TEXT("BattleVisualizer: carta '%s' jogada pelo lado %d."),
		*CardId.ToString(), SideIndex);
}

void ABattleVisualizer::OnBattleFinished(FBattleResult Result)
{
	UE_LOG(LogStrategosBattle, Log,
		TEXT("BattleVisualizer: batalha encerrada — outcome=%d."),
		static_cast<int32>(Result.Outcome));
	RefreshVisuals();
}

void ABattleVisualizer::RefreshVisuals()
{
	BP_RefreshVisuals();
}
