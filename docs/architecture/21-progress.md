# 21 — UProgressSubsystem (Tech Tree, Eras, Vitória)

Sistema de progresso tecnológico, eras históricas e condições de vitória. É o **eixo temporal** do grand strategy: o que transforma 30 anos de jogo numa narrativa de mudança de era. Conecta economia, militar, política e diplomacia num arco coerente.

---

## 1. Princípios Arquiteturais

### Tech ≠ "lista de coisas para clicar"

Tech é **fonte de modificadores e desbloqueios** que outros sistemas consultam. Ela **não executa lógica**, **não simula nada por si só**. É um repositório de "estado tecnológico da nação" + um motor de pesquisa.

### Três responsabilidades, não confundir

| Responsabilidade | Onde mora |
|---|---|
| Definir **o que existe** (nós, eras, vitórias) | DataAssets estáticos |
| Manter **estado de pesquisa** da nação | `UNationTechState` (no save) |
| Aplicar **efeitos** ao desbloquear | Subsistemas que escutam `OnTechResearched` |

> ⚠️ **Antipattern a evitar**: `UProgressSubsystem` modificar diretamente economia, militar etc. Isso vira spaghetti rapidamente. Em vez disso, **emite eventos** e os outros subsistemas decidem como reagir.

### Tech ≠ Doutrina ≠ Lei

| Conceito | O que é | Reversível? | Custo |
|---|---|---|---|
| **Tech** | Conhecimento descoberto | Não (uma vez pesquisada, fica) | Tempo + investimento |
| **Doutrina** | Escolha de aplicação militar | Sim | Reforma militar |
| **Lei** | Decisão política institucional | Sim | Capital político |

---

## 2. Estrutura Hierárquica

```
EraAsset (período histórico)
   └── TechCategoryAsset (ramo: Indústria, Militar, Cultural...)
        └── TechNodeAsset (nó pesquisável)
             ├── Prerequisites[] (outros nós)
             ├── Unlocks[] (efeitos)
             └── ProgressionRequirements (não basta tempo: precisa de X)
```

### `UEraAsset` — Período Histórico

```cpp
UCLASS(BlueprintType)
class UEraAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly) FName EraId;            // "Era.Industrial", "Era.SteamAge"
    UPROPERTY(EditDefaultsOnly) FText DisplayName;
    UPROPERTY(EditDefaultsOnly) int32 EraIndex;         // 0..N ordem cronológica

    UPROPERTY(EditDefaultsOnly) int32 HistoricalStartYear;
    UPROPERTY(EditDefaultsOnly) int32 HistoricalEndYear;

    // Requisitos para a nação "entrar" na era
    UPROPERTY(EditDefaultsOnly) int32 MinimumTechsResearched;
    UPROPERTY(EditDefaultsOnly) TArray<UTechNodeAsset*> RequiredKeyTechs;

    // Modificadores passivos por estar na era
    UPROPERTY(EditDefaultsOnly) FStatModifierSet GlobalModifiers;

    // Eventos disparados ao entrar/sair
    UPROPERTY(EditDefaultsOnly) TArray<UEventDefinitionAsset*> OnEnterEvents;
};
```

**Eras vitorianas sugeridas**:

| Era | Anos | Tecnologias-chave |
|---|---|---|
| Late Mercantile | 1820–1840 | Manufatura, Fragatas, Línea |
| Steam Age | 1840–1870 | Vapor industrial, Ferrovia, Ironclad |
| High Industrial | 1870–1895 | Aço Bessemer, Telégrafo, Rifle Ferrolho |
| Late Imperial | 1895–1910 | Eletricidade, Combustão, Metralhadora |
| Early Modern | 1910–1925 | Aviação, Tanque, Rádio, Fordismo |

### `UTechCategoryAsset` — Ramo

