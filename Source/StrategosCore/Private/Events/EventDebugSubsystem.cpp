#include "Events/EventDebugSubsystem.h"
#include "Events/EventSubsystem.h"
#include "Events/EventAsset.h"
#include "Events/EventType.h"
#include "Events/EventChoice.h"
#include "Events/EventContext.h"
#include "Save/SaveSubsystem.h"
#include "World/WorldState.h"
#include "Foundation/Time/TimeSubsystem.h"
#include "Game/StrategosGameState.h"
#include "StrategosCore.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

// ---------------------------------------------------------------------------
// Lifecycle

bool UEventDebugSubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
#if !UE_BUILD_SHIPPING
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
#else
	return false;
#endif
}

void UEventDebugSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
#if !UE_BUILD_SHIPPING
	RegisterConsoleCommands();
	UE_LOG(LogStrategosCore, Log, TEXT("EventDebugSubsystem: comandos de console registrados."));
#endif
}

void UEventDebugSubsystem::Deinitialize()
{
#if !UE_BUILD_SHIPPING
	UnregisterConsoleCommands();
#endif
	Super::Deinitialize();
}

// ---------------------------------------------------------------------------
// Console commands

void UEventDebugSubsystem::RegisterConsoleCommands()
{
	IConsoleManager& CM = IConsoleManager::Get();

	ConsoleObjects.Add(CM.RegisterConsoleCommand(
		TEXT("Strategos.Event.Fire"),
		TEXT("Dispara um evento por Id. Args: <EventId> <NationId>"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &UEventDebugSubsystem::Cmd_FireEvent),
		ECVF_Cheat
	));

	ConsoleObjects.Add(CM.RegisterConsoleCommand(
		TEXT("Strategos.Event.ListPending"),
		TEXT("Lista todas as decisões pendentes para todas as nações."),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &UEventDebugSubsystem::Cmd_ListPending),
		ECVF_Cheat
	));

	ConsoleObjects.Add(CM.RegisterConsoleCommand(
		TEXT("Strategos.Event.CreateTestDecision"),
		TEXT("Cria e dispara uma Decision efêmera. Args: <CustomId> <NationId>"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &UEventDebugSubsystem::Cmd_CreateTestDecision),
		ECVF_Cheat
	));

	ConsoleObjects.Add(CM.RegisterConsoleCommand(
		TEXT("Strategos.Event.TestPersistence"),
		TEXT("Ciclo de teste save/load para decisões. Args: <CustomId> <NationId>"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &UEventDebugSubsystem::Cmd_TestPersistence),
		ECVF_Cheat
	));
}

void UEventDebugSubsystem::UnregisterConsoleCommands()
{
	IConsoleManager& CM = IConsoleManager::Get();
	for (IConsoleObject* Obj : ConsoleObjects)
	{
		if (Obj)
		{
			CM.UnregisterConsoleObject(Obj);
		}
	}
	ConsoleObjects.Empty();
}

// ---------------------------------------------------------------------------
// Helpers

UEventSubsystem* UEventDebugSubsystem::ResolveEventSubsystem() const
{
	const UWorld* W = GetWorld();
	return W ? W->GetSubsystem<UEventSubsystem>() : nullptr;
}

USaveSubsystem* UEventDebugSubsystem::ResolveSaveSubsystem() const
{
	const UWorld* W = GetWorld();
	if (!W) return nullptr;
	const UGameInstance* GI = W->GetGameInstance();
	return GI ? GI->GetSubsystem<USaveSubsystem>() : nullptr;
}

namespace
{
	UEventAsset* MakeEphemeralDecision(UObject* Outer, FName CustomId)
	{
		UEventAsset* E    = NewObject<UEventAsset>(Outer);
		E->Id             = CustomId;
		E->Title          = FText::FromString(FString::Printf(TEXT("[DEBUG] %s"), *CustomId.ToString()));
		E->Description    = NSLOCTEXT("StrategosDebug", "TestDesc",
			"Evento efêmero criado via debug subsystem para teste de persistência.");
		E->Type           = EEventType::Decision;
		E->Category       = TEXT("political");
		E->TriggerTag     = TEXT("Debug.Manual");
		E->MeanTimeToHappenMonths = 0;

		FEventChoice ChoiceA;
		ChoiceA.Label = NSLOCTEXT("StrategosDebug", "DebugOk", "OK (sem efeito)");

		FEventChoice ChoiceB;
		ChoiceB.Label = NSLOCTEXT("StrategosDebug", "DebugCancel", "Cancelar (sem efeito)");

		E->Choices.Add(ChoiceA);
		E->Choices.Add(ChoiceB);
		return E;
	}

