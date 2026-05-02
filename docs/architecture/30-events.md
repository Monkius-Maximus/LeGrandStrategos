# 30 — UEventSubsystem (Eventos Narrativos)

Sistema de eventos narrativos com triggers, condições, escolhas e ramificações. É o **tecido narrativo** que costura todos os outros subsistemas. Sem ele, o jogo é uma planilha animada. Com ele, vira história.

> **Nota**: este documento descreve eventos *narrativos* (popups, decisões, encadeamentos). A infraestrutura genérica de pub/sub está em [`03-event-bus.md`](03-event-bus.md). São coisas distintas e complementares.

---

## 1. Princípios Arquiteturais

### Eventos não são "popups"

Esse é o erro mais comum. Em jogos pobres, evento = popup que pausa tudo. Em grand strategy maduro, evento é uma **mudança narrativa de estado** — pode ou não envolver UI, pode ou não pausar, pode ou não ter escolha.

Tipos:

| Tipo | UI? | Pausa? | Exemplo |
|---|---|---|---|
| **Decision Event** | Sim | Sim | "Crise diplomática: aceitar ultimato?" |
| **Notification Event** | Sim | Não | "Bessemer publicado em Manchester" |
| **Silent Event** | Não | Não | "Modificador 'Boas Colheitas' aplicado por 6 meses" |
| **Cinematic Event** | Sim, especial | Sim | "Coroação", "Declaração de Guerra Mundial" |
| **Background Event** | Indireto | Não | "Imigrante chega, aumentando POP" |

Todos passam pelo mesmo subsistema, mas são **categorizados** para que UI, áudio, IA e save tratem diferente.

### Eventos são *dados*, não código

Um evento bem projetado vive 100% num `UPrimaryDataAsset`. **Designers, modders e tradutores** podem criar eventos sem tocar em C++. A lógica é composta de pequenos blocos plugáveis (`UEventTrigger`, `UEventCondition`, `UEventEffect`).

### Eventos não causam loops infinitos

Erro clássico: evento A dispara evento B, que muda estado, que dispara A de novo. **O subsistema deve garantir terminação** via cooldowns, fired-once flags, e debounce.

### Eventos não são pesquisa de força bruta

Com 500 eventos definidos e 50 nações, avaliar tudo a cada tick é catastrófico. Solução: **indexação por trigger** + **avaliação preguiçosa**.

---

## 2. Estrutura Hierárquica

```
UEventSubsystem (UWorldSubsystem)
   ├── EventRegistry              [pool de UEventDefinitionAsset]
   ├── TriggerIndex               [hash: tipo de trigger → eventos relevantes]
   ├── PendingQueue               [eventos a apresentar ao jogador]
   ├── ActiveModifiers            [efeitos de eventos persistentes]
   ├── EventHistory               [log para narrativa, condições, save]
   └── ScopeResolver              [resolve "quem é o alvo" do evento]
```

---

## 3. `UEventDefinitionAsset` — A Unidade Narrativa

```cpp
UCLASS(BlueprintType)
class UEventDefinitionAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    // Identidade
    UPROPERTY(EditDefaultsOnly) FName EventId;          // "Event.GreatExhibition.1851"
    UPROPERTY(EditDefaultsOnly) FText Title;
    UPROPERTY(EditDefaultsOnly) FText Description;
    UPROPERTY(EditDefaultsOnly) UTexture2D* Illustration;
    UPROPERTY(EditDefaultsOnly) USoundBase* AmbientCue;

    // Tipo e escopo
    UPROPERTY(EditDefaultsOnly) EEventType Type;        // Decision, Notification, Silent,
                                                        //  Cinematic, Background
    UPROPERTY(EditDefaultsOnly) EEventScope Scope;      // Global, Nation, Province, Pop, Commander
    UPROPERTY(EditDefaultsOnly) EEventCategory Category;// Political, Economic, Military, Cultural,
                                                        //  Religious, Scientific, Diplomatic,
                                                        //  Disaster, Personal

    // Triggers — quando o jogo deve considerar este evento
    UPROPERTY(EditDefaultsOnly, Instanced)
    TArray<UEventTrigger*> Triggers;

    // Condições — depois de triggered, deve ser verdadeiro para disparar de fato
    UPROPERTY(EditDefaultsOnly, Instanced)
    TArray<UEventCondition*> Conditions;

    // Probabilidade base (0..1) modificada por weights condicionais
    UPROPERTY(EditDefaultsOnly) float BaseProbability;
    UPROPERTY(EditDefaultsOnly, Instanced)
    TArray<UEventWeightModifier*> WeightModifiers;

    // Opções (vazio para Notification/Silent/Background)
    UPROPERTY(EditDefaultsOnly) TArray<FEventOption> Options;

    // Efeito imediato (sem opção, comum em Notification/Silent)
    UPROPERTY(EditDefaultsOnly, Instanced)
    TArray<UEventEffect*> ImmediateEffects;

    // Controle de repetição
    UPROPERTY(EditDefaultsOnly) EEventRepeatPolicy RepeatPolicy;
        // FireOnce, FireOncePerNation, OnCooldown, AlwaysAvailable
    UPROPERTY(EditDefaultsOnly) int32 CooldownDays;

    // Prioridade (quando vários competem na mesma janela)
    UPROPERTY(EditDefaultsOnly) int32 Priority;

    // Tags para queries e mods
    UPROPERTY(EditDefaultsOnly) FGameplayTagContainer Tags;

    // Encadeamento — eventos que ficam "agendados" como follow-up
    UPROPERTY(EditDefaultsOnly) TArray<FEventChainLink> ChainedEvents;
};
```