```cpp
UCLASS(BlueprintType)
class UTechCategoryAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly) FName CategoryId;       // "Tech.Industry"
    UPROPERTY(EditDefaultsOnly) FText DisplayName;
    UPROPERTY(EditDefaultsOnly) ETechBranch Branch;     // Industry, Military, Culture, Naval, Politics

    // Atributo da nação que acelera pesquisa nesta categoria
    UPROPERTY(EditDefaultsOnly) EPopType PrimaryResearchPop;  // ex: Engineer para Industry
    UPROPERTY(EditDefaultsOnly) ENationAttribute Attribute;   // Innovation, Discipline, Tradition
};
```

Categorias propostas:

| Branch | Categorias internas |
|---|---|
| Industry | Manufatura, Metalurgia, Energia, Logística, Química |
| Military (Land) | Armamento Leve, Artilharia, Engenharia, Mecanização, Doutrina Terrestre |
| Naval | Casco, Propulsão, Armamento Naval, Doutrina Naval |
| Air | Balonismo, Dirigíveis, Aviação |
| Culture | Educação, Imprensa, Identidade Nacional, Artes |
| Politics | Administração, Direito, Reforma Social |
| Commerce | Banco, Comércio Internacional, Seguros, Bolsa |

### `UTechNodeAsset` — Nó Pesquisável

```cpp
UCLASS(BlueprintType)
class UTechNodeAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly) FName NodeId;           // "Tech.Firearms.BoltAction"
    UPROPERTY(EditDefaultsOnly) FText DisplayName;
    UPROPERTY(EditDefaultsOnly) FText FlavorText;

    UPROPERTY(EditDefaultsOnly) UTechCategoryAsset* Category;
    UPROPERTY(EditDefaultsOnly) UEraAsset* Era;         // a qual era pertence
    UPROPERTY(EditDefaultsOnly) int32 Tier;             // 1..5 dentro da era

    // Pré-requisitos (todos AND)
    UPROPERTY(EditDefaultsOnly) TArray<UTechNodeAsset*> Prerequisites;
    // Pré-requisitos alternativos (qualquer um OR)
    UPROPERTY(EditDefaultsOnly) TArray<UTechNodeAsset*> AlternativePrerequisites;

    // Custo de pesquisa
    UPROPERTY(EditDefaultsOnly) int32 BaseResearchPoints;
    UPROPERTY(EditDefaultsOnly) FResearchRequirements ExtraRequirements;
        // ex: precisa de 1 universidade construída, ou de evento histórico, etc

    // Efeitos ao desbloquear
    UPROPERTY(EditDefaultsOnly) FTechUnlockSet Unlocks;

    // Para tech tree dinâmica
    UPROPERTY(EditDefaultsOnly) bool bUnique;           // só uma nação pode ter (ex: "primeiro a"
    UPROPERTY(EditDefaultsOnly) bool bRepeatable;       // ex: "Eficiência Industrial +1%" infinito
    UPROPERTY(EditDefaultsOnly) bool bHidden;           // só aparece após trigger
};
```

### `FTechUnlockSet` — O que o nó libera

Esse é o coração da integração com outros sistemas:

```cpp
USTRUCT()
struct FTechUnlockSet
{
    GENERATED_BODY()

    // Para Economia
    UPROPERTY(EditDefaultsOnly) TArray<UIndustryTypeAsset*>   IndustriesUnlocked;
    UPROPERTY(EditDefaultsOnly) TArray<UGoodAsset*>           GoodsUnlocked;

    // Para Militar
    UPROPERTY(EditDefaultsOnly) TArray<UEquipmentAsset*>      EquipmentUnlocked;
    UPROPERTY(EditDefaultsOnly) TArray<UUnitClassAsset*>      UnitClassesUnlocked;
    UPROPERTY(EditDefaultsOnly) TArray<UDoctrineAsset*>       DoctrinesUnlocked;
    UPROPERTY(EditDefaultsOnly) TArray<UBattleCardAsset*>     CardsUnlocked;

    // Para Política
    UPROPERTY(EditDefaultsOnly) TArray<ULawAsset*>            LawsUnlocked;
    UPROPERTY(EditDefaultsOnly) TArray<FName>                 GovernmentTypesUnlocked;

    // Para Diplomacia
    UPROPERTY(EditDefaultsOnly) TArray<FName>                 DiplomaticActionsUnlocked;

    // Para Construção
    UPROPERTY(EditDefaultsOnly) TArray<UBuildingAsset*>       BuildingsUnlocked;

    // Modificadores passivos (sempre ativos depois de pesquisado)
    UPROPERTY(EditDefaultsOnly) FStatModifierSet              PassiveModifiers;

    // Modificadores condicionais (só em contextos específicos)
    UPROPERTY(EditDefaultsOnly) TArray<FConditionalModifier>  ConditionalModifiers;

    // Eventos narrativos disparados
    UPROPERTY(EditDefaultsOnly) TArray<UEventDefinitionAsset*> TriggeredEvents;
};
```

> **Por que tudo num único struct**: simplifica o broadcast. Quando o nó é pesquisado, `UProgressSubsystem` emite **um evento** com o `FTechUnlockSet` completo. Cada subsistema interessado consulta os campos que lhe importam.

---

## 3. Estado da Nação — `UNationTechState`

```cpp
UCLASS()
class UNationTechState : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY() TSet<FName> ResearchedNodes;           // ids de UTechNodeAsset
    UPROPERTY() FName CurrentResearchNodeId;           // o que está pesquisando
    UPROPERTY() float CurrentResearchProgress;         // pontos acumulados
    UPROPERTY() TArray<FName> ResearchQueue;           // até 3 próximos

    UPROPERTY() FName CurrentEraId;
    UPROPERTY() TMap<ETechBranch, int32> RepeatableLevels; // para techs infinitas

    UPROPERTY() float CachedResearchPointsPerDay;      // recalcula on-change
};
```

> Vive dentro de `UNation`. Faz parte do save da nação.

---

## 4. Pesquisa — Fluxo

### Geração de pontos de pesquisa

```cpp
float UProgressSubsystem::ComputeResearchOutput(UNation* Nation,
                                                ETechBranch Branch)
{
    float Output = 0.0f;

    // POPs contribuem conforme tipo e literacia
    for (UProvince* P : Nation->Provinces)
    {
        for (FPopGroup& Pop : P->Economy->Pops)
        {
            float Contribution = Pop.Size
                                * Pop.Literacy
                                * GetPopBranchAffinity(Pop.Type, Branch);
            Output += Contribution;
        }
    }

    // Universidades / Academias / Laboratórios
    Output += Nation->CountBuildings(Branch) * BuildingMultiplier;

    // Modificadores nacionais (leis, ideologia, traços)
    Output *= Nation->GetResearchMultiplier(Branch);

    // Bônus por estar atrás historicamente (catch-up)
    Output *= ComputeCatchUpBonus(Nation, Branch);

    // Penalidade por estar à frente (diminishing returns)
    Output *= ComputeFrontierPenalty(Nation, Branch);

    return Output;
}
```

### Catch-up e Frontier — modulação importante

| Situação | Modificador | Justificativa |
|---|---|---|
| Pesquisando tech que **outra nação já tem** | × 1.3 a × 2.0 | Difusão de conhecimento (livros, espionagem, imigração) |
| Pesquisando tech que **ninguém tem** ainda | × 0.7 a × 1.0 | Pioneirismo é caro |
| Tech 2+ eras atrasada | × 0.5 | Falta base científica para entender |

Sem isso, a nação líder vira hegemônica permanente. Com isso, há *rubber-banding* histórico realista.

### Tick de pesquisa

