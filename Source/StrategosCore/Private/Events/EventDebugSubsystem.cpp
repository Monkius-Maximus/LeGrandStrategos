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
#include "Widgets/SOverlay.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Styling/SlateTypes.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"

// ---------------------------------------------------------------------------
// Slate overlay — definido aqui para não exportar tipos Slate no header público.

namespace
{
	// Pincel sólido reutilizável para o fundo escuro do painel.
	const FSlateBrush* GetDarkBrush()
	{
		static FSlateColorBrush Brush(FLinearColor(0.04f, 0.04f, 0.06f, 0.93f));
		return &Brush;
	}

	const FSlateBrush* GetSeparatorBrush()
	{
		static FSlateColorBrush Brush(FLinearColor(0.2f, 0.2f, 0.22f, 1.f));
		return &Brush;
	}
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

	const FTextBlockStyle BodyStyle = FTextBlockStyle()
		.SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 9))
		.SetColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.85f, 0.85f)));

	const FTextBlockStyle DimStyle = FTextBlockStyle()
		.SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 9))
		.SetColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)));

	const FTextBlockStyle TitleStyle = FTextBlockStyle()
		.SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 10))
		.SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.72f, 0.f)));

	const FTextBlockStyle ItemStyle = FTextBlockStyle()
		.SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 9))
		.SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.9f, 0.7f)));

	const FTextBlockStyle LogStyle = FTextBlockStyle()
		.SetFont(FCoreStyle::GetDefaultFontStyle("Mono", 8))
		.SetColorAndOpacity(FSlateColor(FLinearColor(0.65f, 0.88f, 0.65f)));

	ChildSlot
	[
		// Ancora no canto superior direito da tela.
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		.Padding(FMargin(0.f, 8.f, 12.f, 0.f))
		[
			SNew(SBox)
			.WidthOverride(400.f)
			[
				SNew(SBorder)
				.BorderImage(GetDarkBrush())
				.Padding(FMargin(10.f, 8.f))
				[
					SNew(SVerticalBox)

					// ── Título ────────────────────────────────────────────
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 0.f, 0.f, 6.f)
					[
						SNew(STextBlock)
						.TextStyle(&TitleStyle)
						.Text(NSLOCTEXT("StrategosDebug","OverlayTitle","EVENT DEBUG  [Strategos.Event.ToggleDebugUI]"))
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 0.f, 0.f, 8.f)
					[
						SNew(SBorder)
						.BorderImage(GetSeparatorBrush())
						.Padding(FMargin(0.f, 1.f))
					]

					// ── Inputs ────────────────────────────────────────────
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
							.TextStyle(&BodyStyle)
							.Text(NSLOCTEXT("StrategosDebug","EventIdLbl","EventId:"))
						]
						+ SHorizontalBox::Slot()
						.FillWidth(0.72f)
						[
							SAssignNew(EventIdBox, SEditableTextBox)
							.HintText(NSLOCTEXT("StrategosDebug","EventIdHint","ex: ForeignInvestor"))
							.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
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
							.TextStyle(&BodyStyle)
							.Text(NSLOCTEXT("StrategosDebug","NationIdLbl","NationId:"))
						]
						+ SHorizontalBox::Slot()
						.FillWidth(0.72f)
						[
							SAssignNew(NationIdBox, SEditableTextBox)
							.HintText(NSLOCTEXT("StrategosDebug","NationIdHint","ex: Albion"))
							.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
						]
					]

					// ── Botões de ação ─────────────────────────────────────
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
							.Text(NSLOCTEXT("StrategosDebug","BtnFire","Disparar"))
							.OnClicked_Raw(this, &SEventDebugOverlay::OnFireClicked)
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(0.f, 0.f, 4.f, 0.f)
						[
							SNew(SButton)
							.Text(NSLOCTEXT("StrategosDebug","BtnCreate","Criar Teste"))
							.OnClicked_Raw(this, &SEventDebugOverlay::OnCreateTestClicked)
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SButton)
							.Text(NSLOCTEXT("StrategosDebug","BtnTestPersistence","Testar Persistencia"))
							.OnClicked_Raw(this, &SEventDebugOverlay::OnTestPersistenceClicked)
						]
					]

					// ── Decisions pendentes ────────────────────────────────
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
							.TextStyle(&BodyStyle)
							.Text(NSLOCTEXT("StrategosDebug","PendingHeader","Decisions Pendentes:"))
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SButton)
							.Text(NSLOCTEXT("StrategosDebug","BtnRefresh","Atualizar"))
							.OnClicked_Raw(this, &SEventDebugOverlay::OnRefreshClicked)
						]
					]

					+ SVerticalBox::Slot()
					.MaxHeight(130.f)
					.Padding(0.f, 0.f, 0.f, 8.f)
					[
						SNew(SScrollBox)
						+ SScrollBox::Slot()
						[
							SAssignNew(DecisionsBox, SVerticalBox)
						]
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 0.f, 0.f, 4.f)
					[
						SNew(SBorder)
						.BorderImage(GetSeparatorBrush())
						.Padding(FMargin(0.f, 1.f))
					]

					// ── Log ───────────────────────────────────────────────
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 0.f, 0.f, 3.f)
					[
						SNew(STextBlock)
						.TextStyle(&BodyStyle)
						.Text(NSLOCTEXT("StrategosDebug","LogHeader","Log:"))
					]

					+ SVerticalBox::Slot()
					.MaxHeight(140.f)
					[
						SAssignNew(LogScroll, SScrollBox)
						+ SScrollBox::Slot()
						[
							SAssignNew(LogBlock, STextBlock)
							.TextStyle(&LogStyle)
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

FReply SEventDebugOverlay::OnFireClicked()
{
	if (!DebugSys.IsValid())
	{
		AppendLog(TEXT("[ERR] DebugSys invalido."));
		return FReply::Handled();
	}
	const FName EvId(*EventIdBox->GetText().ToString());
	const FName NaId(*NationIdBox->GetText().ToString());
	if (EvId.IsNone() || NaId.IsNone())
	{
		AppendLog(TEXT("Preencha EventId e NationId."));
		return FReply::Handled();
	}
	DebugSys->FireEventDebug(EvId, NaId);
	AppendLog(FString::Printf(TEXT("Disparado: %s -> %s"), *EvId.ToString(), *NaId.ToString()));
	RefreshDecisions();
	return FReply::Handled();
}

FReply SEventDebugOverlay::OnCreateTestClicked()
{
	if (!DebugSys.IsValid())
	{
		AppendLog(TEXT("[ERR] DebugSys invalido."));
		return FReply::Handled();
	}
	const FName EvId(*EventIdBox->GetText().ToString());
	const FName NaId(*NationIdBox->GetText().ToString());
	if (EvId.IsNone() || NaId.IsNone())
	{
		AppendLog(TEXT("Preencha EventId e NationId."));
		return FReply::Handled();
	}
	DebugSys->CreateAndFireTestDecision(EvId, NaId);
	AppendLog(FString::Printf(TEXT("Criado e disparado: %s -> %s"), *EvId.ToString(), *NaId.ToString()));
	RefreshDecisions();
	return FReply::Handled();
}

FReply SEventDebugOverlay::OnTestPersistenceClicked()
{
	if (!DebugSys.IsValid())
	{
		AppendLog(TEXT("[ERR] DebugSys invalido."));
		return FReply::Handled();
	}
	const FName EvId(*EventIdBox->GetText().ToString());
	const FName NaId(*NationIdBox->GetText().ToString());
	if (EvId.IsNone() || NaId.IsNone())
	{
		AppendLog(TEXT("Preencha EventId e NationId."));
		return FReply::Handled();
	}
	const bool bPass = DebugSys->TestSaveLoadPersistence(EvId, NaId);
	AppendLog(FString::Printf(TEXT("Persistencia %s: %s -> %s"),
		bPass ? TEXT("PASS") : TEXT("FAIL"),
		*EvId.ToString(), *NaId.ToString()));
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

	const FTextBlockStyle ItemStyle = FTextBlockStyle()
		.SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 9))
		.SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.9f, 0.7f)));

	const FTextBlockStyle DimStyle = FTextBlockStyle()
		.SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 9))
		.SetColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)));

	if (!World.IsValid())
	{
		DecisionsBox->AddSlot().AutoHeight()
		[
			SNew(STextBlock).TextStyle(&DimStyle)
			.Text(NSLOCTEXT("StrategosDebug","NoWorld","[world invalido]"))
		];
		return;
	}

	UEventSubsystem* Events = World->GetSubsystem<UEventSubsystem>();
	if (!Events)
	{
		DecisionsBox->AddSlot().AutoHeight()
		[
			SNew(STextBlock).TextStyle(&DimStyle)
			.Text(NSLOCTEXT("StrategosDebug","NoEventSys","[EventSubsystem indisponivel]"))
		];
		return;
	}

	const TMap<FName, TArray<FPendingDecision>>& Raw = Events->GetPendingDecisionsRaw();
	if (Raw.Num() == 0)
	{
		DecisionsBox->AddSlot().AutoHeight()
		[
			SNew(STextBlock).TextStyle(&DimStyle)
			.Text(NSLOCTEXT("StrategosDebug","NoPending","(nenhuma decisao pendente)"))
		];
		return;
	}

	for (const auto& Pair : Raw)
	{
		for (const FPendingDecision& P : Pair.Value)
		{
			const FString Line = FString::Printf(TEXT("• %s  ->  %s"),
				*Pair.Key.ToString(), *P.Context.EventId.ToString());

			DecisionsBox->AddSlot()
			.AutoHeight()
			.Padding(FMargin(0.f, 1.f))
			[
				SNew(STextBlock)
				.TextStyle(&ItemStyle)
				.Text(FText::FromString(Line))
			];
		}
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
	// Auto-scroll para o topo (linha mais recente).
	if (LogScroll.IsValid())
	{
		LogScroll->ScrollToStart();
	}
}

