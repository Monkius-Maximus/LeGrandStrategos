# 41 — UMilitarySubsystem

Sistema militar estratégico: ponte entre o mundo macro (mapa, exércitos, suprimento, frentes) e o tático (`UBattleSubsystem`). Cobre recrutamento, movimento, logística, cerco, ocupação, mobilização e operações navais.

---

## 1. Princípios Arquiteturais

### Militar é a ponte entre Estratégia e Tática

```
Estratégia (Director, Diplomacia, Política, Economia)
            │
            ▼
   UMilitarySubsystem
   ├─ recrutamento
   ├─ posicionamento
   ├─ movimento
   ├─ suprimento
   ├─ engajamento (cria Battles)
   └─ ocupação
            │
            ▼
   UBattleSubsystem (tática)
```

### Não duplicar combate

`UMilitarySubsystem` **não resolve combate**. Ele decide **se há combate**, **quem**, **onde**, e delega para `UBattleSubsystem` ou `UBattleResolverService`.

### Suprimento é sistema, não detalhe

Em jogos pobres, suprimento é "atrito mensal". Em grand strategy maduro, é uma rede logística com gargalos, vulnerabilidades, e pontos de decisão estratégica.

### Frota como caso especial

Marinha tem regras próprias: rotas, portos, projeção, bloqueio. Tratar igual a exército terrestre é erro.

---

## 2. Estrutura Hierárquica

```
UMilitarySubsystem
   ├── ArmyRegistry              [todas as UArmy do mundo]
   ├── FleetRegistry             [todas as UFleet]
   ├── RecruitmentEngine         [conscrição, voluntariado, mercenários]
   ├── MovementEngine            [pathfinding estratégico, movimento por dia]
   ├── SupplyNetwork             [grafo logístico nacional]
   ├── EngagementDetector        [identifica encontros, propõe batalhas]
   ├── SiegeEngine               [cercos a fortificações]
   ├── OccupationEngine          [controle de províncias inimigas]
   ├── MobilizationEngine        [transição paz↔guerra]
   └── MilitaryAIResolver        [executa propostas do MilitaryAdvisor]
```

---

## 3. `UArmy` — A Pilha Estratégica

```cpp
UCLASS()
class UArmy : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY() FGuid ArmyId;
    UPROPERTY() int32 OwnerNationId;
    UPROPERTY() FText DisplayName;          // "1ª Brigada do Reno"

    // Localização
    UPROPERTY() int32 LocationProvinceId;
    UPROPERTY() int32 TargetProvinceId;     // 0 se parado
    UPROPERTY() float MovementProgress;     // 0..1, percurso atual
    UPROPERTY() TArray<int32> PlannedPath;  // próximas províncias

    // Composição
    UPROPERTY() TArray<URegiment*> Regiments;
    UPROPERTY() TWeakObjectPtr<UCommander> Commander;

    // Estado
    UPROPERTY() float Morale;                // moral agregada
    UPROPERTY() float Supply;                // 0..100, atual
    UPROPERTY() float Cohesion;              // 0..1
    UPROPERTY() EArmyPosture Posture;        // Defend, Skirmish, Assault, Raid, Garrison
    UPROPERTY() EArmyStance Stance;          // Aggressive, Cautious, Evasive

    // Estado tático
    UPROPERTY() bool bIsEngaged;
    UPROPERTY() FGuid CurrentBattleId;
    UPROPERTY() bool bIsBesieging;
    UPROPERTY() int32 BesiegingProvinceId;

    // Logística
    UPROPERTY() int32 SupplyHubProvinceId;   // de onde puxa suprimento
    UPROPERTY() FSupplyRoute ActiveSupplyRoute;
};
```

### `EArmyPosture` vs `EArmyStance`

- **Posture**: o que faz quando engaja (Defend = entrincheirar, Assault = atacar)
- **Stance**: como reage a movimento inimigo (Aggressive = persegue, Evasive = recua)

> Ambos são **dicas para o Battle**, não para o movimento. Movimento é decidido pelo jogador/IA via `MovementEngine`.

---

## 4. `URegiment` — Unidade Componente

