# 31 — UPoliticsSubsystem

Sistema político interno: ideologias, leis, governos, capital político, eleições, facções e revoltas. É onde POPs, governo e instabilidade se encontram — o lugar onde o jogador sente que **gerencia uma sociedade**, não apenas uma planilha.

---

## 1. Princípios Arquiteturais

### Política não é "menu de leis"

Erro comum: implementar um menu de leis com cliques que mudam números. Isso é UI sem sistema. Política em grand strategy é a **resolução de tensão entre forças sociais** — POPs querem coisas opostas, e o governo precisa equilibrar para sobreviver.

### Três loops fundamentais

```
┌──────────────────────────────────────────────────┐
│  Loop 1: POPs ──► Ideologias ──► Facções        │ (formação social)
│                                                  │
│  Loop 2: Facções ──► Governo ──► Leis ──► POPs   │ (governança)
│                                                  │
│  Loop 3: Insatisfação ──► Militância ──► Revolta │ (instabilidade)
└──────────────────────────────────────────────────┘
```

Cada loop tem cadência diferente. Loop 1 é lento (anos). Loop 2 é médio (meses). Loop 3 é rápido (semanas, em crise).

### Política deve oferecer agência, não burocracia

Se o jogador aprova 30 leis sem ter tempo de entender o impacto de cada uma, falhamos. Cada decisão política precisa:
- Ser visível (pelo menos 1 facção celebra/protesta)
- Ser custosa (capital político, tempo)
- Ter trade-off real (ninguém ganha tudo)

### Política se conecta a tudo

| Sistema | Como Política afeta |
|---|---|
| Economia | Leis tributárias, comerciais, trabalhistas |
| Militar | Lei de conscrição, gasto militar |
| Diplomacia | Diplomacia depende de governo (autocracia ≠ democracia) |
| Tech | Leis educacionais aceleram pesquisa |
| Eventos | Maioria dos eventos políticos vem do EventSubsystem |
| Batalha | Doutrinas militares dependem de leis (ex: "Conscrição Universal" libera doutrinas de massa) |

---

## 2. Estrutura Hierárquica

```
UPoliticsSubsystem
   ├── GovernmentRegistry         [tipos de governo]
   ├── LawRegistry                [todas as leis disponíveis]
   ├── IdeologyRegistry           [ideologias disponíveis]
   ├── PartyRegistry              [partidos por nação]
   ├── ReformEngine               [aprovação de leis]
   ├── ElectionEngine             [eleições e mudança de governo]
   ├── RevoltEngine               [detecção e disparo de revoltas]
   └── PoliticalAIResolver        [IA decide reformas, repressão]
```

---

## 3. Ideologias — A Camada Fundamental

Ideologia é **o eixo de preferências políticas** de um POP. Um POP não tem opinião sobre cada lei individual; tem uma ideologia que **gera** preferências.

### `UIdeologyAsset`

```cpp
UCLASS(BlueprintType)
class UIdeologyAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly) FName IdeologyId;       // "Ideology.Liberal"
    UPROPERTY(EditDefaultsOnly) FText DisplayName;
    UPROPERTY(EditDefaultsOnly) EIdeologyAxis Axis;     // Reactionary, Conservative,
                                                        // Liberal, Socialist, Anarchist
    UPROPERTY(EditDefaultsOnly) FLinearColor PartyColor;

    // Preferências por categoria de lei (-1..1)
    UPROPERTY(EditDefaultsOnly) TMap<ELawCategory, FLawPreferenceCurve> Preferences;

    // Pré-requisitos para o POP adotar esta ideologia
    UPROPERTY(EditDefaultsOnly) FName RequiredTech;     // ex: socialismo precisa "Class Theory"
    UPROPERTY(EditDefaultsOnly) FIdeologyAttraction Attraction;  // o que torna POPs propensos
};
```

### Ideologias vitorianas sugeridas

