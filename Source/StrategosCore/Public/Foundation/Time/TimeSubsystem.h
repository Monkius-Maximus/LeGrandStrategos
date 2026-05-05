#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "Foundation/Time/TimeSpeed.h"
#include "TimeSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDayTick,   FDateTime, CurrentDate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMonthTick, FDateTime, CurrentDate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnYearTick,  FDateTime, CurrentDate);

/**
 * UTimeSubsystem — Metrônomo da simulação.
 *
 * Avança um calendário interno (FDateTime) em incrementos de dias.
 * A velocidade é controlada por ETimeSpeed; o subsistema converte segundos
 * reais em dias simulados conforme a velocidade ativa.
 *
 * Subsistemas que precisam reagir ao tempo se inscrevem em OnDayTick,
 * OnMonthTick ou OnYearTick conforme a granularidade desejada — economia
 * roda mensalmente, eventos diariamente, vitória anualmente, etc.
 *
 * Ver docs/architecture/02-time.md.
 */
UCLASS()
class STRATEGOSCORE_API UTimeSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;
	virtual bool IsTickableWhenPaused() const override { return false; }
	virtual bool IsTickableInEditor() const override { return false; }

	UFUNCTION(BlueprintCallable, Category = "Strategos|Time")
	void Pause() { SetSpeed(ETimeSpeed::Paused); }

	UFUNCTION(BlueprintCallable, Category = "Strategos|Time")
	void Resume();

	UFUNCTION(BlueprintCallable, Category = "Strategos|Time")
	void SetSpeed(ETimeSpeed NewSpeed);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Strategos|Time")
	ETimeSpeed GetSpeed() const { return CurrentSpeed; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Strategos|Time")
	bool IsPaused() const { return CurrentSpeed == ETimeSpeed::Paused; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Strategos|Time")
	FDateTime GetCurrentDate() const { return CurrentDate; }

	UFUNCTION(BlueprintCallable, Category = "Strategos|Time")
	void SetStartDate(int32 Year, int32 Month, int32 Day);

	UPROPERTY(BlueprintAssignable, Category = "Strategos|Time")
	FOnDayTick OnDayTick;

	UPROPERTY(BlueprintAssignable, Category = "Strategos|Time")
	FOnMonthTick OnMonthTick;

	UPROPERTY(BlueprintAssignable, Category = "Strategos|Time")
	FOnYearTick OnYearTick;

private:
	float DaysPerSecondForSpeed(ETimeSpeed Speed) const;
	void AdvanceOneDay();

	UPROPERTY()
	FDateTime CurrentDate;

	UPROPERTY()
	ETimeSpeed CurrentSpeed = ETimeSpeed::Paused;

	UPROPERTY()
	ETimeSpeed LastNonPausedSpeed = ETimeSpeed::Slow;

	double DayAccumulator = 0.0;
};