Já bem definido pelo sistema de unidades ([`11-units.md`](11-units.md)). Aqui só recapitulando o ciclo de vida:

```cpp
URegiment
   ├── ClassRef → UUnitClassAsset
   ├── EquippedItems → UEquipmentAsset[]
   ├── Experience → FRegimentExperience
   ├── CurrentStrength (perdas em batalha)
   ├── MaxStrength
   └── (no engajamento) FRegimentRuntimeProfile resolvido
```

### Reforço e Reposição

Regimentos perdem força em batalha. Reforço acontece em províncias amigas com pop disponível:

```cpp
void URegiment::Replenish(UProvince* HomeProvince, float DaysPassed)
{
    if (CurrentStrength >= MaxStrength) return;
    if (HomeProvince->Owner != OwnerNation) return;  // só em casa

    float Available = HomeProvince->GetMilitaryPopAvailable();
    float Needed = MaxStrength - CurrentStrength;
    float Speed = ReplenishSpeedPerDay * DaysPassed;

    float Replenished = FMath::Min({Available, Needed, Speed});
    CurrentStrength += Replenished;
    HomeProvince->ConsumeMilitaryPop(Replenished);
}
```

---

## 5. `UFleet` — Forças Navais

Análogo a UArmy, mas com regras navais:

```cpp
UCLASS()
class UFleet : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY() FGuid FleetId;
    UPROPERTY() int32 OwnerNationId;

    UPROPERTY() int32 LocationSeaZoneId;     // zona marítima, não província
    UPROPERTY() int32 HomePortProvinceId;
    UPROPERTY() int32 TargetSeaZoneId;

    UPROPERTY() TArray<URegiment*> Ships;    // navios são "regimentos navais"
    UPROPERTY() UCommander* Admiral;

    UPROPERTY() float Morale;
    UPROPERTY() float Supply;
    UPROPERTY() float HullIntegrity;          // dano acumulado, repara em porto

    UPROPERTY() EFleetMission Mission;        // Patrol, Blockade, Escort, Raid, Transport
    UPROPERTY() TArray<FGuid> EmbarkedArmies; // exércitos a bordo

    UPROPERTY() TArray<int32> BlockadedProvinces;
};
```

### Zonas Marítimas vs Províncias

```cpp
USTRUCT()
struct FSeaZone
{
    int32 ZoneId;
    FText Name;                              // "Mar do Norte", "Estreito de Gibraltar"
    TArray<int32> CoastalProvinces;          // províncias adjacentes
    TArray<int32> AdjacentSeaZones;          // navegação entre zonas
    ESeaZoneType Type;                       // Coastal, Open, Strait, Polar
    float StormFrequency;                    // afeta operações
};
```

> Mar não é uma "província especial". É um **grafo paralelo** com regras próprias. Ataques navais ocorrem em SeaZones, bloqueios afetam províncias costeiras.

---

## 6. Recrutamento — `URecruitmentEngine`

```cpp
class URecruitmentEngine
{
public:
    URegiment* RecruitRegiment(UNation* N, UProvince* P,
                                UUnitClassAsset* Class,
                                TArray<UEquipmentAsset*> Equipment);
    void ProcessConscription(UNation* N);
    void DrainSoldierPops(UNation* N);
};
```

### Regras

- Recrutamento puxa de **POP do tipo Soldier** (ou Worker em emergência)
- Lei de conscrição limita ritmo:

| Lei | Limite |
|---|---|
| Profissional | 0.5% da pop por ano |
| Voluntário | 1% por ano |
| Conscrição Limitada | 2% por ano + flag de guerra |
| Conscrição Universal | 5% por ano em guerra |

- Equipamento **deve estar disponível** no estoque nacional (`UEconomySubsystem`)
- Custo de manutenção mensal vai para `UNationTreasury`

### Mercenários

Empresas mercenárias são **nações neutras** que vendem regimentos prontos. Caro, sem lealdade duradoura, mas instantâneo.

```cpp
URegiment* HireMercenary(UNation* N, UMercenaryCompany* Company);
```

---

## 7. Movimento — `UMovementEngine`

