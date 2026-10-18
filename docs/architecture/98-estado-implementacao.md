# 98 — Estado de Implementação (snapshot)

> Retrato do que está **realmente implementado em código** no momento, por módulo e subsistema. Serve de fonte única para acompanhar progresso, montar mapas mentais e fluxogramas.
>
> **Não substitui** o `PROJECT_STATE.md` (Obsidian, estado vivo do dia-a-dia) nem os docs de arquitetura (design-alvo). Este arquivo é um *snapshot por commit* — regenerar quando um sistema novo aterrissar.

**Referência:** branch `main` @ commit `eaac6ed` · **SaveVersion:** 6 · **Data:** 2026-06-08

Legenda: ✅ implementado · ⚠️ parcial · ❌ não existe (planejado)

---

## Visão de uma linha

O loop central **Tempo → Economia → Eventos → Decisões** está completo e determinístico.
Movimento militar, diplomacia (data layer), save/load e geração de geografia funcionam.
O **combate tático** está implementado no núcleo (loop, cartas, efeitos, IA, auto-resolve,
replay) — falta a camada visual BP e o gatilho a partir do mapa estratégico.
**Politics, Progress (tech) e AI Director** ainda não existem (hoje a IA é um placeholder por regras).

---

## StrategosCore — fundação e simulação macro

### Fundação
| Subsistema | Tipo | Estado | Observação |
|---|---|---|---|
| `UTimeSubsystem` | GameInstance | ✅ | Metrônomo; speeds Paused→VeryFast; delegates OnDay/OnMonth/OnYear |
| `UGameFlowSubsystem` | GameInstance | ✅ | FSM 7 estados com tabela de transições |
| `UEventBusSubsystem` | GameInstance | ✅ | Pub/sub por FName tag |

### Mundo e mapa
| Subsistema | Tipo | Estado | Observação |
|---|---|---|---|
| `UMapSubsystem` | World | ✅ v1 | Índice espacial + picking + seleção/hover; sem modos de mapa nem fog |
| `UWorldState` | UObject | ✅ | Containers Nations/Provinces/Armies por FName |
| `UNation` | UObject | ✅ | Cor/identidade, território, líder, ArchetypeAffinity, Treasury, Stockpile, StrategicIndices |
| `UProvince` | UObject | ✅ | `FProvinceGeography` (4 eixos), POPs, slots, buildings, adjacências |
| `UArmy` | UObject | ✅ v2 | Movimento, stats, modifiers, estado, XP |
| `UWorldBootstrapper` | BlueprintFunctionLibrary | ✅ | Bootstrap por asset ou sandbox hardcoded (Albion/Galia/Norden) |
| `AStrategosGameMode` / `AStrategosGameState` | Actor | ✅ | Orquestração de bootstrap + holder do WorldState |

### Militar (estratégico)
| Subsistema | Tipo | Estado | Observação |
|---|---|---|---|
| `UMilitarySubsystem` | World | ✅ v1 | Ordens de movimento por adjacência, countdown diário, `OnArmyArrived` |
| | | ❌ | Suprimento, frentes, naval, cerco, mobilização — pendentes |

