# 40 — UAIDirectorSubsystem

Subsistema mais difícil de projetar porque é o único que **não tem responsabilidade própria** — ele orquestra todos os outros. Se mal feito, vira mil heurísticas espalhadas que se contradizem. Se bem feito, é uma "consciência nacional" que dá personalidade a cada IA.

Arquitetura: **Diretor + Conselheiros + Brain** que escala para dezenas de nações sem custar performance.

---

## 1. Princípios Arquiteturais

### IA Nacional ≠ IA Tática

Já temos:
- `UBattleAIController` (decisões dentro de uma batalha)
- `UPoliticalAIResolver` (reformas, repressão)
- `UDiplomaticAIResolver` (alianças, guerras)
- IA de capitalistas (expansão de indústria)

O `UAIDirectorSubsystem` **não substitui** esses. Ele os **coordena**. Cada um sabe escolher dentro do seu domínio; o Diretor decide **qual domínio priorizar agora** e **com que objetivo geral**.

### O Director pensa em Goals, não em Actions

Erro: Director decide "construa fábrica X". Isso usurpa o trabalho do `UEconomySubsystem`. Acoplamento explode.

Solução: Director define **objetivos** ("Industrializar país", "Preparar para guerra com França"). Os resolvers especializados executam **como** atingir.

```
Director: "Goal: IndustrializeRegion(NorthernProvinces, Priority=High)"
   │
   ├──► EconomyResolver: decide quais indústrias e quando
   ├──► PoliticsResolver: pressiona por leis pró-indústria
   ├──► DiplomacyResolver: busca tratados comerciais
   └──► ProgressResolver: prioriza tech industrial
```

### Personalidade > Otimização

Em grand strategy, IA "perfeita" é tediosa e previsível. IA com **personalidade** é memorável. Bismarck calculista, Napoleão III aventureiro, Tsar reacionário. A arquitetura precisa expor **knobs de personalidade** em todos os níveis de decisão.

### Performance — IA Nacional é cara

Com 50 nações × decisões em economia + política + diplomacia + militar + tech = milhares de avaliações por mês. Soluções:
- **Schedule rotativo**: nem toda nação pensa no mesmo tick
- **Cache de avaliação**: reuso até estado mudar
- **Hierarquia de profundidade**: Grandes Potências pensam mais, Minor Powers menos
- **Async TaskGraph** para decisões pesadas

---

## 2. Estrutura Hierárquica

```
UAIDirectorSubsystem (UWorldSubsystem)
   │
   ├── NationBrains[]              [um UNationBrain por nação IA]
   │     │
   │     ├── UNationStrategy        [estado de longo prazo: goals, doctrine]
   │     ├── UNationPersonality     [knobs de comportamento, vindos do líder]
   │     ├── UNationContextCache    [snapshot do mundo do ponto de vista da nação]
   │     │
   │     └── Advisors[]             [um por domínio]
   │           ├── UEconomicAdvisor
   │           ├── UPoliticalAdvisor
   │           ├── UDiplomaticAdvisor
   │           ├── UMilitaryAdvisor
   │           ├── UScienceAdvisor
   │           └── UColonialAdvisor
   │
   ├── DirectorScheduler           [decide qual nação pensa quando]
   ├── GlobalIntelligence          [estado mundial agregado para consultas]
   └── DirectorTelemetry           [métricas para tuning e debug]
```

---

## 3. `UNationBrain` — Consciência da Nação

```cpp
UCLASS()
class UNationBrain : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY() int32 NationId;
    UPROPERTY() UNationStrategy* Strategy;
    UPROPERTY() UNationPersonality* Personality;
    UPROPERTY() UNationContextCache* Context;
    UPROPERTY() TArray<UAdvisor*> Advisors;

    // Loop principal
    void Reevaluate();
    void ExecuteScheduledActions();

    // Para outros sistemas pedirem opinião
    EReaction EvaluateExternalEvent(const FEventContext& Ctx);
    float ScoreActionDesirability(const FProposedAction& Action);
};
```

### Loop principal (`Reevaluate`)