```cpp
void UProgressSubsystem::OnDayPassed()
{
    for (UNation* Nation : World->Nations)
    {
        UNationTechState* State = Nation->TechState;
        if (State->CurrentResearchNodeId.IsNone()) continue;

        UTechNodeAsset* Node = ResolveNode(State->CurrentResearchNodeId);
        State->CurrentResearchProgress += State->CachedResearchPointsPerDay;

        if (State->CurrentResearchProgress >= Node->BaseResearchPoints)
        {
            CompleteResearch(Nation, Node);
            AdvanceQueue(Nation);
        }
    }
}
```

### Conclusão de pesquisa

```cpp
void UProgressSubsystem::CompleteResearch(UNation* Nation, UTechNodeAsset* Node)
{
    Nation->TechState->ResearchedNodes.Add(Node->NodeId);
    Nation->TechState->CurrentResearchNodeId = NAME_None;
    Nation->TechState->CurrentResearchProgress = 0.0f;

    // Broadcast — outros subsistemas reagem
    OnTechResearched.Broadcast(Nation->NationId, Node);

    // Verifica progressão de era
    if (CheckEraAdvancement(Nation))
        AdvanceEra(Nation);

    // Verifica condições de vitória
    VictorySubsystem->ReevaluateVictory(Nation);

    // Histórico (para análise pós-jogo, replays, eventos)
    ResearchHistory.Add(FResearchRecord{Nation->NationId, Node->NodeId,
                                         CurrentTickDay});
}
```

---

## 5. Como os Outros Subsistemas Reagem

### `UEconomySubsystem`
```cpp
void UEconomySubsystem::OnTechResearched(int32 NationId, UTechNodeAsset* Node)
{
    UNation* Nation = World->GetNation(NationId);

    for (UIndustryTypeAsset* Industry : Node->Unlocks.IndustriesUnlocked)
        Nation->AvailableIndustries.Add(Industry);

    for (UGoodAsset* Good : Node->Unlocks.GoodsUnlocked)
        Nation->Markets.RegisterNewGood(Good);

    // Modificadores passivos (ex: "Bessemer dá +20% throughput em SteelMill")
    Nation->ApplyEconomicModifiers(Node->Unlocks.PassiveModifiers);
}
```

### `UMilitarySubsystem`
```cpp
void UMilitarySubsystem::OnTechResearched(int32 NationId, UTechNodeAsset* Node)
{
    UNation* Nation = World->GetNation(NationId);

    for (UEquipmentAsset* Eq : Node->Unlocks.EquipmentUnlocked)
        Nation->ProducibleEquipment.Add(Eq);

    for (UUnitClassAsset* Cls : Node->Unlocks.UnitClassesUnlocked)
        Nation->RecruitableClasses.Add(Cls);

    for (UDoctrineAsset* Doc : Node->Unlocks.DoctrinesUnlocked)
        Nation->AvailableDoctrines.Add(Doc);

    // Notifica IA militar para reconsiderar modernização
    AIDirector->NotifyMilitaryOptionsExpanded(NationId);
}
```

### `UPoliticsSubsystem`, `UDiplomacySubsystem`
Análogo — escutam `OnTechResearched` e adicionam leis, ações diplomáticas, e modificadores de prestígio conforme aplicável.

### `UBattleSubsystem`
Não escuta diretamente — consome o estado já atualizado via `URegimentResolver`. Mantém o subsistema de batalha **stateless em relação à tech**, recebendo tudo composto.

---

## 6. Tech Tree Dinâmica — Não Apenas Estática

A diferença entre tech tree de Civ (linear) e a vitoriana realista é que **algumas techs só aparecem em contexto**.

### Triggers de descoberta

```cpp
USTRUCT()
struct FResearchRequirements
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly) bool bRequiresWar;          // tech militar avançada
    UPROPERTY(EditDefaultsOnly) bool bRequiresPeace;        // tech cultural
    UPROPERTY(EditDefaultsOnly) int32 MinUrbanizationPercent;
    UPROPERTY(EditDefaultsOnly) int32 MinLiteracy;
    UPROPERTY(EditDefaultsOnly) TArray<FName> RequiredBuildings; // ex: precisa universidade
    UPROPERTY(EditDefaultsOnly) TArray<FName> RequiredEvents;    // ex: "Industrial Revolution"
    UPROPERTY(EditDefaultsOnly) TArray<UGoodAsset*> RequiredAccessibleGoods;
                                                          // ex: precisa importar/produzir borracha
                                                          // para pesquisar pneumáticos
};
```

