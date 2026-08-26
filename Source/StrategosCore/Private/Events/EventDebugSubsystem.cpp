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
#include "Engine/GameViewportClient.h"

// Slate
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateColor.h"

// ---------------------------------------------------------------------------
// Overlay Slate — definido no .cpp para não expor tipos Slate no header público.

namespace
{
	// Brushes e cores como estáticos de função: Slate guarda ponteiro cru para
	// FSlateBrush, então o objeto precisa sobreviver ao Construct().
	const FSlateBrush* PanelBrush()
	{
		static FSlateColorBrush Brush(FLinearColor(0.04f, 0.04f, 0.06f, 0.93f));
		return &Brush;
	}

	const FSlateBrush* RuleBrush()
	{
		static FSlateColorBrush Brush(FLinearColor(0.20f, 0.20f, 0.22f, 1.f));
		return &Brush;
	}

	const FSlateColor ColText  { FLinearColor(0.85f, 0.85f, 0.85f) };
	const FSlateColor ColDim   { FLinearColor(0.50f, 0.50f, 0.50f) };
	const FSlateColor ColTitle { FLinearColor(1.00f, 0.72f, 0.00f) };
	const FSlateColor ColItem  { FLinearColor(0.90f, 0.90f, 0.70f) };
	const FSlateColor ColLog   { FLinearColor(0.65f, 0.88f, 0.65f) };
}

class SEventDebugOverlay : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SEventDebugOverlay) {}
		SLATE_ARGUMENT(TWeakObjectPtr<UEventDebugSubsystem>, DebugSys)
		SLATE_ARGUMENT(TWeakObjectPtr<UWorld>, World)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void AppendLog(const FString& Line);
	void RefreshDecisions();

private:
	/** Lê os dois campos de texto. Retorna false e loga se algum estiver vazio. */
	bool ReadIds(FName& OutEventId, FName& OutNationId);

	FReply OnFireClicked();
	FReply OnCreateTestClicked();
	FReply OnTestPersistenceClicked();
	FReply OnRefreshClicked();

	TWeakObjectPtr<UEventDebugSubsystem> DebugSys;
	TWeakObjectPtr<UWorld>               World;

	TSharedPtr<SEditableTextBox> EventIdBox;
	TSharedPtr<SEditableTextBox> NationIdBox;
	TSharedPtr<SVerticalBox>     DecisionsBox;
	TSharedPtr<STextBlock>       LogBlock;
	TSharedPtr<SScrollBox>       LogScroll;

	TArray<FString> LogLines;
	static constexpr int32 MaxLogLines = 25;
};