### Pathfinding Estratégico

```cpp
class UMovementEngine
{
public:
    TArray<int32> FindPath(int32 FromProvinceId, int32 ToProvinceId,
                            const FArmyContext& Ctx);
    void TickMovement(UArmy* A, float DaysPassed);
};
```

A* sobre grafo de províncias, com pesos:

```cpp
float ComputeEdgeCost(int32 FromId, int32 ToId, const FArmyContext& Ctx)
{
    float Cost = BaseDays(FromId, ToId);
    Cost *= TerrainMultiplier(GetTerrain(ToId));
    Cost *= WeatherMultiplier(GetWeather(ToId));
    Cost /= (1.0f + Ctx.Commander->Logistics * 0.3f);
    Cost *= InfrastructureMultiplier(GetInfra(ToId));   // ferrovia reduz

    if (IsHostileTerritory(ToId, Ctx.OwnerNation))
        Cost *= 1.5f;
    if (IsBlockedByEnemy(ToId, Ctx.OwnerNation))
        return INFINITY;

    return Cost;
}
```

### Movimento por Tick

```cpp
void TickMovement(UArmy* A, float DaysPassed)
{
    if (A->TargetProvinceId == 0) return;
    if (A->bIsEngaged) return;  // engajado, não move

    float DaysToReach = ComputeEdgeCost(A->LocationProvinceId,
                                         A->TargetProvinceId,
                                         BuildContext(A));
    A->MovementProgress += DaysPassed / DaysToReach;

    if (A->MovementProgress >= 1.0f)
    {
        A->LocationProvinceId = A->TargetProvinceId;
        A->MovementProgress = 0;
        AdvancePath(A);
        EngagementDetector->CheckOnArrival(A);
    }
}
```

### Ferrovias — Movimento Acelerado

```cpp
bool CanUseRail(UArmy* A, int32 FromId, int32 ToId)
{
    return HasRail(FromId) && HasRail(ToId)
        && IsConnectedRail(FromId, ToId, A->OwnerNationId)
        && !A->bIsEngaged;
}

float RailMovementSpeed = 5x base movement;
```

> Ferrovia muda profundamente o jogo. Antes da era do vapor, mover exército leva semanas. Depois, dias. **Conectar fronteira ferroviariamente é decisão estratégica de peso**.

### Travessias Marítimas

Embarcar exército em frota de transporte:

```cpp
bool EmbarkArmy(UArmy* A, UFleet* F)
{
    if (!A->LocationProvince->IsCoastal()) return false;
    if (F->LocationSeaZone != GetSeaZoneFor(A->LocationProvince)) return false;
    if (F->TransportCapacity < A->TotalSize()) return false;

    F->EmbarkedArmies.Add(A->ArmyId);
    A->bIsEmbarked = true;
    return true;
}
```

---

## 8. Suprimento — `USupplyNetwork`

Sistema crítico e complexo. Inspirado em Hearts of Iron + Vic3.

### Conceito

Cada nação tem **Supply Hubs** (cidades, capitais, portos) que **emanam capacidade logística**. Exércitos consomem suprimento conforme tamanho. Suprimento flui pelo **grafo de infraestrutura**.

```cpp
class USupplyNetwork
{
public:
    void RecomputeForNation(UNation* N);
    float GetSupplyAt(int32 ProvinceId, int32 NationId) const;
    bool CanSupplyArmy(UArmy* A) const;
    void TickSupplyConsumption();
};
```

### Cálculo

```cpp
struct FSupplyHub
{
    int32 ProvinceId;
    float Capacity;            // baseado em prédios, infraestrutura, economia local
    float Range;               // distância máxima alcançada
};

float ComputeSupplyAt(int32 ProvinceId, int32 NationId)
{
    float MaxSupply = 0;
    for (FSupplyHub Hub : GetHubsFor(NationId))
    {
        float Distance = ProvinceDistance(Hub.ProvinceId, ProvinceId);
        if (Distance > Hub.Range) continue;

        float Decay = FMath::Pow(0.85f, Distance);
        float Throughput = Hub.Capacity * Decay;
        Throughput *= InfrastructureMultiplier(Path);

        MaxSupply = FMath::Max(MaxSupply, Throughput);
    }

    // Fora do território nacional: foraging
    if (!IsOwnedBy(ProvinceId, NationId))
        MaxSupply = FMath::Min(MaxSupply, ForagingCapacity(ProvinceId));

    return MaxSupply;
}
```