> Isso resolve um vício clássico: nação sem litoral pesquisando tech naval. **Se não pode acessar borracha, não pode estudar pneumáticos.** Conexão entre geografia e ciência.

### Hidden techs e descobertas espontâneas

Algumas techs não aparecem na árvore visível até serem **disparadas** por eventos:
- Uma expedição científica pode revelar `Biology.Microbiology`.
- Espionagem pode copiar tech de outra nação.
- Eventos históricos (descoberta de petróleo na Pérsia) mudam o que é relevante.

Isso é gerenciado pelo `UEventSubsystem` chamando `UProgressSubsystem::RevealHiddenTech(NationId, NodeId)`.

---

## 7. Mecânicas de Difusão e Espionagem

```cpp
class UProgressSubsystem
{
    // Espionagem copia tech, mas com custo e risco
    bool TryStealTech(int32 SpyNation, int32 TargetNation, FName NodeId);

    // Difusão automática entre aliados próximos
    void ProcessTechDiffusion();

    // Tratado de transferência tecnológica (diplomático)
    void TransferTech(int32 FromNation, int32 ToNation, FName NodeId,
                       const FTechTransferTerms& Terms);
};
```

A difusão automática roda mensalmente:
- Para cada par (NationA, NationB) com tech disparidade > 2 nós
- Se há rota comercial / aliança / fronteira
- Acelera pesquisa do atrasado em techs que o avançado já tem

> **Não é instantâneo**. Apenas reduz custo. Realismo: ideias se espalham mas exigem absorção.

---

## 8. Sistema de Eras

### Avanço de era

```cpp
bool UProgressSubsystem::CheckEraAdvancement(UNation* Nation)
{
    UEraAsset* CurrentEra = ResolveEra(Nation->TechState->CurrentEraId);
    UEraAsset* NextEra = GetNextEra(CurrentEra);
    if (!NextEra) return false;

    // Critério 1: número de techs na era atual
    int32 TechsInCurrentEra = CountTechsInEra(Nation, CurrentEra);
    if (TechsInCurrentEra < NextEra->MinimumTechsResearched) return false;

    // Critério 2: techs-chave obrigatórias
    for (UTechNodeAsset* Key : NextEra->RequiredKeyTechs)
        if (!Nation->TechState->ResearchedNodes.Contains(Key->NodeId))
            return false;

    return true;
}

void UProgressSubsystem::AdvanceEra(UNation* Nation)
{
    UEraAsset* OldEra = ResolveEra(Nation->TechState->CurrentEraId);
    UEraAsset* NewEra = GetNextEra(OldEra);

    Nation->TechState->CurrentEraId = NewEra->EraId;
    Nation->ApplyModifiers(NewEra->GlobalModifiers);

    // Eventos narrativos
    for (UEventDefinitionAsset* Ev : NewEra->OnEnterEvents)
        EventSubsystem->QueueEvent(Nation, Ev);

    // Anuncio global (impacta percepção, prestígio)
    OnNationEraAdvanced.Broadcast(Nation->NationId, NewEra->EraId);
    Nation->Prestige += NewEra->PrestigeOnEntry;
}
```

### Implicações narrativas da era

- **Ser primeiro a entrar numa era** dá prestígio e às vezes técnica única (`bUnique` techs).
- **Ficar muito atrás** (2+ eras de defasagem) dispara eventos de "humilhação", crise política, ou pressão diplomática.
- IA usa era como heurística simples para avaliar ameaças e oportunidades.

---

