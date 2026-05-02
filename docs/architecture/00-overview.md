# 00 — Visão Geral da Arquitetura

Documento raiz da arquitetura de **Le Grand Strategos**: um grand strategy inspirado em Victoria 2/3, Civilization, Humankind e Romance of the Three Kingdoms, com foco em simulação profunda, guerra por regiões e batalhas táticas baseadas em comandante + tropas + "deck" de decisões.

---

## 1. Camadas Arquiteturais

A arquitetura é dividida em **5 camadas**, cada uma com responsabilidade isolada e comunicação por **eventos + interfaces**, nunca por referência direta entre sistemas distantes.

```
┌──────────────────────────────────────────────────────────────┐
│  CAMADA 5 — APRESENTAÇÃO (UI / HUD / Câmera / Input)         │
├──────────────────────────────────────────────────────────────┤
│  CAMADA 4 — ORQUESTRAÇÃO (GameMode, GameState, FSM Global)   │
├──────────────────────────────────────────────────────────────┤
│  CAMADA 3 — SUBSISTEMAS (Economia, Diplomacia, IA, Eventos…) │
├──────────────────────────────────────────────────────────────┤
│  CAMADA 2 — DOMÍNIO (Nação, Província, Exército, Comandante) │
├──────────────────────────────────────────────────────────────┤
│  CAMADA 1 — DADOS / SERVIÇOS (DataAssets, Save, RNG, Tempo)  │
└──────────────────────────────────────────────────────────────┘
```

### Mapeamento Unreal das camadas

| Camada | Tipo Unreal | Justificativa |
|---|---|---|
| 1 | `UPrimaryDataAsset`, `USaveGame`, `UGameInstanceSubsystem` | Dados estáticos e serviços globais persistentes entre níveis. |
| 2 | `UObject` puro (não-Actor) + `UStruct` | Entidades simuladas; não precisam de Tick nem Transform. **Performance crítica.** |
| 3 | `UWorldSubsystem` | Vida atrelada ao mundo carregado; resetam ao trocar mapa. |
| 4 | `AGameModeBase`, `AGameStateBase`, FSM custom | Autoridade do fluxo. |
| 5 | `UUserWidget`, `APlayerController`, `ACameraActor` | Apresentação. |

### Regras de dependência (críticas)

- Camada superior **pode** chamar inferior.
- Camada inferior **nunca** chama superior — só **emite eventos** (delegates dinâmicos).
- Subsistemas **nunca** chamam outros subsistemas diretamente; passam pelo **`UEventBusSubsystem`** ou pelo `GameState`.
- Domínio (Nação, Província) **nunca** referencia UI.

> ⚠️ *Por que isso importa:* num grand strategy, a quantidade de cross-talk entre sistemas (economia → política → diplomacia → guerra) explode. Se permitir ponteiros diretos, o grafo de dependência vira spaghetti e o save/load quebra.

### Sobre C# / C++ / Blueprint

- **C++**: toda a Camada 1, 2 e 3 (simulação tick-pesada, mapas grandes, AI).
- **Blueprint**: Camada 5 (UI, animações, FX) e *scripting* de eventos narrativos.
- **C# (caso queira)**: trate como **camada externa de scripting** via plugin (ex.: USharp/UnrealCLR ou um socket IPC). Útil para *modders* ou ferramentas de design (editor de eventos, balanceamento). **Nunca** no hot path de simulação.

---

## 2. Inventário de Subsistemas

