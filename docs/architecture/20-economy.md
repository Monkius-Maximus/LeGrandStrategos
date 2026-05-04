# 20 — UEconomySubsystem

Sistema econômico baseado em **POPs + Bens + Mercados Regionais**, inspirado em Victoria 3 mas simplificado para ser implementável. A economia é o que conecta todos os outros subsistemas: sustenta o exército, gera tensão política, e cria interdependência diplomática.

---

## 1. Princípios do Modelo Econômico

A economia tem três funções centrais no jogo:

1. **Sustentar o poder militar** — quem produz mais aço e rifles ganha guerras.
2. **Gerar tensão política** — POPs com necessidades não atendidas geram instabilidade.
3. **Conectar as nações** — comércio cria interdependência e alvos diplomáticos.

Para isso, modelamos **quatro elementos fundamentais** que se relacionam:

```
POPs ──demandam──► Bens ──produzidos por──► Indústrias ──empregam──► POPs
  │                  │                          │
  │                  └─precificados em──► Mercados ◄──conectados por──► Rotas
  │
  └──tributados por──► Tesouro Nacional ──financia──► Indústrias / Exército / Pesquisa
```

> ⚠️ **Decisão arquitetural**: economia não simula cada indivíduo. Simula **estratos agregados (POPs)** e **bens em escala industrial**. Resolução: província × tipo de POP × bem.

---

## 2. Estrutura de Dados

### `UPopGroup` — agregação de pessoas

```cpp
USTRUCT()
struct FPopGroup
{
    GENERATED_BODY()

    EPopType Type;             // Aristocrat, Bourgeois, Bureaucrat, Officer, Clergyman,
                               // Engineer, Artisan, Worker, Farmer, Soldier, Slave
    int32 Size;                // número de pessoas (em milhares)
    float Wealth;              // riqueza acumulada do estrato
    float Literacy;            // 0..1
    float PoliticalAwareness;  // 0..1
    float Militancy;           // 0..1, propensão a revolta

    ECultureId Culture;
    EReligionId Religion;
    FName Ideology;            // "Conservative", "Liberal", "Socialist", "Reactionary"

    // Necessidades atendidas (preenchidas pelo tick econômico)
    TMap<EGoodType, float> NeedsSatisfaction;  // 0..1 por categoria
};
```

> **Por que estratos agregados?** Simular 50M de indivíduos é inviável. Mas estratos por província dão granularidade suficiente para política e economia emergente.

### `UProvinceEconomy` — economia local

```cpp
UCLASS()
class UProvinceEconomy : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY() TArray<FPopGroup> Pops;
    UPROPERTY() TArray<UIndustry*> Industries;     // fábricas, fazendas, minas
    UPROPERTY() TMap<EGoodType, float> Stockpile;
    UPROPERTY() TMap<EGoodType, float> LocalProduction;
    UPROPERTY() TMap<EGoodType, float> LocalConsumption;

    UPROPERTY() float Infrastructure;              // 0..1, afeta capacidade de comércio
    UPROPERTY() float Urbanization;                // 0..1, derivado da composição de POPs
    UPROPERTY() int32 MarketRegionId;              // a qual mercado pertence
    UPROPERTY() FProvinceModifiers Modifiers;      // bônus/penalidades ativos
};
```

### `UIndustry` — unidade produtiva

```cpp
UCLASS()
class UIndustry : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY() UIndustryTypeAsset* Type;
    UPROPERTY() int32 Level;                       // 1..N, escala
    UPROPERTY() float Throughput;                  // 0..1, eficiência atual
    UPROPERTY() TMap<EPopType, int32> Workforce;   // empregados por tipo
    UPROPERTY() float ProfitMargin;                // último tick
    UPROPERTY() bool bSubsidized;                  // estado banca prejuízo
    UPROPERTY() EOwnership Ownership;              // Private, State, Foreign
};
```

### `UIndustryTypeAsset` — definição estática (DataAsset)

