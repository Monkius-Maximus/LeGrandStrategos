# 10 — UBattleSubsystem

Sistema de batalha tática: orquestra o loop de fases, mãos de cartas, resolução de efeitos, dano automático de tropas, IA tática e auto-resolve para batalhas que o jogador não acompanha.

---

## 1. Visão Geral do Subsistema

`UBattleSubsystem : public UWorldSubsystem`

**Vida útil**: existe apenas enquanto o `GameFlowSubsystem` está em `EGameFlowState::Battle`. É criado quando o mapa de batalha é carregado e destruído ao retornar ao mapa estratégico.

**Responsabilidades**:
1. Orquestrar o loop de fases táticas
2. Manter o `FBattleContext` (estado vivo da batalha)
3. Gerenciar mãos de cartas dos dois lados
4. Resolver efeitos de cartas e ticks de combate
5. Comunicar resultado de volta ao `WorldState`

**Não-responsabilidades**:
- Renderizar tropas (delegado a `ABattleVisualizer`)
- Mover câmera (delegado a `ABattleCameraPawn`)
- UI de cartas (delegado a `UBattleHUDWidget`)
- Decidir engajamento (delegado a `UMilitarySubsystem`)

---

## 2. Estrutura de Dados Central

### `FBattleContext` — o "DTO vivo" da batalha

```cpp
USTRUCT()
struct FBattleContext
{
    GENERATED_BODY()

    // Identidade
    FGuid BattleId;
    EBattleType Type;              // DirectClash, BorderSkirmish, Raid, Siege, RangedHarass

    // Sides
    FBattleSide Attacker;
    FBattleSide Defender;

    // Ambiente
    EBattleTerrain Terrain;        // Plains, Forest, Hills, Mountain, River, Urban, Coast
    EBattleWeather Weather;        // Clear, Rain, Storm, Fog, Snow
    int32 ProvinceId;              // referência ao mapa estratégico

    // Estado temporal
    EBattlePhase CurrentPhase;     // Setup, Engagement, Climax, Pursuit, Resolved
    int32 CurrentRound;            // round dentro da fase
    int32 TickInRound;             // sub-tick para animações/UI

    // Modificadores ativos (efeitos de carta com duração)
    TArray<FActiveBattleEffect> AttackerEffects;
    TArray<FActiveBattleEffect> DefenderEffects;

    // Histórico para replay e UI
    TArray<FBattleLogEntry> Log;
};
```

### `FBattleSide` — um dos lados

```cpp
USTRUCT()
struct FBattleSide
{
    GENERATED_BODY()

    int32 NationId;
    TWeakObjectPtr<UCommander> Commander;
    TArray<FRegimentBattleState> Regiments;

    // Recursos táticos
    int32 CommandPoints;           // moeda para jogar cartas no round
    int32 MaxCommandPoints;
    float Morale;                  // 0..100, abaixo de 25 = rota
    float Supply;                  // afeta efetividade
    float Cohesion;                // 0..1, formação intacta?

    // Deck
    TArray<UBattleCardAsset*> DrawPile;
    TArray<UBattleCardAsset*> Hand;
    TArray<UBattleCardAsset*> DiscardPile;
    TArray<UBattleCardAsset*> ExhaustPile;  // cartas "queimadas" no jogo

    // Posição relativa (abstrata, não coordenada real)
    EBattlePosition Position;      // Frontline, Flank, Rear, HighGround, Crossing
    bool bHasInitiative;
};
```

### `FRegimentBattleState` — uma unidade dentro da batalha

```cpp
USTRUCT()
struct FRegimentBattleState
{
    GENERATED_BODY()

    FGuid RegimentId;              // bate com o do mapa estratégico
    ERegimentType Type;            // Infantry, Cavalry, Archer, Pike, Artillery
    int32 InitialStrength;
    int32 CurrentStrength;
    float Morale;
    float OrganizationLeft;        // 0..1, se zera = não pode lutar
    EBattleStance Stance;          // Aggressive, Hold, Skirmish, Reserve
};
```