	void ShowOnScreen(bool bGreen, const FString& Msg)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 8.f,
				bGreen ? FColor::Green : FColor::Red, Msg);
		}
	}
}

// ---------------------------------------------------------------------------
// UFUNCTION implementations

void UEventDebugSubsystem::FireEventDebug(FName EventId, FName NationId)
{
	UEventSubsystem* Events = ResolveEventSubsystem();
	if (!Events) return;

	const UWorld* W = GetWorld();
	FEventContext Ctx;
	Ctx.EventId        = EventId;
	Ctx.SourceNationId = NationId;
	Ctx.TriggerTag     = TEXT("Debug");
	if (W)
	{
		if (const UGameInstance* GI = W->GetGameInstance())
		{
			if (const UTimeSubsystem* Time = GI->GetSubsystem<UTimeSubsystem>())
			{
				Ctx.FireDate = Time->GetCurrentDate();
			}
		}
	}
	Events->FireEventById(EventId, Ctx);
	UE_LOG(LogStrategosCore, Log, TEXT("[DEBUG] FireEvent '%s' -> '%s'"),
		*EventId.ToString(), *NationId.ToString());
}

void UEventDebugSubsystem::CreateAndFireTestDecision(FName CustomEventId, FName NationId)
{
	UEventSubsystem* Events = ResolveEventSubsystem();
	if (!Events) return;

	UEventAsset* E = MakeEphemeralDecision(this, CustomEventId);
	Events->RegisterEphemeralEvent(E);

	const UWorld* W = GetWorld();
	FEventContext Ctx;
	Ctx.EventId        = CustomEventId;
	Ctx.SourceNationId = NationId;
	Ctx.TriggerTag     = TEXT("Debug.Manual");
	if (W)
	{
		if (const UGameInstance* GI = W->GetGameInstance())
		{
			if (const UTimeSubsystem* Time = GI->GetSubsystem<UTimeSubsystem>())
			{
				Ctx.FireDate = Time->GetCurrentDate();
			}
		}
	}
	Events->FireEventById(CustomEventId, Ctx);
	UE_LOG(LogStrategosCore, Log, TEXT("[DEBUG] CreateAndFireTestDecision '%s' -> '%s'"),
		*CustomEventId.ToString(), *NationId.ToString());
}

FString UEventDebugSubsystem::DumpPendingDecisions() const
{
	UEventSubsystem* Events = ResolveEventSubsystem();
	if (!Events)
	{
		return TEXT("EventSubsystem indisponível.");
	}

	const TMap<FName, TArray<FPendingDecision>>& Raw = Events->GetPendingDecisionsRaw();
	if (Raw.Num() == 0)
	{
		UE_LOG(LogStrategosCore, Log, TEXT("[DEBUG] Nenhuma decisão pendente."));
		return TEXT("Nenhuma decisão pendente.");
	}

	FString Result;
	int32 Total = 0;
	for (const auto& Pair : Raw)
	{
		for (const FPendingDecision& P : Pair.Value)
		{
			const FString Line = FString::Printf(TEXT("  Nação=%s  EventId=%s"),
				*Pair.Key.ToString(), *P.Context.EventId.ToString());
			UE_LOG(LogStrategosCore, Log, TEXT("[DEBUG]%s"), *Line);
			Result += Line + TEXT("\n");
			++Total;
		}
	}
	UE_LOG(LogStrategosCore, Log, TEXT("[DEBUG] Total: %d decisão(ões) pendente(s)."), Total);
	return Result;
}