```cpp
UCLASS(BlueprintType)
class UIndustryTypeAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly) FName IndustryId;        // "Industry.SteelMill"
    UPROPERTY(EditDefaultsOnly) FText DisplayName;
    UPROPERTY(EditDefaultsOnly) EIndustryCategory Category; // Extraction, Heavy, Light, Agriculture, Service

    UPROPERTY(EditDefaultsOnly) TMap<EGoodType, float> InputsPerLevel;
    UPROPERTY(EditDefaultsOnly) TMap<EGoodType, float> OutputsPerLevel;
    UPROPERTY(EditDefaultsOnly) TMap<EPopType, int32>  WorkforcePerLevel;

    UPROPERTY(EditDefaultsOnly) FName RequiredTech;       // só constrói após pesquisa
    UPROPERTY(EditDefaultsOnly) TArray<EProvinceTrait> RequiredTraits; // ex: precisa de "CoalDeposit"
    UPROPERTY(EditDefaultsOnly) FIndustrialCost ConstructionCost;
};
```

> **Reuso**: `FIndustrialCost` é o mesmo struct que `UEquipmentAsset` usa. Economia e militar falam a mesma linguagem.

### `UNationTreasury` — finanças nacionais

```cpp
UCLASS()
class UNationTreasury : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY() float Gold;                          // tesouro líquido
    UPROPERTY() float DebtPrincipal;
    UPROPERTY() float InterestRate;
    UPROPERTY() float CreditRating;                  // 0..1, baseado em histórico

    UPROPERTY() TMap<ETaxCategory, float> TaxRates;  // Aristocrat, Middle, Lower
    UPROPERTY() TMap<EBudgetLine, float> BudgetAllocations; // % do tesouro

    UPROPERTY() FFinancialLedger Ledger;             // últimos 12 meses
};
```

`EBudgetLine`: Military, Navy, Education, Administration, Construction, Subsidies, DebtService, Diplomacy.

---

## 3. Bens — Hierarquia e Categorias

```cpp
UENUM(BlueprintType)
enum class EGoodCategory : uint8
{
    Staple,        // Grain, Fish — comida básica
    Industrial,    // Coal, Iron, Steel, Lumber
    Manufactured,  // Cloth, Furniture, Glass, Paper
    Military,      // Smallarms, Artillery, Ammunition, Warships
    Luxury,        // Wine, Tobacco, Tea, Opium
    Strategic      // Oil, Rubber, Telegraph, Rails (final do período)
};
```

`EGoodType` enumera os bens individuais. Em DataAsset:

```cpp
UCLASS(BlueprintType)
class UGoodAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly) EGoodType GoodId;
    UPROPERTY(EditDefaultsOnly) EGoodCategory Category;
    UPROPERTY(EditDefaultsOnly) float BasePrice;
    UPROPERTY(EditDefaultsOnly) float DemandElasticity;   // -2..-0.1
    UPROPERTY(EditDefaultsOnly) float SupplyElasticity;   // 0.1..2
    UPROPERTY(EditDefaultsOnly) FName RequiredTech;       // alguns só existem após tech
    UPROPERTY(EditDefaultsOnly) bool bStrategic;          // sujeito a embargo prioritário
};
```

### Necessidades dos POPs

Cada `EPopType` tem necessidades em três tiers:

| Tier | Tipo | Efeito se não atendido |
|---|---|---|
| `LifeNeeds` | Comida, água, abrigo | POP morre / migra |
| `EverydayNeeds` | Roupas, móveis, café | Cresce militancy |
| `LuxuryNeeds` | Vinho, joias, tabaco | Cresce militancy + wealth não cresce |

Definidas em DataAsset por tipo de POP:

```cpp
UCLASS()
class UPopNeedsAsset : public UPrimaryDataAsset
{
    UPROPERTY(EditDefaultsOnly) EPopType PopType;
    UPROPERTY(EditDefaultsOnly) TMap<EGoodType, float> LifeNeeds;
    UPROPERTY(EditDefaultsOnly) TMap<EGoodType, float> EverydayNeeds;
    UPROPERTY(EditDefaultsOnly) TMap<EGoodType, float> LuxuryNeeds;
};
```

---

## 4. Mercados Regionais — Não Globais

A escolha mais importante do modelo. **Mercado global = irreal e sem peso espacial.** **Mercado puramente local = não há comércio.** Solução: **mercado por região comercial**.

### `UMarketRegion`