```cpp
void UNationBrain::Reevaluate()
{
    // 1. Atualiza percepção do mundo
    Context->Refresh();

    // 2. Reavalia objetivos de longo prazo (a cada N meses)
    if (ShouldReevaluateStrategy())
        Strategy->Recompute(Context, Personality);

    // 3. Cada Advisor propõe ações dentro de seu domínio
    TArray<FAdvisorProposal> AllProposals;
    for (UAdvisor* A : Advisors)
        AllProposals.Append(A->ProposeActions(Strategy, Context));

    // 4. Director ranqueia globalmente (orçamento de atenção)
    TArray<FAdvisorProposal> Selected = SelectTopProposals(AllProposals);

    // 5. Despacha cada ação para o subsistema responsável
    for (const FAdvisorProposal& P : Selected)
        DispatchToSystem(P);

    // 6. Telemetria
    Telemetry->LogDecisionCycle(NationId, AllProposals, Selected);
}
```

> **Importante**: o Brain não executa ações diretamente. Ele dispara **comandos** para os subsistemas reais (`UEconomySubsystem::QueueIndustryConstruction`, `UDiplomacySubsystem::ProposeAlliance` etc). Mantém o desacoplamento que toda a arquitetura persegue.

---

## 4. `UNationStrategy` — Objetivos de Longo Prazo

```cpp
UCLASS()
class UNationStrategy : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY() ENationalDoctrine MainDoctrine;       // ver abaixo
    UPROPERTY() TArray<UStrategicGoal*> ActiveGoals;
    UPROPERTY() TMap<int32, ERelationshipPosture> NationPostures;
                                                        // Friend, Rival, Threat, Target,
                                                        // Indifferent, Vassal, Ally
    UPROPERTY() TArray<FName> StrategicResources;     // bens críticos para política
    UPROPERTY() FStrategicMap StrategicMap;           // áreas de interesse geográfico

    void Recompute(UNationContextCache* Ctx, UNationPersonality* Pers);
};
```

### Doutrina Nacional — O "Tema" da IA

```cpp
UENUM()
enum class ENationalDoctrine : uint8
{
    Industrializer,         // foco em economia, tech industrial
    Conqueror,              // expansão militar agressiva
    Diplomat,               // alianças, esferas, prestígio
    Colonialist,            // expansão ultramarina
    Reformer,               // modernização política
    Reactionary,            // preservar ordem antiga
    Isolationist,           // foco interno, evitar conflitos
    Defender,               // status quo, proteger fronteiras
    Revolutionary,          // exportar revolução
};
```

> Doutrina é definida inicialmente pelo cenário (ex: Reino Unido = Colonialist+Industrializer; Áustria = Reactionary+Defender) e **pode mudar** conforme eventos, líderes, crises.

### Goals Estratégicos

```cpp
UCLASS(Abstract, EditInlineNew)
class UStrategicGoal : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY() FName GoalId;
    UPROPERTY() EGoalPriority Priority;            // Critical, High, Medium, Low
    UPROPERTY() float Progress;                    // 0..1
    UPROPERTY() int32 EstablishedAtTick;
    UPROPERTY() int32 DeadlineAtTick;              // 0 = sem prazo

    virtual bool IsAchieved(const FNationContext& Ctx) const PURE_VIRTUAL(...);
    virtual bool IsObsolete(const FNationContext& Ctx) const { return false; }
    virtual TArray<FAdvisorHint> EmitHints() const PURE_VIRTUAL(...);
};
```

Implementações típicas:

| Classe | Significado |
|---|---|
| `UGoal_AnnexProvince` | Tomar província específica |
| `UGoal_ReachEra` | Avançar para era X |
| `UGoal_BeGreatPower` | Entrar no top-8 prestigio |
| `UGoal_DestroyRival` | Eliminar rival como ameaça |
| `UGoal_FormFederation` | Unificar irmãos culturais |
| `UGoal_ColonizeRegion` | Estabelecer presença em região |
| `UGoal_AvoidWar` | Período de paz forçada |
| `UGoal_PassReform` | Aprovar lei específica |
| `UGoal_StabilizeRegion` | Reduzir militância nacional |
| `UGoal_BreakSphere` | Tirar nação Y da esfera de Z |
| `UGoal_FormAlliance` | Concretizar aliança específica |

### Hints — Como Goals influenciam Advisors

```cpp
USTRUCT()
struct FAdvisorHint
{
    EAdvisorDomain Domain;       // Economy, Politics, Military...
    FName HintType;              // "PrioritizeIndustry", "MobilizeArmy", "FundFaction"
    FName TargetId;              // alvo opcional
    float Weight;                // intensidade
};
```

