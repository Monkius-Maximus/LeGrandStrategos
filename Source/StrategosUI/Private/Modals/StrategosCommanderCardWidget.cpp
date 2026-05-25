#include "Modals/StrategosCommanderCardWidget.h"
#include "StrategosUI.h"
#include "World/Leader.h"
#include "World/Nation.h"
#include "World/WorldState.h"
#include "Game/StrategosGameState.h"
#include "Engine/World.h"

void UStrategosCommanderCardWidget::OpenCommander(FName LeaderId)
{
	CurrentLeaderId = LeaderId;
	CurrentLeader   = nullptr;

	// Locate leader in nations' leadership
	const UNation* Nation = FindNationForLeader(LeaderId);
	if (Nation && Nation->CurrentLeader && Nation->CurrentLeader->Id == LeaderId)
	{
		CurrentLeader = Nation->CurrentLeader;
	}
	OnCommanderLoaded(LeaderId);
}

const UNation* UStrategosCommanderCardWidget::FindNationForLeader(FName LeaderId) const
{
	const UWorld* W = GetWorld();
	if (!W) return nullptr;
	const AStrategosGameState* GS = W->GetGameState<AStrategosGameState>();
	if (!GS || !GS->GetWorldState()) return nullptr;

	TArray<TObjectPtr<UNation>> AllNations;
	GS->GetWorldState()->Nations.GenerateValueArray(AllNations);
	for (const TObjectPtr<UNation>& NPtr : AllNations)
	{
		if (NPtr && NPtr->CurrentLeader && NPtr->CurrentLeader->Id == LeaderId)
		{
			return NPtr.Get();
		}
	}
	return nullptr;
}

FText UStrategosCommanderCardWidget::GetCommanderName() const
{
	return CurrentLeader ? CurrentLeader->DisplayName : FText::GetEmpty();
}

FText UStrategosCommanderCardWidget::GetCommanderArchetype() const
{
	if (!CurrentLeader) return FText::GetEmpty();
	return StaticEnum<ELeaderArchetype>()->GetDisplayNameTextByValue((int64)CurrentLeader->Archetype);
}

FLinearColor UStrategosCommanderCardWidget::GetNationColor() const
{
	const UNation* N = FindNationForLeader(CurrentLeaderId);
	return N ? FLinearColor(N->Color) : FLinearColor::White;
}

UTexture2D* UStrategosCommanderCardWidget::GetPortrait() const
{
	// Leaders use CoatOfArmsIcon as stand-in for portrait until dedicated art exists
	const UNation* N = FindNationForLeader(CurrentLeaderId);
	return N ? N->CoatOfArmsIcon.LoadSynchronous() : nullptr;
}

TArray<FCommanderStatRow> UStrategosCommanderCardWidget::GetStatRows() const
{
	TArray<FCommanderStatRow> Out;
	if (!CurrentLeader) return Out;

	// Stats derived from LeaderArchetype weights (design tokens: Tática/Carisma/Logística/Ousadia)
	const UNation* N = FindNationForLeader(CurrentLeaderId);
	if (!N) return Out;

	auto MakeRow = [](FText Label, int32 Val, FLinearColor Color) -> FCommanderStatRow
	{
		FCommanderStatRow R;
		R.Label      = Label;
		R.Value      = Val;
		R.Normalized = FMath::Clamp(Val / 100.f, 0.f, 1.f);
		R.BarColor   = Color;
		return R;
	};

	// Archetype → stat distribution (simplified; replace with data asset in future)
	int32 Tatica = 50, Carisma = 50, Logistica = 50, Ousadia = 50;
	if (N->ArchetypeAffinity.Contains(CurrentLeader->Archetype))
	{
		const float Weight = N->ArchetypeAffinity[CurrentLeader->Archetype];
		switch (CurrentLeader->Archetype)
		{
			case ELeaderArchetype::Martial:    Tatica += FMath::RoundToInt(Weight * 40); break;
			case ELeaderArchetype::Diplomatic: Carisma += FMath::RoundToInt(Weight * 40); break;
			case ELeaderArchetype::Merchant:   Logistica += FMath::RoundToInt(Weight * 40); break;
			case ELeaderArchetype::Scholar:    Logistica += FMath::RoundToInt(Weight * 30); break;
			default: Ousadia += 20; break;
		}
	}

	Out.Add(MakeRow(NSLOCTEXT("Commander", "Tatica",    "Tática"),    FMath::Clamp(Tatica,    0,100), FLinearColor(0.70f,0.31f,0.28f)));
	Out.Add(MakeRow(NSLOCTEXT("Commander", "Carisma",   "Carisma"),   FMath::Clamp(Carisma,   0,100), FLinearColor(0.49f,0.64f,0.78f)));
	Out.Add(MakeRow(NSLOCTEXT("Commander", "Logistica", "Logística"), FMath::Clamp(Logistica, 0,100), FLinearColor(0.49f,0.73f,0.42f)));
	Out.Add(MakeRow(NSLOCTEXT("Commander", "Ousadia",   "Ousadia"),   FMath::Clamp(Ousadia,   0,100), FLinearColor(0.79f,0.65f,0.35f)));
	return Out;
}

TArray<FCommanderTraitRow> UStrategosCommanderCardWidget::GetTraitRows() const
{
	// Placeholder — trait system linked to NationalIdeas in future iteration
	TArray<FCommanderTraitRow> Out;
	if (!CurrentLeader) return Out;

	FCommanderTraitRow R;
	R.Label       = NSLOCTEXT("Commander", "DefaultTrait", "Veterano");
	R.Description = NSLOCTEXT("Commander", "DefaultTraitDesc", "Décadas em campanha endureceram este comandante.");
	R.Kind        = FName("positive");
	R.PillColor   = FLinearColor(0.2f, 0.55f, 0.3f);
	Out.Add(R);
	return Out;
}

TArray<FCommanderBattleRecord> UStrategosCommanderCardWidget::GetBattleRecords() const
{
	// Placeholder — battle history tracked by BattleSubsystem (future)
	return {};
}

int32 UStrategosCommanderCardWidget::GetTotalVictories() const  { return 0; }
int32 UStrategosCommanderCardWidget::GetTotalDefeats() const    { return 0; }
int32 UStrategosCommanderCardWidget::GetTotalCampaigns() const  { return 0; }