```cpp
UCLASS()
class UMarketRegion : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY() int32 RegionId;
    UPROPERTY() TArray<int32> ProvinceIds;          // províncias conectadas
    UPROPERTY() int32 CapitalProvinceId;            // centro do mercado

    UPROPERTY() TMap<EGoodType, float> Supply;      // soma da produção
    UPROPERTY() TMap<EGoodType, float> Demand;
    UPROPERTY() TMap<EGoodType, float> Price;
    UPROPERTY() TMap<EGoodType, float> Stockpile;   // sobra acumulada

    UPROPERTY() TArray<FTradeRoute> ExternalRoutes; // conexões com outros mercados
};
```

### Como mercados se formam

1. Cada nação tem ao menos 1 mercado (no `Capital`).
2. Províncias se conectam ao mercado pela **rede de infraestrutura** (estradas, ferrovias, portos).
3. Província sem conexão (Infrastructure < threshold ou sem rota) vira **mercado isolado** — preços diferentes, produção desperdiçada.
4. **Vassalos / Esfera de Influência**: mercados podem ser **anexados** ao mercado da metrópole.

> Isso é o que torna **ferrovia** uma decisão estratégica, não cosmética. Conectar uma província rica em ferro ao centro industrial muda o jogo.

### Cálculo de preço

```cpp
void UMarketRegion::ComputePrices()
{
    for (auto& Pair : Demand)
    {
        EGoodType Good = Pair.Key;
        float D = Pair.Value;
        float S = Supply.FindRef(Good) + Stockpile.FindRef(Good) * 0.3f;

        float Base = GoodAssets[Good]->BasePrice;
        float Ratio = (D + 1.0f) / (S + 1.0f);  // +1 evita div0 e suaviza

        float Elasticity = GoodAssets[Good]->DemandElasticity;
        Price[Good] = Base * FMath::Pow(Ratio, FMath::Abs(Elasticity));

        // Suaviza variação para evitar oscilação extrema
        Price[Good] = FMath::Lerp(LastPrice[Good], Price[Good], 0.4f);
    }
}
```

> ⚠️ **Suavização é crítica**. Sem o `Lerp`, preços oscilam frame a frame e a UI fica ilegível. 0.4 é arbitrário — testar.

### Comércio entre mercados

```cpp
void ResolveTradeRoutes()
{
    for (FTradeRoute& Route : AllRoutes)
    {
        UMarketRegion* From = GetMarket(Route.FromId);
        UMarketRegion* To = GetMarket(Route.ToId);

        for (EGoodType Good : Route.GoodsAllowed)
        {
            float PriceDiff = To->Price[Good] - From->Price[Good];
            if (PriceDiff < Route.MinProfitMargin) continue;

            float Volume = FMath::Min(
                From->Stockpile[Good] * 0.3f,
                Route.Capacity);

            // Custo de transporte reduz volume
            Volume *= (1.0f - Route.TransportCost);

            From->Stockpile[Good] -= Volume;
            To->Stockpile[Good] += Volume;
            // Lucro vai para mercador / estado conforme política
        }
    }
}
```

---

## 5. Tick Econômico

`UEconomySubsystem` se inscreve no `OnDay` do `UTimeSubsystem`, mas **só processa cada N dias** para reduzir custo.

```cpp
void UEconomySubsystem::OnDayPassed(int32 Day)
{
    // Roda diário só o crítico (consumo, salários)
    ProcessDailyConsumption();
    ProcessWages();

    if (Day % 7 == 0)  ProcessWeekly();   // produção, mercado
    if (Day % 30 == 0) ProcessMonthly();  // POPs, migração, taxas, tesouro
}
```

### Tick semanal — coração da simulação

```
ProcessWeekly():
  1. ProductionPass     → cada UIndustry consome inputs e gera outputs
  2. EmploymentPass     → calcula salários conforme lucro
  3. MarketAggregation  → soma supply/demand por mercado regional
  4. PricingPass        → recalcula preços
  5. TradePass          → resolve rotas externas
  6. NeedsResolution    → POPs compram bens conforme renda + preços
  7. SatisfactionUpdate → atualiza NeedsSatisfaction de cada POP
  8. ProfitTallying     → indústrias contabilizam lucro
  9. EmitEvents         → OnEconomyTick com sumário
```

### Tick mensal — mudanças estruturais