| Ideologia | Atrai | Caracterização |
|---|---|---|
| Reactionary | Aristocratas, Clero | Restaurar ordem antiga, monarquia absoluta |
| Conservative | Aristocratas, Bourgeois ricos | Preservar status quo, mudança gradual |
| Liberal | Bourgeois, Intelligentsia | Reformas econômicas, sufrágio limitado |
| Radical | Artisans, Bourgeois urbanos | Sufrágio amplo, secularização |
| Socialist | Workers, Engineers | Direitos trabalhistas, redistribuição |
| Communist | Workers urbanos | Revolução, propriedade coletiva |
| Anarchist | Workers desesperados | Abolição do estado |
| Nationalist | Transversal | Cultura/raça acima de classe |
| Fascist | Tardio (>1900) | Estado forte, nacionalismo radical |

### Atração Ideológica — Como POPs migram entre ideologias

```cpp
USTRUCT()
struct FIdeologyAttraction
{
    TMap<EPopType, float> PopTypeAffinity;       // Worker × Socialist = +0.8
    TMap<ECultureId, float> CultureAffinity;
    float LiteracyMultiplier;                    // mais letrados → liberal/socialista
    float UrbanizationMultiplier;
    float MilitancyThreshold;                    // só atrai se militância > X
    bool bRequiresUrban;
};
```

```cpp
void UPoliticsSubsystem::UpdateIdeologyDrift(FPopGroup& Pop, UProvince* P)
{
    TMap<FName, float> Scores;
    for (UIdeologyAsset* Ideo : AvailableIdeologies(Pop))
    {
        float S = 0;
        S += Ideo->Attraction.PopTypeAffinity.FindRef(Pop.Type);
        S += Ideo->Attraction.CultureAffinity.FindRef(Pop.Culture);
        S += Pop.Literacy * Ideo->Attraction.LiteracyMultiplier;
        S += P->Urbanization * Ideo->Attraction.UrbanizationMultiplier;
        // Eventos podem aplicar bonus temporário (revolta de 1848)
        S += GetActiveIdeologyBonus(Ideo, P);
        Scores.Add(Ideo->IdeologyId, S);
    }
    // Suavização: POP migra ~5% para a ideologia com maior score por mês
    Pop.IdeologyDistribution = SmoothBlend(Pop.IdeologyDistribution, Scores, 0.05f);
}
```

> **Importante**: POP **não tem ideologia única**. Tem **distribuição** entre ideologias. 60% Liberal + 30% Conservative + 10% Socialist. Isso permite mudanças graduais e representação política proporcional.

---

## 4. Leis — A Saída Política

### `ULawAsset`

```cpp
UCLASS(BlueprintType)
class ULawAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly) FName LawId;            // "Law.Suffrage.Universal"
    UPROPERTY(EditDefaultsOnly) FText DisplayName;
    UPROPERTY(EditDefaultsOnly) ELawCategory Category;
    UPROPERTY(EditDefaultsOnly) int32 ProgressionLevel; // 0..N na escala da categoria

    // Pré-requisitos
    UPROPERTY(EditDefaultsOnly) FName RequiredTech;
    UPROPERTY(EditDefaultsOnly) FName RequiredGovernmentType;
    UPROPERTY(EditDefaultsOnly) TArray<FName> RequiredLaws;

    // Custo
    UPROPERTY(EditDefaultsOnly) int32 PoliticalCapitalCost;
    UPROPERTY(EditDefaultsOnly) int32 EnactmentDays;    // tempo até efeito

    // Efeitos quando ativa
    UPROPERTY(EditDefaultsOnly) FStatModifierSet PassiveModifiers;
    UPROPERTY(EditDefaultsOnly) FTechUnlockSet  Unlocks; // pode liberar doutrinas, ações etc

    // Reação ideológica (preenche FLawPreferenceCurve)
    UPROPERTY(EditDefaultsOnly) TMap<FName, float> IdeologySupport; // -1..1 por ideologia
};
```

### Categorias de Leis Vitorianas