## 9. Sistema de Vitória — `UVictorySubsystem`

Subsistema próprio (composto pelo Progress, mas independente). Vitória não é apenas tech.

```cpp
UCLASS()
class UVictorySubsystem : public UWorldSubsystem
{
    GENERATED_BODY()
public:
    void ReevaluateVictory(UNation* Nation);
    void RegisterCondition(UVictoryConditionAsset* Cond);
    EVictoryStatus GetStatus(int32 NationId) const;
};
```

### `UVictoryConditionAsset`

```cpp
UCLASS(BlueprintType, EditInlineNew)
class UVictoryConditionAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly) FName ConditionId;
    UPROPERTY(EditDefaultsOnly) EVictoryType Type;       // Domination, Industrial, Cultural,
                                                         // Diplomatic, Scientific, Score
    UPROPERTY(EditDefaultsOnly) FText DisplayName;

    UPROPERTY(EditDefaultsOnly, Instanced)
    TArray<UVictoryCheck*> Checks;                       // todos devem ser true (AND)

    UPROPERTY(EditDefaultsOnly) bool bSharedVictory;     // permite vitória conjunta
                                                         // (aliados ganham junto)
    UPROPERTY(EditDefaultsOnly) bool bEndsGame;          // jogo termina ao alcançar
};
```

### `UVictoryCheck` — checks plugáveis

```cpp
UCLASS(Abstract, EditInlineNew)
class UVictoryCheck : public UObject
{
public:
    virtual bool IsSatisfied(const UNation* Nation,
                             const UWorldState* World) const PURE_VIRTUAL(...);
    virtual float GetProgress(const UNation* Nation,
                              const UWorldState* World) const { return 0; }
};
```

Implementações concretas:

| Check | Critério |
|---|---|
| `UVictoryCheck_PrestigeRank` | Top N em prestígio mundial |
| `UVictoryCheck_IndustrialOutput` | Produção total > X% mundial |
| `UVictoryCheck_TechCount` | Tem >= N techs em ramo X |
| `UVictoryCheck_FirstToTech` | Primeiro a pesquisar nó específico |
| `UVictoryCheck_Territory` | Controla regiões específicas |
| `UVictoryCheck_PopulationLiteracy` | Literacia média > X |
| `UVictoryCheck_CulturalReach` | N nações na sua esfera cultural |
| `UVictoryCheck_NavalSupremacy` | Tonelagem > 2× próximo competidor |

### Tipos de vitória vitorianos sugeridos

| Tipo | Resumo |
|---|---|
| **Industrial** | Líder em produção + 5 techs industriais avançadas + 3 ferrovias-tronco |
| **Imperial** | Controla X% das colônias + supremacia naval |
| **Científica** | Primeiro a entrar na Era Moderna + N "primeiras descobertas" |
| **Cultural** | Esfera cultural cobre 30%+ da população mundial + literacia top |
| **Diplomática** | Líder de aliança + nenhum conflito por 50 anos + esfera de paz |
| **Pontuação** | Score composto (prestígio + indústria + colônias) ao fim de data limite |

### Fluxo de avaliação

```cpp
void UVictorySubsystem::ReevaluateVictory(UNation* Nation)
{
    for (UVictoryConditionAsset* Cond : ActiveConditions)
    {
        bool AllSatisfied = true;
        for (UVictoryCheck* Check : Cond->Checks)
        {
            if (!Check->IsSatisfied(Nation, World))
            {
                AllSatisfied = false;
                break;
            }
        }

        if (AllSatisfied)
        {
            HandleVictory(Nation, Cond);
            return;
        }

        // Atualiza progresso para UI mesmo se incompleto
        UpdateProgressTracker(Nation, Cond);
    }
}
```

> ⚠️ **Não rode em todo tick.** Avalie em `OnTechResearched`, `OnTerritoryChanged`, `OnNationDestroyed`, e mensalmente como fallback.

---

