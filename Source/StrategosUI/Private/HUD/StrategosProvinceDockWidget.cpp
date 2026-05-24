#include "HUD/StrategosProvinceDockWidget.h"
#include "StrategosUI.h"
#include "Map/MapSubsystem.h"
#include "Economy/EconomySubsystem.h"
#include "Economy/Building.h"
#include "Economy/BuildingTypeAsset.h"
#include "Economy/PopGroup.h"
#include "World/WorldState.h"
#include "World/Province.h"
#include "World/Nation.h"
#include "World/TerrainType.h"
#include "Game/StrategosGameState.h"
#include "Engine/World.h"

// ── Lifecycle ─────────────────────────────────────────────────────────────────

void UStrategosProvinceDockWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (UMapSubsystem* Map = ResolveMap())
	{
		Map->OnProvinceSelected.AddDynamic(this, &UStrategosProvinceDockWidget::HandleProvinceSelected);
	}
}

void UStrategosProvinceDockWidget::NativeDestruct()
{
	if (UMapSubsystem* Map = ResolveMap())
	{
		Map->OnProvinceSelected.RemoveDynamic(this, &UStrategosProvinceDockWidget::HandleProvinceSelected);
	}
	Super::NativeDestruct();
}

void UStrategosProvinceDockWidget::HandleProvinceSelected(FName ProvinceId)
{
	OnProvinceChanged(ProvinceId);
}

// ── Tab control ───────────────────────────────────────────────────────────────

void UStrategosProvinceDockWidget::SetActiveTab(EProvinceDockTab NewTab)
{
	ActiveTab = NewTab;
}

// ── Helper resolvers ──────────────────────────────────────────────────────────

UMapSubsystem* UStrategosProvinceDockWidget::ResolveMap() const
{
	const UWorld* W = GetWorld();
	return W ? W->GetSubsystem<UMapSubsystem>() : nullptr;
}

const UProvince* UStrategosProvinceDockWidget::ResolveProvince() const
{
	const UMapSubsystem* Map = ResolveMap();
	if (!Map) return nullptr;
	const FName Id = Map->GetSelectedProvinceId();
	if (Id.IsNone()) return nullptr;

	const UWorld* W = GetWorld();
	if (!W) return nullptr;
	const AStrategosGameState* GS = W->GetGameState<AStrategosGameState>();
	if (!GS || !GS->GetWorldState()) return nullptr;
	return GS->GetWorldState()->GetProvince(Id);
}

const UNation* UStrategosProvinceDockWidget::ResolveOwnerNation() const
{
	const UProvince* P = ResolveProvince();
	if (!P) return nullptr;
	const UWorld* W = GetWorld();
	if (!W) return nullptr;
	const AStrategosGameState* GS = W->GetGameState<AStrategosGameState>();
	if (!GS || !GS->GetWorldState()) return nullptr;
	return GS->GetWorldState()->GetNation(P->OwnerNationId);
}

// ── Summary ───────────────────────────────────────────────────────────────────

FProvinceSummary UStrategosProvinceDockWidget::GetProvinceSummary() const
{
	FProvinceSummary Out;
	const UProvince* P = ResolveProvince();
	if (!P) return Out;

	Out.ProvinceName  = P->DisplayName;
	Out.TerrainName   = StaticEnum<ETerrainType>()->GetDisplayNameTextByValue((int64)P->Terrain);
	Out.BuildingSlots = P->BuildingSlots;
	Out.UsedSlots     = P->GetUsedBuildingSlots();

	for (const auto& KV : P->Pops)
	{
		Out.TotalPop += KV.Value.Population;
	}

	if (const UNation* N = ResolveOwnerNation())
	{
		Out.OwnerName  = N->DisplayName;
		Out.OwnerColor = FLinearColor(N->Color);
	}
	return Out;
}

// ── Overview tab ─────────────────────────────────────────────────────────────

float UStrategosProvinceDockWidget::GetProvinceMonthlyIncome() const
{
	const UProvince* P = ResolveProvince();
	if (!P) return 0.f;
	float Total = 0.f;
	for (const TObjectPtr<UBuilding>& B : P->Buildings)
	{
		if (B) Total += B->LastTickProfit;
	}
	return Total;
}