### `FEventOption` — Escolha do jogador

```cpp
USTRUCT()
struct FEventOption
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly) FText Label;
    UPROPERTY(EditDefaultsOnly) FText Tooltip;

    // Pode estar visível mas desabilitada se condições falham
    UPROPERTY(EditDefaultsOnly, Instanced)
    TArray<UEventCondition*> AvailabilityConditions;

    // Efeitos aplicados ao escolher
    UPROPERTY(EditDefaultsOnly, Instanced)
    TArray<UEventEffect*> Effects;

    // Eventos disparados como consequência
    UPROPERTY(EditDefaultsOnly) TArray<FEventChainLink> ChainedEvents;

    // Para a IA: quão "atraente" é esta opção
    UPROPERTY(EditDefaultsOnly, Instanced)
    UEventOptionAIWeight* AIWeight;

    UPROPERTY(EditDefaultsOnly) bool bIsDefault;        // se timeout, escolhe esta
};
```

### `FEventChainLink` — Encadeamento

```cpp
USTRUCT()
struct FEventChainLink
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly) UEventDefinitionAsset* NextEvent;
    UPROPERTY(EditDefaultsOnly) int32 DelayDaysMin;
    UPROPERTY(EditDefaultsOnly) int32 DelayDaysMax;
    UPROPERTY(EditDefaultsOnly) float Probability;
    UPROPERTY(EditDefaultsOnly) bool bRetargetScope;
};
```

> **Encadeamento é a chave da narrativa**: a "Crise dos Bálcãs" pode ser uma cadeia de 8 eventos ao longo de 2 anos, com ramificações conforme escolhas.

---

## 4. Triggers — A Engenharia Anti-Brute-Force

O segredo é: eventos **não são checados a cada tick**. Eles são **inscritos em um índice** por tipo de gatilho. Quando o gatilho ocorre, apenas eventos relevantes são avaliados.

### `UEventTrigger` — Hierarquia

```cpp
UCLASS(Abstract, EditInlineNew, Blueprintable)
class UEventTrigger : public UObject
{
    GENERATED_BODY()
public:
    virtual ETriggerKind GetKind() const PURE_VIRTUAL(...);
    virtual bool MatchesContext(const FEventContext& Ctx) const PURE_VIRTUAL(...);
};
```

### Tipos concretos de Trigger

| Classe | Inscrita em | Exemplo |
|---|---|---|
| `UEventTrigger_OnDate` | Calendar índice | "1851-05-01" |
| `UEventTrigger_OnTechResearched` | TechId índice | "Tech.Steam.Locomotive" pesquisado |
| `UEventTrigger_OnEraEntered` | EraId índice | nação entra "Steam Age" |
| `UEventTrigger_OnWarDeclared` | War events | guerra declarada entre A e B |
| `UEventTrigger_OnBattleEnded` | Battle events | batalha terminou |
| `UEventTrigger_OnLawPassed` | Law events | lei aprovada |
| `UEventTrigger_OnPriceShock` | Economy events | bem ultrapassa threshold |
| `UEventTrigger_OnPopMilitancy` | Pop events | militância > X em província |
| `UEventTrigger_OnEventFired` | EventId índice | outro evento disparou |
| `UEventTrigger_OnElection` | Politics events | eleição encerrada |
| `UEventTrigger_OnCommanderTrait` | Commander events | comandante ganha traço |
| `UEventTrigger_Periodic` | Calendar índice | a cada N dias, baixa prob |
| `UEventTrigger_Manual` | Não-indexado | só por API explícita |

