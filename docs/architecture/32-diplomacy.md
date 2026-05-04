# 32 — UDiplomacySubsystem

Diplomacia é **a política externalizada** — mesmas tensões da Política, escala global. Sistema de relações entre nações: opinião, confiança, tratados, casus belli, esferas de influência, prestígio e guerra.

---

## 1. Princípios Arquiteturais

### Diplomacia ≠ "menu de tratados"

Em jogos pobres, diplomacia é uma janela com botões. "Aliar", "Declarar guerra", "Romper". Em grand strategy maduro, diplomacia é um **espaço estratégico vivo** onde:
- Nações têm **interesses estruturais** (não só humor numérico)
- Ações têm **custos reputacionais** ressonantes
- Justificativas (Casus Belli) **mudam o que é socialmente aceito**
- Esferas e blocos **emergem** das ações

### Opinião não é tudo

Erro de design: reduzir diplomacia a um número de Opinion entre nações. Isso é raso. O sistema precisa de:
- **Interesses** (o que a nação quer)
- **Capacidades** (o que pode fazer)
- **Compromissos** (tratados ativos)
- **Reputação** (como o mundo a vê)
- **Posicionamento** (alinhamentos, blocos)

### Diplomacia tem três escalas temporais

| Escala | Conteúdo | Cadência |
|---|---|---|
| Tática | Ações pontuais (insulto, presente, ultimato) | Diária |
| Estratégica | Tratados, alianças, esferas | Mensal |
| Estrutural | Blocos, ordens mundiais, paradigmas | Anual+ |

---

## 2. Estrutura Hierárquica

```
UDiplomacySubsystem
   ├── RelationsMatrix              [N × N FDiplomaticRelation]
   ├── TreatyRegistry               [tratados ativos no mundo]
   ├── CasusBelliRegistry           [tipos disponíveis]
   ├── DiplomaticActionRegistry     [ações plugáveis]
   ├── SphereOfInfluenceMap         [grafo de esferas]
   ├── PrestigeLedger               [ranking global]
   ├── DiplomaticAIResolver         [IA diplomática por nação]
   └── WorldOpinionEngine           [reação coletiva a ações]
```

---

## 3. `FDiplomaticRelation` — A Unidade Bilateral

```cpp
USTRUCT()
struct FDiplomaticRelation
{
    GENERATED_BODY()

    int32 NationA;
    int32 NationB;

    EDiplomaticStatus Status;        // Peace, Alliance, War, Vassalage, Truce, Embargo,
                                     // ProtectoratePartner, Suzerainty
    float Opinion;                   // -100..+100, BÁSICO (modulado por context)
    float Trust;                     // 0..100, separado de Opinion: histórico de honra
    int32 TruceUntilTick;            // 0 se sem trégua

    TArray<FActiveTreaty*> Treaties; // tratados ativos entre os dois

    // Histórico para tooltips e IA
    TArray<FDiplomaticEvent> RecentHistory;  // últimos 10 atos
    int32 LastWarTick;
    int32 WarsFoughtCount;

    // Modificadores de opinião quebrados em fontes (importante para UI)
    TArray<FOpinionModifier> OpinionBreakdown;
};
```

> **Trust separado de Opinion**: nação pode "gostar" de você (Opinion alta por casamento real) mas **não confiar** em você (Trust baixa por traições passadas). Trust afeta credibilidade de tratados.

### Modificadores de Opinião (decay temporal)

```cpp
USTRUCT()
struct FOpinionModifier
{
    FName Source;            // "Diplomacy.AllianceFormed", "Diplomacy.BackstabbedAlly"
    float Value;
    int64 AppliedAtTick;
    int32 DurationDays;      // 0 = permanente
    float DecayCurve;        // alpha de decay exponencial
};
```

```cpp
float ComputeCurrentOpinionContribution(const FOpinionModifier& M, int64 Now)
{
    if (M.DurationDays == 0) return M.Value;
    float Elapsed = (Now - M.AppliedAtTick) / 365.0f;
    return M.Value * FMath::Exp(-M.DecayCurve * Elapsed);
}
```

---

## 4. Tratados — `UTreatyAsset` + `FActiveTreaty`