Exemplo: `UGoal_DestroyRival(Target=France)` emite:
- `(Military, "ExpandArmy", France, 0.9)`
- `(Diplomacy, "EncircleNation", France, 0.8)`
- `(Economy, "ProduceWeapons", _, 0.7)`
- `(Politics, "PassConscription", _, 0.6)`

Cada Advisor recebe os hints relevantes e os usa como **modificadores de utility**.

---

## 5. `UNationPersonality` — Personalidade Vinda do Líder

```cpp
UCLASS()
class UNationPersonality : public UObject
{
    GENERATED_BODY()
public:
    // Vinda de UNationLeaderProfile + UNationDataAsset (cultura) + ideologia governamental
    UPROPERTY() FPersonalityProfile Profile;

    void Refresh(const UNation* N);
    float ApplyToScore(EAdvisorDomain Domain, FName ActionType, float BaseScore) const;
};

USTRUCT()
struct FPersonalityProfile
{
    // Eixos primários (0..1)
    float Aggression;
    float Honor;
    float Pragmatism;
    float Patience;
    float Greed;
    float Pride;
    float Vindictiveness;

    // Inclinações temáticas
    float MilitaryFocus;
    float EconomicFocus;
    float DiplomaticFocus;
    float TechnologicalFocus;
    float CulturalFocus;

    // Aversões e atrações
    TMap<FName, float> IdeologyAffinity;
    TMap<int32, float> NationAffinity;       // pré-disposições
    TMap<FName, float> ActionAversions;      // -1..0 para ações que evita
};
```

### Como personalidade modula decisões

```cpp
float UNationPersonality::ApplyToScore(EAdvisorDomain Domain, FName ActionType,
                                        float BaseScore) const
{
    float Mod = 1.0f;

    if (Domain == EAdvisorDomain::Military)
        Mod *= (0.5f + Profile.MilitaryFocus);
    if (Domain == EAdvisorDomain::Economy)
        Mod *= (0.5f + Profile.EconomicFocus);

    if (ActionType == "DeclareWar") Mod *= (0.3f + Profile.Aggression * 1.4f);
    if (ActionType == "BreakTreaty") Mod *= (1.5f - Profile.Honor);
    if (ActionType == "FabricateCB") Mod *= (1.0f + Profile.Pragmatism - Profile.Honor);
    if (ActionType == "GrantConcession") Mod *= (0.5f + Profile.Pragmatism);

    return BaseScore * Mod;
}
```

> Personalidade é **um filtro multiplicativo**. Não decide sozinha, mas distorce o ranking de propostas. Bismarck e Napoleão III recebem o **mesmo conjunto de propostas** dos Advisors, mas escolhem diferente.

---

## 6. `UNationContextCache` — Percepção do Mundo

A IA não consulta o mundo real a cada decisão. Ela mantém um **snapshot** atualizado periodicamente.

```cpp
UCLASS()
class UNationContextCache : public UObject
{
    GENERATED_BODY()
public:
    // Auto-percepção
    UPROPERTY() FNationStrengthProfile MyStrength;
    UPROPERTY() FNationFinancialState MyFinances;
    UPROPERTY() FNationStability MyStability;
    UPROPERTY() FNationMilitaryReadiness MyMilitary;
    UPROPERTY() FTechProgressSnapshot MyTech;

    // Percepção dos outros (com fog of war respeitado)
    UPROPERTY() TMap<int32, FPerceivedNation> KnownNations;

    // Ranking estratégico
    UPROPERTY() TArray<int32> PerceivedThreats;        // ordenado por ameaça
    UPROPERTY() TArray<int32> PerceivedRivals;
    UPROPERTY() TArray<int32> PotentialAllies;
    UPROPERTY() TArray<int32> WeakNeighbors;

    // Oportunidades
    UPROPERTY() TArray<FOpportunity> Opportunities;

    int64 LastRefreshTick;
    void Refresh();
};

USTRUCT()
struct FPerceivedNation
{
    int32 NationId;
    float EstimatedMilitaryStrength;     // estimativa, não exato
    float EstimatedEconomicPower;
    EGreatPowerStatus PerceivedStatus;
    ENationalDoctrine PerceivedDoctrine;  // adivinhada por comportamento
    float ThreatLevel;                    // 0..1
    float Affinity;                       // -1..1
    int32 LastObservedTick;
    int32 ConfidenceLevel;                // 0..100
};
```