### Consumo

```cpp
void USupplyNetwork::TickSupplyConsumption()
{
    for (UArmy* A : AllArmies)
    {
        float Demand = A->ComputeSupplyDemand();   // baseado em regimentos
        float Available = ComputeSupplyAt(A->LocationProvinceId, A->OwnerNationId);

        if (Available >= Demand)
        {
            A->Supply = FMath::Min(100.0f, A->Supply + 5.0f);
        }
        else
        {
            float Deficit = (Demand - Available) / Demand;
            A->Supply = FMath::Max(0.0f, A->Supply - Deficit * 10.0f);

            if (A->Supply < 30.0f)
                ApplyAttrition(A, Deficit);
        }
    }
}
```

### Atrição

Exército sem suprimento perde força e moral:

```cpp
void ApplyAttrition(UArmy* A, float Severity)
{
    for (URegiment* R : A->Regiments)
        R->CurrentStrength *= (1.0f - 0.005f * Severity);  // perdas diárias
    A->Morale -= 0.5f * Severity;
}
```

> Sitiar exército inimigo cortando rotas é estratégia válida. Conquistar **Supply Hub** crítico vira objetivo de campanha.

---

## 9. Engajamento — `UEngagementDetector`

Conecta movimento estratégico ao `UBattleSubsystem`.

```cpp
class UEngagementDetector
{
public:
    void CheckOnArrival(UArmy* A);
    void CheckOnTickAll();          // varre fronteiras
    void TriggerEngagement(UArmy* A, UArmy* B, EBattleType Type);
};
```

### Lógica

```cpp
void CheckOnArrival(UArmy* A)
{
    int32 LocId = A->LocationProvinceId;

    // 1. Mesma província: clash direto
    for (UArmy* Other : GetArmiesAt(LocId))
    {
        if (IsHostile(A, Other))
        {
            TriggerEngagement(A, Other, EBattleType::DirectClash);
            return;
        }
    }

    // 2. Província é hostil e tem garnição inimiga: ataque
    UProvince* P = GetProvince(LocId);
    if (IsHostile(A->OwnerNation, P->OwnerNation) && P->HasGarrison())
    {
        TriggerEngagement(A, P->Garrison, EBattleType::Assault);
        return;
    }

    // 3. Adjacente com posture compatível: skirmish
    for (int32 NeighborId : P->Neighbors)
    {
        for (UArmy* Other : GetArmiesAt(NeighborId))
        {
            if (IsHostile(A, Other) && BothAllowSkirmish(A, Other))
                TriggerEngagement(A, Other, EBattleType::BorderSkirmish);
        }
    }

    // 4. Província sem garnição inimiga: ocupação (não batalha)
    if (IsHostile(A->OwnerNation, P->OwnerNation) && !P->HasGarrison())
    {
        OccupationEngine->StartOccupation(A, P);
    }
}
```

---

## 10. Cerco — `USiegeEngine`

```cpp
class USiegeEngine
{
public:
    void StartSiege(UArmy* Besieger, UProvince* Target);
    void TickActiveSieges();
    void ResolveSiege(FActiveSiege& Siege);
};

USTRUCT()
struct FActiveSiege
{
    int32 ProvinceId;
    FGuid BesiegerArmyId;
    int32 FortLevel;
    float Progress;             // 0..1
    int32 DaysSieged;
    bool bRelief;               // chegou exército de socorro?
};
```

### Mecânica

- Cerco progride conforme: `Besieger.Strength × Time × Artillery / FortLevel`
- Defensor pode resistir indefinidamente se tem suprimento
- Exército de socorro pode forçar batalha em campo
- Provincia tomada por cerco vira "ocupada" (não anexada — depende do tratado de paz)