| Subsistema | Camada | Tipo Unreal | Documentação |
|---|---|---|---|
| `UGameFlowSubsystem` | 4 | GameInstanceSubsystem | [`01-game-flow.md`](01-game-flow.md) |
| `UTimeSubsystem` | 1 | GameInstanceSubsystem | [`02-time.md`](02-time.md) |
| `UEventBusSubsystem` | 1 | GameInstanceSubsystem | [`03-event-bus.md`](03-event-bus.md) |
| `UBattleSubsystem` | 3 | WorldSubsystem | [`10-battle.md`](10-battle.md) |
| Sistema de Unidades | 2 | DataAssets + UObjects | [`11-units.md`](11-units.md) |
| `UEconomySubsystem` | 3 | WorldSubsystem | [`20-economy.md`](20-economy.md) |
| `UProgressSubsystem` | 3 | WorldSubsystem | [`21-progress.md`](21-progress.md) |
| `UEventSubsystem` | 3 | WorldSubsystem | [`30-events.md`](30-events.md) |
| `UPoliticsSubsystem` | 3 | WorldSubsystem | [`31-politics.md`](31-politics.md) |
| `UDiplomacySubsystem` | 3 | WorldSubsystem | [`32-diplomacy.md`](32-diplomacy.md) |
| `UAIDirectorSubsystem` | 3 | WorldSubsystem | [`40-ai-director.md`](40-ai-director.md) |
| `UMilitarySubsystem` | 3 | WorldSubsystem | [`41-military.md`](41-military.md) |

---

## 3. Modelo de Classes Sugerido

### Camada 1 — Dados/Serviços

| Classe | Tipo | Responsabilidade |
|---|---|---|
| `UNationDataAsset` | PrimaryDataAsset | Definição estática de nação (cor, cultura inicial, traços) |
| `UProvinceDataAsset` | PrimaryDataAsset | Definição estática de província (terreno, recursos base) |
| `UTechTreeAsset` | PrimaryDataAsset | Árvore tecnológica |
| `UEventDefinitionAsset` | PrimaryDataAsset | Eventos narrativos (triggers + opções) |
| `UBattleCardAsset` | PrimaryDataAsset | Cartas de comando |
| `UCommanderTraitAsset` | PrimaryDataAsset | Traços de comandante |
| `UTimeSubsystem` | GameInstanceSubsystem | Tick simulado, calendário, escala de velocidade |
| `USaveSubsystem` | GameInstanceSubsystem | Serialização de WorldState |
| `URandomSubsystem` | GameInstanceSubsystem | RNG determinístico (importante para multiplayer/replays) |
| `UEventBusSubsystem` | GameInstanceSubsystem | Pub/sub global |

### Camada 2 — Domínio (UObjects puros)

| Classe | Responsabilidade | Owns |
|---|---|---|
| `UWorldState` | Raiz da simulação; container de tudo | `Nations`, `Provinces`, `Armies` |
| `UNation` | Estado de uma nação | `Treasury`, `Government`, `Pops por proxy` |
| `UProvince` | Estado de uma província | `UProvinceEconomy`, `Garrison`, `Buildings` |
| `UArmy` | Pilha militar no mapa | `Regiments`, `Commander` |
| `UCommander` | General/almirante | `Stats`, `Traits`, `Deck` |
| `UCommanderDeck` | Coleção de cartas | `Cards` |
| `UTreaty` | Acordo entre nações | `Clauses` |
| `UFactionInternal` | Facção política dentro da nação | `Influence`, `Agenda` |

### Camada 3 — Subsistemas (`UWorldSubsystem`)

| Subsistema | Responsabilidade |
|---|---|
| `UWorldSimSubsystem` | Orquestra ticks dos demais |
| `UEconomySubsystem` | Produção, mercados, impostos |
| `UPoliticsSubsystem` | Facções, leis, estabilidade |
| `UDiplomacySubsystem` | Relações, tratados, opiniões |
| `UMilitarySubsystem` | Movimento de exércitos, engajamento |
| `UBattleSubsystem` | Batalha tática ativa (instanciada sob demanda) |
| `UAIDirectorSubsystem` | Decisões de alto nível das IAs |
| `UEventSubsystem` | Triggers e processamento de eventos narrativos |
| `UProgressSubsystem` | Tecnologia, idades, vitória |
| `UMapSubsystem` | Visualização do mapa, picking, fog of war |

### Camada 4 — Orquestração

| Classe | Tipo | Responsabilidade |
|---|---|---|
| `AStrategosGameMode` | GameMode | Cria/configura sistemas no nível atual |
| `AStrategosGameState` | GameState | Estado replicado do nível (multiplayer-ready) |
| `UGameFlowSubsystem` | GameInstanceSubsystem | FSM principal |