## 10. Eventos Emitidos

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnTechResearched, int32 /*NationId*/, UTechNodeAsset* /*Node*/);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnNationEraAdvanced, int32 /*NationId*/, FName /*NewEraId*/);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnFirstToResearch, int32 /*NationId*/, UTechNodeAsset* /*Node*/);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnHiddenTechRevealed, FTechRevealEvent);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnVictoryAchieved, int32 /*NationId*/, UVictoryConditionAsset* /*Condition*/);
```

UI ouve para alertas e árvore visual. `UDiplomacySubsystem` ouve `OnFirstToResearch` para ajustar opiniões internacionais (a "corrida científica" tem peso reputacional).

---

## 11. Performance

- **Tech tree validation deve ser pré-computada**. No início do jogo, `UProgressSubsystem` constrói grafos de pré-requisitos resolvidos. Em runtime, consultas `IsAvailable(NodeId)` são O(1).
- **`CachedResearchPointsPerDay`** evita recalcular toda hora. Só invalide quando: POPs mudam, lei muda, prédio é construído.
- **Difusão tecnológica** é O(N²) no número de nações. Limite a vizinhos diretos + aliados + parceiros comerciais.
- **Avaliação de vitória** é cara — não rode em loop. Triggers + fallback mensal.
- **Histórico de pesquisa** pode crescer indefinidamente. Em saves longos, descarte registros > 100 anos para nações destruídas.

---

## 12. Diagrama Final

```
UProgressSubsystem (UWorldSubsystem)
│
├── DataAssets (estático)
│   ├── UEraAsset[]                 [Late Mercantile → Early Modern]
│   ├── UTechCategoryAsset[]        [Industry, Military, Naval, Air, Politics, Culture, Commerce]
│   └── UTechNodeAsset[]
│       ├── Prerequisites
│       ├── ExtraRequirements (war, peace, buildings, goods)
│       └── FTechUnlockSet
│           ├── IndustriesUnlocked
│           ├── GoodsUnlocked
│           ├── EquipmentUnlocked
│           ├── UnitClassesUnlocked
│           ├── DoctrinesUnlocked
│           ├── CardsUnlocked
│           ├── LawsUnlocked
│           ├── BuildingsUnlocked
│           ├── DiplomaticActionsUnlocked
│           ├── PassiveModifiers
│           └── TriggeredEvents
│
├── Estado por nação (no save)
│   └── UNationTechState
│       ├── ResearchedNodes
│       ├── CurrentResearchNodeId / Progress
│       ├── ResearchQueue
│       ├── CurrentEraId
│       └── CachedResearchPointsPerDay
│
├── Motor de pesquisa
│   ├── ComputeResearchOutput (POPs + buildings + leis + catch-up)
│   ├── OnDayPassed → progresso
│   ├── CompleteResearch → broadcast
│   └── CheckEraAdvancement → AdvanceEra
│
├── Mecânicas avançadas
│   ├── Difusão (mensal, entre vizinhos/aliados)
│   ├── Espionagem (TryStealTech)
│   ├── Transferência diplomática
│   └── Hidden techs (reveladas por eventos)
│
└── Pontes para outros sistemas
    ├── ↔ UEconomySubsystem    (indústrias, bens, modificadores econômicos)
    ├── ↔ UMilitarySubsystem   (equipamento, classes, doutrinas)
    ├── ↔ UBattleSubsystem     (cartas via equipment/doctrine)
    ├── ↔ UPoliticsSubsystem   (leis, governos, pressão progressista)
    ├── ↔ UDiplomacySubsystem  (ações diplomáticas, prestígio)
    ├── ↔ UEventSubsystem      (eventos disparados por tech)
    └── ↔ UVictorySubsystem    (reavaliação automática)