### `TriggerIndex` — Despacho eficiente

```cpp
UCLASS()
class UEventTriggerIndex : public UObject
{
    GENERATED_BODY()
public:
    TMap<ETriggerKind, TMap<FName, TArray<UEventDefinitionAsset*>>> ByKindAndKey;
    TArray<UEventDefinitionAsset*> PeriodicEvents;
    TArray<UEventDefinitionAsset*> DateEvents;          // ordenados por data

    void RebuildFromRegistry(const TArray<UEventDefinitionAsset*>& All);

    void QueryEventsForTrigger(ETriggerKind Kind, FName Key,
                                TArray<UEventDefinitionAsset*>& Out) const;
};
```

> Na construção do índice (uma vez no início do jogo), eventos com múltiplos triggers se inscrevem em múltiplos buckets. Em runtime, dispatch é O(1) → seguido de avaliação só dos N eventos pertinentes (geralmente <20).

---

## 5. Conditions — Validação Pós-Trigger

Depois que o trigger marca o evento como "candidato", as `UEventCondition` validam se o jogo está em estado válido para disparo.

```cpp
UCLASS(Abstract, EditInlineNew, Blueprintable)
class UEventCondition : public UObject
{
public:
    virtual bool IsSatisfied(const FEventContext& Ctx) const PURE_VIRTUAL(...);
};
```

Implementações típicas:

| Classe | Critério |
|---|---|
| `UCond_NationHasTech` | Nação alvo possui tech X |
| `UCond_NationLacksTech` | Nação alvo NÃO possui tech X |
| `UCond_AtWar` | Nação alvo em guerra |
| `UCond_AtPeace` | Nação alvo não em guerra |
| `UCond_HasGovernment` | Tipo de governo X |
| `UCond_HasLaw` | Lei X aprovada |
| `UCond_OwnsProvince` | Controla província |
| `UCond_PopulationAbove` | Pop > N |
| `UCond_LiteracyAbove` | Literacia > X |
| `UCond_TreasuryAbove` | Tesouro > X |
| `UCond_HasModifier` | Possui modificador ativo |
| `UCond_PreviousEventFired` | Evento Y já ocorreu |
| `UCond_PreviousChoiceWas` | Escolha Y foi tomada num evento |
| `UCond_DateRange` | Entre datas |
| `UCond_RandomChance` | Roll de RNG |
| `UCond_AND` `UCond_OR` `UCond_NOT` | Composição lógica |

> **Composição lógica via UObjects** permite que designers montem condições no Editor sem código.

---

## 6. Effects — A Saída Mecânica

```cpp
UCLASS(Abstract, EditInlineNew, Blueprintable)
class UEventEffect : public UObject
{
public:
    virtual void Apply(FEventContext& Ctx) PURE_VIRTUAL(...);
};
```

| Classe | Efeito |
|---|---|
| `UEffect_AddModifier` | Adiciona modificador com duração |
| `UEffect_RemoveModifier` | Remove modificador específico |
| `UEffect_ChangeTreasury` | +/- ouro |
| `UEffect_ChangePrestige` | +/- prestígio |
| `UEffect_ChangeOpinion` | Modifica relação diplomática |
| `UEffect_GrantTech` | Concede tech (espionagem, descoberta) |
| `UEffect_AdvanceResearch` | Acelera pesquisa atual |
| `UEffect_SpawnArmy` | Cria exército (rebelião, mercenários) |
| `UEffect_TransferProvince` | Muda dono de província |
| `UEffect_TriggerWar` | Declara guerra com casus belli automático |
| `UEffect_PassLaw` | Aprova lei sem capital político |
| `UEffect_SpawnCommander` | Adiciona general/almirante (ex: "Napoleão" surge) |
| `UEffect_GrantCard` | Concede `UBattleCardAsset` ao deck nacional |
| `UEffect_ChangePopAttribute` | Modifica POPs (literacia, militância, ideologia) |
| `UEffect_TriggerEvent` | Dispara outro evento imediatamente |
| `UEffect_ScheduleEvent` | Agenda follow-up |
| `UEffect_SetFlag` | Marca flag narrativo (queryable depois) |
| `UEffect_GrantBuilding` | Constrói prédio gratuito (ex: "Palácio de Cristal") |