### Tratado como dado plugável

```cpp
UCLASS(BlueprintType)
class UTreatyAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly) FName TreatyId;        // "Treaty.Alliance.Defensive"
    UPROPERTY(EditDefaultsOnly) ETreatyCategory Category;
    UPROPERTY(EditDefaultsOnly) FText DisplayName;

    UPROPERTY(EditDefaultsOnly) TArray<UTreatyClause*> AvailableClauses;
        // Cláusulas que podem ser combinadas
    UPROPERTY(EditDefaultsOnly) int32 MinDurationYears;
    UPROPERTY(EditDefaultsOnly) int32 MaxDurationYears;

    UPROPERTY(EditDefaultsOnly) bool bRequiresMutualConsent;  // ambos podem cancelar?
    UPROPERTY(EditDefaultsOnly) FName RequiredTech;
    UPROPERTY(EditDefaultsOnly) bool bExclusive;  // ex: vassalagem é exclusiva
};
```

### Categorias de Tratado

| Categoria | Exemplos |
|---|---|
| Aliança | Defensiva, Ofensiva, Total |
| Comércio | Livre Comércio, Tarifa Reduzida, Acesso Privilegiado |
| Não-Agressão | NAP por X anos |
| Paz | Tratado de Paz, Cessão Territorial, Reparações |
| Vassalagem | Protetorado, Vassalo, Tributário, União Pessoal |
| Militar | Acesso Militar, Trânsito, Coordenação |
| Dinástico | Casamento Real, Reivindicação Dinástica |
| Cultural | Intercâmbio, Missão Diplomática Permanente |
| Colonial | Tratado de Demarcação Colonial |

### Cláusulas Compostas

```cpp
UCLASS(Abstract, EditInlineNew)
class UTreatyClause : public UObject
{
public:
    virtual void OnTreatyActivated(FActiveTreaty& T) {}
    virtual void OnTreatyTick(FActiveTreaty& T) {}
    virtual void OnTreatyEnded(FActiveTreaty& T, ETreatyEndReason Reason) {}
    virtual bool ValidateState(const FActiveTreaty& T) const { return true; }
};
```

Implementações:

| Classe | Efeito |
|---|---|
| `UClause_MutualDefense` | Se um é atacado, outro entra em guerra |
| `UClause_OffensivePact` | Se um declara guerra, outro pode juntar |
| `UClause_TradeAccess` | Bens fluem livremente entre mercados |
| `UClause_Tariff` | Tarifa modificada |
| `UClause_NonAggression` | Não pode declarar guerra |
| `UClause_TerritoryGuarantee` | Garante território; quebrar = casus belli universal |
| `UClause_TributaryPayment` | Pagamento periódico |
| `UClause_MilitaryAccess` | Permite trânsito |
| `UClause_VassalSubservience` | Vassalo segue diplomacia do suserano |
| `UClause_DynasticUnion` | Sucessão entrelaçada |

### `FActiveTreaty` — instância em jogo

```cpp
USTRUCT()
struct FActiveTreaty
{
    FGuid TreatyInstanceId;
    UTreatyAsset* Definition;
    TArray<UTreatyClause*> ActiveClauses;  // pode ser subset de AvailableClauses
    TArray<int32> SignatoryNations;
    int32 SignedAtTick;
    int32 ExpiresAtTick;
    bool bAutoRenews;
    TMap<int32, ETreatyStance> NationStances;  // alguns tratados têm assimetria
};
```

---

## 5. Casus Belli — Por Que a Guerra é "Aceitável"

Sem CB, declarar guerra:
- Causa **infâmia** internacional (Opinion -30 com todos)
- Trust despenca
- Aliados podem abandonar
- Limita ganhos de paz (não pode anexar muito)

Com CB:
- Custo reputacional reduzido
- Define **ganhos permitidos** no tratado de paz
- Justifica entrada de aliados