> ⚠️ **Decisão importante**: regimentos na batalha são **cópias** do estado estratégico, não referências. No fim, escrevemos perdas de volta. Isso evita corrupção se a batalha for cancelada e simplifica o auto-resolve em paralelo.

---

## 3. Loop de Fases

### Máquina de fases interna

```cpp
UENUM()
enum class EBattlePhase : uint8
{
    Setup,        // 1 round, posicionamento
    Engagement,   // 3 rounds, choque principal
    Climax,       // 1 round, decisão crítica
    Pursuit,      // 1 round, perseguição/recuperação
    Resolved      // estado terminal
};
```

### Diagrama do tick por round

```
RoundStart
  │
  ├─ DrawPhase     → ambos compram cartas até HandSize
  ├─ CommandRefresh→ CommandPoints = MaxCommandPoints + bônus
  ├─ DeclarePhase  → jogador (+IA) escolhem cartas a jogar (ordem oculta)
  ├─ ResolvePhase  → cartas resolvem por prioridade/iniciativa
  ├─ CombatTick    → resolução automática de tropas com modificadores ativos
  ├─ MoraleCheck   → checa rota, captura, fuga
  └─ RoundEnd      → descarta, decrementa duração de efeitos, emit OnRoundEnded
```

### Implementação esquelética

```cpp
void UBattleSubsystem::ProcessRound()
{
    DrawForBothSides();
    RefreshCommandPoints();

    // Coleta declarações (player via input async, IA síncrono)
    FBattleDeclarations Declarations;
    Declarations.Attacker = GatherDeclarationFor(Context.Attacker);
    Declarations.Defender = GatherDeclarationFor(Context.Defender);

    ResolveDeclarations(Declarations);
    ApplyCombatTick();
    CheckMoraleAndRout();
    EndRound();

    if (ShouldAdvancePhase()) AdvancePhase();
    if (IsBattleOver())       Resolve();
}
```

### Critérios de avanço de fase

| Fase atual | Avança quando |
|---|---|
| Setup | Round 1 termina |
| Engagement | Round 3 termina **ou** uma side perde 50% de força **ou** moral cai abaixo de 30 |
| Climax | Round 1 termina |
| Pursuit | Round 1 termina → vai para Resolved |

### Critérios de fim antecipado

```cpp
bool UBattleSubsystem::IsBattleOver() const
{
    return Context.Attacker.HasRouted()
        || Context.Defender.HasRouted()
        || (Context.Attacker.TotalStrength() == 0)
        || (Context.Defender.TotalStrength() == 0)
        || Context.CurrentPhase == EBattlePhase::Resolved;
}
```

---

## 4. Resolução de Cartas

Esta é a parte mais sensível porque define a "sensação" do combate. Camadas de prioridade evitam que cartas conflitantes virem caos.

### Anatomia de uma carta

```cpp
UCLASS(BlueprintType)
class UBattleCardAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

    // Metadados
    UPROPERTY(EditDefaultsOnly) FText DisplayName;
    UPROPERTY(EditDefaultsOnly) FText Description;
    UPROPERTY(EditDefaultsOnly) ECardCategory Category;     // Maneuver, Assault, Support, Stratagem, Reaction
    UPROPERTY(EditDefaultsOnly) ECardRarity Rarity;

    // Custos e timing
    UPROPERTY(EditDefaultsOnly) int32 CommandCost;
    UPROPERTY(EditDefaultsOnly) int32 Priority;             // ordem de resolução
    UPROPERTY(EditDefaultsOnly) ECardTiming Timing;         // OnPlay, Reaction, Persistent

    // Restrições
    UPROPERTY(EditDefaultsOnly) FCardConditions Conditions; // terreno, fase, posição etc
    UPROPERTY(EditDefaultsOnly) TArray<EBattlePhase> ValidPhases;

    // Efeito
    UPROPERTY(EditDefaultsOnly, Instanced) TArray<UBattleEffect*> Effects;

    // Persistência no deck
    UPROPERTY(EditDefaultsOnly) bool bExhaustOnPlay;        // queima do jogo
};
```

### Categoria de carta e quando entra

