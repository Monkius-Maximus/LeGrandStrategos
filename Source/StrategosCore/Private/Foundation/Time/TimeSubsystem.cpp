#include "Foundation/Time/TimeSubsystem.h"
#include "StrategosCore.h"
#include "Stats/Stats.h"

void UTimeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CurrentDate = FDateTime(1836, 1, 1);
	CurrentSpeed = ETimeSpeed::Paused;
	LastNonPausedSpeed = ETimeSpeed::Slow;
	DayAccumulator = 0.0;

	UE_LOG(LogStrategosCore, Log, TEXT("TimeSubsystem initialized at %s"),
		*CurrentDate.ToString());
}

void UTimeSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UTimeSubsystem::IsTickable() const
{
	return CurrentSpeed != ETimeSpeed::Paused;
}

TStatId UTimeSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UTimeSubsystem, STATGROUP_Tickables);
}

float UTimeSubsystem::DaysPerSecondForSpeed(ETimeSpeed Speed) const
{
	switch (Speed)
	{
		case ETimeSpeed::Slow:    return 1.0f;
		case ETimeSpeed::Normal:  return 2.0f;
		case ETimeSpeed::Fast:    return 4.0f;
		case ETimeSpeed::Fastest: return 8.0f;
		case ETimeSpeed::Paused:
		default:                  return 0.0f;
	}
}

void UTimeSubsystem::Tick(float DeltaTime)
{
	const float DaysPerSecond = DaysPerSecondForSpeed(CurrentSpeed);
	if (DaysPerSecond <= 0.0f)
	{
		return;
	}

	DayAccumulator += static_cast<double>(DeltaTime) * DaysPerSecond;

	while (DayAccumulator >= 1.0)
	{
		AdvanceOneDay();
		DayAccumulator -= 1.0;
	}
}

void UTimeSubsystem::AdvanceOneDay()
{
	const FDateTime PreviousDate = CurrentDate;
	CurrentDate += FTimespan::FromDays(1);

	OnDayTick.Broadcast(CurrentDate);

	if (CurrentDate.GetMonth() != PreviousDate.GetMonth())
	{
		OnMonthTick.Broadcast(CurrentDate);
	}

	if (CurrentDate.GetYear() != PreviousDate.GetYear())
	{
		OnYearTick.Broadcast(CurrentDate);
	}
}

void UTimeSubsystem::SetSpeed(ETimeSpeed NewSpeed)
{
	if (NewSpeed == CurrentSpeed)
	{
		return;
	}

	if (CurrentSpeed != ETimeSpeed::Paused)
	{
		LastNonPausedSpeed = CurrentSpeed;
	}

	CurrentSpeed = NewSpeed;
	UE_LOG(LogStrategosCore, Log, TEXT("TimeSubsystem speed -> %s"),
		*UEnum::GetValueAsString(CurrentSpeed));
}

void UTimeSubsystem::Resume()
{
	if (CurrentSpeed == ETimeSpeed::Paused)
	{
		SetSpeed(LastNonPausedSpeed);
	}
}

void UTimeSubsystem::SetStartDate(int32 Year, int32 Month, int32 Day)
{
	CurrentDate = FDateTime(Year, Month, Day);
	DayAccumulator = 0.0;
}