```cpp
UCLASS(BlueprintType)
class UCasusBelliAsset : public UPrimaryDataAsset
{
public:
    UPROPERTY(EditDefaultsOnly) FName CasusBelliId;     // "CB.RetakeProvince"
    UPROPERTY(EditDefaultsOnly) FText DisplayName;
    UPROPERTY(EditDefaultsOnly) FText Justification;

    UPROPERTY(EditDefaultsOnly, Instanced)
    TArray<UCBValidator*> Validators;       // condições para fabricar este CB

    UPROPERTY(EditDefaultsOnly) int32 FabricationDays;  // tempo para forjar
    UPROPERTY(EditDefaultsOnly) float FabricationRiskPerDay; // chance de ser descoberto

    UPROPERTY(EditDefaultsOnly) int32 ExpiresAfterDays;     // CB válido por quanto tempo
    UPROPERTY(EditDefaultsOnly) float InfamyCost;           // se usado, infâmia gerada

    UPROPERTY(EditDefaultsOnly, Instanced)
    TArray<UPeaceTerm*> AllowedPeaceTerms;  // o que pode exigir
};
```

### Casus Belli Vitorianos

| CB | Justificação | Termos permitidos |
|---|---|---|
| Reconquest | Província foi nossa há <50 anos | Anexar província específica |
| Liberation | Cultura subjugada | Liberar nação client |
| Containment | Aliado atacado | Status quo + reparações |
| Civilization | "Levar civilização" (colonial) | Tornar protetorado |
| OpenMarkets | Forçar comércio (Opium War-style) | Concessões portuárias, indenização |
| RoyalSuccession | Reivindicação dinástica | União pessoal, anexação |
| HumanitarianIntervention | Massacre/perseguição | Mudança de governo |
| ColonialDispute | Conflito sobre território não-civilizado | Anexar colônia |
| Unification | Mesma cultura, fragmentada | Anexar irmão cultural |

### Fabricação de CB

```cpp
void UDiplomacySubsystem::FabricateCB(int32 Fabricator, int32 Target,
                                       UCasusBelliAsset* CB)
{
    FFabricationProcess Proc;
    Proc.Fabricator = Fabricator;
    Proc.Target = Target;
    Proc.CB = CB;
    Proc.StartedAtTick = CurrentTick;
    Proc.CompletesAtTick = CurrentTick + CB->FabricationDays;
    Proc.DiscoveryRoll = CB->FabricationRiskPerDay;

    ActiveFabrications.Add(Proc);
}

void UDiplomacySubsystem::TickFabrications()
{
    for (FFabricationProcess& P : ActiveFabrications)
    {
        if (URandomSubsystem::Roll(P.DiscoveryRoll))
        {
            EventBus->Trigger("Event.CBFabricationExposed", P.Fabricator, P.Target);
            AdjustOpinion(P.Fabricator, P.Target, -50);
            WorldOpinion->ApplyInfamy(P.Fabricator, 10);
            ActiveFabrications.Remove(P);
            continue;
        }

        if (CurrentTick >= P.CompletesAtTick)
        {
            GrantCB(P.Fabricator, P.Target, P.CB);
            ActiveFabrications.Remove(P);
        }
    }
}
```

---

## 6. Ações Diplomáticas — Plugáveis

```cpp
UCLASS(Abstract, EditInlineNew, Blueprintable)
class UDiplomaticAction : public UObject
{
public:
    virtual bool CanExecute(const FDiploActionContext& Ctx) const PURE_VIRTUAL(...);
    virtual FActionResult Execute(FDiploActionContext& Ctx) PURE_VIRTUAL(...);
    virtual float AIUtility(const FDiploActionContext& Ctx) const PURE_VIRTUAL(...);
    virtual int32 GetCost(const FDiploActionContext& Ctx) const { return 0; }
    virtual int32 GetCooldownDays() const { return 0; }
};
```

### Ações Disponíveis