### Refresh — Quando atualiza

```cpp
void UNationContextCache::Refresh()
{
    if (CurrentTick - LastRefreshTick < MinRefreshInterval) return;

    UpdateSelfAssessment();
    UpdatePerceivedNations();
    RecomputeThreats();
    RecomputeOpportunities();

    LastRefreshTick = CurrentTick;
}
```

> Refresh é **caro**. Roda a cada 30-60 dias simulados, ou quando evento crítico força atualização (`OnWarDeclared`, `OnTreatyBroken`).

### Cálculo de ameaça

```cpp
float ComputeThreatLevel(int32 OtherNation) const
{
    const FPerceivedNation& Other = KnownNations[OtherNation];

    float Threat = 0;

    // Capacidade militar relativa
    Threat += FMath::Max(0.0f,
        Other.EstimatedMilitaryStrength / MyStrength.Military - 0.7f);

    // Adjacência aumenta
    if (SharesBorder(MyNationId, OtherNation)) Threat += 0.3f;

    // Doutrina hostil
    if (Other.PerceivedDoctrine == ENationalDoctrine::Conqueror)
        Threat += 0.2f;
    if (Other.PerceivedDoctrine == ENationalDoctrine::Isolationist)
        Threat -= 0.2f;

    // Histórico
    if (HasFoughtRecently(MyNationId, OtherNation)) Threat += 0.3f;

    // Reivindicação territorial conhecida
    if (HasClaimAgainstUs(OtherNation)) Threat += 0.4f;

    // Esfera oposta
    if (IsInRivalSphere(OtherNation)) Threat += 0.2f;

    return FMath::Clamp(Threat, 0.0f, 1.0f);
}
```

---

## 7. Advisors — Especialistas por Domínio

```cpp
UCLASS(Abstract)
class UAdvisor : public UObject
{
    GENERATED_BODY()
public:
    virtual EAdvisorDomain GetDomain() const PURE_VIRTUAL(...);

    // Recebe goals (via hints) e contexto, propõe ações ranqueadas
    virtual TArray<FAdvisorProposal> ProposeActions(UNationStrategy* Strat,
                                                    UNationContextCache* Ctx)
                                                    PURE_VIRTUAL(...);

    // Reage a evento externo (ex: outra nação declarou guerra)
    virtual EReaction Reactive(const FEventContext& Event) { return EReaction::Neutral; }
};

USTRUCT()
struct FAdvisorProposal
{
    EAdvisorDomain Domain;
    FName ActionType;
    FName TargetId;                // pode estar vazio
    int32 EstimatedCost;           // PC, ouro, tempo
    float Utility;                 // 0..1, prioridade computada
    FString DebugReason;           // para telemetria
    TFunction<void()> Execute;     // closure para despachar
};
```

### `UEconomicAdvisor`

Decide:
- Quais indústrias construir (ouvindo capitalist AI + hints estratégicos)
- Como ajustar tarifas
- Onde investir em infraestrutura
- Quando contratar empréstimos
- Quando reduzir gastos

```cpp
TArray<FAdvisorProposal> UEconomicAdvisor::ProposeActions(...)
{
    TArray<FAdvisorProposal> Out;

    // Sinal: indústria militar deficiente?
    float ArmsProduction = Ctx->MyFinances.GetSectorOutput(Military);
    float ArmsDemand = Strat->GetEstimatedMilitaryDemand();
    if (ArmsDemand > ArmsProduction * 1.2f)
        Out.Add(MakeProposal("BuildArsenal", _, Utility=0.8f));

    // Sinal: tesouro baixo?
    if (Ctx->MyFinances.Treasury < 100)
    {
        Out.Add(MakeProposal("RaiseTax", _, 0.6f));
        Out.Add(MakeProposal("TakeLoan", _, 0.4f));
    }

    // Hints: Goal pede industrialização
    for (const FAdvisorHint& H : GetMyHints(Strat))
    {
        if (H.HintType == "PrioritizeIndustry")
            Out.Add(MakeProposal("BuildFactory", _, 0.7f * H.Weight));
    }

    return Out;
}
```

### `UPoliticalAdvisor`
Decide: qual reforma propor; quando reprimir vs conceder; como gerenciar facções; quando convocar eleição; atender demandas de quais facções.

### `UDiplomaticAdvisor`
Decide: com quem alinhar; quando fabricar CB; quando declarar guerra; quando propor paz; onde expandir esfera.

