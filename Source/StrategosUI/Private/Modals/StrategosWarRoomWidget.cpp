#include "Modals/StrategosWarRoomWidget.h"
#include "StrategosUI.h"
#include "Diplomacy/DiplomacySubsystem.h"
#include "World/WorldState.h"
#include "World/Nation.h"
#include "Game/StrategosGameState.h"
#include "Engine/World.h"

// ── WarRoom ──────────────────────────────────────────────────────────────────

void UStrategosWarRoomWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UStrategosWarRoomWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UStrategosWarRoomWidget::OpenWarRoom(FName InEnemyNationId)
{
	EnemyNationId = InEnemyNationId;
}

void UStrategosWarRoomWidget::SetActiveTab(EWarRoomTab NewTab)
{
	ActiveTab = NewTab;
}

FText UStrategosWarRoomWidget::GetWarName() const
{
	const UWorld* W = GetWorld();
	if (!W) return FText::GetEmpty();
	const AStrategosGameState* GS = W->GetGameState<AStrategosGameState>();
	if (!GS || !GS->GetWorldState()) return FText::GetEmpty();

	const UNation* Enemy = GS->GetWorldState()->GetNation(EnemyNationId);
	const UNation* Player = GS->GetWorldState()->GetNation(GS->GetWorldState()->PlayerNationId);
	if (!Enemy || !Player) return FText::GetEmpty();

	return FText::Format(
		NSLOCTEXT("WarRoom", "WarName", "Guerra entre {0} e {1}"),
		Player->DisplayName, Enemy->DisplayName);
}

FText UStrategosWarRoomWidget::GetCasusBelliName() const
{
	// Placeholder — CB type tracked by DiplomacySubsystem future extension
	return NSLOCTEXT("WarRoom", "DefaultCB", "Casus Belli: Reivindicação Territorial");
}

int32 UStrategosWarRoomWidget::GetWarScore() const
{
	// Placeholder — war score tracked by WarSubsystem (future)
	return 0;
}

FWarSideSummary UStrategosWarRoomWidget::GetPlayerSide() const
{
	FWarSideSummary Out;
	const UWorld* W = GetWorld();
	if (!W) return Out;
	const AStrategosGameState* GS = W->GetGameState<AStrategosGameState>();
	if (!GS || !GS->GetWorldState()) return Out;

	if (const UNation* N = GS->GetWorldState()->GetNation(GS->GetWorldState()->PlayerNationId))
	{
		Out.NationName  = N->DisplayName;
		Out.NationColor = FLinearColor(N->Color);
	}
	return Out;
}

FWarSideSummary UStrategosWarRoomWidget::GetEnemySide() const
{
	FWarSideSummary Out;
	const UWorld* W = GetWorld();
	if (!W) return Out;
	const AStrategosGameState* GS = W->GetGameState<AStrategosGameState>();
	if (!GS || !GS->GetWorldState()) return Out;

	if (const UNation* N = GS->GetWorldState()->GetNation(EnemyNationId))
	{
		Out.NationName  = N->DisplayName;
		Out.NationColor = FLinearColor(N->Color);
	}
	return Out;
}

TArray<FWarGoalRow> UStrategosWarRoomWidget::GetWarGoals() const
{
	return {}; // Populated by WarSubsystem in future
}

TArray<FWarBattleRecord> UStrategosWarRoomWidget::GetBattleHistory() const
{
	return {}; // Populated by BattleSubsystem in future
}

void UStrategosWarRoomWidget::RequestProposePeace()
{
	OnOpenPeaceNegotiation();
}

void UStrategosWarRoomWidget::RequestSurrender()
{
	UE_LOG(LogStrategosUI, Log, TEXT("WarRoom: RequestSurrender to '%s'"), *EnemyNationId.ToString());
}

// ── PeaceNegotiation ─────────────────────────────────────────────────────────

void UStrategosPeaceNegotiationWidget::OpenPeaceTable(FName InEnemyNationId)
{
	EnemyNationId = InEnemyNationId;
	SelectedTermIds.Empty();
}

void UStrategosPeaceNegotiationWidget::ToggleTerm(FName TermId)
{
	if (SelectedTermIds.Contains(TermId))
		SelectedTermIds.Remove(TermId);
	else
		SelectedTermIds.Add(TermId);
}

TArray<FPeaceTermRow> UStrategosPeaceNegotiationWidget::GetTermRows() const
{
	// Static term catalogue — move to DataAsset in future
	static const TArray<FPeaceTermRow> AllTerms = []
	{
		TArray<FPeaceTermRow> T;
		auto Add = [&](FName Id, FText Label, FText Desc, int32 Cost)
		{
			FPeaceTermRow R; R.TermId=Id; R.Label=Label; R.Description=Desc; R.ScoreCost=Cost; T.Add(R);
		};
		Add("annex_capital",   NSLOCTEXT("Peace","AnnexCap","Anexar Capital"),            NSLOCTEXT("Peace","AnnexCapD","Incorpora a capital do inimigo."),               50);
		Add("reparations_sm",  NSLOCTEXT("Peace","RepSm","Reparações (pequenas)"),         NSLOCTEXT("Peace","RepSmD","£8.000 pagos em 12 parcelas."),                    15);
		Add("reparations_lg",  NSLOCTEXT("Peace","RepLg","Reparações (grandes)"),          NSLOCTEXT("Peace","RepLgD","£15.000 pagos em 24 parcelas."),                   30);
		Add("liberate_region", NSLOCTEXT("Peace","LibReg","Libertar Região"),              NSLOCTEXT("Peace","LibRegD","Cria estado-tampão neutro."),                     20);
		Add("war_guilt",       NSLOCTEXT("Peace","Guilt","Cláusula de Culpa de Guerra"),   NSLOCTEXT("Peace","GuiltD","Inimigo aceita responsabilidade; +10 prestígio."), 10);
		Add("demilitarize",    NSLOCTEXT("Peace","Demil","Desmilitarizar Fronteira"),       NSLOCTEXT("Peace","DemilD","Proíbe exércitos em 2 províncias por 10 anos."),   25);
		Add("white_peace",     NSLOCTEXT("Peace","White","Paz Branca"),                    NSLOCTEXT("Peace","WhiteD","Retorno ao status quo ante. Custo 0."),              0);
		return T;
	}();

	TArray<FPeaceTermRow> Out = AllTerms;
	for (FPeaceTermRow& R : Out)
	{
		R.bSelected = SelectedTermIds.Contains(R.TermId);
	}
	return Out;
}

int32 UStrategosPeaceNegotiationWidget::GetAvailableScore() const
{
	return 50; // placeholder; real value from WarSubsystem
}

int32 UStrategosPeaceNegotiationWidget::GetSelectedCost() const
{
	int32 Total = 0;
	for (const FPeaceTermRow& R : GetTermRows())
	{
		if (R.bSelected) Total += R.ScoreCost;
	}
	return Total;
}

int32 UStrategosPeaceNegotiationWidget::GetScoreBalance() const
{
	return GetAvailableScore() - GetSelectedCost();
}

float UStrategosPeaceNegotiationWidget::GetAIAcceptanceProbability() const
{
	const int32 Cost = GetSelectedCost();
	// Linear decay: full acceptance at 0, zero acceptance at 100
	return FMath::Clamp(1.f - Cost / 100.f, 0.f, 1.f);
}

void UStrategosPeaceNegotiationWidget::SendProposal()
{
	const bool bAccepted = FMath::FRand() < GetAIAcceptanceProbability();
	OnProposalSent(bAccepted);
}