```cpp
void TickActiveSieges()
{
    for (FActiveSiege& S : ActiveSieges)
    {
        UArmy* B = ResolveArmy(S.BesiegerArmyId);
        if (!B || B->bIsEngaged) continue;

        float ProgressPerDay = ComputeSiegeProgress(B, S.FortLevel);
        S.Progress += ProgressPerDay;

        if (S.Progress >= 1.0f)
            ResolveSiege(S);
    }
}
```

---

## 11. Ocupação — `UOccupationEngine`

```cpp
class UOccupationEngine
{
public:
    void StartOccupation(UArmy* A, UProvince* P);
    void TickOccupations();
    void TransferProvinceControl(UProvince* P, int32 NewControllerNation);
};
```

### Regras

- Exército entra em província inimiga sem garnição → ocupação começa
- Ocupação leva dias (varia com hostilidade da pop, fortificação)
- Província ocupada: economia para, recursos param de fluir para owner original
- Ocupação **não muda dono**, só controle. Anexação só com tratado de paz.

```cpp
void TickOccupations()
{
    for (FActiveOccupation& O : ActiveOccupations)
    {
        UArmy* A = ResolveArmy(O.OccupierArmyId);
        if (!A || A->LocationProvinceId != O.ProvinceId) continue;

        O.Progress += ComputeOccupationSpeed(A, GetProvince(O.ProvinceId));
        if (O.Progress >= 1.0f)
        {
            TransferProvinceControl(GetProvince(O.ProvinceId), A->OwnerNationId);
            ActiveOccupations.Remove(O);
        }
    }
}
```

---

## 12. Mobilização — `UMobilizationEngine`

Transição entre exército de paz e exército de guerra.

```cpp
class UMobilizationEngine
{
public:
    void OrderMobilization(UNation* N);
    void OrderDemobilization(UNation* N);
    void TickMobilization();
};
```

### Fases

```
Peacetime → PartialMobilization → FullMobilization
              (30 dias)              (60 dias)
```

- **PartialMobilization**: chama reservistas; +50% força em ~30 dias
- **FullMobilization**: chama todos elegíveis; +200% força em ~60 dias; economia sofre

### Custos

- Mobilização total para economia (workers vão para o exército)
- POPs em produção caem → indústria perde throughput
- Custo direto de equipamento + uniformes
- **Não pode ser revertida instantaneamente**: desmobilização também leva tempo

> Mobilização força **decisão estratégica antecipada**. Mobilizar tarde = perder primeiras batalhas. Mobilizar cedo = inimigo vê e reage diplomaticamente.

---

## 13. Frontes e Coordenação

Em guerras grandes, gerenciar 20 exércitos individualmente é tedioso. Solução: **frontes**.

```cpp
USTRUCT()
struct FMilitaryFront
{
    FGuid FrontId;
    int32 OwnerNationId;
    FText Name;                    // "Frente Ocidental"
    TArray<int32> ProvincesCovered;
    TArray<FGuid> AssignedArmies;
    UCommander* CommandingGeneral; // pode ser hierarquia
    EFrontStrategy Strategy;       // Defensive, Offensive, Holding
};
```

Jogador (ou AI) atribui exércitos a frentes; **MilitaryAdvisor** ou jogador define estratégia da frente; exércitos individuais podem ser microgerenciados ou seguir IA local.

> Sem frentes, jogo grande vira micromanagement infernal.

---