UVictorySubsystem (paralelo)
│
├── UVictoryConditionAsset[]
│   ├── Industrial / Imperial / Scientific / Cultural / Diplomatic / Score
│   └── UVictoryCheck[] (composição AND)
│
└── ReevaluateVictory (em triggers, não no tick)
```

---

## 13. Plano de Implementação

1. **DataAssets básicos**: `UEraAsset`, `UTechCategoryAsset`, `UTechNodeAsset` com 1 era, 2 categorias, 5 nós encadeados.
2. **`UNationTechState`** + persistência no save.
3. **Tick de pesquisa diário**: progresso linear sem catch-up ainda.
4. **`OnTechResearched` broadcast** + 1 ouvinte (Military, para liberar 1 equipamento).
5. **`FTechUnlockSet` completo**: ligações com Economy, Military, Politics, Diplomacy.
6. **Pré-requisitos** (AND + OR) e validação de disponibilidade.
7. **`FResearchRequirements`** (war, peace, buildings, goods).
8. **Catch-up bonus + frontier penalty**.
9. **Sistema de eras**: avanço, modificadores, eventos.
10. **Difusão tecnológica** entre nações próximas.
11. **`UVictorySubsystem`** com 2 conditions iniciais (Industrial e Score).
12. **Conditions completas** (6 tipos de vitória).
13. **Espionagem e transferência diplomática**.
14. **Hidden techs e triggers narrativos**.
15. **UI**: árvore visual, painel de pesquisa, tracker de vitória.

---

## 14. Pontos de Atenção Específicos

- **Não overengineer a árvore inicial**. Comece com 30 nós. É melhor balancear 30 bem do que 200 mal.
- **Cada nó precisa ter consequência sentida**. Se pesquisar não muda nada visível, o jogador desconecta. Cada tech deve liberar pelo menos 1 coisa concreta (carta, equipamento, indústria, lei).
- **Evite "techs filler"**. "+5% eficiência" sozinho é tédio. Combine: "+5% eficiência + libera carta X + +1 indústria disponível".
- **Tech militar deve sentir-se na batalha**. Se pesquisar "Rifle de Repetição" não mudar a sensação de combate, o sistema falhou. A integração via `UBattleCardAsset` é o que cria isso.
- **Vitória tecnológica não pode ser trivial**. Exija combinação (techs + indústria + tempo), nunca apenas "pesquisar última tech".
- **Reflita defasagem na narrativa**. Nação em era 1 enquanto vizinhos estão na era 3 deve receber eventos "humilhação", "missão estrangeira", "reforma urgente". Sem isso, atraso é só números.
- **Não dependa de pesquisa para diversão**. Tech é meta-camada. O loop principal (mapa, batalha, economia) tem que funcionar mesmo se ninguém pesquisar.

---

## 15. Visão Conjunta — Como Tech Conecta Tudo

```
UProgressSubsystem (Tech)
        │
        ├──► desbloqueia UIndustryTypeAsset / UGoodAsset
        │       └──► UEconomySubsystem produz mais / novos bens
        │
        ├──► desbloqueia UEquipmentAsset / UUnitClassAsset / UDoctrineAsset
        │       └──► UMilitarySubsystem re-equipa
        │               └──► URegimentResolver compõe profile
        │                       └──► UBattleSubsystem deck enriquecido
        │
        ├──► desbloqueia ULawAsset
        │       └──► UPoliticsSubsystem permite reforma
        │
        ├──► desbloqueia FName DiplomaticAction
        │       └──► UDiplomacySubsystem nova opção
        │
        ├──► dispara UEventDefinitionAsset
        │       └──► UEventSubsystem narrativa
        │
        └──► UVictorySubsystem reavalia condições
                ├── Industrial ← UEconomy
                ├── Imperial ← UMilitary + UDiplomacy
                ├── Cultural ← UPolitics + UPopulation
                ├── Scientific ← UProgress
                └── Diplomatic ← UDiplomacy
```

A tech é o **fio condutor temporal** que costura os subsistemas. Cada nó é um *evento de mudança* que se propaga em ondas concêntricas pelo resto do jogo.