void SEventDebugOverlay::Construct(const FArguments& InArgs)
{
	DebugSys = InArgs._DebugSys;
	World    = InArgs._World;

	const FSlateFontInfo FontBody  = FCoreStyle::GetDefaultFontStyle("Regular", 9);
	const FSlateFontInfo FontTitle = FCoreStyle::GetDefaultFontStyle("Bold", 10);
	const FSlateFontInfo FontLog   = FCoreStyle::GetDefaultFontStyle("Mono", 8);

	ChildSlot
	.HAlign(HAlign_Right)
	.VAlign(VAlign_Top)
	.Padding(FMargin(0.f, 8.f, 12.f, 0.f))
	[
		SNew(SBox)
		.WidthOverride(400.f)
		[
			SNew(SBorder)
			.BorderImage(PanelBrush())
			.Padding(FMargin(10.f, 8.f))
			[
				SNew(SVerticalBox)

				// ── Título ────────────────────────────────────────────────
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 6.f)
				[
					SNew(STextBlock)
					.Font(FontTitle)
					.ColorAndOpacity(ColTitle)
					.Text(NSLOCTEXT("StrategosDebug", "OverlayTitle",
						"EVENT DEBUG   [Strategos.Event.ToggleDebugUI]"))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 8.f)
				[
					SNew(SBorder).BorderImage(RuleBrush()).Padding(FMargin(0.f, 1.f))
				]

				// ── Inputs ────────────────────────────────────────────────
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 2.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(0.28f)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Font(FontBody).ColorAndOpacity(ColText)
						.Text(NSLOCTEXT("StrategosDebug", "EventIdLbl", "EventId:"))
					]
					+ SHorizontalBox::Slot()
					.FillWidth(0.72f)
					[
						SAssignNew(EventIdBox, SEditableTextBox)
						.Font(FontBody)
						.HintText(NSLOCTEXT("StrategosDebug", "EventIdHint", "ex: ForeignInvestor"))
					]
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 2.f, 0.f, 6.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(0.28f)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Font(FontBody).ColorAndOpacity(ColText)
						.Text(NSLOCTEXT("StrategosDebug", "NationIdLbl", "NationId:"))
					]
					+ SHorizontalBox::Slot()
					.FillWidth(0.72f)
					[
						SAssignNew(NationIdBox, SEditableTextBox)
						.Font(FontBody)
						.HintText(NSLOCTEXT("StrategosDebug", "NationIdHint", "ex: Albion"))
					]
				]

				// ── Ações ─────────────────────────────────────────────────
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 2.f, 0.f, 8.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(0.f, 0.f, 4.f, 0.f)
					[
						SNew(SButton)
						.Text(NSLOCTEXT("StrategosDebug", "BtnFire", "Disparar"))
						.ToolTipText(NSLOCTEXT("StrategosDebug", "BtnFireTip",
							"Dispara um evento ja registrado (registry ou fallback)."))
						.OnClicked(this, &SEventDebugOverlay::OnFireClicked)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(0.f, 0.f, 4.f, 0.f)
					[
						SNew(SButton)
						.Text(NSLOCTEXT("StrategosDebug", "BtnCreate", "Criar Teste"))
						.ToolTipText(NSLOCTEXT("StrategosDebug", "BtnCreateTip",
							"Cria uma Decision efemera com esse Id e a dispara."))
						.OnClicked(this, &SEventDebugOverlay::OnCreateTestClicked)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SButton)
						.Text(NSLOCTEXT("StrategosDebug", "BtnPersist", "Testar Persistencia"))
						.ToolTipText(NSLOCTEXT("StrategosDebug", "BtnPersistTip",
							"Cria -> salva -> recarrega -> verifica se a decisao sobreviveu."))
						.OnClicked(this, &SEventDebugOverlay::OnTestPersistenceClicked)
					]
				]

				// ── Fila de decisions ─────────────────────────────────────
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 4.f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Font(FontBody).ColorAndOpacity(ColText)
						.Text(NSLOCTEXT("StrategosDebug", "PendingHeader", "Decisions pendentes:"))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SButton)
						.Text(NSLOCTEXT("StrategosDebug", "BtnRefresh", "Atualizar"))
						.OnClicked(this, &SEventDebugOverlay::OnRefreshClicked)
					]
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 8.f)
				[
					SNew(SBox)
					.MaxDesiredHeight(130.f)
					[
						SNew(SScrollBox)
						+ SScrollBox::Slot()
						[
							SAssignNew(DecisionsBox, SVerticalBox)
						]
					]
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 4.f)
				[
					SNew(SBorder).BorderImage(RuleBrush()).Padding(FMargin(0.f, 1.f))
				]

				// ── Log ───────────────────────────────────────────────────
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 3.f)
				[
					SNew(STextBlock)
					.Font(FontBody).ColorAndOpacity(ColText)
					.Text(NSLOCTEXT("StrategosDebug", "LogHeader", "Log:"))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SBox)
					.MaxDesiredHeight(140.f)
					[
						SAssignNew(LogScroll, SScrollBox)
						+ SScrollBox::Slot()
						[
							SAssignNew(LogBlock, STextBlock)
							.Font(FontLog)
							.ColorAndOpacity(ColLog)
							.AutoWrapText(true)
							.Text(FText::GetEmpty())
						]
					]
				]
			]
		]
	];

	RefreshDecisions();
}

