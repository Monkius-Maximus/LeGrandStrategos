#include "Foundation/EventBus/EventBusSubsystem.h"
#include "StrategosCore.h"

void UEventBusSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogStrategosCore, Log, TEXT("EventBusSubsystem initialized."));
}

void UEventBusSubsystem::Deinitialize()
{
	ChannelsByTag.Empty();
	Super::Deinitialize();
}

FDelegateHandle UEventBusSubsystem::Subscribe(FName EventTag, FStrategosEventDelegate::FDelegate Delegate)
{
	FStrategosEventDelegate& Channel = ChannelsByTag.FindOrAdd(EventTag);
	return Channel.Add(Delegate);
}

void UEventBusSubsystem::Unsubscribe(FName EventTag, FDelegateHandle Handle)
{
	if (FStrategosEventDelegate* Channel = ChannelsByTag.Find(EventTag))
	{
		Channel->Remove(Handle);
	}
}

void UEventBusSubsystem::Publish(const FStrategosEvent& Event)
{
	if (Event.EventTag.IsNone())
	{
		UE_LOG(LogStrategosCore, Warning, TEXT("EventBus: publish ignored (EventTag is None)."));
		return;
	}

	if (FStrategosEventDelegate* Channel = ChannelsByTag.Find(Event.EventTag))
	{
		Channel->Broadcast(Event);
	}
}

int32 UEventBusSubsystem::GetSubscriberCount(FName EventTag) const
{
	const FStrategosEventDelegate* Channel = ChannelsByTag.Find(EventTag);
	return Channel ? Channel->GetAllocatedSize() : 0;
}

void UEventBusSubsystem::K2_Subscribe(FName EventTag, FStrategosEventDynamicDelegate Callback)
{
	if (!Callback.IsBound())
	{
		return;
	}

	FStrategosEventDelegate& Channel = ChannelsByTag.FindOrAdd(EventTag);
	Channel.AddLambda([Callback](const FStrategosEvent& Event)
	{
		if (Callback.IsBound())
		{
			Callback.Execute(Event);
		}
	});
}

void UEventBusSubsystem::K2_Unsubscribe(FName EventTag, FStrategosEventDynamicDelegate Callback)
{
	if (FStrategosEventDelegate* Channel = ChannelsByTag.Find(EventTag))
	{
		Channel->RemoveAll(Callback.GetUObject());
	}
}