| Categoria | Quando se joga | Exemplo |
|---|---|---|
| `Maneuver` | DeclarePhase | "Flanco pela direita" |
| `Assault` | DeclarePhase | "Carga total" |
| `Support` | DeclarePhase | "Reagrupar" (cura morale) |
| `Stratagem` | DeclarePhase | "Emboscada" (só Setup) |
| `Reaction` | ResolvePhase, em resposta a outra carta | "Contra-ataque" |

### Pipeline de resolução

```
1. Coleta:        TArray<FCardPlay> declared = AttackerPlays + DefenderPlays
2. Ordenação:     Sort por (Priority DESC, Initiative DESC)
3. Reaction:      para cada carta resolvida, lado oposto pode jogar Reaction se válida
4. Aplicação:     UBattleEffect::Apply(BattleContext, SourceSide, TargetSide)
5. Log:           cada efeito gera FBattleLogEntry
6. Validação:     se condição quebrar no meio do round, efeito é anulado
```

### `UBattleEffect` — efeitos plugáveis

```cpp
UCLASS(Abstract, EditInlineNew, Blueprintable)
class UBattleEffect : public UObject
{
    GENERATED_BODY()
public:
    virtual bool CanApply(const FBattleContext&, const FBattleSide& Source,
                          const FBattleSide& Target) const { return true; }
    virtual void Apply(FBattleContext&, FBattleSide& Source, FBattleSide& Target)
        PURE_VIRTUAL(UBattleEffect::Apply,);
};
```

Implementações concretas:

| Classe | Efeito |
|---|---|
| `UEffect_DamageRegiment` | Aplica dano em N regimentos do alvo |
| `UEffect_MoraleShift` | +/- moral de um lado |
| `UEffect_AddPersistent` | Adiciona `FActiveBattleEffect` por X rounds |
| `UEffect_RepositionSide` | Muda `EBattlePosition` |
| `UEffect_DrawCards` | Compra extra |
| `UEffect_ExhaustEnemyCard` | Queima carta da mão inimiga |
| `UEffect_TerrainExploit` | Bônus condicional ao terreno |

> O combate sai "vivo" justamente da combinação. Em vez de hardcodar "Carga de Cavalaria", você compõe: `[DamageRegiment(target=Frontline, modifier=Cavalry)] + [MoraleShift(target=Enemy, -10)] + [AddPersistent(SelfVulnerable, 1 round)]`.

### Iniciativa e ordem

```cpp
int32 ComputeInitiative(const FBattleSide& Side, const UBattleCardAsset* Card)
{
    int32 Base = Card->Priority;
    Base += Side.Commander->Stats.Tactics / 10;
    Base += Side.bHasInitiative ? 5 : 0;
    Base += GetTerrainModifier(Side, Context.Terrain);
    return Base;
}
```

Empate → `URandomSubsystem::ResolveTie()` (determinístico via seed da batalha).

---

## 5. CombatTick — Resolução Automática de Tropas

Depois das cartas, as tropas trocam dano "passivo" baseado em estado atual.

```cpp
void UBattleSubsystem::ApplyCombatTick()
{
    FCombatTickResult AttToDef = ComputeDamage(Context.Attacker, Context.Defender);
    FCombatTickResult DefToAtt = ComputeDamage(Context.Defender, Context.Attacker);

    ApplyDamage(Context.Defender, AttToDef);
    ApplyDamage(Context.Attacker, DefToAtt);
}

FCombatTickResult UBattleSubsystem::ComputeDamage(const FBattleSide& Source,
                                                  const FBattleSide& Target)
{
    float RawPower = Source.ComputeFightingPower();   // soma regimentos × stance
    float Modifier = 1.0f;

    Modifier *= TerrainCoeff(Source, Context.Terrain);
    Modifier *= WeatherCoeff(Context.Weather);
    Modifier *= MoraleCoeff(Source.Morale);
    Modifier *= SupplyCoeff(Source.Supply);
    Modifier *= ActiveEffectsModifier(Source);
    Modifier *= CommanderCoeff(Source.Commander);
    Modifier *= PositionCoeff(Source.Position, Target.Position);

    float DealtRaw = RawPower * Modifier;
    float Mitigated = DealtRaw / Target.ComputeDefense();

    return FCombatTickResult{ Mitigated, /* breakdown for log */ };
}
```