### `UMilitaryAdvisor`
Decide: tamanho do exército; composição (infantaria vs cavalaria vs artilharia); modernização de equipamento; posicionamento estratégico (frentes, fortificações); quando mobilizar.

### `UScienceAdvisor`
Decide: próxima tech a pesquisar; aceitar tech transfer ou recusar; investir em educação; espionar tech alheia.

### `UColonialAdvisor`
(Para nações com `EGreatPowerStatus::ColonialPower` ou disposição colonial.) Decide: onde estabelecer colônias; quando explorar regiões; como administrar territórios; quando reprimir vs negociar com nativos.

---

## 8. Director — Ranqueamento Global

Os Advisors propõem **muito mais ações** do que a nação pode executar. O Director funciona como o cérebro executivo: filtra, prioriza, despacha.

```cpp
TArray<FAdvisorProposal> UNationBrain::SelectTopProposals(
    TArray<FAdvisorProposal>& All)
{
    // 1. Aplica personalidade
    for (FAdvisorProposal& P : All)
        P.Utility = Personality->ApplyToScore(P.Domain, P.ActionType, P.Utility);

    // 2. Aplica modificadores de strategy (doutrina nacional)
    for (FAdvisorProposal& P : All)
        P.Utility *= Strategy->GetDoctrineMultiplier(P.Domain, P.ActionType);

    // 3. Penaliza ações redundantes ou bloqueadas
    FilterImpossible(All);
    PenalizeOverlap(All);

    // 4. Ordena
    All.Sort([](const FAdvisorProposal& A, const FAdvisorProposal& B) {
        return A.Utility > B.Utility;
    });

    // 5. Aplica budget de ações por ciclo
    int32 ActionBudget = ComputeActionBudget();  // varia por status, era, crise
    return All.Slice(0, FMath::Min(ActionBudget, All.Num()));
}
```

### Action Budget — Limita "atenção" da nação

```cpp
int32 ComputeActionBudget() const
{
    int32 Base = 3;
    if (Strategy->IsInCrisis()) Base += 2;
    if (MyStatus == GreatPower) Base += 2;
    if (MyStability < 0.4f) Base -= 1;
    return FMath::Clamp(Base, 1, 8);
}
```

> Limitar atenção é **realismo + performance**. Nação não age em 50 frentes simultaneamente. E Director processa só N ações por ciclo, não ranqueamento exaustivo.

---

## 9. `DirectorScheduler` — Quem Pensa Quando

Não dá para reavaliar 50 nações a cada tick. O Scheduler distribui o pensamento.

```cpp
class UDirectorScheduler
{
public:
    void Tick();
private:
    TArray<int32> NationIds;           // todas as nações IA
    int32 NextIndex;                   // round-robin
    TMap<int32, int32> PriorityScores; // nações em crise pulam fila
};

void UDirectorScheduler::Tick()
{
    int32 NationsPerTick = ComputeBudget();  // ex: 3 nações por dia simulado

    // Crises pulam à frente
    TArray<int32> Urgent = GetUrgentNations();
    for (int32 N : Urgent) ProcessNation(N);

    // Round-robin para o resto
    for (int32 i = 0; i < NationsPerTick; ++i)
    {
        int32 N = NationIds[NextIndex];
        NextIndex = (NextIndex + 1) % NationIds.Num();
        if (Urgent.Contains(N)) continue;  // já processou
        ProcessNation(N);
    }
}
```

### Eventos forçam reavaliação

```cpp
void UDirectorScheduler::OnExternalEvent(const FEventContext& Ev)
{
    // Eventos críticos forçam reavaliação imediata
    if (Ev.Definition->Tags.HasTag("Critical"))
    {
        for (int32 NId : Ev.SecondaryNationIds)
            ScheduleImmediate(NId);
        ScheduleImmediate(Ev.PrimaryNationId);
    }
}
```

### Tiers de complexidade

Para escalar, **não toda nação merece o mesmo cérebro**:

| Tier | Quem | Reavaliação |
|---|---|---|
| Full | Grandes Potências (8) | Mensal completa, todos Advisors |
| Standard | Potências Secundárias (16) | Bimestral, Advisors prioritários |
| Light | Minor Powers | Trimestral, heurísticas simples |
| Minimal | Tribos/Não-civilizadas | Semestral, decisões reativas |