## 14. Eventos Emitidos

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnArmyRecruited,    UArmy*);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnArmyDestroyed,    FGuid);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnArmyMoved,       FGuid, int32);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSiegeStarted,     const FActiveSiege&);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSiegeEnded,       const FSiegeOutcome&);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnProvinceOccupied,int32, int32);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMobilization,     int32);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSupplyShortage,   FGuid);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNavalBlockade,    int32);
```

`UEventSubsystem` consome quase todos para narrativas. `UEconomySubsystem` ouve `OnNavalBlockade` para invalidar rotas.

---

## 15. Diagrama Final

```
UMilitarySubsystem (UWorldSubsystem)
│
├── Registries
│   ├── ArmyRegistry[]
│   └── FleetRegistry[]
│
├── Engines
│   ├── URecruitmentEngine     [conscrição, voluntariado, mercenários]
│   ├── UMovementEngine        [pathfinding A*, ferrovia, transporte naval]
│   ├── USupplyNetwork         [hubs, fluxo, decay, atrição]
│   ├── UEngagementDetector    [propõe batalhas via UBattleSubsystem]
│   ├── USiegeEngine           [cercos a provincias fortificadas]
│   ├── UOccupationEngine      [controle de provincias inimigas]
│   ├── UMobilizationEngine    [Peacetime → Full]
│   └── UMilitaryAIResolver    [executa propostas do MilitaryAdvisor]
│
├── Frentes
│   └── FMilitaryFront[]       [agrupamento estratégico]
│
├── Loop
│   ├── OnDay → movimento, suprimento, atrição, cerco, ocupação
│   ├── OnWeek → recrutamento, replenish, mobilização
│   └── OnMonth → revisão de frentes, deserção em moral baixa
│
└── Pontes
    ├── ↔ UBattleSubsystem      (engajamento → batalha → resultado aplicado)
    ├── ↔ UEconomySubsystem     (custo de manutenção, equipamento, suprimento)
    ├── ↔ UDiplomacySubsystem   (war state, ocupação afeta war score)
    ├── ↔ UPoliticsSubsystem    (mobilização causa militância)
    ├── ↔ UAIDirectorSubsystem  (MilitaryAdvisor define ações)
    ├── ↔ UEventSubsystem       (gera + consome eventos militares)
    └── ↔ UProgressSubsystem    (techs liberam novas táticas, fortificações)
```

---

## 16. Plano de Implementação

1. **Esqueleto** + `UArmy` básico + recrutamento manual.
2. **`UMovementEngine`** com pathfinding A* simples.
3. **`UEngagementDetector`** mesma-província → dispara `UBattleResolverService`.
4. **`USupplyNetwork`** v1: capacidade fixa por província, decay simples.
5. **Atrição** baseada em supply.
6. **`UOccupationEngine`**: província inimiga sem garnição → ocupada após N dias.
7. **`USiegeEngine`**: provincia fortificada precisa cerco.
8. **Adjacência e skirmish** (engajamento sem mesma província).
9. **Ferrovias** aceleram movimento.
10. **`UFleet`** + zonas marítimas + transporte de exército.
11. **Bloqueio naval** integrado com Economia.
12. **`UMobilizationEngine`**: peace → full com fases.
13. **Mercenários**.
14. **Frentes** com estratégia agrupada.
15. **Replenish** baseado em pop civil.
16. **`UMilitaryAIResolver`** integrado ao `UMilitaryAdvisor`.
17. **Polish**: visualização de exércitos, rotas planejadas, suprimento como overlay.

---

## 17. Pontos de Atenção Específicos

- **Suprimento é UI difícil**. Sem visualização clara (overlay no mapa), jogador não entende por que perdeu campanha. Invista em mostrar.
- **Cerco vs Batalha**: separar é crucial. Misturar gera comportamento estranho ("estava cercando, apareceu inimigo, virou batalha automática?")
- **Ocupação ≠ Anexação**. Ensinar isso ao jogador é importante. UI deve diferenciar visualmente.
- **Frentes economizam clique**. Mas devem ser opcionais — jogador hardcore quer microgerenciar.
- **Naval é distinto**. Não force "frota se move como exército". SeaZones, missões, projeção de poder são paradigmas próprios.
- **Mobilização tem custos diplomáticos**. Outras nações vêem mobilização parcial e reagem (recall ambassadors, escalada).
- **Performance**: 200 exércitos × pathfinding por dia = caro. Cache paths até estado mudar; recompute só quando necessário.
- **AI militar deve sentir personalidade**. Bismarck planeja meticulosamente; Napoleão III improvisa. Use `UNationPersonality.MilitaryFocus + Aggression`.
- **Não automatize tudo**. Ofereça assistência (sugestões de movimento, alertas de fronteira) sem tomar decisão.
- **Determinismo no movimento**: ordem de processamento de exércitos pode afetar engajamento. Use IDs estáveis para tie-breaking.