### Distribuição de dano nos regimentos

Não distribua igualmente — quebra a fantasia de "cavalaria atacou os arqueiros".

```cpp
void DistributeDamage(FBattleSide& Target, float TotalDamage,
                      ERegimentType AttackerFocus)
{
    // Pesos por tipo conforme matchup
    // Ex: Cavalry foca Archer/Light, evita Pike
    TArray<float> Weights = ComputeMatchupWeights(AttackerFocus, Target);
    NormalizeWeights(Weights);

    for (int32 i = 0; i < Target.Regiments.Num(); ++i)
    {
        float Share = TotalDamage * Weights[i];
        Target.Regiments[i].CurrentStrength -= Share;
        Target.Regiments[i].OrganizationLeft -= Share * OrgFactor;
    }
}
```

---

## 6. Moral, Rota e Captura

```cpp
void UBattleSubsystem::CheckMoraleAndRout()
{
    UpdateSideMorale(Context.Attacker);
    UpdateSideMorale(Context.Defender);

    if (Context.Attacker.Morale < RoutThreshold) Context.Attacker.Rout();
    if (Context.Defender.Morale < RoutThreshold) Context.Defender.Rout();

    // Captura de comandante: durante Pursuit, lado vencedor rola contra Tactics inimigo
    if (Context.CurrentPhase == EBattlePhase::Pursuit)
    {
        TryCaptureCommander(GetWinningSide(), GetLosingSide());
    }
}
```

Moral cai com:
- perdas relativas no round
- comandante ferido/morto
- flanco exposto (`Position` ruim)
- suprimento baixo

Moral sobe com:
- carta de Support
- vitória parcial num round (mais dano causado que recebido)
- traço do comandante (`Inspiring`)

---

## 7. IA Tática — `UBattleAIController`

Não é um `AAIController` da Unreal (essa classe é para Pawns físicos). É um `UObject` puro instanciado para o lado controlado por IA.

```cpp
UCLASS()
class UBattleAIController : public UObject
{
    GENERATED_BODY()
public:
    void Initialize(UBattleSubsystem* Owner, int32 SideIndex,
                    UBattleAIProfile* Profile);

    FBattleDeclaration ChooseDeclaration(const FBattleContext&,
                                         const FBattleSide& Self,
                                         const FBattleSide& Enemy);
private:
    UPROPERTY() UBattleAIProfile* Profile;
    TWeakObjectPtr<UBattleSubsystem> Battle;
    int32 SideIndex;
};
```

### Pipeline de decisão (Utility AI)

```
1. Enumere cartas jogáveis na mão (filtrar por Conditions)
2. Para cada combinação possível dentro do CommandPoints:
   - Simule efeito num clone do BattleContext (lookahead 1 round)
   - Pontue resultado:
       Score = w_morale  * MoraleDelta(self)
             - w_morale  * MoraleDelta(enemy)
             + w_dmg     * DamageDealt
             - w_taken   * DamageTaken
             + w_phase   * PhaseGoalAlignment
             - w_cost    * CommandCost
3. Aplique aleatoriedade controlada (softmax com temperatura por Profile.Aggression)
4. Retorne melhor combinação
```

### `UBattleAIProfile` — perfis plugáveis

```cpp
UCLASS()
class UBattleAIProfile : public UPrimaryDataAsset
{
    UPROPERTY(EditDefaultsOnly) float Aggression;       // 0..1
    UPROPERTY(EditDefaultsOnly) float RiskTolerance;
    UPROPERTY(EditDefaultsOnly) float TerrainAwareness;
    UPROPERTY(EditDefaultsOnly) TMap<ECardCategory, float> CategoryBias;
    UPROPERTY(EditDefaultsOnly) float LookaheadTemperature;
};
```

Permite criar "generais" distintos: um Cao Cao agressivo, um Fabius defensivo, sem reescrever IA.