| Classe | Efeito |
|---|---|
| `UAction_DeclareWar` | Inicia guerra (com ou sem CB) |
| `UAction_ProposeAlliance` | Sugere tratado de aliança |
| `UAction_ProposeTrade` | Sugere tratado comercial |
| `UAction_OfferGift` | +Opinion, custa ouro |
| `UAction_DemandTribute` | Exige pagamento, -Opinion ou guerra |
| `UAction_SendInsult` | -Opinion, -prestige próprio (mas faz IA reagir) |
| `UAction_RoyalMarriage` | Liga dinasticamente |
| `UAction_GuaranteeIndependence` | Promete proteger; quebrar = catástrofe reputacional |
| `UAction_FabricateCB` | Inicia fabricação |
| `UAction_BrokerPeace` | 3ª parte tenta encerrar guerra alheia |
| `UAction_SwayToSphere` | Tenta atrair pequena nação à esfera |
| `UAction_Embargo` | Corta comércio |
| `UAction_BreakTreaty` | Cancela tratado (custo de Trust) |
| `UAction_RequestMilitaryAccess` | Pede passagem |
| `UAction_DemandConcession` | Ultimato sem guerra (aceitar ou guerra) |
| `UAction_OfferProtectorate` | Propõe ser protegido |
| `UAction_SupportRevolt` | Financia rebeldes em outra nação |
| `UAction_SendMission` | Diplomata permanente; +informação, +bônus contínuo |

> Cada ação é uma classe pequena. Designers adicionam novas sem tocar core.

---

## 7. Esferas de Influência — `USphereOfInfluence`

```cpp
UCLASS()
class USphereOfInfluence : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY() int32 LeaderNationId;
    UPROPERTY() TArray<FSphereMember> Members;
    UPROPERTY() float Cohesion;            // 0..1, quão alinhada
    UPROPERTY() ESphereType Type;          // Economic, Political, Military, Total
};

USTRUCT()
struct FSphereMember
{
    int32 NationId;
    float Influence;            // 0..100, leader's influence sobre este membro
    int32 JoinedAtTick;
    bool bWillingMember;
};
```

### Mecânica

- Grandes potências competem por **influência** sobre nações pequenas.
- Influência sobe via: presentes, missões, casamentos, intervenções militares, ajuda em desastres.
- Quando influência > threshold, nação pequena entra na esfera (consensual ou coercitivo).
- Membros de esfera: comércio favorecido para o líder, voto diplomático alinhado, exclusividade militar.
- **Conflito de esferas** é prelúdio comum a guerras (Crimean War, Scramble for Africa).

```cpp
void USphereOfInfluenceEngine::TickInfluence(int32 GreatPower, int32 SmallNation)
{
    float InfluenceDelta = 0;
    InfluenceDelta += GiftsGiven(GP, SN) * 0.5f;
    InfluenceDelta += PermanentMission ? 1.0f : 0;
    InfluenceDelta += SharedReligion ? 0.3f : 0;
    InfluenceDelta -= RivalInfluence(SN) * 0.4f;
    InfluenceDelta -= NationalismScore(SN) * 0.6f;  // nacionalismo resiste

    UpdateInfluence(GP, SN, InfluenceDelta);

    if (Influence(GP, SN) > 80 && !IsInSphere(SN))
        ProposeSphereEntry(GP, SN);
}
```

---

## 8. Prestígio — A Moeda Reputacional

```cpp
UCLASS()
class UPrestigeLedger : public UObject
{
public:
    void RegisterAchievement(int32 NationId, EPrestigeSource Source, float Value);
    float GetPrestige(int32 NationId) const;
    int32 GetWorldRank(int32 NationId) const;
    EGreatPowerStatus GetStatus(int32 NationId) const;
};

UENUM()
enum class EPrestigeSource : uint8
{
    EraAdvancement,
    BattleVictory,
    SuccessfulColonization,
    TechFirstResearched,
    DiplomaticVictory,
    CulturalAchievement,    // Great Exhibition, Olympic-style events
    ScientificDiscovery,
    HumanitarianAct,
    InfamousAct,            // negativo
    DefeatInWar,            // negativo
};
```

### Status de Grande Potência

```cpp
EGreatPowerStatus ComputeStatus(int32 NationId)
{
    float Score = ComputeCompositeScore(NationId);
        // Industrial output × 0.3
        // Military strength × 0.3
        // Prestige × 0.2
        // Population × 0.1
        // Tech advancement × 0.1

    int32 Rank = GetWorldRank(NationId);

    if (Rank <= 8)  return EGreatPowerStatus::GreatPower;
    if (Rank <= 16) return EGreatPowerStatus::SecondaryPower;
    if (HasColonies(NationId)) return EGreatPowerStatus::ColonialPower;
    return EGreatPowerStatus::MinorPower;
}
```