---

## 10. `GlobalIntelligence` — Cache Mundial

Algumas consultas são caras e compartilhadas entre todas as IAs:
- "Qual o ranking mundial de tecnologia?"
- "Quais nações estão em guerra?"
- "Quem são os maiores produtores de aço?"

```cpp
UCLASS()
class UGlobalIntelligence : public UObject
{
public:
    void RefreshGlobalSnapshot();          // mensal

    TArray<int32> GetRankedByPrestige() const;
    TArray<int32> GetRankedByMilitary() const;
    TArray<int32> GetGreatPowers() const;
    TArray<int32> GetNationsAtWar() const;
    float GetAverageEra() const;
    TArray<FName> GetTopProducedGoods(int32 NationId) const;
};
```

**Computado uma vez por mês**, consultado por todas as IAs sem custo.

---

## 11. Integração com EventSubsystem

O Director **escuta extensivamente** o EventBus para reagir a mudanças relevantes.

### Eventos que disparam reavaliação

| Evento | Ação |
|---|---|
| `OnWarDeclared` | Ambas as nações reavaliam imediatamente |
| `OnTreatyBroken` | Reavaliação para honra/trust |
| `OnTechBreakthrough` (rival) | ScienceAdvisor reage |
| `OnNationDeclaredPariah` | DiplomaticAdvisor considera coalizão |
| `OnRevoltStarted` (vizinho) | Oportunidade ou ameaça |
| `OnGreatPowerStatusChanged` | Reranking de PerceivedNations |
| `OnLeaderChanged` | Personality refresh |
| `OnPriceShock` | EconomicAdvisor considera ação |

### Reação a eventos como Advisors

Quando um evento chega para uma nação IA (Decision Event), o `UEventAIResolver` consulta **o Brain** para escolher opção:

```cpp
int32 UEventAIResolver::ChooseOption(int32 NationId, UEventDefinitionAsset* Def,
                                      FEventContext& Ctx)
{
    UNationBrain* Brain = Director->GetBrain(NationId);

    TArray<float> Scores;
    for (int32 i = 0; i < Def->Options.Num(); ++i)
    {
        const FEventOption& Opt = Def->Options[i];
        float Score = Opt.AIWeight->ScoreOption(Ctx, Opt);
        Score = Brain->Personality->ApplyToScore(
            Domain, "EventChoice", Score);
        Score = Brain->Strategy->ApplyDoctrineModifier(Score, Opt);
        Scores.Add(Score);
    }

    return SelectWithSoftmax(Scores, Brain->Personality->Profile.Decisiveness);
}
```

> O Director é **consultor** do EventResolver, não substituto.

---

## 12. Diplomacia entre IAs — O Caso Especial

Quando duas IAs negociam, há risco de loop ou comportamento incoerente. Solução:

```cpp
class UDiplomaticAIResolver
{
    bool EvaluateOfferFromOther(int32 ReceiverNation, int32 ProposerNation,
                                 const FDiplomaticOffer& Offer);
};

bool UDiplomaticAIResolver::EvaluateOfferFromOther(...)
{
    UNationBrain* Brain = Director->GetBrain(ReceiverNation);

    // 1. Score baseado em Strategy alignment
    float Score = Brain->Strategy->EvaluateOfferAlignment(Offer);

    // 2. Modulado por percepção do proponente
    const FPerceivedNation& Prop = Brain->Context->KnownNations[ProposerNation];
    Score *= (1.0f + Prop.Affinity);
    Score -= Prop.ThreatLevel * 0.3f;

    // 3. Personalidade
    Score = Brain->Personality->ApplyToScore(EAdvisorDomain::Diplomacy,
                                              "AcceptOffer", Score);

    // 4. Trust (não opinion!) define tolerância a riscos
    float TrustWeight = ComputeTrustFactor(ReceiverNation, ProposerNation);

    return Score * TrustWeight > AcceptanceThreshold;
}
```

> **Importante**: jamais permita loops infinitos de "IA A propõe → IA B contra-propõe → A propõe novamente". Negociação tem **rounds limitados**, e ofertas rejeitadas entram em **cooldown**.

---

## 13. Telemetria e Debug — Crítico para Tuning

IA é o sistema mais difícil de debugar porque "está errada" é subjetivo. Sem ferramentas, impossível ajustar.