```
ProcessMonthly():
  1. PopGrowth        → POPs crescem/encolhem conforme satisfação
  2. PopPromotion     → mobilidade entre estratos (Worker → Artisan, etc)
  3. PopMigration     → POPs migram entre províncias (wealth diferencial)
  4. PopMilitancy     → atualiza Militancy conforme satisfação
  5. TaxCollection    → tesouro coleta
  6. BudgetSpending   → paga militares, burocratas, juros
  7. DebtServicing    → paga juros, ajusta CreditRating
  8. IndustryDecisions→ capitalistas decidem expandir/fechar
  9. EmitMonthlyReport
```

---

## 6. Produção — Pipeline de uma Indústria

```cpp
void UIndustry::Tick()
{
    // 1. Verifica workforce
    float WorkforceRatio = ComputeWorkforceCoverage();  // 0..1

    // 2. Verifica inputs disponíveis no mercado
    float InputRatio = TryAcquireInputs();              // 0..1

    // 3. Throughput é o mínimo
    Throughput = FMath::Min(WorkforceRatio, InputRatio);

    // 4. Aplica modificadores (tecnologia, infraestrutura, modificadores de província)
    Throughput *= ProvinceEconomy->GetThroughputModifier(Type);
    Throughput *= Nation->GetTechModifier(Type->IndustryId);

    // 5. Produz
    for (auto& Pair : Type->OutputsPerLevel)
    {
        float Produced = Pair.Value * Level * Throughput;
        ProvinceEconomy->LocalProduction[Pair.Key] += Produced;
    }

    // 6. Calcula receita e custo
    float Revenue = ComputeRevenue();
    float Cost    = ComputeCost();
    ProfitMargin  = (Revenue - Cost) / Revenue;
}
```

### Decisões automáticas dos capitalistas (IA econômica)

Para indústrias `Private`, um sub-sistema de IA decide:

```cpp
EIndustryDecision DecideExpansion(UIndustry* Ind)
{
    if (Ind->ProfitMargin > 0.15f && Ind->Throughput > 0.85f)
        return Expand;
    if (Ind->ProfitMargin < -0.10f && !Ind->bSubsidized)
        return Close;
    if (Ind->Throughput < 0.30f)
        return ReduceLevel;
    return Maintain;
}
```

> **Indústrias estatais** (`Ownership = State`) seguem decisões do jogador / IA nacional, não capitalistas.

---

## 7. Conexão com o Sistema de Unidades Militares

Aqui o ciclo se fecha. Lembra que `UEquipmentAsset` tem `FIndustrialCost`?

```cpp
USTRUCT()
struct FIndustrialCost
{
    TMap<EGoodType, float> Goods;     // ex: 50 Steel + 20 Smallarms
    float MoneyCost;
    int32 ConstructionDays;
};
```

### Pipeline de produção militar

```
Jogador ordena: "Re-equipar 10 Regimentos de Infantaria com Rifle de Ferrolho"
        │
        ▼
UMilitaryProductionQueue cria FProductionOrder
        │   ├─ EquipmentAsset = "Rifle.BoltAction"
        │   ├─ Quantity = 10
        │   └─ TargetRegiments = [...]
        ▼
UEconomySubsystem.ReserveGoods(Cost × Quantity)
        │   └─ Se mercado nacional não tem stockpile suficiente:
        │      ├─ Aumenta Demand pelo bem ausente
        │      ├─ Ordem entra em fila ("Awaiting Materials")
        │      └─ Preço sobe → estimula produção
        ▼
Quando bens disponíveis → consome do stockpile, debita Tesouro
        ▼
ConstructionDays decorrem (tick diário)
        ▼
Equipamento entregue → MilitarySubsystem.ReequipRegiments()
        ▼
Próxima batalha: URegimentResolver inclui novas cards
```

> **Consequência de design**: nação com pouco aço **não consegue modernizar**. Nação que perde províncias industriais perde capacidade militar futura. Bloqueio naval (corta rotas) sufoca economia inimiga. Tudo conectado.

---

## 8. Comércio Internacional e Diplomacia

`UDiplomacySubsystem` interage com `UEconomySubsystem` via:

### `FTradeAgreement`

