#include "Modals/StrategosCountryCardWidget.h"
#include "StrategosUI.h"
#include "Diplomacy/DiplomacySubsystem.h"
#include "World/WorldState.h"
#include "World/Nation.h"
#include "World/Province.h"
#include "World/Leader.h"
#include "Economy/PopGroup.h"
#include "Game/StrategosGameState.h"
#include "Engine/World.h"

// ── Helpers ───────────────────────────────────────────────────────────────────

const UWorldState* UStrategosCountryCardWidget::ResolveWorldState() const
{
	const UWorld* W = GetWorld();
	if (!W) return nullptr;
	const AStrategosGameState* GS = W->GetGameState<AStrategosGameState>();
	return GS ? GS->GetWorldState() : nullptr;
}

const UNation* UStrategosCountryCardWidget::ResolveNation() const
{
	const UWorldState* WS = ResolveWorldState();
	return WS ? WS->GetNation(CurrentNationId) : nullptr;
}

const UNation* UStrategosCountryCardWidget::ResolvePlayerNation() const
{
	const UWorldState* WS = ResolveWorldState();
	return WS ? WS->GetNation(WS->PlayerNationId) : nullptr;
}

// ── Open ──────────────────────────────────────────────────────────────────────

void UStrategosCountryCardWidget::OpenCountry(FName NationId)
{
	CurrentNationId = NationId;
	OnCountryLoaded(NationId);
}

// ── Identity ─────────────────────────────────────────────────────────────────

FText UStrategosCountryCardWidget::GetNationName() const
{
	const UNation* N = ResolveNation();
	return N ? N->DisplayName : FText::GetEmpty();
}

FText UStrategosCountryCardWidget::GetNationFullName() const
{
	// Placeholder — FullName property to be added to UNation in future
	return GetNationName();
}

FLinearColor UStrategosCountryCardWidget::GetNationColor() const
{
	const UNation* N = ResolveNation();
	return N ? FLinearColor(N->Color) : FLinearColor::White;
}

UTexture2D* UStrategosCountryCardWidget::GetFlagTexture() const
{
	const UNation* N = ResolveNation();
	return N ? N->FlagTexture.LoadSynchronous() : nullptr;
}

UTexture2D* UStrategosCountryCardWidget::GetCoatOfArmsIcon() const
{
	const UNation* N = ResolveNation();
	return N ? N->CoatOfArmsIcon.LoadSynchronous() : nullptr;
}

// ── Government ───────────────────────────────────────────────────────────────

FText UStrategosCountryCardWidget::GetGovernmentType() const
{
	// Government type tracked per NationalIdeas — placeholder string
	const UNation* N = ResolveNation();
	if (!N) return FText::GetEmpty();

	// Derive from archetype affinities as heuristic
	ELeaderArchetype Dominant = ELeaderArchetype::Pragmatist;
	float MaxAff = 0.f;
	for (const auto& KV : N->ArchetypeAffinity)
	{
		if (KV.Value > MaxAff) { MaxAff = KV.Value; Dominant = KV.Key; }
	}

	switch (Dominant)
	{
		case ELeaderArchetype::Martial:    return NSLOCTEXT("Gov","Militar","Monarquia Militar");
		case ELeaderArchetype::Diplomatic: return NSLOCTEXT("Gov","Diplo","Monarquia Constitucional");
		case ELeaderArchetype::Merchant:   return NSLOCTEXT("Gov","Merc","República Censitária");
		default:                           return NSLOCTEXT("Gov","Abs","Monarquia Absoluta");
	}
}

FText UStrategosCountryCardWidget::GetRulerName() const
{
	const UNation* N = ResolveNation();
	return (N && N->CurrentLeader) ? N->CurrentLeader->DisplayName : FText::GetEmpty();
}

ELeaderArchetype UStrategosCountryCardWidget::GetRulerArchetype() const
{
	const UNation* N = ResolveNation();
	return (N && N->CurrentLeader) ? N->CurrentLeader->Archetype : ELeaderArchetype::Pragmatist;
}

// ── Stats ────────────────────────────────────────────────────────────────────