bool SEventDebugOverlay::ReadIds(FName& OutEventId, FName& OutNationId)
{
	if (!DebugSys.IsValid())
	{
		AppendLog(TEXT("[ERRO] DebugSubsystem invalido."));
		return false;
	}
	const FString EvStr = EventIdBox.IsValid()  ? EventIdBox->GetText().ToString().TrimStartAndEnd()  : FString();
	const FString NaStr = NationIdBox.IsValid() ? NationIdBox->GetText().ToString().TrimStartAndEnd() : FString();

	if (EvStr.IsEmpty() || NaStr.IsEmpty())
	{
		AppendLog(TEXT("Preencha EventId e NationId."));
		return false;
	}
	OutEventId  = FName(*EvStr);
	OutNationId = FName(*NaStr);
	return true;
}

FReply SEventDebugOverlay::OnFireClicked()
{
	FName EvId, NaId;
	if (!ReadIds(EvId, NaId)) return FReply::Handled();

	DebugSys->FireEventDebug(EvId, NaId);
	AppendLog(FString::Printf(TEXT("Disparado: %s -> %s"), *EvId.ToString(), *NaId.ToString()));
	RefreshDecisions();
	return FReply::Handled();
}

FReply SEventDebugOverlay::OnCreateTestClicked()
{
	FName EvId, NaId;
	if (!ReadIds(EvId, NaId)) return FReply::Handled();

	DebugSys->CreateAndFireTestDecision(EvId, NaId);
	AppendLog(FString::Printf(TEXT("Criado + disparado: %s -> %s"), *EvId.ToString(), *NaId.ToString()));
	RefreshDecisions();
	return FReply::Handled();
}

FReply SEventDebugOverlay::OnTestPersistenceClicked()
{
	FName EvId, NaId;
	if (!ReadIds(EvId, NaId)) return FReply::Handled();

	const bool bPass = DebugSys->TestSaveLoadPersistence(EvId, NaId);
	AppendLog(FString::Printf(TEXT("Persistencia %s: %s -> %s"),
		bPass ? TEXT("PASS") : TEXT("FAIL"), *EvId.ToString(), *NaId.ToString()));
	RefreshDecisions();
	return FReply::Handled();
}

FReply SEventDebugOverlay::OnRefreshClicked()
{
	RefreshDecisions();
	AppendLog(TEXT("Lista atualizada."));
	return FReply::Handled();
}

void SEventDebugOverlay::RefreshDecisions()
{
	if (!DecisionsBox.IsValid()) return;
	DecisionsBox->ClearChildren();

	const FSlateFontInfo FontBody = FCoreStyle::GetDefaultFontStyle("Regular", 9);

	auto AddLine = [&](const FText& Txt, const FSlateColor& Col)
	{
		DecisionsBox->AddSlot()
		.AutoHeight()
		.Padding(FMargin(0.f, 1.f))
		[
			SNew(STextBlock).Font(FontBody).ColorAndOpacity(Col).Text(Txt)
		];
	};

	if (!World.IsValid())
	{
		AddLine(NSLOCTEXT("StrategosDebug", "NoWorld", "[world invalido]"), ColDim);
		return;
	}

	UEventSubsystem* Events = World->GetSubsystem<UEventSubsystem>();
	if (!Events)
	{
		AddLine(NSLOCTEXT("StrategosDebug", "NoEventSys", "[EventSubsystem indisponivel]"), ColDim);
		return;
	}

	const TMap<FName, TArray<FPendingDecision>>& Raw = Events->GetPendingDecisionsRaw();

	int32 Total = 0;
	for (const auto& Pair : Raw)
	{
		for (const FPendingDecision& P : Pair.Value)
		{
			AddLine(FText::FromString(FString::Printf(TEXT("- %s  ->  %s"),
				*Pair.Key.ToString(), *P.Context.EventId.ToString())), ColItem);
			++Total;
		}
	}

	if (Total == 0)
	{
		AddLine(NSLOCTEXT("StrategosDebug", "NoPending", "(nenhuma decisao pendente)"), ColDim);
	}
}