```cpp
UENUM()
enum class ELawCategory : uint8
{
    Government,        // Monarquia Absoluta → Constitucional → Parlamentarismo → República
    Suffrage,          // Nenhum → Censitário → Universal Masculino → Universal
    Slavery,           // Legal → Apenas Colônias → Abolido
    Trade,             // Mercantilismo → Protecionismo → Livre Comércio
    Taxation,          // Por Estamento → Proporcional → Progressivo
    Conscription,      // Profissional → Voluntário → Conscrição Limitada → Universal
    Education,          // Privada → Religiosa → Estatal Básica → Universal
    LaborRights,       // Nenhum → Mínimo → Sindicatos → Direitos Plenos
    Religion,          // Estado Confessional → Tolerância → Secularismo
    Press,             // Censura Total → Censura Parcial → Liberdade
    Colonial,          // Exploração → Assimilação → Autonomia
}
```

> Cada categoria tem **uma lei ativa por nação**. Trocar lei = passar pelo `ReformEngine`.

### Curvas de Preferência

```cpp
USTRUCT()
struct FLawPreferenceCurve
{
    // Ideologia × Lei: cada ideologia tem preferência por nível de progressão
    UPROPERTY() TMap<int32, float> ProgressionPreference; // level → -1..1
};
```

Exemplo: Ideologia Socialist em LaborRights:
- Level 0 (Nenhum): -1.0
- Level 1 (Mínimo): -0.5
- Level 2 (Sindicatos): +0.5
- Level 3 (Plenos): +1.0

Conservative na mesma escala:
- Level 0: +0.5
- Level 1: +0.3
- Level 2: -0.5
- Level 3: -1.0

> **Curvas em vez de booleanos** permitem que reformas graduais sejam politicamente viáveis (todos toleram +1, ninguém aceita +3 de uma vez).

---

## 5. Governos — O Container Institucional

### `UGovernmentTypeAsset`

```cpp
UCLASS(BlueprintType)
class UGovernmentTypeAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly) FName GovernmentId;     // "Gov.ConstitutionalMonarchy"
    UPROPERTY(EditDefaultsOnly) FText DisplayName;
    UPROPERTY(EditDefaultsOnly) EGovernmentFamily Family; // Monarchy, Republic, Dictatorship,
                                                          // Theocracy, Council

    // Quem governa?
    UPROPERTY(EditDefaultsOnly) EPoliticalAuthority Authority;

    // Eleições?
    UPROPERTY(EditDefaultsOnly) bool bHasElections;
    UPROPERTY(EditDefaultsOnly) int32 ElectionPeriodYears;

    // Quem vota?
    UPROPERTY(EditDefaultsOnly) FName DefaultSuffrageLawId;

    // Composição obrigatória (quais leis devem estar ativas)
    UPROPERTY(EditDefaultsOnly) TMap<ELawCategory, int32> MinimumLawProgression;
    UPROPERTY(EditDefaultsOnly) TMap<ELawCategory, int32> MaximumLawProgression;

    // Modificadores estruturais
    UPROPERTY(EditDefaultsOnly) FStatModifierSet StructuralModifiers;

    // Geração de capital político
    UPROPERTY(EditDefaultsOnly) float PoliticalCapitalPerMonth;

    // Velocidade de reforma
    UPROPERTY(EditDefaultsOnly) float ReformSpeedMultiplier;
};
```

### Tipos de Governo Vitorianos

| Governo | Família | Eleições | Característica |
|---|---|---|---|
| Monarquia Absoluta | Monarchy | Não | Rei decide; reformas lentas; estabilidade artificial |
| Monarquia Constitucional | Monarchy | Sim, limitadas | Rei + Parlamento; reformas moderadas |
| Monarquia Parlamentar | Monarchy | Sim | Rei figurativo; parlamento decide |
| República Censitária | Republic | Sim, censitárias | Ricos votam |
| República Liberal | Republic | Sim, masculinas | Burguesia governa |
| República Democrática | Republic | Sim, universais | Sufrágio amplo |
| Ditadura | Dictatorship | Não | Estabilidade pela força |
| Junta Militar | Dictatorship | Não | Comandantes governam |
| Estado Revolucionário | Council | Pseudo | Pós-revolta socialista |
| Teocracia | Theocracy | Não | Clero governa |

