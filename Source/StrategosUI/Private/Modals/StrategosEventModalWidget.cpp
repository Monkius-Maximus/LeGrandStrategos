#include "Modals/StrategosEventModalWidget.h"
#include "StrategosUI.h"
#include "Events/EventSubsystem.h"
#include "Events/EventAsset.h"
#include "Events/EventChoice.h"
#include "Events/EventCondition.h"
#include "Events/EventEffect.h"
#include "World/WorldState.h"
#include "World/Nation.h"
#include "Game/StrategosGameState.h"
#include "Engine/World.h"

UEventSubsystem* UStrategosEventModalWidget::ResolveEvents() const
{
	const UWorld* W = GetWorld();
	return W ? W->GetSubsystem<UEventSubsystem>() : nullptr;
}

UWorldState* UStrategosEventModalWidget::ResolveWorldState() const
{
	const UWorld* W = GetWorld();
	if (!W) return nullptr;
	AStrategosGameState* GS = W->GetGameState<AStrategosGameState>();
	return GS ? GS->GetWorldState() : nullptr;
}

void UStrategosEventModalWidget::LoadEventInternal(FName EventId)
{
	CurrentEventId = EventId;
	CurrentAsset   = nullptr;
	if (UEventSubsystem* Events = ResolveEvents())
	{
		CurrentAsset = Events->GetEventById(EventId);
	}
	OnEventLoaded(EventId);
}

void UStrategosEventModalWidget::OpenEvent(FName EventId)
{
	CurrentContext            = FEventContext{};
	CurrentContext.EventId    = EventId;
	if (const UWorldState* WS = ResolveWorldState())
	{
		CurrentContext.SourceNationId = WS->PlayerNationId;
	}
	LoadEventInternal(EventId);
}

void UStrategosEventModalWidget::OpenEventWithContext(FName EventId, const FEventContext& Context)
{
	CurrentContext         = Context;
	CurrentContext.EventId = EventId;
	LoadEventInternal(EventId);
}

void UStrategosEventModalWidget::ResolveChoice(int32 ChoiceIndex)
{
	UEventSubsystem* Events = ResolveEvents();
	if (!Events || CurrentEventId.IsNone()) return;

	const UWorldState* WS = ResolveWorldState();
	if (!WS) return;

	Events->ResolveDecision(WS->PlayerNationId, CurrentEventId, ChoiceIndex);
	OnChoiceResolved(ChoiceIndex);
}

FText UStrategosEventModalWidget::GetEventTitle() const
{
	return CurrentAsset ? CurrentAsset->Title : FText::GetEmpty();
}

FText UStrategosEventModalWidget::GetEventDescription() const
{
	return CurrentAsset ? CurrentAsset->Description : FText::GetEmpty();
}

FName UStrategosEventModalWidget::GetEventCategory() const
{
	if (!CurrentAsset) return NAME_None;

	// Category explícita no asset tem prioridade.
	if (!CurrentAsset->Category.IsNone())
	{
		return CurrentAsset->Category;
	}

	// Fallback por Type para assets sem Category preenchida.
	switch (CurrentAsset->Type)
	{
		case EEventType::Decision:     return FName("political");
		case EEventType::Notification: return FName("notification");
		default:                       return FName("notification");
	}
}

FLinearColor UStrategosEventModalWidget::GetCategoryColor() const
{
	const FName Cat = GetEventCategory();
	if (Cat == "economic")   return FLinearColor(0.49f, 0.73f, 0.42f);   // verdigris
	if (Cat == "political")  return FLinearColor(0.49f, 0.64f, 0.78f);   // cobalt
	if (Cat == "military")   return FLinearColor(0.70f, 0.31f, 0.28f);   // oxblood
	if (Cat == "diplomatic") return FLinearColor(0.79f, 0.65f, 0.35f);   // brass
	return FLinearColor(0.65f, 0.62f, 0.55f);
}

TArray<FEventChoiceRow> UStrategosEventModalWidget::GetChoiceRows() const
{
	TArray<FEventChoiceRow> Out;
	if (!CurrentAsset) return Out;

	UWorldState* WS = ResolveWorldState();

	for (int32 i = 0; i < CurrentAsset->Choices.Num(); ++i)
	{
		const FEventChoice& C = CurrentAsset->Choices[i];
		FEventChoiceRow R;
		R.ChoiceIndex = i;
		R.Label       = C.Label;
		R.bAvailable  = true;

		// Avaliar condições de disponibilidade da choice.
		for (const TObjectPtr<UEventCondition>& CondPtr : C.AvailabilityConditions)
		{
			UEventCondition* Cond = CondPtr.Get();
			if (Cond && !Cond->Evaluate(WS, CurrentContext))
			{
				R.bAvailable = false;
				break;
			}
		}

		// Tooltip: usa UnavailableTooltip quando bloqueada, Tooltip padrão quando disponível.
		R.Tooltip = (!R.bAvailable && !C.UnavailableTooltip.IsEmpty())
			? C.UnavailableTooltip
			: C.Tooltip;

		// EffectsPreview: GetDescription() de cada efeito, separados por ", ".
		TArray<FString> Parts;
		for (const TObjectPtr<UEventEffect>& EffPtr : C.Effects)
		{
			if (UEventEffect* Eff = EffPtr.Get())
			{
				const FText Desc = Eff->GetDescription();
				if (!Desc.IsEmpty())
				{
					Parts.Add(Desc.ToString());
				}
			}
		}
		R.EffectsPreview = Parts.Num() > 0
			? FText::FromString(FString::Join(Parts, TEXT(", ")))
			: FText::GetEmpty();

		Out.Add(R);
	}
	return Out;
}