int32 UStrategosProvinceDockWidget::GetProvinceTotalPop() const
{
	const UProvince* P = ResolveProvince();
	if (!P) return 0;
	int32 Total = 0;
	for (const auto& KV : P->Pops) Total += KV.Value.Population;
	return Total;
}

float UStrategosProvinceDockWidget::GetProvinceMilitancy() const
{
	const UProvince* P = ResolveProvince();
	if (!P) return 0.f;
	float Total = 0.f; int32 Count = 0;
	// Militancy = 1 - Loyalty (Loyalty is [0..1], 1 = fully loyal / no militancy)
	for (const auto& KV : P->Pops) { Total += (1.f - KV.Value.Loyalty); ++Count; }
	return Count > 0 ? Total / Count : 0.f;
}

float UStrategosProvinceDockWidget::GetProvinceStability() const
{
	const UProvince* P = ResolveProvince();
	if (!P) return 1.f;
	float Total = 0.f; int32 Count = 0;
	for (const auto& KV : P->Pops) { Total += KV.Value.Loyalty; ++Count; }
	return Count > 0 ? Total / Count : 1.f;
}

// ── Buildings tab ─────────────────────────────────────────────────────────────

TArray<FProvinceBuildingRow> UStrategosProvinceDockWidget::GetBuildingRows() const
{
	TArray<FProvinceBuildingRow> Out;
	const UProvince* P = ResolveProvince();
	if (!P) return Out;

	// Palette for building categories (matches HTML mockup category colors)
	static const TMap<FName, FLinearColor> CategoryColors = {
		{ TEXT("textile"),  FLinearColor(0.4f, 0.5f, 0.8f) },
		{ TEXT("mining"),   FLinearColor(0.6f, 0.4f, 0.2f) },
		{ TEXT("steel"),    FLinearColor(0.5f, 0.5f, 0.5f) },
		{ TEXT("lumber"),   FLinearColor(0.3f, 0.55f, 0.3f) },
		{ TEXT("consumer"), FLinearColor(0.7f, 0.55f, 0.3f) },
	};

	for (const TObjectPtr<UBuilding>& BPtr : P->Buildings)
	{
		const UBuilding* B = BPtr.Get();
		if (!B) continue;

		FProvinceBuildingRow R;
		R.BuildingId    = B->Id;
		R.Level         = B->Level;
		R.bConstructing = B->IsUnderConstruction();
		R.DaysRemaining = B->ConstructionDaysRemaining;
		R.LastProfit    = B->LastTickProfit;
		R.bIsPrivate    = (B->OwnerKind == EBuildingOwnerKind::Private);

		if (UBuildingTypeAsset* BT = B->BuildingType.LoadSynchronous())
		{
			R.TypeName = BT->DisplayName;
			if (const FLinearColor* Col = CategoryColors.Find(BT->Category))
			{
				R.CategoryColor = *Col;
			}
		}
		Out.Add(R);
	}
	return Out;
}

int32 UStrategosProvinceDockWidget::GetFreeBuildingSlots() const
{
	const UProvince* P = ResolveProvince();
	return P ? P->GetFreeBuildingSlots() : 0;
}

FText UStrategosProvinceDockWidget::GetNextBuildingSuggestion() const
{
	const UProvince* P = ResolveProvince();
	if (!P) return FText::GetEmpty();

	// Simple heuristic: suggest based on terrain raw resource potential
	FName TopGood; float TopPot = 0.f;
	for (const auto& KV : P->RawResourcePotential)
	{
		if (KV.Value > TopPot) { TopPot = KV.Value; TopGood = KV.Key; }
	}
	if (!TopGood.IsNone() && TopPot > 0.f)
	{
		return FText::Format(
			NSLOCTEXT("ProvinceDock", "BuildSuggestion", "Sugestão: explorar {0} (pot. {1}%)"),
			FText::FromName(TopGood),
			FText::AsNumber(FMath::RoundToInt(TopPot * 100)));
	}
	return NSLOCTEXT("ProvinceDock", "BuildSuggestionDefault", "Construir manufatura de bens de consumo");
}