> Governo **define o framework**. Leis específicas vivem dentro dele. Trocar governo é evento dramático (revolução, golpe, abdicação) — não reforma comum.

---

## 6. Partidos e Composição Parlamentar

### `UPoliticalParty`

```cpp
UCLASS()
class UPoliticalParty : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY() FName PartyId;
    UPROPERTY() FText DisplayName;
    UPROPERTY() FName PrimaryIdeologyId;
    UPROPERTY() TMap<FName, float> SecondaryIdeologies; // composição de coalizão

    UPROPERTY() float PopularSupport;       // % votos esperados
    UPROPERTY() float SeatShare;            // % cadeiras no parlamento atual
    UPROPERTY() bool bInGovernment;         // partido governista?
    UPROPERTY() float Loyalty;              // alinhado com líder?
    UPROPERTY() TArray<FName> PreferredPolicies; // leis que pressiona
};
```

### Composição do Parlamento

```cpp
struct FParliamentComposition
{
    TArray<UPoliticalParty*> Parties;
    UPoliticalParty* RulingParty;
    TArray<UPoliticalParty*> Coalition;
    int32 TotalSeats;

    bool HasMajority() const;
    float ComputeLawSupport(ULawAsset* Law) const;
};
```

Composição é **derivada** dos POPs e da `SuffrageLaw` ativa:

```cpp
void UPoliticsSubsystem::RecomputeParliament(UNation* N)
{
    TMap<FName, float> IdeologyVotes;

    for (UProvince* P : N->Provinces)
    {
        for (FPopGroup& Pop : P->Economy->Pops)
        {
            float VotingWeight = ComputeVotingWeight(Pop, N->ActiveSuffrageLaw);
            if (VotingWeight <= 0) continue;

            for (auto& Pair : Pop.IdeologyDistribution)
                IdeologyVotes.FindOrAdd(Pair.Key) += Pair.Value * VotingWeight;
        }
    }

    DistributeSeats(N, IdeologyVotes);
    DetermineRulingParty(N);
}
```

```cpp
float ComputeVotingWeight(const FPopGroup& Pop, ULawAsset* SuffrageLaw)
{
    switch (SuffrageLaw->ProgressionLevel)
    {
    case 0: return 0;                                   // Nenhum
    case 1: return Pop.Type == Aristocrat ? Pop.Size : 0;  // Aristocracia
    case 2: return Pop.Wealth > Threshold ? Pop.Size : 0;  // Censitário
    case 3: return IsAdultMale(Pop) ? Pop.Size : 0;        // Universal Masculino
    case 4: return Pop.Size;                               // Universal
    }
}
```

> **Aqui mora muita política emergente**: aprovar Sufrágio Universal Masculino faz Workers explodirem em representação. Aristocracia pode ainda ter maioria via Lordes (House of Lords) → fricção entre câmaras.

---

## 7. Capital Político — A Moeda das Reformas

```cpp
USTRUCT()
struct FPoliticalCapital
{
    float Current;
    float MaxCapacity;          // teto (governos democráticos têm mais)
    float MonthlyGain;          // base + bônus
};
```

### Geração

```cpp
float UPoliticsSubsystem::ComputePoliticalCapitalGain(UNation* N)
{
    float Base = N->Government->PoliticalCapitalPerMonth;
    float Bonus = 0;

    Bonus += N->RulingParty->Loyalty * 0.5f;
    Bonus += (1.0f - GetAverageMilitancy(N)) * 0.3f;     // calma gera capital
    Bonus += N->Treasury->Stability * 0.2f;

    if (N->IsAtWar()) Bonus -= 0.3f;
    if (HasActiveCrisis(N)) Bonus -= 0.5f;

    return FMath::Max(0.0f, Base + Bonus);
}
```