bool UEventDebugSubsystem::TestSaveLoadPersistence(FName CustomEventId, FName NationId)
{
	UEventSubsystem* Events = ResolveEventSubsystem();
	USaveSubsystem*  Save   = ResolveSaveSubsystem();
	if (!Events || !Save)
	{
		UE_LOG(LogStrategosCore, Warning, TEXT("[DEBUG] TestSaveLoadPersistence: subsistemas indisponíveis."));
		return false;
	}

	// 1. Cria e registra o asset efêmero (necessário antes e depois do load).
	UEventAsset* E = MakeEphemeralDecision(this, CustomEventId);
	Events->RegisterEphemeralEvent(E);

	// 2. Dispara para enfileirar.
	const UWorld* W = GetWorld();
	FEventContext Ctx;
	Ctx.EventId        = CustomEventId;
	Ctx.SourceNationId = NationId;
	Ctx.TriggerTag     = TEXT("Debug.Manual");
	if (W)
	{
		if (const UGameInstance* GI = W->GetGameInstance())
		{
			if (const UTimeSubsystem* Time = GI->GetSubsystem<UTimeSubsystem>())
			{
				Ctx.FireDate = Time->GetCurrentDate();
			}
		}
	}
	Events->FireEventById(CustomEventId, Ctx);

	const bool bEnqueued = Events->HasPendingDecisions(NationId);
	if (!bEnqueued)
	{
		UE_LOG(LogStrategosCore, Warning,
			TEXT("[DEBUG] TestSaveLoadPersistence AVISO: '%s' não foi enfileirado para '%s'. "
			     "Verifique se NationId corresponde ao jogador (nações AI auto-resolvem)."),
			*CustomEventId.ToString(), *NationId.ToString());
	}

	// 3. Salva.
	const FString SlotName = TEXT("_debug_test");
	Save->SaveToSlot(SlotName);

	// 4. Carrega.
	Save->LoadFromSlot(SlotName);

	// 5. Re-registra o asset efêmero pois LoadFromSlot não o restaura (apenas FEventContext).
	Events->RegisterEphemeralEvent(E);

	// 6. Verifica se a decisão sobreviveu ao ciclo.
	const TArray<FPendingDecision> AfterLoad = Events->GetPendingDecisions(NationId);
	const bool bFound = AfterLoad.ContainsByPredicate([&](const FPendingDecision& P)
	{
		return P.Context.EventId == CustomEventId;
	});

	const bool bPass = bFound;
	const FString Verdict = bPass ? TEXT("PASS") : TEXT("FAIL");
	const FString Msg = FString::Printf(
		TEXT("[DEBUG] TestSaveLoadPersistence %s: '%s' para '%s'."),
		*Verdict, *CustomEventId.ToString(), *NationId.ToString());

	UE_LOG(LogStrategosCore, Log, TEXT("%s"), *Msg);
	ShowOnScreen(bPass, Msg);
	return bPass;
}

// ---------------------------------------------------------------------------
// Console command delegates

void UEventDebugSubsystem::Cmd_FireEvent(const TArray<FString>& Args)
{
	if (Args.Num() < 2)
	{
		UE_LOG(LogStrategosCore, Warning,
			TEXT("Uso: Strategos.Event.Fire <EventId> <NationId>"));
		return;
	}
	FireEventDebug(FName(*Args[0]), FName(*Args[1]));
}

void UEventDebugSubsystem::Cmd_ListPending(const TArray<FString>& Args)
{
	DumpPendingDecisions();
}

void UEventDebugSubsystem::Cmd_CreateTestDecision(const TArray<FString>& Args)
{
	if (Args.Num() < 2)
	{
		UE_LOG(LogStrategosCore, Warning,
			TEXT("Uso: Strategos.Event.CreateTestDecision <CustomId> <NationId>"));
		return;
	}
	CreateAndFireTestDecision(FName(*Args[0]), FName(*Args[1]));
}

void UEventDebugSubsystem::Cmd_TestPersistence(const TArray<FString>& Args)
{
	if (Args.Num() < 2)
	{
		UE_LOG(LogStrategosCore, Warning,
			TEXT("Uso: Strategos.Event.TestPersistence <CustomId> <NationId>"));
		return;
	}
	TestSaveLoadPersistence(FName(*Args[0]), FName(*Args[1]));
}
