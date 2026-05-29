#include "BattleHUDWidget.h"
#include "BattleSubsystem.h"
#include "StrategosBattle.h"

void UBattleHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UBattleHUDWidget::NativeDestruct()
{
	if (UBattleSubsystem* Sub = BoundSubsystem.Get())
	{
		Sub->OnPhaseChanged  .RemoveDynamic(this, &UBattleHUDWidget::HandlePhaseChanged);
		Sub->OnRoundEnded    .RemoveDynamic(this, &UBattleHUDWidget::HandleRoundEnded);
		Sub->OnCardPlayed    .RemoveDynamic(this, &UBattleHUDWidget::HandleCardPlayed);
		Sub->OnSideRouted    .RemoveDynamic(this, &UBattleHUDWidget::HandleSideRouted);
		Sub->OnBattleFinished.RemoveDynamic(this, &UBattleHUDWidget::HandleBattleFinished);
	}
	BoundSubsystem.Reset();
	Super::NativeDestruct();
}

void UBattleHUDWidget::InitializeHUD(UBattleSubsystem* Subsystem)
{
	if (!Subsystem)
	{
		UE_LOG(LogStrategosBattle, Warning,
			TEXT("BattleHUDWidget::InitializeHUD — subsistema nulo."));
		return;
	}

	BoundSubsystem = Subsystem;

	Subsystem->OnPhaseChanged  .AddDynamic(this, &UBattleHUDWidget::HandlePhaseChanged);
	Subsystem->OnRoundEnded    .AddDynamic(this, &UBattleHUDWidget::HandleRoundEnded);
	Subsystem->OnCardPlayed    .AddDynamic(this, &UBattleHUDWidget::HandleCardPlayed);
	Subsystem->OnSideRouted    .AddDynamic(this, &UBattleHUDWidget::HandleSideRouted);
	Subsystem->OnBattleFinished.AddDynamic(this, &UBattleHUDWidget::HandleBattleFinished);

	UE_LOG(LogStrategosBattle, Log, TEXT("BattleHUDWidget: HUD inicializado."));
}

void UBattleHUDWidget::HandlePhaseChanged(EBattlePhase OldPhase, EBattlePhase NewPhase)
{
	OnPhaseChangedUI(OldPhase, NewPhase);
}

void UBattleHUDWidget::HandleRoundEnded(int32 Round)
{
	if (const UBattleSubsystem* Sub = BoundSubsystem.Get())
	{
		OnRoundEndedUI(Round, Sub->GetContext());
	}
}

void UBattleHUDWidget::HandleCardPlayed(int32 SideIndex, FName CardId)
{
	OnCardPlayedUI(SideIndex, CardId);
}

void UBattleHUDWidget::HandleSideRouted(int32 SideIndex)
{
	OnSideRoutedUI(SideIndex);
}

void UBattleHUDWidget::HandleBattleFinished(FBattleResult Result)
{
	OnBattleFinishedUI(Result);
}