> Status muda **ações disponíveis**. Grandes potências podem garantir independência, mediar paz, formar esferas. Menores não.

---

## 9. World Opinion — Reação Coletiva

```cpp
class UWorldOpinionEngine : public UObject
{
public:
    void ApplyInfamy(int32 NationId, float Amount);
    void EvaluateAggression(const FActOfAggression& Act);
    void EvaluateBenevolence(const FBenevolentAct& Act);
    float GetGlobalOpinionOfNation(int32 NationId) const;
};
```

### Infâmia

```cpp
void ApplyInfamy(int32 N, float Amount)
{
    Nations[N].Infamy += Amount;

    // Limiar: se passar de threshold, desencadeia "Pariah"
    if (Nations[N].Infamy > 50 && !Nations[N].bIsPariah)
    {
        DeclareInternationalPariah(N);
        // Coalizão pode se formar contra
    }
}

void DeclareInternationalPariah(int32 N)
{
    EventBus->Trigger("Event.NationDeclaredPariah", N);
    // Outras grandes potências ganham CB "ContainPariah" gratuito
    for (int32 GP : GetGreatPowers())
        if (GP != N)
            GrantCB(GP, N, ContainmentCB);
}
```

### Decay de Infâmia

Infâmia decai lentamente (~5 por ano), mais rápido se nação faz **atos benévolos** (ajuda humanitária, hospeda conferência de paz).

---

## 10. Guerra — Estado e Resolução

```cpp
USTRUCT()
struct FActiveWar
{
    FGuid WarId;
    int32 PrimaryAttacker;
    int32 PrimaryDefender;
    TArray<int32> AttackerSide;     // aliados que entraram
    TArray<int32> DefenderSide;
    UCasusBelliAsset* WarCB;
    int32 StartedAtTick;
    float WarScore;                 // -100..+100, atacante negativo, defensor positivo
    TArray<FWarGoal> WarGoals;
    TArray<FBattleRecord> Battles;
};
```

### War Score

Atualizado por:
- Batalhas vencidas (peso conforme tamanho)
- Províncias ocupadas
- Capital tomada (massivo)
- Comandantes capturados/mortos
- Tempo passado (decai se sem ação — "war exhaustion")

### War Exhaustion

POPs em nação em guerra ganham militância contínua. Nação não consegue sustentar guerra eterna. Loop: guerra → militância → revolta → forçar paz.

### Tratado de Paz

Quando uma das partes propõe paz:
- War Score determina quanto pode exigir
- CB original limita tipos de demanda
- Lado perdedor pode aceitar ou continuar (mas pagará mais se perder mais)

```cpp
class UPeaceNegotiation
{
public:
    TArray<FPeaceOffer> ProposeOffers(const FActiveWar& War, int32 Proposer);
    bool ResolveOffer(const FPeaceOffer& Offer);
};

USTRUCT()
struct FPeaceOffer
{
    int32 Proposer;
    int32 Target;
    TArray<UPeaceTerm*> Terms;
    float ScoreCost;     // quanto War Score precisa pagar para impor
};
```

---

## 11. IA Diplomática — `UDiplomaticAIResolver`

```cpp
class UDiplomaticAIResolver
{
public:
    void DecideMonthlyDiplomacy(UNation* N);
private:
    TArray<FRankedAction> RankAvailableActions(UNation* N);
    UNation* IdentifyPrimaryRival(UNation* N);
    UNation* IdentifyPotentialAlly(UNation* N);
};
```

### Heurísticas