void SEventDebugOverlay::AppendLog(const FString& Line)
{
	LogLines.Insert(Line, 0);
	if (LogLines.Num() > MaxLogLines)
	{
		LogLines.SetNum(MaxLogLines);
	}
	if (LogBlock.IsValid())
	{
		LogBlock->SetText(FText::FromString(FString::Join(LogLines, TEXT("\n"))));
	}
	// Linha mais recente fica no topo.
	if (LogScroll.IsValid())
	{
		LogScroll->ScrollToStart();
	}
}

// ---------------------------------------------------------------------------
// UEventDebugSubsystem — Lifecycle

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
	if (DebugOverlay.IsValid())
	{
		if (UWorld* W = GetWorld())
		{
			if (UGameViewportClient* VP = W->GetGameViewport())
			{
				VP->RemoveViewportWidgetContent(DebugOverlay.ToSharedRef());
			}
		}
		DebugOverlay.Reset();
	}
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
		TEXT("Lista todas as decisoes pendentes para todas as nacoes."),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &UEventDebugSubsystem::Cmd_ListPending),
		ECVF_Cheat
	));

	ConsoleObjects.Add(CM.RegisterConsoleCommand(
		TEXT("Strategos.Event.CreateTestDecision"),
		TEXT("Cria e dispara uma Decision efemera. Args: <CustomId> <NationId>"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &UEventDebugSubsystem::Cmd_CreateTestDecision),
		ECVF_Cheat
	));

	ConsoleObjects.Add(CM.RegisterConsoleCommand(
		TEXT("Strategos.Event.TestPersistence"),
		TEXT("Ciclo de teste save/load para decisoes. Args: <CustomId> <NationId>"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &UEventDebugSubsystem::Cmd_TestPersistence),
		ECVF_Cheat
	));

	ConsoleObjects.Add(CM.RegisterConsoleCommand(
		TEXT("Strategos.Event.ToggleDebugUI"),
		TEXT("Mostra/esconde o painel visual de debug de eventos."),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &UEventDebugSubsystem::Cmd_ToggleUI),
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
// Overlay toggle

void UEventDebugSubsystem::ToggleDebugUI()
{
	UWorld* W = GetWorld();
	if (!W) return;

	UGameViewportClient* Viewport = W->GetGameViewport();
	if (!Viewport)
	{
		UE_LOG(LogStrategosCore, Warning, TEXT("[DEBUG] Sem GameViewport; overlay indisponivel."));
		return;
	}

	if (DebugOverlay.IsValid())
	{
		Viewport->RemoveViewportWidgetContent(DebugOverlay.ToSharedRef());
		DebugOverlay.Reset();
		UE_LOG(LogStrategosCore, Log, TEXT("[DEBUG] Overlay de eventos escondido."));
		return;
	}

	TSharedRef<SEventDebugOverlay> Overlay = SNew(SEventDebugOverlay)
		.DebugSys(this)
		.World(W);

	DebugOverlay = Overlay;
	Viewport->AddViewportWidgetContent(Overlay, 100);
	UE_LOG(LogStrategosCore, Log, TEXT("[DEBUG] Overlay de eventos exibido."));
}

void UEventDebugSubsystem::RefreshDebugUI()
{
	if (!DebugOverlay.IsValid()) return;
	StaticCastSharedPtr<SEventDebugOverlay>(DebugOverlay)->RefreshDecisions();
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
		UEventAsset* E = NewObject<UEventAsset>(Outer);
		E->Id          = CustomId;
		E->Title       = FText::FromString(FString::Printf(TEXT("[DEBUG] %s"), *CustomId.ToString()));
		E->Description = NSLOCTEXT("StrategosDebug", "TestDesc",
			"Evento efemero criado pelo debug subsystem para teste de persistencia.");
		E->Type        = EEventType::Decision;
		E->Category    = TEXT("political");
		E->TriggerTag  = TEXT("Debug.Manual");
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
			GEngine->AddOnScreenDebugMessage(-1, 8.f, bGreen ? FColor::Green : FColor::Red, Msg);
		}
	}

	/** Preenche FireDate a partir do TimeSubsystem, quando disponível. */
	void StampFireDate(const UWorld* W, FEventContext& Ctx)
	{
		if (!W) return;
		if (const UGameInstance* GI = W->GetGameInstance())
		{
			if (const UTimeSubsystem* Time = GI->GetSubsystem<UTimeSubsystem>())
			{
				Ctx.FireDate = Time->GetCurrentDate();
			}
		}
	}
}