void UStrategosProvinceDockWidget::RequestBuildConstruction(FName BuildingTypeId)
{
	// Forwarded to Economy subsystem in a future iteration.
	// Placeholder: log intent.
	UE_LOG(LogStrategosUI, Log, TEXT("ProvinceDock: RequestBuild '%s'"), *BuildingTypeId.ToString());
}

// ── Population tab ────────────────────────────────────────────────────────────

TArray<FProvincePopRow> UStrategosProvinceDockWidget::GetPopRows() const
{
	TArray<FProvincePopRow> Out;
	const UProvince* P = ResolveProvince();
	if (!P) return Out;

	static const TMap<EPopStratum, FLinearColor> StratumColors = {
		{ EPopStratum::Lower,  FLinearColor(0.4f, 0.55f, 0.4f) },
		{ EPopStratum::Middle, FLinearColor(0.55f, 0.55f, 0.35f) },
		{ EPopStratum::Upper,  FLinearColor(0.7f, 0.55f, 0.25f) },
	};

	for (const auto& KV : P->Pops)
	{
		FProvincePopRow R;
		R.StratumName  = StaticEnum<EPopStratum>()->GetDisplayNameTextByValue((int64)KV.Key);
		R.PopCount     = KV.Value.Population;
		R.Wage         = KV.Value.WageEarnedLastMonth;
		R.Satisfaction = KV.Value.Loyalty;
		if (const FLinearColor* Col = StratumColors.Find(KV.Key))
		{
			R.StratumColor = *Col;
		}
		Out.Add(R);
	}
	return Out;
}

// ── Information tab ───────────────────────────────────────────────────────────

FText UStrategosProvinceDockWidget::GetTerrainDescription() const
{
	const UProvince* P = ResolveProvince();
	if (!P) return FText::GetEmpty();

	static const TMap<ETerrainType, FText> Descriptions = {
		{ ETerrainType::Plains,    NSLOCTEXT("Terrain", "Plains",    "Planícies férteis propícias à agricultura e ao movimento de exércitos.") },
		{ ETerrainType::Hills,     NSLOCTEXT("Terrain", "Hills",     "Colinas moderadas com potencial minerador e posições defensivas naturais.") },
		{ ETerrainType::Mountains, NSLOCTEXT("Terrain", "Mountains", "Terreno montanhoso rico em minério; passagens dificultam o avanço inimigo.") },
		{ ETerrainType::Forest,    NSLOCTEXT("Terrain", "Forest",    "Floresta densa — madeira abundante, mas mobilidade reduzida.") },
		{ ETerrainType::Desert,    NSLOCTEXT("Terrain", "Desert",    "Deserto árido. Suprimento precário; baixa densidade populacional.") },
		{ ETerrainType::Coastal,   NSLOCTEXT("Terrain", "Coastal",   "Costa navegável: acesso a rotas comerciais marítimas e pesca.") },
	};

	if (const FText* Desc = Descriptions.Find(P->Terrain)) return *Desc;
	return FText::GetEmpty();
}

TArray<FName> UStrategosProvinceDockWidget::GetRawResourceIds() const
{
	TArray<FName> Out;
	if (const UProvince* P = ResolveProvince())
	{
		P->RawResourcePotential.GetKeys(Out);
	}
	return Out;
}

float UStrategosProvinceDockWidget::GetRawResourcePotential(FName GoodId) const
{
	const UProvince* P = ResolveProvince();
	if (!P) return 0.f;
	const float* Val = P->RawResourcePotential.Find(GoodId);
	return Val ? *Val : 0.f;
}

// ── Actions ───────────────────────────────────────────────────────────────────

void UStrategosProvinceDockWidget::RequestRecruitArmy()
{
	UE_LOG(LogStrategosUI, Log, TEXT("ProvinceDock: RequestRecruitArmy in '%s'"),
		*ResolveMap()->GetSelectedProvinceId().ToString());
}