### Gasto

| Ação | Custo aproximado |
|---|---|
| Propor reforma de lei | 50–200 PC |
| Convocar eleição antecipada | 100 PC |
| Reprimir revolta | 80 PC |
| Mudar partido governista (sem eleição) | 150 PC |
| Propor mudança de governo | 500+ PC |
| Conceder demanda de facção | 30 PC |
| Vetar reforma popular | 100 PC + queda de aprovação |

> **Capital político escasso é o que torna decisões dolorosas.** Sem isso, jogador aprova tudo e a fantasia desaba.

---

## 8. Reform Engine — O Pipeline de Aprovação

```cpp
class UReformEngine : public UObject
{
public:
    EReformResult ProposeReform(UNation* N, ULawAsset* NewLaw);
    void TickActiveReforms();
};
```

### Pipeline

```
ProposeReform(NewLaw)
  │
  ├─ ValidatePrerequisites           [tech, governo, leis]
  ├─ ChargePoliticalCapital
  ├─ ComputeParliamentSupport        [usa preferências de partidos]
  │     ├─ Maioria simples → aprovado direto
  │     ├─ Minoria forte → debate (rolls diários)
  │     └─ Oposição esmagadora → rejeitado
  ├─ EnactmentPeriod (EnactmentDays)
  │     └─ Eventos podem disparar (apoio crescendo, oposição organizando)
  ├─ Aplicar Lei → emitir OnLawPassed
  │     ├─ Atualizar Modifiers
  │     ├─ Atualizar Unlocks
  │     └─ Recompute Militancy de POPs afetados
  └─ Possível backlash (POPs perdedores ganham militância)
```

### Resultado de Reforma

```cpp
USTRUCT()
struct FReformProposal
{
    ULawAsset* NewLaw;
    int64 ProposedAtTick;
    int64 EnactsAtTick;
    EReformPhase Phase;     // Debating, Enacting, Passed, Failed
    float CurrentSupport;    // 0..1
    TArray<FFactionStance> FactionStances;
};
```

> Reformas **não são instantâneas**. Levam de 30 a 365 dias entre propor e aplicar — janela onde eventos narrativos enriquecem (greves, marchas, debates, atentados).

---

## 9. Eleições

`UElectionEngine` dispara conforme `Government->ElectionPeriodYears` ou via `EarlyElection` event.

### Pipeline

```
ElectionTriggered
  │
  ├─ Campaign Phase (60-90 dias)
  │     ├─ Eventos disparam: debates, escândalos, tentativas de fraude
  │     └─ Jogador pode usar PC para apoiar partidos
  │
  ├─ Voting Phase (1 dia simulado)
  │     ├─ ComputeVotingWeight para cada POP
  │     ├─ Distribuir votos por ideologia
  │     └─ Aplicar fraude/coerção se governo autoritário
  │
  ├─ Result Phase
  │     ├─ Recompute Parliament
  │     ├─ Determine Ruling Party
  │     └─ Decision Event "Resultado das Eleições"
  │
  └─ Possíveis follow-ups
        ├─ Coalizão necessária
        ├─ Hung parliament → instabilidade
        └─ Vitória esmagadora → bônus de PC
```

### Fraude e Coerção

Governos autoritários podem rolar fraude:

```cpp
void ApplyElectoralManipulation(UNation* N, TMap<FName, float>& Votes)
{
    float Manipulation = N->Government->Family == EGovernmentFamily::Dictatorship ? 0.4f : 0.0f;
    Manipulation -= N->FreedomOfPress->ProgressionLevel * 0.1f;

    // Inflar voto do partido governista, reduzir oposição
    Votes[N->RulingParty->PartyId] += Manipulation * TotalVotes;
    NormalizeVotes(Votes);

    // Risco: se descoberto, dispara evento "Fraude Eleitoral" → revolta
    if (URandomSubsystem::Roll(Manipulation * 0.3f))
        EventBus->TriggerEvent("Event.ElectionFraud.Discovered", N);
}
```