### `FEventContext` — O DTO que circula entre tudo

```cpp
USTRUCT()
struct FEventContext
{
    GENERATED_BODY()

    UPROPERTY() UEventDefinitionAsset* Definition;
    UPROPERTY() FGuid InstanceId;                  // identidade do disparo

    // Escopo resolvido
    UPROPERTY() int32 PrimaryNationId;
    UPROPERTY() TArray<int32> SecondaryNationIds;
    UPROPERTY() int32 ProvinceId;
    UPROPERTY() FGuid CommanderId;
    UPROPERTY() FGuid PopAggregateId;

    // Dados do trigger original (variáveis substituíveis no texto)
    UPROPERTY() TMap<FName, FString> TextVariables;

    // Estado mundial mínimo
    UPROPERTY() UWorldState* World;

    // Resultado de escolha (preenchido após o jogador/IA decidir)
    UPROPERTY() int32 ChosenOptionIndex;
};
```

---

## 7. Scope Resolver

Eventos têm escopo definido (`Nation`, `Province`, `Pop`, `Commander`, `Global`), mas precisam ser **resolvidos** para alvos concretos.

```cpp
class UEventScopeResolver : public UObject
{
public:
    bool ResolveScope(UEventDefinitionAsset* Def, const FEventTriggerSignal& Signal,
                       FEventContext& OutCtx);
};
```

Exemplo: evento "Crise Industrial" tem escopo `Nation` e trigger `OnPriceShock`. O resolver pega o `Signal` (que veio do `EconomySubsystem` com `PriceShock.GoodId=Coal, NationId=12`) e preenche `Ctx.PrimaryNationId = 12`.

Para escopos `Global`, todas as nações que satisfaçam condições recebem **uma instância cada** do evento. Ou apenas uma instância "no mundo", dependendo de design.

---

## 8. Loop Principal do Subsistema

```cpp
void UEventSubsystem::OnTriggerSignal(FEventTriggerSignal Signal)
{
    TArray<UEventDefinitionAsset*> Candidates;
    TriggerIndex->QueryEventsForTrigger(Signal.Kind, Signal.Key, Candidates);

    for (UEventDefinitionAsset* Def : Candidates)
    {
        // Filtro de cooldown / política de repetição
        if (!CanFire(Def, Signal)) continue;

        // Resolve escopo (pode gerar múltiplos contextos para Global)
        TArray<FEventContext> Contexts;
        ScopeResolver->ResolveAll(Def, Signal, Contexts);

        for (FEventContext& Ctx : Contexts)
        {
            // Trigger específico precisa bater contexto
            bool TriggerMatches = false;
            for (UEventTrigger* T : Def->Triggers)
                if (T->MatchesContext(Ctx)) { TriggerMatches = true; break; }
            if (!TriggerMatches) continue;

            // Conditions
            if (!EvaluateConditions(Def->Conditions, Ctx)) continue;

            // Probabilidade
            float Prob = ComputeFinalProbability(Def, Ctx);
            if (!URandomSubsystem::Roll(Prob, Ctx.InstanceId)) continue;

            // Aceito — fila ou disparo direto
            DispatchEvent(Def, Ctx);
        }
    }
}
```

### Despacho conforme tipo

```cpp
void UEventSubsystem::DispatchEvent(UEventDefinitionAsset* Def, FEventContext Ctx)
{
    switch (Def->Type)
    {
    case EEventType::Silent:
    case EEventType::Background:
        ApplyImmediateEffects(Def, Ctx);
        RecordHistory(Def, Ctx);
        break;

    case EEventType::Notification:
        ApplyImmediateEffects(Def, Ctx);
        UI->ShowNotification(Def, Ctx);
        RecordHistory(Def, Ctx);
        break;

    case EEventType::Decision:
    case EEventType::Cinematic:
        EnqueueForPresentation(Def, Ctx);   // espera turno do jogador/IA
        break;
    }
}
```

---

## 9. Fila de Apresentação — Decision Events

Aqui mora a complexidade UX. Não pode haver:
- 5 popups empilhados sufocando o jogador
- evento crítico (declaração de guerra) atrás de "boas colheitas"
- IA pausando jogo do humano para decidir

### Regras