```cpp
void DecideMonthlyDiplomacy(UNation* N)
{
    // 1. Identificar rivais (alta capacidade militar + interesses opostos)
    UNation* Rival = IdentifyPrimaryRival(N);

    // 2. Buscar aliados contra rival
    if (Rival && N->Diplomacy->StrengthRatio(Rival) < 0.7f)
    {
        UNation* PotentialAlly = FindAllyAgainst(N, Rival);
        if (PotentialAlly) ProposeAlliance(N, PotentialAlly);
    }

    // 3. Expandir esfera
    if (N->IsGreatPower())
    {
        TArray<UNation*> Targets = FindWeakNeighbors(N);
        for (UNation* T : Targets) AccrueInfluence(N, T);
    }

    // 4. Fabricar CB se há objetivo territorial
    if (N->HasTerritorialClaim() && !N->HasCBAgainst(Target))
        FabricateCB(N, Target, BestCB);

    // 5. Declarar guerra se condições alinhadas
    if (ReadyForWar(N))
        DeclareWar(N, Target);

    // 6. Manter relações (presentes, missões em chave)
    MaintainCriticalRelations(N);
}
```

### Personalidade da IA

```cpp
USTRUCT()
struct FDiplomaticPersonality
{
    float Aggression;         // propensão a guerra
    float Honor;              // resistência a quebrar tratados
    float Expansionism;       // busca por território
    float CulturalAffinity;   // peso de cultura comum
    float Pragmatism;         // ignora ideologia se conveniente
    float Vindictiveness;     // memória de ofensas
};
```

Vem de `UNationLeaderProfile` ou `UCommanderProfile`. Bismarck ≠ Napoleão III ≠ Tsar.

---

## 12. Eventos Emitidos

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTreatySigned,        const FActiveTreaty&);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTreatyBroken,        const FActiveTreaty&);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWarDeclared,         const FActiveWar&);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWarEnded,            const FWarOutcome&);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInfluenceShift,     int32, int32);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSphereChange,        const FSphereChangeEvent&);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInfamyChanged,      int32, float);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPrestigeRanking,    int32, EGreatPowerStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCBGranted,           const FCBGrantEvent&);
```

Outros sistemas ouvem extensivamente: `UEconomySubsystem` para tratados comerciais, `UMilitarySubsystem` para guerras, `UPoliticsSubsystem` para repercussão interna de tratados ("vendido para o estrangeiro!").

---

## 13. Diagrama Final do `UDiplomacySubsystem`

```
UDiplomacySubsystem (UWorldSubsystem)
│
├── DataAssets (estático)
│   ├── UTreatyAsset[]              [Alliance, Trade, NAP, Vassalage, Royal Marriage...]
│   ├── UTreatyClause[]             [MutualDefense, TradeAccess, Tributary...]
│   ├── UCasusBelliAsset[]          [Reconquest, Liberation, OpenMarkets, Civilization...]
│   ├── UDiplomaticAction[]         [DeclareWar, ProposeAlliance, FabricateCB, Embargo...]
│   └── UPeaceTerm[]                [AnnexProvince, LiberateNation, Reparations, Tributary...]
│
├── Estado runtime (no save)
│   ├── RelationsMatrix             [N×N FDiplomaticRelation]
│   ├── ActiveTreaties[]
│   ├── ActiveWars[]
│   ├── ActiveFabrications[]
│   ├── SpheresOfInfluence[]
│   ├── PrestigeLedger
│   └── InfamyTracker
│
├── Engines
│   ├── UWorldOpinionEngine
│   ├── USphereOfInfluenceEngine
│   ├── UPeaceNegotiation
│   └── UDiplomaticAIResolver
│
├── Loop
│   ├── OnDay → tick fabricações, war score, war exhaustion
│   ├── OnMonth → IA diplomática, decay opinion, decay infamy, sphere influence
│   └── OnYear → reranking de prestígio, status de Grande Potência
│
└── Pontes
    ├── ↔ UEconomySubsystem    (tratados comerciais, embargos, bloqueios)
    ├── ↔ UMilitarySubsystem   (declaração e condução de guerra)
    ├── ↔ UPoliticsSubsystem   (governo afeta opções, tratados afetam estabilidade)
    ├── ↔ UProgressSubsystem   (techs liberam ações diplomáticas, prestígio por descoberta)
    ├── ↔ UEventSubsystem      (gera + consome eventos diplomáticos massivamente)
    └── ↔ UBattleSubsystem     (resultado afeta War Score)