### Economia (subsistema mais pesado)
| Elemento | Tipo | Estado | Observação |
|---|---|---|---|
| `UEconomySubsystem` | World | ✅ v2 | Tick mensal de **9 fases** (crescimento POP → emprego → produção DAG por tier → consumo → salários/lucros → impostos → despesas → tesouraria/empréstimo → índices) |
| | | ✅ | Ciclo de vida de prédio: build/demolish/upgrade, troca de PM, toggle de modifier, privatizar/nacionalizar, auto-invest da Bourgeoisie |
| `EPopStratum` / `FPopGroup` | Enum/Struct | ✅ | 4 strata ativos (Laborer, Artisan, FactoryWorker, Bourgeoisie); 3 stubs |
| `FTreasury` | Struct | ✅ | Receitas/despesas, dívida, juros, nível de imposto por stratum |
| `FNationalStockpile` | Struct | ✅ | Estoques + supply/demand → preço dinâmico |
| `FStrategicIndices` | Struct | ✅ | MilitaryReadiness / CivilianMorale / IndustrialCapacity [0.5,1.5] |
| `UGoodAsset` / `UBuildingTypeAsset` | DataAsset | ✅ | Bens (tier, preço, categoria); prédios (slots, custo, métodos) |
| `UProductionMethodAsset` / `UProductionModifierAsset` | DataAsset | ✅ | Recipes (inputs/outputs/emprego/tier/tech) e modifiers (mutex groups, multiplicadores) |
| `FProvinceGeography` | Struct | ✅ v6 | Topografia, clima, vegetação, hidrografia, fertilidade, recurso principal |

### Eventos
| Elemento | Tipo | Estado | Observação |
|---|---|---|---|
| `UEventSubsystem` | World | ✅ v2 | Indexação por trigger, gate MTTH determinístico, fila de decisões por nação, auto-resolve de IA |
| `UEventAsset` | DataAsset | ✅ | Trigger, MTTH, conditions, choices, auto-effects |
| `UEventCondition` (base) | UObject plugável | ✅ | Implementadas: TreasuryBelow, LoyaltyBelow, HasGoodInStockpile |
| `UEventEffect` (base) | UObject plugável | ✅ | Implementadas: AddGold, AddPopLoyalty, AddGoodsToStockpile, FireEvent (chaining) |

### Diplomacia
| Elemento | Tipo | Estado | Observação |
|---|---|---|---|
| `UDiplomacySubsystem` | World | ✅ v1 | Matriz esparsa Status+Opinion, getters/setters, `AreAtWar`/`AreAllied` |
| `FDiplomaticRelation` / `EDiplomaticStatus` | Struct/Enum | ✅ v1 | Peace, NAP, Alliance, War (normalizado por par) |
| | | ❌ | Ações (DeclareWar, ProposeAlliance), tratados compostos, CB — pendentes |

### Save/Load
| Elemento | Tipo | Estado | Observação |
|---|---|---|---|
| `USaveSubsystem` | GameInstance | ✅ v1 | Save/Load por slot, delegates de conclusão |
| `UStrategosSaveData` | USaveGame | ✅ v6 | Records: Pop, Building, Nation, Province (com Geography), Army, PendingDecision, DiplomaticRelation |
| | | ❌ | Sem lógica de migração — saves antigos carregam com defaults nos campos novos |

---

## StrategosBattle — combate tático

| Elemento | Tipo | Estado | Observação |
|---|---|---|---|
| `UBattleSubsystem` | World | ✅ | Loop de fases (Setup→Engagement→Climax→Pursuit→Resolved), rounds, CombatTick, moral/rota |
| `UBattleResolverService` | Service | ✅ | Auto-resolve (`ResolveQuick`) — sub-etapa 3 |
| `UBattleCardAsset` + `FBattleContext`/`FBattleSide` | DataAsset/Struct | ✅ | Sistema de cartas, mão/deck/Command Points, tipos completos |
| `UBattleEffect` (base) + 6 efeitos | UObject plugável | ✅ | DamageRegiment, MoraleShift, AddPersistent, RepositionSide, DrawCards, ExhaustEnemyCard |
| `UBattleAIController` + `UBattleAIProfile` | UObject/DataAsset | ✅ | Utility AI por perfil; **sem lookahead** (placeholder na sub-etapa 8) |
| `UBattleReplayService` | Service | ✅ | Logs/replay para pós-batalha |
| `ABattleVisualizer` / `ABattleCameraPawn` / `UBattleHUDWidget` | Actor/Pawn/Widget | ⚠️ | Scaffolding C++ existe; conteúdo BP/visual e input de jogador pendentes |
| Gatilho mapa→batalha | — | ❌ | `UMilitarySubsystem` ainda não dispara `FBattleProposal` ao colidir exércitos |