---

## 10. Facções e Movimentos

Facções são **agrupamentos políticos com agenda específica**, mais granulares que partidos. Podem ser internas ao governo (cortesãos, generais, igreja) ou externas (movimentos sociais).

```cpp
UCLASS()
class UFaction : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY() FName FactionId;
    UPROPERTY() EFactionType Type;          // Court, Military, Religious, Industrial,
                                            // Labor, Nationalist, Colonial, Reformist
    UPROPERTY() float Influence;            // 0..1, peso político
    UPROPERTY() float Loyalty;              // -1..1, alinhamento com governo
    UPROPERTY() TArray<FFactionDemand> ActiveDemands;
    UPROPERTY() TArray<FName> SupportedLaws;
    UPROPERTY() TArray<FName> OpposedLaws;
};

USTRUCT()
struct FFactionDemand
{
    EFactionDemandType Type;       // PassLaw, FireMinister, DeclareWar, GrantConcession,
                                   // RepressMovement, IncreaseBudget
    FName TargetId;                // lei, ministro, etc
    int32 ExpiresOnTick;
    float UrgencyMultiplier;
};
```

### Loyalty e Coups

Quando facção militar tem **Influence alto + Loyalty muito baixo**, dispara `UEvent.MilitaryCoup`. Igualmente: facções industriais perdendo paciência podem **financiar partidos opostos**, facções coloniais podem **declarar autonomia**.

```cpp
void UPoliticsSubsystem::CheckFactionStability(UNation* N)
{
    for (UFaction* F : N->Factions)
    {
        if (F->Loyalty < -0.7f && F->Influence > 0.5f)
        {
            float CoupChance = (-F->Loyalty) * F->Influence * 0.05f;
            if (URandomSubsystem::Roll(CoupChance))
                TriggerCoupEvent(N, F);
        }
    }
}
```

---

## 11. Militância e Revolta — `URevoltEngine`

Militância é calculada por POP, agregada por província, e por nação.

### Cálculo de Militância

```cpp
void UPoliticsSubsystem::UpdateMilitancy(FPopGroup& Pop, UProvince* P, UNation* N)
{
    float Delta = 0;

    // Necessidades não atendidas
    Delta += (1.0f - Pop.NeedsSatisfaction[Life])     * 0.10f;
    Delta += (1.0f - Pop.NeedsSatisfaction[Everyday]) * 0.05f;
    Delta += (1.0f - Pop.NeedsSatisfaction[Luxury])   * 0.02f;

    // Leis contrárias à ideologia dominante do POP
    for (auto& Pair : Pop.IdeologyDistribution)
    {
        UIdeologyAsset* Ideo = ResolveIdeology(Pair.Key);
        for (auto& LawPair : N->ActiveLaws)
        {
            float Pref = ComputePreference(Ideo, LawPair.Value);
            if (Pref < 0)
                Delta += (-Pref) * Pair.Value * 0.03f;
        }
    }

    // Repressão recente alivia mas só temporariamente
    Delta -= GetActiveRepressionModifier(P) * 0.05f;

    // Conscrição em guerra
    if (N->IsAtWar() && N->ConscriptionLaw->ProgressionLevel >= 3)
        Delta += 0.02f * Pop.Size / 1000.0f;

    // Eventos religiosos, culturais, modificadores ativos
    Delta += ModifierRegistry->QueryMilitancyDelta(P, Pop);

    Pop.Militancy = FMath::Clamp(Pop.Militancy + Delta * MonthsPassed, 0.0f, 1.0f);
}
```

### Detecção de Revolta

```cpp
void URevoltEngine::CheckRevolts(UNation* N)
{
    for (UProvince* P : N->Provinces)
    {
        float AvgMilitancy = ComputeAverageMilitancy(P);
        if (AvgMilitancy < RevoltThreshold) continue;

        // Identificar ideologia dominante entre POPs militantes
        FName RevoltIdeology = FindDominantMilitantIdeology(P);

        // Calcular força da revolta
        float RebelStrength = ComputeRebelStrength(P, RevoltIdeology);

        // Decision Event para o jogador, ou IA decide direto
        if (RebelStrength > MinRevoltStrength)
            FireRevolt(P, RevoltIdeology, RebelStrength);
    }
}
```

