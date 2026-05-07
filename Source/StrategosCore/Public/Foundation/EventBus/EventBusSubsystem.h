#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Foundation/EventBus/StrategosEvent.h"
#include "EventBusSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FStrategosEventDelegate, const FStrategosEvent& /*Event*/);

DECLARE_DYNAMIC_DELEGATE_OneParam(FStrategosEventDynamicDelegate, const FStrategosEvent&, Event);

/**
 * UEventBusSubsystem — Pub/sub global indexado por tag.
 *
 * Quem publica um evento NÃO conhece quem escuta. Quem escuta declara
 * interesse por uma EventTag e recebe broadcast quando alguém publica
 * algo com aquela tag.
 *
 * Determinismo: a ordem de iteração interna de FMulticastDelegate é
 * estável, mas entre runs depende da ordem de Subscribe. Subsistemas
 * que precisam de determinismo absoluto devem se inscrever em ordem
 * fixa durante Initialize.
 *
 * Ver docs/architecture/03-event-bus.md.
 */
UCLASS()
class STRATEGOSCORE_API UEventBusSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	FDelegateHandle Subscribe(FName EventTag, FStrategosEventDelegate::FDelegate Delegate);
	void Unsubscribe(FName EventTag, FDelegateHandle Handle);

	void Publish(const FStrategosEvent& Event);

	UFUNCTION(BlueprintCallable, Category = "Strategos|EventBus", DisplayName = "Subscribe (Blueprint)")
	void K2_Subscribe(FName EventTag, FStrategosEventDynamicDelegate Callback);

	UFUNCTION(BlueprintCallable, Category = "Strategos|EventBus", DisplayName = "Unsubscribe (Blueprint)")
	void K2_Unsubscribe(FName EventTag, FStrategosEventDynamicDelegate Callback);

	UFUNCTION(BlueprintCallable, Category = "Strategos|EventBus", DisplayName = "Publish")
	void K2_Publish(const FStrategosEvent& Event) { Publish(Event); }

	int32 GetSubscriberCount(FName EventTag) const;

private:
	TMap<FName, FStrategosEventDelegate> ChannelsByTag;
};