```cpp
USTRUCT()
struct FTradeAgreement
{
    int32 NationA, NationB;
    TArray<EGoodType> GoodsAllowed;
    float TariffRate;             // 0..1
    int32 ExpiresOnTick;
};
```

### Embargo

```cpp
void UDiplomacySubsystem::ImposeEmbargo(int32 ImposerId, int32 TargetId)
{
    // Remove rotas de comércio entre os dois
    EconomySubsystem->DisableRoutesBetween(ImposerId, TargetId);
    EmitEvent(OnEmbargo, ImposerId, TargetId);

    // Custo: outros aliados podem ver mal
    AdjustOpinions(ImposerId, TargetId, OpinionShift::EmbargoImposed);
}
```

### Bloqueio Naval (durante guerra)

`UMilitarySubsystem` reporta províncias costeiras bloqueadas → `UEconomySubsystem` desativa rotas marítimas dessas províncias → preços disparam, indústrias param, POPs ficam furiosos.

### Sphere of Influence

Grande potência pode "incluir" pequena nação em sua **esfera comercial**:
- Nação esfericada vende ao preço da metrópole
- Metrópole tem **direito de primeira compra** nos bens estratégicos
- Vínculo cria **lealdade econômica** (cara de quebrar diplomaticamente)

---

## 9. Conexão com Política

Militancy dos POPs alimenta `UPoliticsSubsystem`:

```cpp
void UPoliticsSubsystem::OnMonthlyTick()
{
    for (UProvince* P : World->Provinces)
    {
        for (FPopGroup& Pop : P->Economy->Pops)
        {
            // POP infeliz cresce militancy
            float Unsatisfaction = 1.0f - Pop.NeedsSatisfaction[EverydayNeeds];
            Pop.Militancy += Unsatisfaction * 0.05f * MonthsPassed;
        }

        if (P->ComputeAverageMilitancy() > RevoltThreshold)
            TriggerRevoltCheck(P);
    }
}
```

Tributação alta → POPs perdem renda → caem em níveis de necessidade → militancy sobe → revolta.

Gastos militares altos sem guerra → orçamento estourado → cortes em educação/saúde → POPs Liberais ficam infelizes → reformas pressionadas.

---

## 10. Tesouro, Dívida e Crédito

```cpp
class UNationTreasury : public UObject
{
    void TakeLoan(float Principal, int32 Lender, float TermYears);
    void PayInterest();        // mensal
    void DefaultOnDebt();      // catastrófico

    float GetMaxLoanSize() const
    {
        return AnnualRevenue * CreditRating * 3.0f;
    }
};
```

- **Default** zera a dívida mas destrói `CreditRating` por décadas.
- Dívida acumulada em paz → financia exército maior → mas juros comem orçamento futuro.
- Empréstimos podem vir de **outras nações**, criando alavanca diplomática ("perdoamos sua dívida em troca de…").

---