```cpp
class UEventPresentationQueue : public UObject
{
    TArray<FQueuedEvent> PlayerQueue;       // só eventos para a nação humana
    TArray<FQueuedEvent> AIDecisionQueue;   // resolvidos automaticamente

    void Enqueue(FQueuedEvent Q);
    void TickPresentation();                // avança fila se vazia
};
```

- Eventos para nações IA são resolvidos pelo `UEventAIResolver` **sem pausar o jogo** e sem fila visível.
- Eventos para a nação humana entram em `PlayerQueue`, ordenados por `Priority`.
- Apenas **um** Decision/Cinematic é apresentado por vez. Próximo abre quando atual fecha.
- `Cinematic` força pausa global. `Decision` pausa por padrão, mas configurável (alguns podem ter timer).
- Notifications acumulam num "log lateral" — não bloqueiam.

> ⚠️ **Cuidado de UX**: jogadores se sentem tiranizados por excesso de popups. Permita filtro por categoria nas configurações ("não me avise sobre X"). Histórico permite revisar depois.

---

## 10. Probabilidade e Pesos

```cpp
UCLASS(Abstract, EditInlineNew, Blueprintable)
class UEventWeightModifier : public UObject
{
public:
    virtual float ComputeMultiplier(const FEventContext& Ctx) const PURE_VIRTUAL(...);
};
```

Implementações:

| Classe | Efeito |
|---|---|
| `UWeight_IfAtWar` | × 2.0 se em guerra |
| `UWeight_IfMilitancyHigh` | escala com militância média |
| `UWeight_IfTechBehind` | × 1.5 se atrás na era |
| `UWeight_IfRecentEvent` | reduz se evento similar ocorreu há pouco |
| `UWeight_IfPlayerNation` | aumenta levemente para o jogador (mais conteúdo narrativo) |

```cpp
float UEventSubsystem::ComputeFinalProbability(UEventDefinitionAsset* Def,
                                                const FEventContext& Ctx)
{
    float P = Def->BaseProbability;
    for (UEventWeightModifier* W : Def->WeightModifiers)
        P *= W->ComputeMultiplier(Ctx);
    return FMath::Clamp(P, 0.0f, 1.0f);
}
```

> **Escalar pesos para o jogador** é truque honesto: a nação humana recebe mais variedade narrativa, sem mudar mecânica significativamente.

---

## 11. Política de Repetição

```cpp
UENUM()
enum class EEventRepeatPolicy : uint8
{
    FireOnce,             // jamais repete (ex: "Coroação da Rainha Vitória")
    FireOncePerNation,    // cada nação pode receber uma vez
    OnCooldown,           // pode repetir após CooldownDays
    AlwaysAvailable,      // sempre elegível (ex: rebeliões camponesas)
};
```

Persistido em `UNationEventState`:

```cpp
UCLASS()
class UNationEventState : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY() TSet<FName> FiredOnceEvents;
    UPROPERTY() TMap<FName, int64> CooldownUntilTick;
    UPROPERTY() TMap<FName, FEventChoiceMemory> ChoiceHistory;
    UPROPERTY() TArray<FScheduledEvent> ScheduledFollowUps;
};
```

`ChoiceHistory` é importante: condições podem consultar **"qual escolha foi feita no evento Y"**, permitindo narrativas que se lembram.

---

## 12. IA Decidindo Eventos

Para cada Decision Event apresentado a uma nação IA, `UEventAIResolver` escolhe entre opções por **utility scoring**.

```cpp
class UEventOptionAIWeight : public UObject
{
public:
    virtual float ScoreOption(const FEventContext& Ctx,
                              const FEventOption& Option) const PURE_VIRTUAL(...);
};
```

Implementações típicas:

| Classe | Score |
|---|---|
| `UAIWeight_PreferStability` | favorece opções que reduzem militância |
| `UAIWeight_PreferEconomy` | favorece opções com bônus econômico |
| `UAIWeight_PreferAggression` | favorece opções militares |
| `UAIWeight_FollowIdeology` | alinha com ideologia do governo |
| `UAIWeight_PersonalityTrait` | usa traços do líder |

> **Regra**: cada `FEventOption` tem **um** `UEventOptionAIWeight` ou cai num default. IA escolhe `argmax(Score(o))` com pequena temperatura softmax para variedade.

> ⚠️ **Performance da IA**: nunca simule consequências profundas (lookahead) em eventos de IA. São centenas por mês. Score direto.

---

