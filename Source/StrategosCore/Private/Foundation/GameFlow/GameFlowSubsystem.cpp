#include "Foundation/GameFlow/GameFlowSubsystem.h"
#include "StrategosCore.h"

void UGameFlowSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	BuildTransitionTable();
	CurrentState = EGameFlowState::MainMenu;
	UE_LOG(LogStrategosCore, Log, TEXT("GameFlowSubsystem initialized in state %s"),
		*UEnum::GetValueAsString(CurrentState));
}

void UGameFlowSubsystem::Deinitialize()
{
	AllowedTransitions.Empty();
	Super::Deinitialize();
}

void UGameFlowSubsystem::BuildTransitionTable()
{
	AllowedTransitions.Empty();

	AllowedTransitions.Add(EGameFlowState::MainMenu,	{ EGameFlowState::Loading });
	AllowedTransitions.Add(EGameFlowState::Loading,		{ EGameFlowState::Running, EGameFlowState::MainMenu });
	AllowedTransitions.Add(EGameFlowState::Running,		{ EGameFlowState::Paused, EGameFlowState::Battle, EGameFlowState::Event, EGameFlowState::GameOver, EGameFlowState::MainMenu });
	AllowedTransitions.Add(EGameFlowState::Paused,		{ EGameFlowState::Running, EGameFlowState::MainMenu });
	AllowedTransitions.Add(EGameFlowState::Battle,		{ EGameFlowState::Running, EGameFlowState::GameOver });
	AllowedTransitions.Add(EGameFlowState::Event,		{ EGameFlowState::Running });
	AllowedTransitions.Add(EGameFlowState::GameOver,	{ EGameFlowState::MainMenu });
}

bool UGameFlowSubsystem::IsTransitionAllowed(EGameFlowState From, EGameFlowState To) const
{
	if (From == To)
	{
		return false;
	}

	const TSet<EGameFlowState>* Targets = AllowedTransitions.Find(From);
	return Targets && Targets->Contains(To);
}

bool UGameFlowSubsystem::TransitionTo(EGameFlowState NewState)
{
	if (!IsTransitionAllowed(CurrentState, NewState))
	{
		UE_LOG(LogStrategosCore, Warning, TEXT("GameFlow: transition %s -> %s rejected"),
			*UEnum::GetValueAsString(CurrentState),
			*UEnum::GetValueAsString(NewState));
		return false;
	}

	const EGameFlowState OldState = CurrentState;
	CurrentState = NewState;

	UE_LOG(LogStrategosCore, Log, TEXT("GameFlow: %s -> %s"),
		*UEnum::GetValueAsString(OldState),
		*UEnum::GetValueAsString(NewState));

	OnStateChanged.Broadcast(OldState, NewState);
	return true;
}