// ---------------------------------------------------------------------------
// UFUNCTION implementations

void UEventDebugSubsystem::FireEventDebug(FName EventId, FName NationId)
{
	UEventSubsystem* Events = ResolveEventSubsystem();
	if (!Events) return;

	if (!Events->GetEventById(EventId))
	{
		UE_LOG(LogStrategosCore, Warning,
			TEXT("[DEBUG] FireEvent: '%s' nao esta registrado. Use CreateTestDecision para criar um efemero."),
			*EventId.ToString());
		return;
	}

	FEventContext Ctx;
	Ctx.EventId        = EventId;
	Ctx.SourceNationId = NationId;
	Ctx.TriggerTag     = TEXT("Debug");
	StampFireDate(GetWorld(), Ctx);

	Events->FireEventById(EventId, Ctx);
	UE_LOG(LogStrategosCore, Log, TEXT("[DEBUG] FireEvent '%s' -> '%s'"),
		*EventId.ToString(), *NationId.ToString());
	RefreshDebugUI();
}

void UEventDebugSubsystem::CreateAndFireTestDecision(FName CustomEventId, FName NationId)
{
	UEventSubsystem* Events = ResolveEventSubsystem();
	if (!Events) return;

	UEventAsset* E = MakeEphemeralDecision(this, CustomEventId);
	Events->RegisterEphemeralEvent(E);

	FEventContext Ctx;
	Ctx.EventId        = CustomEventId;
	Ctx.SourceNationId = NationId;
	Ctx.TriggerTag     = TEXT("Debug.Manual");
	StampFireDate(GetWorld(), Ctx);

	Events->FireEventById(CustomEventId, Ctx);
	UE_LOG(LogStrategosCore, Log, TEXT("[DEBUG] CreateAndFireTestDecision '%s' -> '%s'"),
		*CustomEventId.ToString(), *NationId.ToString());
	RefreshDebugUI();
}

FString UEventDebugSubsystem::DumpPendingDecisions() const
{
	UEventSubsystem* Events = ResolveEventSubsystem();
	if (!Events)
	{
		return TEXT("EventSubsystem indisponivel.");
	}

	const TMap<FName, TArray<FPendingDecision>>& Raw = Events->GetPendingDecisionsRaw();

	FString Result;
	int32 Total = 0;
	for (const auto& Pair : Raw)
	{
		for (const FPendingDecision& P : Pair.Value)
		{
			const FString Line = FString::Printf(TEXT("  Nacao=%s  EventId=%s"),
				*Pair.Key.ToString(), *P.Context.EventId.ToString());
			UE_LOG(LogStrategosCore, Log, TEXT("[DEBUG]%s"), *Line);
			Result += Line + TEXT("\n");
			++Total;
		}
	}

	if (Total == 0)
	{
		UE_LOG(LogStrategosCore, Log, TEXT("[DEBUG] Nenhuma decisao pendente."));
		return TEXT("Nenhuma decisao pendente.");
	}

	UE_LOG(LogStrategosCore, Log, TEXT("[DEBUG] Total: %d decisao(oes) pendente(s)."), Total);
	return Result;
}