### Camada 5 — Apresentação

| Classe | Responsabilidade |
|---|---|
| `AStrategosPlayerController` | Input, seleção |
| `UStrategicHUDWidget` | UI do mapa-mundi |
| `UBattleHUDWidget` | UI tática (mão de cartas, ordens) |
| `UProvinceTooltipWidget` | Hover de província |
| `AMapCameraPawn` | Câmera estratégica (RTS-style) |

### Diagrama de dependência (resumido)

```
GameFlowSubsystem
   ├─► TimeSubsystem
   ├─► (load level) GameMode/GameState
   │       └─► WorldSubsystems (Economy, Diplo, Military, AI, Event)
   │              └─► WorldState (Nations, Provinces, Armies)
   │                     └─► DataAssets (read-only)
   └─► SaveSubsystem ◄─► WorldState (serialização)

Tudo emite eventos via EventBus, que UI consome.
```

---

## 4. Save / Load

- **Não** use o `USaveGame` direto para serializar UObjects da simulação — vai gerar GC drama.
- Crie um **DTO** (`FWorldSaveDTO`) puro de `UStruct`/POD que reconstrói o `UWorldState`.
- Versione o DTO (`uint32 Version`) e tenha *upgraders* (`FSaveMigrator_v1_to_v2`).
- Use `FArchive` custom + compressão (`FOodleCompressedArchive` se disponível).
- RNG seed faz parte do save → **replay determinístico**.

---

## 5. Resumo Hierárquico Macro

```
StrategosGame
├── GameInstance
│   ├── GameFlowSubsystem        [FSM: MainMenu/Loading/Running/Paused/Battle/Event/GameOver]
│   ├── TimeSubsystem            [tick simulado, OnDay/Month/Year]
│   ├── SaveSubsystem            [DTO + versionamento]
│   ├── RandomSubsystem          [RNG determinístico]
│   ├── EventBusSubsystem        [pub/sub global]
│   └── WorldState (UObject)
│       ├── Nations[]
│       ├── Provinces[]
│       ├── Armies[]
│       └── Treaties[]
│
├── GameMode + GameState (por mapa)
│   └── WorldSubsystems
│       ├── WorldSimSubsystem    [orquestra]
│       ├── EconomySubsystem
│       ├── PoliticsSubsystem
│       ├── DiplomacySubsystem
│       ├── MilitarySubsystem
│       ├── BattleSubsystem      [instanciado em mapa de batalha]
│       ├── AIDirectorSubsystem
│       ├── EventSubsystem
│       ├── ProgressSubsystem
│       └── MapSubsystem
│
├── DataAssets (read-only)
│   ├── NationDataAsset
│   ├── ProvinceDataAsset
│   ├── TechTreeAsset
│   ├── EventDefinitionAsset
│   ├── BattleCardAsset
│   ├── CommanderTraitAsset
│   └── BattleAIProfileAsset
│
└── Apresentação
    ├── PlayerController
    ├── MapCameraPawn
    ├── StrategicHUDWidget
    ├── BattleHUDWidget
    └── ProvinceTooltipWidget
```

---

## 6. Pontos de Atenção Final

- **Determinismo desde o dia 1**: RNG centralizado, ordem de iteração estável (use `TArray` com IDs, não `TMap` para ordem).
- **Não use Actors para entidades simuladas**. Eles têm overhead de transform/tick que você não precisa.
- **Tudo que é "data" vira DataAsset**. Tudo que é "estado" vira UObject. Tudo que é "comportamento global" vira Subsystem.
- **Teste save/load semanalmente**. Adicionar campo novo é barato; consertar save quebrado depois de 6 meses é caro.
- **Profile cedo**: rodar 100 anos simulados em < 30 segundos deve ser meta desde o protótipo.

---

## 7. Próxima Leitura

Para entender o fluxo do jogo do ponto de vista do usuário, siga para [`01-game-flow.md`](01-game-flow.md). Para mergulhar em subsistemas específicos, consulte o índice em [`docs/README.md`](../README.md).
