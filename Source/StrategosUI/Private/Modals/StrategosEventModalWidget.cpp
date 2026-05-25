#include "Modals/StrategosEventModalWidget.h"
#include "StrategosUI.h"
#include "Events/EventSubsystem.h"
#include "Events/EventAsset.h"
#include "Events/EventChoice.h"
#include "World/WorldState.h"
#include "World/Nation.h"
#include "Game/StrategosGameState.h"
#include "Engine/World.h"

UEventSubsystem* UStrategosEventModalWidget::ResolveEvents() const
{
	const UWorld* W = GetWorld();
	return W ? W->GetSubsystem<UEventSubsystem>() : nullptr;
}

void UStrategosEventModalWidget::OpenEvent(FName EventId)
{
	CurrentEventId = EventId;
	CurrentAsset   = nullptr;

	if (UEventSubsystem* Events = ResolveEvents())
	{
		CurrentAsset = Events->GetEventById(EventId);
	}
	OnEventLoaded(EventId);
}

void UStrategosEventModalWidget::ResolveChoice(int32 ChoiceIndex)
{
	UEventSubsystem* Events = ResolveEvents();
	if (!Events || CurrentEventId.IsNone()) return;

	const UWorld* W = GetWorld();
	if (!W) return;
	const AStrategosGameState* GS = W->GetGameState<AStrategosGameState>();
	if (!GS || !GS->GetWorldState()) return;

	const FName PlayerNationId = GS->GetWorldState()->PlayerNationId;
	Events->ResolveDecision(PlayerNationId, CurrentEventId, ChoiceIndex);
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
	// EEventType only has Decision/Notification/Silent in current codebase.
	// Category tag derived from event type; richer categorisation via tags in future.
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

	for (int32 i = 0; i < CurrentAsset->Choices.Num(); ++i)
	{
		const FEventChoice& C = CurrentAsset->Choices[i];
		FEventChoiceRow R;
		R.ChoiceIndex    = i;
		R.Label          = C.Label;
		R.Tooltip        = C.Tooltip;
		// EffectsPreview built from Effects array count; rich description deferred to future
		R.EffectsPreview = FText::Format(
			NSLOCTEXT("EventModal","EffectCount","{0} efeito(s)"), FText::AsNumber(C.Effects.Num()));
		R.bAvailable     = true; // condition evaluation deferred to EventSubsystem
		Out.Add(R);
	}
	return Out;
}