> ⚠️ **Performance da IA**: combinatória de cartas pode explodir. **Limite a busca**:
> - máximo 3 cartas por declaração
> - poda: descarte combinações com Score abaixo de threshold após 1ª carta
> - em auto-resolve, use heurística direta (top-1 por carta) sem lookahead

---

## 8. Integração com Mapa Estratégico

### Entrada na batalha

`UMilitarySubsystem` detecta engajamento → emite `OnBattleProposed(FBattleProposal)` → `UGameFlowSubsystem` decide:

```
SE jogador envolvido E preferência = Manual
   → Loading → Battle (mapa tático)
SENÃO
   → BattleResolverService.ResolveAuto(Proposal) sincronamente ou async
```

### `FBattleProposal` — DTO de entrada

```cpp
USTRUCT()
struct FBattleProposal
{
    int32 ProvinceId;
    EBattleType Type;
    int32 AttackerNationId;
    int32 DefenderNationId;
    TArray<FGuid> AttackerArmyIds;
    TArray<FGuid> DefenderArmyIds;
    EBattleTerrain Terrain;
    EBattleWeather Weather;
};
```

### `FBattleResult` — DTO de saída

```cpp
USTRUCT()
struct FBattleResult
{
    FGuid BattleId;
    EBattleOutcome Outcome;          // AttackerVictory, DefenderVictory, Stalemate
    TMap<FGuid, int32> RegimentLosses;   // RegimentId → strength lost
    TArray<FGuid> CapturedCommanders;
    TArray<FGuid> KilledCommanders;
    float MoraleHitAttacker;
    float MoraleHitDefender;
    float SupplyConsumed;
    int32 ProvinceControlChange;     // delta de controle
};
```

`UMilitarySubsystem::ApplyBattleResult(Result)` aplica de volta no `WorldState`. **Ponto único de mutação** — facilita debug e replay.

### Saída da batalha

```
Resolved → BattleSubsystem::Finalize() monta FBattleResult
        → emite OnBattleFinished(Result)
        → UGameFlowSubsystem: Battle → Loading → Running
        → carrega mapa estratégico
        → MilitarySubsystem aplica Result no WorldState
        → UI mostra tela de pós-batalha
```

---

## 9. Auto-Resolve — `UBattleResolverService`

Para batalhas IA vs IA (maioria do mapa) e quando jogador escolhe pular.

```cpp
UCLASS()
class UBattleResolverService : public UObject
{
    GENERATED_BODY()
public:
    // Síncrono, rápido — uso típico em loop de tick estratégico
    FBattleResult ResolveQuick(const FBattleProposal& Proposal);

    // Assíncrono via TaskGraph — para muitas batalhas no mesmo tick
    void ResolveAsync(const FBattleProposal& Proposal,
                      TFunction<void(FBattleResult)> OnComplete);
};
```

### Algoritmo do `ResolveQuick`

```
1. Construa FBattleContext minimalista (sem visualizadores)
2. Simule N rounds (N = soma das fases) usando IA tática "rasa":
   - Cada lado escolhe top-1 carta sem lookahead
   - Aplica CombatTick
   - Aplica moral check
3. Determine outcome
4. Retorne FBattleResult
```

> **Reusa o mesmo solver da batalha visual.** Isso é crucial: balanceamento sai consistente entre auto-resolve e tático. Não tenha dois sistemas de combate.

> ⚠️ Se houver 50 batalhas IA vs IA num mesmo dia simulado, **dispare via `FAsyncTask` no TaskGraph**. Cada batalha é isolada (não compartilha estado), então paraleliza trivialmente. Resultados voltam pelo `OnComplete` no GameThread.

---

