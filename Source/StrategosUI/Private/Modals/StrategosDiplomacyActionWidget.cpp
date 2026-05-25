#include "Modals/StrategosDiplomacyActionWidget.h"
#include "StrategosUI.h"
#include "Diplomacy/DiplomacySubsystem.h"
#include "Economy/EconomySubsystem.h"
#include "World/WorldState.h"
#include "World/Nation.h"
#include "Game/StrategosGameState.h"
#include "Engine/World.h"

// ── Action catalogue ──────────────────────────────────────────────────────────

TMap<FName, UStrategosDiplomacyActionWidget::FActionDef>
UStrategosDiplomacyActionWidget::BuildActionDefs()
{
	TMap<FName, FActionDef> M;
	auto Add = [&](FName Id, FText Title, FText Summary, FName Cat,
	               int32 PC, float Gold, int32 CD, float Inf, bool HR, bool CB)
	{
		FActionDef D;
		D.Title=Title; D.Summary=Summary; D.Category=Cat;
		D.PolitCapCost=PC; D.TreasuryCost=Gold; D.CooldownDays=CD;
		D.InfamyCost=Inf; D.bHighRisk=HR; D.bCBFab=CB;
		M.Add(Id, D);
	};

	Add("propose_alliance",
		NSLOCTEXT("Diplo","Alliance","Propor Aliança"),
		NSLOCTEXT("Diplo","AllianceDesc","Propõe um pacto de defesa mútua."),
		"alliance", 25, 0.f, 365, 0.f, false, false);

	Add("non_aggression_pact",
		NSLOCTEXT("Diplo","NAP","Propor Não-Agressão"),
		NSLOCTEXT("Diplo","NAPDesc","Compromisso de não declarar guerra por 10 anos."),
		"alliance", 10, 0.f, 180, 0.f, false, false);

	Add("embargo",
		NSLOCTEXT("Diplo","Embargo","Declarar Embargo Comercial"),
		NSLOCTEXT("Diplo","EmbargoDesc","Bloqueia o comércio bilateral, reduzindo receitas de ambos."),
		"economic", 15, 0.f, 365, 5.f, false, false);

	Add("offer_gift",
		NSLOCTEXT("Diplo","Gift","Oferecer Presente Diplomático"),
		NSLOCTEXT("Diplo","GiftDesc","Envia £500 ao tesouro do alvo; melhora opinião."),
		"economic", 5, 500.f, 90, 0.f, false, false);

	Add("request_passage",
		NSLOCTEXT("Diplo","Passage","Solicitar Passagem Militar"),
		NSLOCTEXT("Diplo","PassageDesc","Pede direito de mover exércitos pelo território."),
		"alliance", 10, 0.f, 180, 0.f, false, false);

	Add("fabricate_cb",
		NSLOCTEXT("Diplo","FabCB","Fabricar Casus Belli"),
		NSLOCTEXT("Diplo","FabCBDesc","Agentes secretos fabricam justificativa legal para guerra."),
		"hostile", 20, 0.f, 720, 10.f, false, true);

	Add("declare_war",
		NSLOCTEXT("Diplo","DeclWar","Declarar Guerra (sem CB)"),
		NSLOCTEXT("Diplo","DeclWarDesc","Declara guerra sem justificativa formal. Alta infâmia; aliados podem não honrar."),
		"warning", 0, 0.f, 0, 25.f, true, false);

	return M;
}

// ── Helpers ───────────────────────────────────────────────────────────────────

const UWorldState* UStrategosDiplomacyActionWidget::ResolveWorldState() const
{
	const UWorld* W = GetWorld();
	if (!W) return nullptr;
	const AStrategosGameState* GS = W->GetGameState<AStrategosGameState>();
	return GS ? GS->GetWorldState() : nullptr;
}

// ── Open ──────────────────────────────────────────────────────────────────────

void UStrategosDiplomacyActionWidget::OpenAction(FName ActionId, FName TargetNationId)
{
	CurrentActionId = ActionId;
	CurrentTargetId = TargetNationId;
	OnActionLoaded(ActionId, TargetNationId);
}

// ── Header ────────────────────────────────────────────────────────────────────

static const TMap<FName, UStrategosDiplomacyActionWidget::FActionDef>& GetDefs()
{
	static TMap<FName, UStrategosDiplomacyActionWidget::FActionDef> Defs =
		UStrategosDiplomacyActionWidget::BuildActionDefs();
	return Defs;
}

FText UStrategosDiplomacyActionWidget::GetActionTitle() const
{
	if (const FActionDef* D = GetDefs().Find(CurrentActionId)) return D->Title;
	return FText::GetEmpty();
}

FText UStrategosDiplomacyActionWidget::GetActionSummary() const
{
	if (const FActionDef* D = GetDefs().Find(CurrentActionId)) return D->Summary;
	return FText::GetEmpty();
}

FName UStrategosDiplomacyActionWidget::GetActionCategory() const
{
	if (const FActionDef* D = GetDefs().Find(CurrentActionId)) return D->Category;
	return NAME_None;
}