bool UEventDebugSubsystem::TestSaveLoadPersistence(FName CustomEventId, FName NationId)
{
	UEventSubsystem* Events = ResolveEventSubsystem();
	USaveSubsystem*  Save   = ResolveSaveSubsystem();
	if (!Events || !Save)
	{
		UE_LOG(LogStrategosCore, Warning,
			TEXT("[DEBUG] TestSaveLoadPersistence: subsistemas indisponiveis."));
		return false;
	}

	// 1. Cria e registra o asset efemero.
	UEventAsset* E = MakeEphemeralDecision(this, CustomEventId);
	Events->RegisterEphemeralEvent(E);

	// 2. Dispara: enfileira se a nacao for do jogador, auto-resolve caso contrario.
	FEventContext Ctx;
	Ctx.EventId        = CustomEventId;
	Ctx.SourceNationId = NationId;
	Ctx.TriggerTag     = TEXT("Debug.Manual");
	StampFireDate(GetWorld(), Ctx);
	Events->FireEventById(CustomEventId, Ctx);

	// 3. Sem enfileirar nao ha o que testar — o ciclo passaria trivialmente.
	if (!Events->HasPendingDecisions(NationId))
	{
		const FString Msg = FString::Printf(
			TEXT("[DEBUG] TestSaveLoadPersistence INCONCLUSIVO: '%s' nao foi enfileirado para '%s'. "
			     "Confirme que essa nacao e a do jogador (nacoes de IA auto-resolvem)."),
			*CustomEventId.ToString(), *NationId.ToString());
		UE_LOG(LogStrategosCore, Warning, TEXT("%s"), *Msg);
		ShowOnScreen(false, Msg);
		return false;
	}

	// 4. Save -> Load no mesmo slot.
	const FString SlotName = TEXT("_debug_test");
	if (!Save->SaveToSlot(SlotName))
	{
		UE_LOG(LogStrategosCore, Error, TEXT("[DEBUG] TestSaveLoadPersistence: SaveToSlot falhou."));
		ShowOnScreen(false, TEXT("[DEBUG] TestSaveLoadPersistence FAIL: save falhou."));
		return false;
	}
	if (!Save->LoadFromSlot(SlotName))
	{
		UE_LOG(LogStrategosCore, Error, TEXT("[DEBUG] TestSaveLoadPersistence: LoadFromSlot falhou."));
		ShowOnScreen(false, TEXT("[DEBUG] TestSaveLoadPersistence FAIL: load falhou."));
		return false;
	}

	// 5. O asset efemero nao e serializado (so o FEventContext); re-registra para
	//    que a decisao restaurada volte a resolver para um UEventAsset valido.
	Events->RegisterEphemeralEvent(E);

	// 6. Verifica que a decisao sobreviveu ao ciclo.
	const TArray<FPendingDecision> AfterLoad = Events->GetPendingDecisions(NationId);
	const bool bFound = AfterLoad.ContainsByPredicate([&](const FPendingDecision& P)
	{
		return P.Context.EventId == CustomEventId;
	});

	const FString Msg = FString::Printf(
		TEXT("[DEBUG] TestSaveLoadPersistence %s: '%s' para '%s'."),
		bFound ? TEXT("PASS") : TEXT("FAIL"),
		*CustomEventId.ToString(), *NationId.ToString());

	UE_LOG(LogStrategosCore, Log, TEXT("%s"), *Msg);
	ShowOnScreen(bFound, Msg);
	RefreshDebugUI();
	return bFound;
}

// ---------------------------------------------------------------------------
// Console command delegates

void UEventDebugSubsystem::Cmd_FireEvent(const TArray<FString>& Args)
{
	if (Args.Num() < 2)
	{
		UE_LOG(LogStrategosCore, Warning, TEXT("Uso: Strategos.Event.Fire <EventId> <NationId>"));
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

void UEventDebugSubsystem::Cmd_ToggleUI(const TArray<FString>& Args)
{
	ToggleDebugUI();
}