**Resumo:** sub-etapas 1–10 presentes em código; falta sub-etapa 11 (polish/VFX/animação), o conteúdo visual BP e a ligação com o militar estratégico.

---

## StrategosAI

| Elemento | Tipo | Estado | Observação |
|---|---|---|---|
| `UAIPlaceholderSubsystem` | World | ✅ (Etapa 1) | Regras determinísticas por arquétipo (Militarist move/ataca, Diplomat opinião/aliança, Merchant investe, Pragmatist aleatório) + sucessão de líder anual |
| `UAIDirectorSubsystem` (Director/Advisors/Brain) | — | ❌ | Não existe; é o alvo da Etapa 3 |

---

## StrategosUI

| Elemento | Tipo | Estado | Observação |
|---|---|---|---|
| `UStrategosHUDWidget` | UserWidget | ✅ v2 (C++) | Provider de dados completo: tempo, tesouraria, índices, estoques/preços, shortfalls, buildings, decisões pendentes; layout fica no filho BP |
| `AStrategosPlayerController` / `AStrategosCameraPawn` | Controller/Pawn | ⚠️ | Presentes (input/câmera estratégica); profundidade não auditada em C++ |
| `AStrategosMapActor` / `AStrategosProvinceVisualActor` | Actor | ⚠️ | Camada de render do mapa |
| Modais (Event, CountryCard, WarRoom, CommanderCard, DiplomacyAction) | UserWidget | ⚠️ | Scaffolding C++; arte/layout final em BP |

---

## StrategosData

| Elemento | Estado | Observação |
|---|---|---|
| Módulo | ✅ | Registro de módulo apenas; DataAssets vivem hoje em StrategosCore (ativação pendente) |

---

## StrategosWorldGen (experimental, fora do core)

| Elemento | Estado | Observação |
|---|---|---|
| Pipeline (`Heightmap`, `Climate`, `Biome`, `Voronoi`, `RiverTracer`, `ProvinceGeographyClassifier`, `WorldGenSubsystem`) | ✅ | Geração procedural determinística; produz `FProvinceGeography` consumida pelas províncias |
| Export PNG de debug (`AWorldGenDebugActor`) | ✅ | Texturas por estágio |

---

## O que NÃO existe ainda (planejado)

- **Politics** (`UPoliticsSubsystem`) — ideologias, leis, governos, facções, revoltas
- **Progress/Tech** (`UProgressSubsystem` + Vitória) — só `RequiredTechId` reservado nos assets
- **AI Director** real (Director + Advisors + Brain por nação)
- **Diplomacia v2** — ações, tratados compostos, Casus Belli, esferas, prestígio
- **Militar avançado** — suprimento, frentes, cerco, mobilização, naval
- **Composição de unidade em 5 eixos** (Class/Equipment/Doctrine/Experience/Modifiers) — hoje só `UUnitTypeAsset` + stats
- **Mercados regionais** — economia é pool nacional
- **Migração de save** entre versões
- **Recursos/Produção Sessões 2–8** (slots, elegibilidade, recurso principal, tech tier, integração, modificadores, UI)
- **Visualização tática BP** e gatilho mapa→batalha

---

## Próximas prioridades sugeridas (alinhadas ao roadmap)

1. Ligar `UMilitarySubsystem` → `UBattleSubsystem` (gerar `FBattleProposal` no encontro de exércitos) e fechar a visualização BP.
2. Diplomacia v1 → ações mínimas (DeclareWar/Peace/Alliance) sobre a matriz existente.
3. Recursos Sessão 2 (slots) sobre a geografia já entregue na v6.
4. `UProgressSubsystem` mínimo (3 nós) destravando os `RequiredTechId` que já existem nos assets.
5. `UPoliticsSubsystem` para ativar os strata stub (Aristocracy/Soldier/Clergy) e fechar o loop social.