// ---------------------------------------------------------------------------
// Variável de módulo — referência ao overlay atual (no máximo um por mundo).

namespace
{
	TSharedPtr<SWidget> GDebugOverlayWidget;
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
	// Garante que o overlay é removido ao destruir o subsistema.
	if (GDebugOverlayWidget.IsValid())
	{
		if (UWorld* W = GetWorld())
		{
			if (UGameViewportClient* VP = W->GetGameViewport())
			{
				VP->RemoveViewportWidgetContent(GDebugOverlayWidget.ToSharedRef());
			}
		}
		GDebugOverlayWidget.Reset();
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
		TEXT("Mostra/esconde o painel de debug de eventos."),
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
// ToggleDebugUI

void UEventDebugSubsystem::ToggleDebugUI()
{
	UWorld* W = GetWorld();
	if (!W) return;

	UGameViewportClient* Viewport = W->GetGameViewport();
	if (!Viewport) return;

	if (GDebugOverlayWidget.IsValid())
	{
		Viewport->RemoveViewportWidgetContent(GDebugOverlayWidget.ToSharedRef());
		GDebugOverlayWidget.Reset();
		UE_LOG(LogStrategosCore, Log, TEXT("[DEBUG] Event overlay escondido."));
	}
	else
	{
		TSharedRef<SEventDebugOverlay> Overlay = SNew(SEventDebugOverlay)
			.DebugSys(this)
			.World(W);

		GDebugOverlayWidget = Overlay;
		Viewport->AddViewportWidgetContent(Overlay, 100);
		UE_LOG(LogStrategosCore, Log, TEXT("[DEBUG] Event overlay exibido."));
	}
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
			"Evento efemero criado via debug subsystem para teste de persistencia.");
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
		return TEXT("EventSubsystem indisponivel.");
	}