## 13. Modificadores Persistentes — Hub de Estado

Muitos efeitos são "Adicionar modificador X por N dias". O subsistema mantém:

```cpp
UCLASS()
class UActiveModifierRegistry : public UObject
{
    UPROPERTY() TArray<FActiveModifier> AllActive;

    void Add(const FActiveModifier& Mod);
    void RemoveExpired(int64 CurrentTick);
    void RemoveBySource(FName SourceId);
    TArray<FActiveModifier> Query(EModifierTarget Target, int32 Scope) const;
};

USTRUCT()
struct FActiveModifier
{
    FName ModifierId;                  // ex: "Modifier.GoodHarvest"
    FName SourceEventId;
    int32 ScopeId;                     // nation/province/pop
    int64 ExpiresAtTick;               // 0 = permanente
    FStatModifierSet Modifiers;
    FText DisplayText;                 // para UI (tooltip)
};
```

Outros subsistemas **consultam** modificadores por target. Ex: `UEconomySubsystem` pergunta "quais modificadores afetam produção desta província?".

> **Vantagem**: a fonte de verdade dos modificadores fica num único lugar. Save é trivial. Auditoria de "por que minha produção caiu?" é trivial (lista os modificadores ativos).

---

## 14. Encadeamento e Cadeias Narrativas

`FScheduledEvent` é a unidade de agendamento:

```cpp
USTRUCT()
struct FScheduledEvent
{
    GENERATED_BODY()
    FName EventId;
    FEventContext PartialContext;      // pré-preenchido (alvo, variáveis)
    int64 FireAtTick;
    float ProbabilityAtFire;           // ainda pode falhar
};
```

Toda vez que um efeito agenda um follow-up, o registro vai para `ScheduledFollowUps` da nação relevante. O subsistema, no `OnDayPassed`, verifica `FireAtTick` e dispara via mesmo pipeline.

### Exemplo: Crise dos Bálcãs (cadeia simplificada)

```
Trigger inicial: 1908-10-05, Áustria-Hungria anexa Bósnia
  ├─ Evento "Anexação" (Cinematic, escolhe sérvios)
  │
  ├─ Schedule "Mobilização Sérvia" em 30..60 dias
  │   └─ Se "Apoio Russo" já fired: prob 0.9 / senão 0.3
  │
  ├─ Schedule "Ultimato Otomano" em 10..30 dias
  │   └─ Se Otomanos têm "ReformistGov": prob 0.7
  │
  └─ Set flag "BalkanCrisis_1908"
       └─ habilita pool de eventos secundários por 5 anos
```

> **Flags** são essenciais. Eles são marcadores narrativos consultáveis por triggers e conditions. Permite cadeias longas e ramificadas sem hardcoding.

---

## 15. Eventos como Pontes para Outros Sistemas

| Sistema | Como dispara eventos | Como reage a eventos |
|---|---|---|
| `UProgressSubsystem` | `OnTechResearched` | Eventos podem grant tech |
| `UEconomySubsystem` | `OnPriceShock`, `OnMarketCollapse`, `OnIndustryClosed` | Eventos modificam produção, dão bens |
| `UMilitarySubsystem` | `OnWarDeclared`, `OnBattleEnded`, `OnArmyDestroyed` | Eventos spawnam armies, modificam morale |
| `UDiplomacySubsystem` | `OnTreatyBroken`, `OnAllianceFormed` | Eventos mudam opiniões, justificam guerras |
| `UPoliticsSubsystem` | `OnElection`, `OnRevoltFired`, `OnLawPassed` | Eventos passam leis, mudam ideologia POP |
| `UBattleSubsystem` | `OnCommanderKilled`, `OnSiegeStarted` | Eventos concedem cartas, modificam moral |
| `UTimeSubsystem` | `OnDate`, `OnPeriodic` | Eventos avançam tempo, agendam follow-ups |

A dependência é circular **logicamente**, mas **fisicamente** todos passam pelo `UEventBusSubsystem`. Nenhum subsistema chama `EventSubsystem` diretamente — eles **emitem signals** que o EventSubsystem **escuta**.

```
UEconomySubsystem ──[OnPriceShock]──► UEventBusSubsystem ──► UEventSubsystem
                                                                  │
                                                                  └─[Effect_AddModifier]──► UActiveModifierRegistry
                                                                                                  │
                                                                                                  └─consulta─► UEconomySubsystem
```

Essa indireção é **o que torna o sistema robusto**.