```cpp
UCLASS()
class UDirectorTelemetry : public UObject
{
public:
    void LogDecisionCycle(int32 NationId,
                           const TArray<FAdvisorProposal>& AllProposed,
                           const TArray<FAdvisorProposal>& Selected);

    // Para painel de debug em runtime
    FAIDebugSnapshot CaptureSnapshot(int32 NationId) const;

    // Para análise pós-jogo
    void ExportSession(const FString& Path) const;
};

USTRUCT()
struct FAIDebugSnapshot
{
    int32 NationId;
    ENationalDoctrine CurrentDoctrine;
    TArray<UStrategicGoal*> ActiveGoals;
    TArray<FString> RecentDecisions;
    TArray<FRankedThreat> PerceivedThreats;
    float DecisionCycleMS;            // performance
    int32 ProposalsConsidered;
    int32 ProposalsExecuted;
};
```

### Painel de Debug em Runtime

Uma janela ImGui (em build de desenvolvimento) mostra para qualquer nação selecionada:
- Doutrina ativa
- Goals atuais e progresso
- Top 5 ameaças percebidas
- Últimas 20 decisões com `DebugReason`
- Heatmap de utility por ação

> Sem isso, você passa meses calibrando IA no escuro.

---

## 14. Personalidade Adaptativa

Personalidade não precisa ser estática. Pode reagir ao mundo:

```cpp
void UNationPersonality::OnEvent(const FEventContext& Ctx)
{
    if (Ctx.Definition->Tags.HasTag("Humiliation"))
        Profile.Vindictiveness += 0.05f;
    if (Ctx.Definition->Tags.HasTag("Betrayal"))
        Profile.Honor *= 0.95f;  // perde fé
    if (Ctx.Definition->Tags.HasTag("DiplomaticTriumph"))
        Profile.Pride += 0.03f;
}
```

> Personalidade adaptativa cria **arcos narrativos**: nação humilhada vira vingativa, nação traída vira cínica. Storytelling emergente.

### Mudança de Líder

```cpp
void UNationPersonality::OnLeaderChange(UNationLeaderProfile* NewLeader)
{
    Profile.Aggression = Lerp(Profile.Aggression, NewLeader->BaseAggression, 0.7f);
    Profile.Honor = Lerp(Profile.Honor, NewLeader->BaseHonor, 0.7f);
    // ... etc
}
```

> Mudança de líder é **descontinuidade marcante** — cria pontos de inflexão na campanha.

---

## 15. Performance — Pontos Críticos

- **Schedule rotativo**: 3 nações por tick simulado vs 50 nações por tick = 17x mais barato
- **Tier de complexidade**: 80% das nações usam advisors light
- **Context cache**: refresh mensal, não diário
- **Global intelligence cache**: snapshot mundial 1× por mês
- **Avaliação preguiçosa**: Advisors só rodam quando reconvocados, não em loop
- **Async**: decisões pesadas (planejamento de campanha militar) podem rodar em TaskGraph
- **Pre-allocation**: `TArray<FAdvisorProposal>` reutilizadas por Brain, não criadas por ciclo

> Meta: **<10% do tempo de tick total** para 50 nações IA.

---

## 16. Diagrama Final do `UAIDirectorSubsystem`