	const TMap<FName, TArray<FPendingDecision>>& Raw = Events->GetPendingDecisionsRaw();
	if (Raw.Num() == 0)
	{
		UE_LOG(LogStrategosCore, Log, TEXT("[DEBUG] Nenhuma decisao pendente."));
		return TEXT("Nenhuma decisao pendente.");
	}

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
	UE_LOG(LogStrategosCore, Log, TEXT("[DEBUG] Total: %d decisao(oes) pendente(s)."), Total);
	return Result;
}

bool UEventDebugSubsystem::TestSaveLoadPersistence(FName CustomEventId, FName NationId)
{
	UEventSubsystem* Events = ResolveEventSubsystem();
	USaveSubsystem*  Save   = ResolveSaveSubsystem();
	if (!Events || !Save)
	{
		UE_LOG(LogStrategosCore, Warning, TEXT("[DEBUG] TestSaveLoadPersistence: subsistemas indisponiveis."));
		return false;
	}

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

	const bool bEnqueued = Events->HasPendingDecisions(NationId);
	if (!bEnqueued)
	{
		UE_LOG(LogStrategosCore, Warning,
			TEXT("[DEBUG] TestSaveLoadPersistence AVISO: '%s' nao foi enfileirado para '%s'. "
			     "Verifique se NationId corresponde ao jogador."),
			*CustomEventId.ToString(), *NationId.ToString());
	}

	const FString SlotName = TEXT("_debug_test");
	Save->SaveToSlot(SlotName);
	Save->LoadFromSlot(SlotName);

	// Re-registra pois LoadFromSlot nao restaura assets efemeros, apenas FEventContext.
	Events->RegisterEphemeralEvent(E);

	const TArray<FPendingDecision> AfterLoad = Events->GetPendingDecisions(NationId);
	const bool bFound = AfterLoad.ContainsByPredicate([&](const FPendingDecision& P)
	{
		return P.Context.EventId == CustomEventId;
	});

	const FString Verdict = bFound ? TEXT("PASS") : TEXT("FAIL");
	const FString Msg = FString::Printf(
		TEXT("[DEBUG] TestSaveLoadPersistence %s: '%s' para '%s'."),
		*Verdict, *CustomEventId.ToString(), *NationId.ToString());

	UE_LOG(LogStrategosCore, Log, TEXT("%s"), *Msg);
	ShowOnScreen(bFound, Msg);
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