---

## 16. Determinismo, Save e Replay

Todo evento que dispara recebe um `InstanceId` derivado de:

```
Hash(EventId, PrimaryNationId, FireTick, GlobalSeed)
```

Isso permite:
- Rolls de RNG determinísticos (`URandomSubsystem::Roll(prob, instanceId)`)
- Replay idêntico do mesmo save
- Debug: jogador pode reportar `InstanceId` e dev reproduz

### O que vai no save

```cpp
USTRUCT()
struct FEventSubsystemSaveData
{
    TArray<FFiredEventRecord> History;            // lista compacta de eventos passados
    TMap<int32, UNationEventState*> ByNation;     // estado por nação
    TArray<FActiveModifier> ActiveModifiers;
    TArray<FScheduledEvent> GlobalScheduled;
    TSet<FName> GlobalFlags;
};
```

> ⚠️ **History pode crescer demais**. Em saves longos (200 anos in-game), descartar Notifications/Silent antigos. Manter Decision/Cinematic para narrativa.

---

## 17. Performance — Pontos Críticos

- **Indexação**: o índice de triggers reduz avaliação de O(N×M) para O(K) onde K é o tamanho do bucket. **Sem isso o sistema não escala.**
- **Avaliação preguiçosa**: condições só são avaliadas após o trigger bater. Não rode `IsSatisfied()` em loop global.
- **`OnPeriodic` deve ser raro**. Cada periodic event roda em todos os ticks da granularidade definida. 5 periodic events × 50 nações × 365 dias = 91k avaliações/ano. Limite!
- **Cache `ScopeResolver`**: resolução de escopo "todas as nações em guerra" muda raramente. Cache por trigger window.
- **History em estrutura compacta**: `FFiredEventRecord` deve ser pequeno (FName + tick + nationId + chosenOption). Não armazenar texto.
- **Modifier expiry**: ordene por `ExpiresAtTick` em heap; remova só quando o tick passar. Não escaneie todos a cada tick.

> Meta: subsistema deve consumir **<5% do tempo de tick total** mesmo com 200 eventos definidos e 50 nações.

---

## 18. Eventos Vitorianos Exemplares

Para concretizar, alguns eventos típicos:

| EventId | Tipo | Trigger | Efeito principal |
|---|---|---|---|
| `Event.GreatExhibition.1851` | Cinematic | OnDate | +Prestige UK, libera ChainLink |
| `Event.SpringOfNations.1848` | Cinematic Global | OnDate | +Militancy em monarquias |
| `Event.OpiumWar` | Decision | OnTrigger entre UK/China | Justifica guerra ou recua |
| `Event.IndustrialAccident` | Decision | OnPeriodic + UCond_HasIndustry | Reforma trabalhista vs lucro |
| `Event.PotatoFamine` | Cinematic | OnDate + UCond_HasProvince | Migração massiva, militância |
| `Event.NewspaperFlourishes` | Notification | OnTechResearched(Press) | +Awareness POPs |
| `Event.ColonialUprising` | Decision | OnPopMilitancy + colônia | Reprimir vs negociar |
| `Event.GreatComposer` | Notification | OnPeriodic | +Prestige + carta cultural |
| `Event.PrinceMarried` | Decision | OnPeriodic + monarquia | Aliança dinástica |
| `Event.SocialistAgitator` | Decision | OnPriceShock alimento | Reprimir vs reformar |
| `Event.GreatGeneralBorn` | Background | OnDate per nation | Spawn comandante com traço |

---

## 19. Diagrama Final