```

---

## 14. Plano de Implementação

1. **`FDiplomaticRelation`** + matriz N×N + ajuste manual de Opinion.
2. **3 ações básicas**: ProposeAlliance, DeclareWar, OfferGift.
3. **`FActiveTreaty`** + 2 tratados (Alliance Defensiva, NAP).
4. **`UCasusBelliAsset`** + 2 CBs iniciais (Reconquest, Containment).
5. **Sistema de Trust separado de Opinion**.
6. **`OpinionBreakdown` com decay temporal** (modificadores agrupados por fonte).
7. **`UWorldOpinionEngine` + Infâmia** com pariah threshold.
8. **`UPrestigeLedger`** + status de Grande Potência.
9. **`FActiveWar`** + War Score + War Exhaustion.
10. **`UPeaceNegotiation`** com Peace Terms validados por CB.
11. **Fabricação de CB** + risco de exposição.
12. **Esferas de Influência** com cálculo de influência mensal.
13. **Tratados compostos por cláusulas** (`UTreatyClause`).
14. **`UDiplomaticAIResolver` v1**.
15. **Personalidades diplomáticas** vindas de `UNationLeaderProfile`.
16. **Coalizões automáticas contra pariah**.
17. **Royal Marriage e União Pessoal**.
18. **Polish**: UI de relações, mapa de esferas, painel de tratados.

---

## 15. Pontos de Atenção Específicos

- **Opinion vs Trust**: separar é crítico. Sem isso, jogador não entende por que aliados o abandonam mesmo gostando dele.
- **Infâmia tem que doer**. Se infâmia só dá pequena penalidade, jogador agride sem peso. Pariah deve ser ameaça existencial real.
- **CB precisa ter narrativa**. "Você tem CB válido" sem explicação é frio. Mostre a justificativa textual e o ato histórico que gerou.
- **War Score não é só batalhas**. Ocupação territorial, capitais tomadas, tempo, exhaustion — todos contam. Caso contrário guerra vira só shoot-em-up.
- **Esferas devem competir explicitamente**. Mostrar mapa de influência por nação pequena (ex: Sérvia 60% Rússia, 30% Áustria, 10% UK) torna geopolítica visível.
- **IA diplomática deve ter memória**. Se você atacou Prússia em 1850 e busca aliança em 1870, eles devem lembrar. Use `RecentHistory` do `FDiplomaticRelation`.
- **Personalidade > Otimização**. IA puramente racional vira "todos formam aliança contra o jogador". Personalidades garantem que cada nação se comporte de forma reconhecível.
- **Não esconder informação por padrão**. Em grand strategy, transparência diplomática é essencial. Esconda só com "espionagem" mecânica explícita.
- **Performance**: matriz N×N escala mal com 100+ nações. Use `TMap<FNationPair, FDiplomaticRelation>` esparsa quando opinion = 0 (default). Tick mensal é fino, não diário.

---

## 16. Visão Conjunta do Ciclo Político-Diplomático

```
                                ┌──────────────────┐
                                │  POPs em uma     │
                                │  nação           │
                                └────────┬─────────┘
                                         │
                                         ▼
             ┌─────────────────────────────────────────────────┐
             │            UPoliticsSubsystem                    │
             │  Ideologia → Facções → Governo → Leis → POPs    │
             └────────────┬─────────────────────┬──────────────┘
                          │                     │
                          │ governo afeta       │ militância afeta
                          │ opções              │ war exhaustion
                          ▼                     ▼
             ┌─────────────────────────────────────────────────┐
             │           UDiplomacySubsystem                   │
             │  Tratados ↔ Esferas ↔ CB ↔ Guerra ↔ Prestígio  │
             └────────────┬─────────────────────┬──────────────┘
                          │                     │
              guerras ────┘                     └──── tratados comerciais
                          │                                         │
                          ▼                                         ▼
                ┌──────────────────┐                  ┌──────────────────┐
                │  UMilitarySys    │                  │  UEconomySys     │
                └─────────┬────────┘                  └─────────┬────────┘
                          │                                     │
                          └─────► UBattleSubsystem ◄────────────┘
                                  (cartas, comandantes, decisão)

                   UEventSubsystem permeia TUDO
                   (eventos disparam, recebem efeitos, encadeiam narrativa)
```