```
UAIDirectorSubsystem (UWorldSubsystem)
│
├── DirectorScheduler              [round-robin + urgência]
├── GlobalIntelligence             [cache mundial, refresh mensal]
├── DirectorTelemetry              [logs, snapshots, debug]
│
└── NationBrains[] (um por nação IA)
    │
    ├── UNationStrategy
    │   ├── MainDoctrine (Industrializer, Conqueror, Diplomat...)
    │   ├── ActiveGoals[]
    │   │   ├── UGoal_AnnexProvince
    │   │   ├── UGoal_BeGreatPower
    │   │   ├── UGoal_PassReform
    │   │   └── ... emite FAdvisorHint
    │   └── NationPostures (Friend, Rival, Threat, Target...)
    │
    ├── UNationPersonality
    │   ├── FPersonalityProfile (Aggression, Honor, Pragmatism...)
    │   ├── Vinda de NationLeaderProfile + Cultura + Ideologia
    │   └── Adaptativa (eventos podem alterar)
    │
    ├── UNationContextCache
    │   ├── MyStrength / MyFinances / MyStability
    │   ├── KnownNations[] (FPerceivedNation com fog of war)
    │   ├── PerceivedThreats[]
    │   ├── PotentialAllies[]
    │   ├── Opportunities[]
    │   └── Refresh() mensal
    │
    └── Advisors[]
        ├── UEconomicAdvisor  → propõe construção, taxação, empréstimo
        ├── UPoliticalAdvisor → propõe reformas, repressão, eleições
        ├── UDiplomaticAdvisor→ propõe alianças, CB, guerras
        ├── UMilitaryAdvisor  → propõe expansão, modernização, mobilização
        ├── UScienceAdvisor   → propõe tech, espionagem, educação
        └── UColonialAdvisor  → propõe colonização, repressão colonial

Loop:
  Reevaluate()
    ├─ Context.Refresh()
    ├─ Strategy.Recompute() (a cada N meses)
    ├─ for Advisor: ProposeActions() → FAdvisorProposal[]
    ├─ SelectTopProposals (personality + doctrine + budget)
    └─ DispatchToSystem (UEconomy, UPolitics, UMilitary, UDiplomacy...)

Pontes:
  ├─► UEconomySubsystem    (queue construction, set tax)
  ├─► UPoliticsSubsystem   (propose reform, address faction)
  ├─► UDiplomacySubsystem  (declare war, propose alliance, fabricate CB)
  ├─► UMilitarySubsystem   (recruit, modernize, position)
  ├─► UProgressSubsystem   (set research target)
  ├─► UEventSubsystem      (consultor para EventAIResolver)
  └─► UBattleSubsystem     (configurar BattleAI quando batalha começa)
```

---

## 17. Plano de Implementação

1. **Esqueleto**: `UAIDirectorSubsystem` + `UNationBrain` esqueleto + Scheduler round-robin.
2. **`UNationContextCache`** com auto-percepção básica.
3. **`UNationStrategy`** com 3 doutrinas (Industrializer, Conqueror, Defender).
4. **`UNationPersonality`** com 3 eixos (Aggression, Honor, Pragmatism).
5. **1 Advisor**: `UEconomicAdvisor` propondo só construção de fábrica.
6. **Director ranqueia** + despacha para `UEconomySubsystem`.
7. **`UMilitaryAdvisor`** + `UScienceAdvisor`.
8. **`UPoliticalAdvisor`** + `UDiplomaticAdvisor`.
9. **Hints de Strategy** para Advisors.
10. **Goals concretos**: 5 implementações iniciais.
11. **`UNationContextCache` completo** com PerceivedNations + ameaças.
12. **`UGlobalIntelligence`** com snapshots mundiais.
13. **Tiers de complexidade** (Full/Standard/Light/Minimal).
14. **Reação a eventos críticos** (reschedule imediato).
15. **Personalidade adaptativa** (eventos modulam profile).
16. **Mudança de líder** modifica personality.
17. **`DirectorTelemetry`** + painel de debug ImGui.
18. **`UColonialAdvisor`** para potências coloniais.
19. **Integração com EventAIResolver** (Brain consultor).
20. **Polish + tuning de balanceamento**.

---

## 18. Pontos de Atenção Específicos

- **Não centralize execução**. Director **propõe**; subsistemas **executam**. Quebrar isso vira monolito.
- **Personalidade > otimização**. IA "perfeita" é tediosa. Distorça com knobs.
- **Telemetria desde o dia 1**. Sem logs estruturados, debugar IA é impossível.
- **Tier complexidade**: 50 Brains pensando como Bismarck = catastrófico em performance. Maioria das nações pode ter cérebros simples.
- **Context é estimativa**. Nunca permita IA consultar `WorldState` diretamente. Sempre via `KnownNations` com fog of war respeitado.
- **Goals expiram**. Sem deadline ou condição de obsolescência, IA persegue objetivos sem sentido (anexar França em 1900 quando França virou aliada).
- **Action Budget**: realismo + perf. Sem isso IA spam cliques.
- **Cooldowns**: ações repetidas (declarar guerra, propor aliança) precisam cooldown — caso contrário IA propõe a mesma coisa toda semana.
- **Personalidade visível**: jogador deve **sentir** que Bismarck é diferente de Napoleão III. Use textos/diálogos no UI da diplomacia que reflitam o profile.
- **Não esconda demais via fog of war**. Se IA é "burra" porque não sabe nada, jogador acha o jogo quebrado. Faça fog of war narrativo, não punitivo.