## 10. Eventos Emitidos

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBattleStarted,    const FBattleContext&);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPhaseChanged,    EBattlePhase, EBattlePhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundEnded,       int32);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCardPlayed,       const FBattleCardPlayedEvent&);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRegimentDestroyed,FGuid);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSideRouted,       int32);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBattleFinished,   const FBattleResult&);
```

UI ouve `OnCardPlayed`, `OnPhaseChanged`, `OnRoundEnded`. Sons/VFX ouvem `OnRegimentDestroyed`. `MilitarySubsystem` ouve apenas `OnBattleFinished`.

---

## 11. Determinismo e Replay

- **Seed por batalha**: `FBattleContext::Seed = Hash(BattleId, GlobalSeed)`. Permite replay idêntico.
- **Log estruturado**: `TArray<FBattleLogEntry>` grava cada decisão e roll. Save inclui logs das últimas 10 batalhas.
- **Ordem fixa**: itere lados em ordem de `NationId`, regimentos em ordem de `RegimentId`.

```cpp
USTRUCT()
struct FBattleLogEntry
{
    int32 Round;
    EBattlePhase Phase;
    int32 ActorSideIndex;
    EBattleLogType Type;       // CardPlayed, EffectApplied, DamageDealt, MoraleChanged
    FString Description;
    int32 NumericValue;
    FGuid TargetId;
};
```

---

## 12. Diagrama Final do BattleSubsystem

```
UBattleSubsystem (UWorldSubsystem)
├── FBattleContext              [estado vivo]
│   ├── Attacker: FBattleSide
│   │   ├── Commander
│   │   ├── Regiments[]
│   │   ├── Deck (Draw/Hand/Discard/Exhaust)
│   │   └── CommandPoints, Morale, Supply, Position
│   ├── Defender: FBattleSide   [idem]
│   ├── Terrain, Weather
│   └── CurrentPhase, Round
│
├── PhaseRunner                 [avança Setup→Engagement→Climax→Pursuit]
├── CardResolver                [ordena por Priority, aplica UBattleEffects]
├── CombatTickResolver          [dano automático round a round]
├── MoraleResolver              [rota e captura]
├── BattleAIController × 2      [um por lado IA]
└── BattleLog                   [replay determinístico]

Comunica via EventBus:
  OnBattleStarted → UI prepara HUD tático
  OnCardPlayed    → UI anima carta, log
  OnPhaseChanged  → UI muda overlay de fase
  OnBattleFinished→ MilitarySubsystem aplica em WorldState

Reusa serviço:
  UBattleResolverService.ResolveQuick(Proposal) → mesmo solver, sem render
```

---

## 13. Plano de Implementação do BattleSubsystem (sub-etapas)

1. **Esqueleto**: `UBattleSubsystem` + `FBattleContext` + transições de fase com logs.
2. **Combate puro sem cartas**: `CombatTick` + moral + rota → já dá batalhas funcionais.
3. **`UBattleResolverService` com `ResolveQuick`**: integra com mapa estratégico antes do tático.
4. **Sistema de cartas mínimo**: 5 cartas (`Charge`, `HoldLine`, `Flank`, `Rally`, `Ambush`), `UBattleEffect` base.
5. **Mão / deck / Command Points**: pipeline de Draw/Discard.
6. **`UBattleAIController` v1**: utility AI sem lookahead.
7. **Reactions e Priority**: ordenação avançada.
8. **`UBattleAIProfile` + lookahead**: IA com personalidade.
9. **Mapa tático visual**: `ABattleVisualizer`, `ABattleCameraPawn`, `UBattleHUDWidget`.
10. **Replay e logs**: para debug e tela de pós-batalha.
11. **Polish**: animações, VFX de cartas, narração de fases.

---

## 14. Pontos de Atenção Específicos do Combate

- **Não vincule cartas a animações**. Carta dispara efeito → efeito emite evento → camada de apresentação anima. Sem isso, balanceamento fica refém de timing de animação.
- **Limite o tamanho do deck inicial** (15–20 cartas). Decks grandes diluem a fantasia de "general especialista".
- **Cartas devem ter custos não-triviais**. Se tudo custa 1 CP, a decisão evapora. Faça curva de custo 1–4.
- **Reactions são poderosas mas raras**. Limitar a 1–2 cartas reativas no deck força bluff e timing.
- **Auto-resolve precisa ser ~95% consistente com o tático**. Teste estatisticamente: rode 1000 batalhas iguais nos dois modos e compare distribuição de outcomes.
- **Não exponha probabilidades cruas**. Mostre "Vantagem: Significativa" em vez de "67.3%" — preserva tensão.