FLinearColor UStrategosDiplomacyActionWidget::GetCategoryColor() const
{
	const FName Cat = GetActionCategory();
	if (Cat == "alliance")  return FLinearColor(0.49f, 0.64f, 0.78f);  // cobalt
	if (Cat == "hostile")   return FLinearColor(0.70f, 0.31f, 0.28f);  // oxblood
	if (Cat == "economic")  return FLinearColor(0.49f, 0.73f, 0.42f);  // verdigris
	if (Cat == "warning")   return FLinearColor(0.85f, 0.35f, 0.20f);  // red-orange
	return FLinearColor(0.79f, 0.65f, 0.35f);
}

// ── Target ───────────────────────────────────────────────────────────────────

FText UStrategosDiplomacyActionWidget::GetTargetNationName() const
{
	const UWorldState* WS = ResolveWorldState();
	if (!WS) return FText::GetEmpty();
	const UNation* N = WS->GetNation(CurrentTargetId);
	return N ? N->DisplayName : FText::GetEmpty();
}

FLinearColor UStrategosDiplomacyActionWidget::GetTargetNationColor() const
{
	const UWorldState* WS = ResolveWorldState();
	if (!WS) return FLinearColor::White;
	const UNation* N = WS->GetNation(CurrentTargetId);
	return N ? FLinearColor(N->Color) : FLinearColor::White;
}

// ── Cost ─────────────────────────────────────────────────────────────────────

FDiploActionCost UStrategosDiplomacyActionWidget::GetActionCost() const
{
	FDiploActionCost Out;
	if (const FActionDef* D = GetDefs().Find(CurrentActionId))
	{
		Out.PoliticalCapital = D->PolitCapCost;
		Out.Treasury         = D->TreasuryCost;
		Out.CooldownDays     = D->CooldownDays;
		Out.Infamy           = D->InfamyCost;
	}
	return Out;
}

bool UStrategosDiplomacyActionWidget::CanAffordAction() const
{
	// Simplified — real check queries EconomySubsystem + PoliticsSubsystem
	return true;
}

// ── Acceptance breakdown ──────────────────────────────────────────────────────

TArray<FDiploModifierRow> UStrategosDiplomacyActionWidget::ComputeModifiers() const
{
	TArray<FDiploModifierRow> Out;
	const UWorldState* WS = ResolveWorldState();
	if (!WS) return Out;

	const UWorld* W = GetWorld();
	if (!W) return Out;
	const UDiplomacySubsystem* Diplo = W->GetSubsystem<UDiplomacySubsystem>();
	if (!Diplo) return Out;

	const FName PlayerNationId = WS->PlayerNationId;
	const FDiplomaticRelation Rel = Diplo->GetRelation(PlayerNationId, CurrentTargetId);

	auto Add = [&](FText Label, float Val)
	{
		FDiploModifierRow R;
		R.Label    = Label;
		R.Value    = Val;
		R.bPositive = Val >= 0.f;
		R.RowColor  = Val >= 0.f
			? FLinearColor(0.49f, 0.73f, 0.42f)   // verdigris
			: FLinearColor(0.70f, 0.31f, 0.28f);   // oxblood
		Out.Add(R);
	};

	// Opinion-based modifier
	Add(NSLOCTEXT("Diplo","OpinionMod","Opinião atual"), Rel.Opinion * 0.3f);

	// Common enemies boost
	if (Diplo->AreAtWar(PlayerNationId, CurrentTargetId))
	{
		Add(NSLOCTEXT("Diplo","AtWarMod","Em guerra"), -50.f);
	}

	// Alliance category base bonus
	if (GetActionCategory() == "alliance")
	{
		Add(NSLOCTEXT("Diplo","AllianceBase","Proposta amistosa"), +10.f);
	}
	if (GetActionCategory() == "hostile" || GetActionCategory() == "warning")
	{
		Add(NSLOCTEXT("Diplo","HostileBase","Ação hostil"), -30.f);
	}
	return Out;
}

TArray<FDiploModifierRow> UStrategosDiplomacyActionWidget::GetAcceptanceModifiers() const
{
	return ComputeModifiers();
}

float UStrategosDiplomacyActionWidget::GetAcceptanceProbability() const
{
	float Total = 0.f;
	for (const FDiploModifierRow& R : ComputeModifiers())
	{
		Total += R.Value;
	}
	return FMath::Clamp((Total + 50.f) / 100.f, 0.f, 1.f);
}

FText UStrategosDiplomacyActionWidget::GetAcceptanceVerdict() const
{
	const float P = GetAcceptanceProbability();
	if (P > 0.75f) return NSLOCTEXT("Diplo","VerdictLikely","Provável aceitação");
	if (P > 0.40f) return NSLOCTEXT("Diplo","VerdictUncertain","Resultado incerto");
	return NSLOCTEXT("Diplo","VerdictUnlikely","Provável recusa");
}

bool UStrategosDiplomacyActionWidget::IsHighRiskAction() const
{
	const FActionDef* D = GetDefs().Find(CurrentActionId);
	return D && D->bHighRisk;
}

bool UStrategosDiplomacyActionWidget::IsCBFabrication() const
{
	const FActionDef* D = GetDefs().Find(CurrentActionId);
	return D && D->bCBFab;
}

void UStrategosDiplomacyActionWidget::ExecuteAction()
{
	UE_LOG(LogStrategosUI, Log, TEXT("DiploAction: ExecuteAction '%s' on '%s'"),
		*CurrentActionId.ToString(), *CurrentTargetId.ToString());
	OnActionExecuted(true);
}