### Tipos de Revolta

| Tipo | Ideologia | Demanda |
|---|---|---|
| Camponesa | Reactionary | Reverter modernização |
| Liberal | Liberal | Reformas constitucionais |
| Socialista | Socialist | Direitos trabalhistas, redistribuição |
| Comunista | Communist | Mudança total de governo |
| Nacionalista | Nationalist | Independência regional |
| Religiosa | Theocratic | Reverter secularização |
| Colonial | Independência | Autonomia/independência colonial |
| Anarquista | Anarchist | Destruir estado |

### Disparo

`URevoltEngine` cria `UArmy` rebelde via `UMilitarySubsystem`, com unidades fracas mas em quantidade. Ocupa províncias, ameaça capital. Resolução: derrota militar **OU** concessões políticas (passar leis exigidas).

> **Cíclo virtuoso**: revolta socialista derrotada militarmente sem concessões = militância sobe, próxima revolta vem mais forte. Concessões parciais reduzem militância mas alienam Conservatives. Não há saída fácil.

---

## 12. Repressão vs Concessão — Trade-off Central

Quando militância sobe, jogador tem opções:

### Repressão
```cpp
void RepressMovement(UNation* N, UProvince* P, ERepressionLevel Level)
{
    int32 Cost = 30 + Level * 50;  // PC
    N->SpendPoliticalCapital(Cost);

    for (FPopGroup& Pop : P->Economy->Pops)
    {
        if (Pop.IsMilitant())
        {
            Pop.Militancy *= (1.0f - 0.2f * Level);  // alívio temporário
            Pop.Wealth *= (1.0f - 0.05f * Level);    // confiscos
            // Mas militância dos não-militantes sobe (medo + indignação)
            for (FPopGroup& Other : P->Economy->Pops)
                if (!Other.IsMilitant())
                    Other.Militancy += 0.05f * Level;
        }
    }

    // Custo internacional: outras nações reprovam
    DiplomacySubsystem->AdjustGlobalOpinion(N, -5 * Level);
    EventBus->TriggerEvent("Event.RepressionUsed", N, Level);
}
```

### Concessão
Passar uma das leis demandadas pela facção militante. Custa PC mas reduz militância estruturalmente. Aliena facções opostas.

> **Não existe solução universal**: é o que torna política interessante.

---

## 13. IA Política — `UPoliticalAIResolver`

Para nações IA, decide:
- Qual reforma propor
- Quando convocar eleição antecipada
- Quando reprimir vs conceder
- Como reagir a demandas de facções

```cpp
class UPoliticalAIResolver
{
public:
    void DecideMonthlyActions(UNation* N);
private:
    EPoliticalAction ScoreActions(UNation* N);
    ULawAsset* SelectReformProposal(UNation* N);
};
```

### Heurísticas

```cpp
EPoliticalAction UPoliticalAIResolver::ScoreActions(UNation* N)
{
    float StabilityScore = 1.0f - GetAverageMilitancy(N);
    float CapitalScore   = N->PoliticalCapital.Current / 200.0f;

    if (StabilityScore < 0.3f)
        return EPoliticalAction::AddressUnrest;  // reprimir ou conceder

    if (CapitalScore > 0.7f && N->RulingParty->IdeologicalGap > 0.3f)
        return EPoliticalAction::ProposeReform;

    if (N->Government->bHasElections && YearsSinceLastElection(N) > 0.8f * Period)
        return EPoliticalAction::PrepareElection;

    return EPoliticalAction::Maintain;
}
```

> IA política deve sentir **personalidade** vinda do líder e da ideologia dominante. Use `UNationLeaderProfile` para modular pesos (líder agressivo reprime mais; líder reformista concede mais).

---