TArray<FCountryStatRow> UStrategosCountryCardWidget::GetStatRows() const
{
	TArray<FCountryStatRow> Out;
	const UNation* N = ResolveNation();
	if (!N) return Out;

	auto Add = [&](FText Label, FText Val, FLinearColor Col = FLinearColor(0.93f,0.89f,0.82f))
	{
		FCountryStatRow R; R.Label=Label; R.Value=Val; R.ValueColor=Col; Out.Add(R);
	};

	int32 TotalPop = 0;
	// Population — sum all province pops
	const UWorldState* WS = ResolveWorldState();
	if (WS)
	{
		for (const FName& PId : N->OwnedProvinceIds)
		{
			if (const UProvince* P = WS->GetProvince(PId))
			{
				for (const auto& KV : P->Pops) TotalPop += KV.Value.Population;
			}
		}
	}

	Add(NSLOCTEXT("CountryCard","Pop","População"),
	    FText::AsNumber(TotalPop));
	Add(NSLOCTEXT("CountryCard","Provinces","Províncias"),
	    FText::AsNumber(N->OwnedProvinceIds.Num()));
	Add(NSLOCTEXT("CountryCard","Treasury","Tesouro"),
	    FText::Format(NSLOCTEXT("CountryCard","TreasuryFmt","£{0}"), FText::AsNumber((int32)N->Treasury.Balance)));
	Add(NSLOCTEXT("CountryCard","MilIdx","Cap. Militar"),
	    FText::AsPercent(N->StrategicIndices.MilitaryReadinessIndex));
	Add(NSLOCTEXT("CountryCard","IndIdx","Cap. Industrial"),
	    FText::AsPercent(N->StrategicIndices.IndustrialCapacityIndex));
	Add(NSLOCTEXT("CountryCard","MoraleIdx","Moral"),
	    FText::AsPercent(N->StrategicIndices.CivilianMoraleIndex));

	return Out;
}

// ── Indices ──────────────────────────────────────────────────────────────────

float UStrategosCountryCardWidget::GetMilitaryIndex() const
{
	const UNation* N = ResolveNation();
	return N ? N->StrategicIndices.MilitaryReadinessIndex : 0.f;
}

float UStrategosCountryCardWidget::GetMoraleIndex() const
{
	const UNation* N = ResolveNation();
	return N ? N->StrategicIndices.CivilianMoraleIndex : 0.f;
}

float UStrategosCountryCardWidget::GetIndustryIndex() const
{
	const UNation* N = ResolveNation();
	return N ? N->StrategicIndices.IndustrialCapacityIndex : 0.f;
}

// ── Relation ─────────────────────────────────────────────────────────────────

EDiplomaticStatus UStrategosCountryCardWidget::GetRelationStatus() const
{
	const UNation* Player = ResolvePlayerNation();
	if (!Player) return EDiplomaticStatus::Peace;

	const UWorld* W = GetWorld();
	if (!W) return EDiplomaticStatus::Peace;
	const UDiplomacySubsystem* Diplo = W->GetSubsystem<UDiplomacySubsystem>();
	if (!Diplo) return EDiplomaticStatus::Peace;

	return Diplo->GetStatus(Player->Id, CurrentNationId);
}

float UStrategosCountryCardWidget::GetOpinion() const
{
	const UNation* Player = ResolvePlayerNation();
	if (!Player) return 0.f;

	const UWorld* W = GetWorld();
	if (!W) return 0.f;
	const UDiplomacySubsystem* Diplo = W->GetSubsystem<UDiplomacySubsystem>();
	return Diplo ? Diplo->GetOpinion(Player->Id, CurrentNationId) : 0.f;
}

FText UStrategosCountryCardWidget::GetStatusLabel() const
{
	switch (GetRelationStatus())
	{
		case EDiplomaticStatus::Alliance:   return NSLOCTEXT("DipStatus","Alliance","Aliado");
		case EDiplomaticStatus::War:        return NSLOCTEXT("DipStatus","War","Em Guerra");
		case EDiplomaticStatus::Truce:      return NSLOCTEXT("DipStatus","Truce","Trégua");
		default:                            return NSLOCTEXT("DipStatus","Peace","Paz");
	}
}

FLinearColor UStrategosCountryCardWidget::GetStatusColor() const
{
	switch (GetRelationStatus())
	{
		case EDiplomaticStatus::Alliance: return FLinearColor(0.49f, 0.73f, 0.42f);  // verde
		case EDiplomaticStatus::War:      return FLinearColor(0.70f, 0.31f, 0.28f);  // vermelho
		case EDiplomaticStatus::Truce:    return FLinearColor(0.79f, 0.65f, 0.35f);  // dourado
		default:                          return FLinearColor(0.65f, 0.62f, 0.55f);  // neutro
	}
}

// ── Quick actions ─────────────────────────────────────────────────────────────

void UStrategosCountryCardWidget::OpenDiplomacyAction(FName ActionId)
{
	OnOpenDiploAction(ActionId, CurrentNationId);
}