```
UEventSubsystem (UWorldSubsystem)
│
├── DataAssets (estático)
│   ├── UEventDefinitionAsset[]
│   │   ├── Type / Scope / Category
│   │   ├── UEventTrigger[]      [OnDate, OnTechResearched, OnWar, OnPriceShock...]
│   │   ├── UEventCondition[]    [HasTech, AtWar, OwnsProvince, AND/OR/NOT...]
│   │   ├── UEventWeightModifier[] [scaling de probabilidade]
│   │   ├── FEventOption[]
│   │   │   ├── AvailabilityConditions
│   │   │   ├── UEventEffect[]
│   │   │   └── UEventOptionAIWeight
│   │   ├── ImmediateEffects
│   │   ├── ChainedEvents (FEventChainLink)
│   │   ├── RepeatPolicy + CooldownDays
│   │   └── Tags + Priority
│
├── Estado runtime
│   ├── UEventTriggerIndex       [O(1) dispatch por kind]
│   ├── UEventScopeResolver
│   ├── UEventPresentationQueue  [Player vs AI]
│   ├── UEventAIResolver         [utility scoring]
│   ├── UActiveModifierRegistry  [hub de modificadores]
│   ├── EventHistory             [registro compacto]
│   └── UNationEventState[]      [fired-once, cooldown, choice memory, scheduled]
│
├── Loop
│   ├── OnTriggerSignal → Query → ResolveScope → Conditions → Probability → Dispatch
│   ├── OnDayPassed → FireScheduled, ExpireModifiers, PeriodicEvents
│   └── OnPlayerChoice / OnAIChoice → ApplyOptionEffects → ChainNext
│
└── Pontes
    ├── ↔ EventBusSubsystem      (escuta signals de outros)
    ├── ↔ UProgressSubsystem     (techs disparam eventos / eventos concedem techs)
    ├── ↔ UEconomySubsystem      (price shocks / modificadores)
    ├── ↔ UMilitarySubsystem     (war, batalhas / spawns)
    ├── ↔ UDiplomacySubsystem    (treaties broken / opinion shifts)
    ├── ↔ UPoliticsSubsystem     (elections, revolts / law passing)
    ├── ↔ UBattleSubsystem       (commander events / battle outcomes)
    └── ↔ UTimeSubsystem         (date / periodic)
```

---

## 20. Plano de Implementação

1. **Esqueleto do subsistema** + `UEventDefinitionAsset` mínimo (Title, Description, ImmediateEffects). Sem triggers ainda.
2. **API de disparo manual** (`FireEvent(EventId, Context)`) — útil para debug e cinematics scripts.
3. **3 efeitos básicos**: `ChangeTreasury`, `AddModifier`, `ChangePrestige`.
4. **`UActiveModifierRegistry`** + integração com Economia (lê modificadores).
5. **`UEventBusSubsystem`** + 1 trigger: `OnTechResearched`. Conecta ao Progress.
6. **Indexação** (`UEventTriggerIndex`) + dispatch O(1).
7. **`UEventCondition`** com 5 implementações + composição lógica.
8. **Decision Events** + UI básica (popup, opções).
9. **`UEventPresentationQueue`** com prioridade.
10. **AI Resolver** com `UEventOptionAIWeight` simples.
11. **`UEventWeightModifier`** + probabilidade dinâmica.
12. **Encadeamento** (`FEventChainLink`, `FScheduledEvent`).
13. **Flags narrativos** (`SetFlag`, `Cond_HasFlag`).
14. **Repeat policies** + `UNationEventState` persistido.
15. **Triggers adicionais** (OnDate, OnWarDeclared, OnPriceShock, OnPopMilitancy, OnPeriodic).
16. **Cinematics** com pausa global e UI especial.
17. **History/log lateral** + filtros UI.
18. **Pacote inicial de 30 eventos vitorianos** para validar o pipeline.
19. **Determinismo** (`InstanceId`, RNG por evento).
20. **Polish**: sons, ilustrações, animações.

---

## 21. Pontos de Atenção Específicos

- **Eventos são conteúdo**. A arquitetura aguenta 1000 eventos. Mas **escrever** 1000 eventos balanceados é o trabalho. Comece com 30 bons.
- **Não use eventos como cola de bug**. Se um sistema precisa de evento toda vez que algo acontece, talvez o sistema esteja mal modelado.
- **Eventos devem ter consequência sentida**. Popup que dá +5 ouro é tédio. Popup que muda o jogo é narrativa.
- **Preserve agência**. Se 80% das opções são "ok" sem peso, o jogador desliga. Cada opção precisa ter trade-off.
- **Localização**: textos vivem em `FText` ligado a tabelas. **Nunca hardcode strings** em `UEventDefinitionAsset`.
- **Mods**: eventos são o ponto natural de modding. Exponha o `UEventDefinitionAsset` como criável em diretório de mod, e o registry carregue dinamicamente.
- **Tracking analítico**: meça quais eventos disparam quanto, quais opções são escolhidas. Direciona balanceamento e novas cadeias.
- **Não confie no jogador para ler**. Eventos com 8 parágrafos são pulados. Title curto + Description em 2-3 frases + tooltip detalhado.
- **Visual identity**: cada categoria deve ter look distinto (cor, ícone, som). Decision Político ≠ Decision Militar visualmente.
