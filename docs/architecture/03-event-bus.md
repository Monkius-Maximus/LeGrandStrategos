# 03 — UEventBusSubsystem

O Event Bus é o **canal de comunicação desacoplado** entre subsistemas. Sem ele, o grafo de dependências entre Economia, Política, Diplomacia, Militar e Eventos vira spaghetti rapidamente.

> **Nota**: este documento descreve o Event Bus como infraestrutura de pub/sub. O `UEventSubsystem` (eventos narrativos com triggers, condições, escolhas) é descrito em [`30-events.md`](30-events.md). São coisas distintas.

---

## 1. Princípio

Subsistemas **nunca** chamam outros subsistemas diretamente. Em vez disso:

- **Emitem signals** quando algo acontece (ex: `OnNationStatusChanged`)
- **Escutam signals** quando precisam reagir
- O Event Bus orquestra o despacho

Isso garante que adicionar um novo subsistema (ou remover) não exige refatorar os existentes.

---

## 2. Definição

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnNationStatusChanged, int32, NationId, ENationStatus, NewStatus);

class UEventBusSubsystem : public UGameInstanceSubsystem
{
    UPROPERTY(BlueprintAssignable) FOnNationStatusChanged OnNationStatusChanged;
    UPROPERTY(BlueprintAssignable) FOnBattleStarted       OnBattleStarted;
    UPROPERTY(BlueprintAssignable) FOnEconomyTick         OnEconomyTick;
    UPROPERTY(BlueprintAssignable) FOnDayPassed           OnDayPassed;
    // etc
};
```

---

## 3. Interfaces Principais (Plug-ins)

- `ITickable` (custom) — quem responde ao tick simulado.
- `ISerializableEntity` — contrato para save.
- `IDiplomaticAction` — ação diplomática plugável.
- `IBattleEffect` — efeito de carta plugável.
- `IAIBrain` — cérebro substituível (script, utility, GOAP).

---

## 4. Padrão de Comunicação

```
Subsystem A                    EventBus                   Subsystem B
    │                              │                           │
    │── emit signal ──────────────►│                           │
    │                              │── dispatch ──────────────►│
    │                              │                           │── reage
    │                              │◄── (sem retorno)──────────│
```

- **Sem retorno**: o emissor não espera resposta.
- **Múltiplos ouvintes**: signals são `MulticastDelegate`. Vários subsistemas e a UI podem escutar o mesmo evento.
- **Ordem indefinida**: não dependa da ordem em que os ouvintes processam o signal — caso precise, use prioridades ou cadeia explícita.

---

## 5. Lista de Signals (visão geral)

| Origem | Signal | Quem ouve |
|---|---|---|
| `TimeSubsystem` | `OnDay`, `OnMonth`, `OnYear` | Todos os subsistemas com tick |
| `EconomySubsystem` | `OnEconomyTick`, `OnPriceShock`, `OnIndustryBuilt` | UI, Politics, Events |
| `MilitarySubsystem` | `OnArmyMoved`, `OnSiegeStarted`, `OnNavalBlockade` | UI, Economy, Events |
| `BattleSubsystem` | `OnBattleStarted`, `OnBattleFinished`, `OnCardPlayed` | UI, Military, Events |
| `DiplomacySubsystem` | `OnTreatySigned`, `OnWarDeclared`, `OnInfamyChanged` | UI, AI, Events |
| `PoliticsSubsystem` | `OnLawPassed`, `OnElectionResult`, `OnRevoltStarted` | UI, AI, Events |
| `ProgressSubsystem` | `OnTechResearched`, `OnNationEraAdvanced` | Todos os subsistemas |
| `EventSubsystem` | `OnEventFired`, `OnPlayerChoice` | UI, AI |

> A lista completa de signals por subsistema está documentada em cada `XX-*.md` correspondente.

---

## 6. Pontos de Atenção

- **Não emita signals em loops apertados.** Cada `Broadcast` percorre todos os ouvintes — em um tick econômico que processa 2000 províncias, emitir um signal por província é catastrófico. Agregue.
- **Determinismo**: a ordem de inscrição de ouvintes deve ser estável. Em multiplayer/replay, dois clientes precisam processar signals na mesma ordem.
- **Não use signals para fluxo síncrono crítico.** Se o subsistema B *precisa* responder antes do A continuar, a comunicação não é via Bus — é via interface direta com contrato explícito.
- **Logs de signals** em build de desenvolvimento são valiosíssimos para debug. Considere uma flag global `bLogAllSignals` que imprime cada emissão.