## 14. Eventos Emitidos

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLawPassed,        int32, ULawAsset*);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGovernmentChanged,int32, UGovernmentTypeAsset*);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FOnElectionResult,   const FElectionResult&);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FOnRevoltStarted,    const FRevoltEvent&);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam (FOnRevoltEnded,      const FRevoltOutcome&);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFactionDemand,    int32, const FFactionDemand&);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnIdeologyShift,    int32, FName);
```

UI ouve para painéis de governo. `UEventSubsystem` ouve quase todos para disparar eventos narrativos. `UDiplomacySubsystem` ouve `OnGovernmentChanged` (regimes podem invalidar tratados).

---

## 15. Diagrama Final do `UPoliticsSubsystem`

```
UPoliticsSubsystem (UWorldSubsystem)
│
├── DataAssets (estático)
│   ├── UIdeologyAsset[]            [Reactionary → Anarchist + Nationalist + Fascist]
│   ├── ULawAsset[]                 [11 categorias × níveis]
│   ├── UGovernmentTypeAsset[]      [Monarquia → República → Ditadura...]
│   └── UFactionTemplateAsset[]     [Court, Military, Religious, Labor...]
│
├── Estado por nação (no save)
│   ├── ActiveGovernment
│   ├── ActiveLaws (por categoria)
│   ├── PoliticalCapital
│   ├── Parliament (parties + seats)
│   ├── Factions[]
│   ├── ActiveReforms[]
│   └── LastElectionTick
│
├── Estado por POP (no save)
│   ├── IdeologyDistribution
│   └── Militancy
│
├── Engines
│   ├── UReformEngine        [propor → debater → aplicar leis]
│   ├── UElectionEngine      [campanha → voto → resultado]
│   ├── URevoltEngine        [detecção → spawn rebelde → resolução]
│   └── UPoliticalAIResolver [IA decide reformas, repressão, eleições]
│
├── Loop
│   ├── OnMonth → UpdateMilitancy, UpdateIdeologyDrift, GenerateCapital,
│   │             FactionDemands, CheckRevolts
│   ├── OnYear → ElectionTrigger se devido
│   └── OnReformTick → avança ActiveReforms
│
└── Pontes
    ├── ↔ UEconomySubsystem    (taxação, comércio, trabalho)
    ├── ↔ UMilitarySubsystem   (conscrição, gasto militar, golpes)
    ├── ↔ UDiplomacySubsystem  (governo afeta opções diplomáticas)
    ├── ↔ UProgressSubsystem   (techs liberam leis)
    ├── ↔ UEventSubsystem      (gera + consome eventos políticos)
    └── ↔ UBattleSubsystem     (doutrinas dependentes de leis)
```

---

## 16. Plano de Implementação

1. **Esqueleto + GovernmentTypeAsset** com 3 governos (Absoluta, Constitucional, República).
2. **`ULawAsset` mínimo**: 3 categorias (Government, Suffrage, Trade), 3 níveis cada.
3. **`UIdeologyAsset`**: 4 ideologias (Reactionary, Conservative, Liberal, Socialist).
4. **POPs ganham `IdeologyDistribution`** + drift mensal.
5. **`PoliticalCapital`**: geração + UI.
6. **`UReformEngine`** v1: propor + aplicar (sem debate ainda).
7. **`OnLawPassed` broadcast** + integrações básicas (Economia, Tech).
8. **Parlamento e partidos**: composição derivada de POPs.
9. **`UElectionEngine`**: ciclo simples, sem fraude.
10. **`URevoltEngine`** v1: militância → spawn rebelde via Military.
11. **Curvas de preferência completas** + reação política a leis.
12. **Facções** + demandas + lealdade.
13. **Repressão vs concessão** com trade-offs.
14. **`UPoliticalAIResolver`**.
15. **Fraude eleitoral** + eventos correlacionados.
16. **Coups, golpes e mudanças não-eleitorais de governo**.
17. **Polish**: UI de parlamento, painel de leis, alertas de militância.