## 11. Eventos Emitidos

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEconomyTick, const FEconomyTickReport&);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPriceShock, EGoodType, float);    // bem, magnitude
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIndustryBuilt, const FIndustryEvent&);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIndustryClosed, const FIndustryEvent&);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPopPromoted, FGuid, EPopType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNationBankrupt, int32);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTradeRouteDisrupted, FTradeRoute);
```

UI ouve para gráficos/alertas. `UPoliticsSubsystem` ouve `OnPriceShock` (alimento caro → revolta). `UMilitarySubsystem` ouve `OnTradeRouteDisrupted` para validar suprimento.

---

## 12. Performance — Pontos Críticos

A economia é o sistema mais caro do jogo. Cuidados:

- **Não simule POPs por indivíduo**. Mantenha agregado por estrato/província.
- **Tick em períodos diferentes**: produção semanal, POPs mensal. Reduz 4-5x o custo.
- **Mercados regionais limitam combinatória**. Com 10 mercados e 30 bens, é 300 cálculos de preço — trivial. Com mercado global e 2000 províncias, viraria 60k.
- **Cache decisões IA**. Capitalista revê expansão a cada 6 meses, não toda semana.
- **Use `TArray` indexado por enum**, não `TMap<EGoodType, float>` em hot path. Mais cache-friendly.
- **Threading**: `ProductionPass` é embaraçosamente paralelo (cada indústria é independente). `FParallelFor` por província.
- **POP migration** é O(N²) ingênuo. Limite a "vizinhos próximos + nação" para virar O(N).

> ⚠️ **Meta de performance**: 1 ano simulado em < 2 segundos. Se chegar a 10 segundos, jogador desiste.

---

## 13. Diagrama Final

```
UEconomySubsystem (UWorldSubsystem)
│
├── Tick Driver
│   ├── OnDay    → consumo, salários
│   ├── OnWeek   → produção, mercado, preços, comércio, satisfação
│   └── OnMonth  → POPs, migração, taxas, tesouro, decisões industriais
│
├── Mercados
│   └── UMarketRegion[]
│       ├── ProvinceIds[]
│       ├── Supply / Demand / Price / Stockpile
│       └── ExternalRoutes[]
│
├── Produção
│   ├── UIndustry (instâncias por província)
│   │   ├── UIndustryTypeAsset (DataAsset)
│   │   ├── Workforce[]
│   │   └── Throughput, ProfitMargin
│   └── DecisionAI: expand/close/maintain
│
├── POPs
│   ├── FPopGroup (em UProvinceEconomy)
│   │   └── Type, Size, Wealth, Needs, Militancy
│   └── UPopNeedsAsset (DataAsset por tipo)
│
├── Bens
│   └── UGoodAsset (DataAsset por bem)
│       ├── Category, BasePrice
│       └── Elasticity
│
├── Tesouro
│   └── UNationTreasury
│       ├── Gold, Debt, CreditRating
│       ├── TaxRates por estrato
│       └── BudgetAllocations
│
└── Pontes para outros sistemas
    ├── ↔ UMilitarySubsystem  (FIndustrialCost para equipamento)
    ├── ↔ UDiplomacySubsystem (TradeAgreement, Embargo, Sphere)
    ├── ↔ UPoliticsSubsystem  (Militancy, Tributação, Reformas)
    └── ↔ UProgressSubsystem  (Tech desbloqueia indústrias e bens)
```

---

## 14. Plano de Implementação

1. **Esqueleto + Bens básicos**: 5 bens (Grain, Iron, Coal, Cloth, Smallarms). 1 mercado por nação.
2. **Indústrias mínimas**: 4 tipos (Farm, Mine, Mill, Arsenal). Produção sem mercado complexo.
3. **POPs simples**: 3 estratos (Aristocrat, Worker, Farmer). Necessidades só de Tier 1 (Life).
4. **Tesouro + tributação**: imposto único, gasto militar.
5. **Mercado regional v1**: oferta/demanda → preço. Sem rotas externas ainda.
6. **Necessidades completas**: 3 tiers, militancy, satisfação.
7. **Comércio inter-mercados**: `FTradeRoute`, lucro de transporte.
8. **Indústrias privadas + IA capitalista**: decisões automáticas.
9. **Integração militar**: `FIndustrialCost` ao equipar regimentos.
10. **Integração diplomática**: tratados de comércio, embargo.
11. **Dívida e crédito**: empréstimos, default.
12. **Migração de POPs**: entre províncias.
13. **Promoção de POPs**: mobilidade social.
14. **Esferas de influência econômica**: vínculos coloniais.
15. **Polish**: gráficos econômicos, painéis de mercado, alertas.

---

## 15. Resumo da Trinca Combate ↔ Unidades ↔ Economia

```
UProgressSubsystem (Tech)
        │
        ├─► UEconomySubsystem
        │   ├─ desbloqueia UIndustryTypeAsset
        │   ├─ desbloqueia UGoodAsset
        │   └─ Indústrias produzem bens (incluindo Smallarms, Artillery, Steel)
        │
        ├─► UMilitarySubsystem
        │   ├─ jogador ordena re-equipar regimentos
        │   ├─ consome FIndustrialCost do mercado nacional
        │   └─ regimentos ganham UEquipmentAsset
        │
        └─► UBattleSubsystem
            ├─ URegimentResolver compõe Class+Equipment+Doctrine+Exp
            ├─ Profile gera FinalStats e AvailableCards
            └─ Deck do lado = união dos profiles + Commander + Doctrine

Resultado: nação industrializada → cartas modernas → vantagem em batalha
           nação atrasada → cartas obsoletas → derrota mesmo com numérica
```
