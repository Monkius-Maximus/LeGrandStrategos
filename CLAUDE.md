# CLAUDE.md — Le Grand Strategos

> Este arquivo é lido automaticamente pelo Claude Code no início de toda sessão dentro deste repositório. Mantenha-o focado no que **não muda** sessão-a-sessão. Para "onde paramos hoje", olhe o `PROJECT_STATE.md` que o usuário cola no início da conversa (ele mora no Obsidian do usuário, não no repo).

---

## O que é este projeto

**Le Grand Strategos** — Grand Strategy 3D em Unreal Engine 5.5, ambientado na transição industrial (1820–1880). Inspirações: Victoria 2/3, EU4, Civilization, Romance of the Three Kingdoms.

Mapa **2D puro** (estilo Vic2 / Civ3 / Humankind sem mapa 3D). UI **gridada com módulo 8px** @ 1920×1080.

---

## Stack

- **Unreal Engine 5.5**, C++, Build Settings V5
- **Idioma primário: Português (PT-BR).** Código, comentários, commits, UI — tudo PT exceto identificadores C++ (que ficam em inglês por convenção UE).
- 5 módulos:
  - **StrategosCore** — subsistemas de simulação (Time, Map, Military, Economy, Events, Diplomacy, Save, GameFlow, World containers)
  - **StrategosData** — DataAssets (ativação pendente; hoje vivem em StrategosCore)
  - **StrategosBattle** — combate tático (futuro)
  - **StrategosAI** — `UAIPlaceholderSubsystem` por arquétipo (Militarist/Diplomat/Merchant/Pragmatist)
  - **StrategosUI** — UMG, HUD widget, popups de Decision

---

## Arquitetura — onde está documentada

- `docs/architecture/*.md` — 15 docs cobrindo cada subsistema (Time, Map, Military, Economy, Events, Politics, Diplomacy, Progress, Battle, AI, etc.)
- `docs/setup/*.md` — guias para popular conteúdo via editor (DataAssets de Eventos, Economia, UI grid)
- `PROJECT_STATE.md` — **mora no Obsidian do usuário**. Estado vivo do projeto: feito, em andamento, próximo, perguntas abertas. Usuário cola no início de cada sessão.

---

## Convenções

### Naming
- **Bens** (`UGoodAsset`): `G_Grain`, `G_IronOre`...
- **Métodos de produção** (`UProductionMethodAsset`): `PM_BasicGrainFarming`...
- **Modificadores** (`UProductionModifierAsset`): `M_DoubleShifts`, `M_QualityFocus`...
- **Prédios** (`UBuildingTypeAsset`): `BT_GrainFarm`...
- **Unidades** (`UUnitTypeAsset`): `U_Infantry_Line`, `U_Cavalry_Line`...
- **Eventos** (`UEventAsset`): `E_BountifulHarvest`, `E_WorkerStrike`...
- **Registries** (`UEventContentRegistry` etc): `DA_EventRegistry`, `DA_EconomyRegistry`...

### Padrões adotados
- **DataAssets > código hardcoded** para conteúdo de jogo (designer-editável)
- **Determinismo absoluto** via `FRandomStream` seeded por `(NationId, Date, EventId)` — crítico para save/load e multiplayer futuro
- **Plugin pattern** para Conditions/Effects de Events: cada novo tipo = 1 header + 1 cpp, zero mudança no core
- **Padrão fallback**: subsistemas funcionam sem DataAssets do editor (ex.: 5 eventos hardcoded se `UEventContentRegistry` não estiver setado)

### Padrões evitados
- Singletons fora dos subsystems do UE
- Exceptions (use return enums / TOptional)
- Comentários redundantes — código bem nomeado dispensa explicação
- Emojis em código, commits, docs

### Commits
- Conventional commits: `feat(scope):`, `fix(scope):`, `docs(scope):`, `refactor(scope):`
- Mensagem em inglês (convenção git), mas docs e código em PT
- **1 sessão = 1 commit lógico** (deploy é responsabilidade do usuário, mas Claude pode commitar quando autorizado)
- NUNCA usar `--amend` automaticamente — sempre novo commit
- NUNCA `--no-verify`

### Branches
- Desenvolvimento atual em `claude/grand-strategy-architecture-eIYSr`
- Não fazer merge pra `main` sem aprovação explícita

---

## Save format — atenção crítica

`UStrategosSaveData::SaveVersion` é incrementado a cada expansão do snapshot. Versão atual: **5**.

Histórico:
- 1: base (Nations, Provinces, Armies)
- 2: POPs + Buildings em províncias
- 3: economia completa + pending decisions de Events
- 4: identidade visual de Nation + expansão de Army (stats, modifiers, state, XP)
- 5: matriz de Diplomatic Relations

**Toda mudança que adiciona campo a algum `FRecord` precisa bumpar SaveVersion** e atualizar o comentário no header.

---

## Decisões travadas (não reabrir sem motivo forte)

- **Mapa 2D puro**, não 3D. Sprites/billboards para exércitos.
- **Módulo de grid: 8px** @ 1920×1080. Resolução-alvo PC fixa por agora.
- **Stats de unidade:** ATQ, DEF, MOB, MOR, ORG, SUP (principais) + ALC, PREC, SUPR, REC, CST (secundárias). Travado em `docs/setup/etapa-2-ui-grid.md`.
- **3 nações canônicas:** Albion, Galia, Norden.
- **Rarity de unidades:** descartado. Sem Common/Rare/Epic.
- **Stance de unidade** (Carga/Reconhecer/Escaramuça): adiado, entra com BattleResolver.
- **Hybrid building ownership:** Government + Private (Bourgeoisie auto-investe via profitability scoring com FRandomStream).
- **3 mutex groups de PM modifiers:** Pace / Labor / Quality.
- **Bourgeoisie:** stratum ativo no MVP. Aristocracy/Soldier/Clergy são stubs.

---

## Fora de escopo (não sugerir)

- Mapa 3D com terreno/iluminação dinâmica
- Animação de unidades estilo Total War
- Multiplayer (planejado para Etapa 4, não MVP)
- Casus Belli, Trust separado, Prestígio, Esferas de Influência (Diplomacy v2+)
- Sistema de tecnologias completo (Progress v1 = 3 nós, suficiente)
- UI mobile / touch
- Localização para outras línguas (PT-BR só, no MVP)

---

## Pegadinhas conhecidas

- **`TMap` iteration order não é estável.** Sempre que ordem importa (eventos, AI), ordenar explicitamente antes (alfabético por Id).
- **`AddDynamic` delegate requer `UFUNCTION()`** no handler. Esquecer trava em runtime sem erro de compile.
- **`TSoftObjectPtr::LoadSynchronous()`** dentro de tick é proibido — preload no Initialize.
- **`UObject::GetWorld()` para subobjetos** walks outer chain. Funciona se outer é Actor; falha se outer é puro UObject.
- **Save format**: se você adicionar campo a um `FRecord` e esquecer de serializar em `CaptureSnapshot`/`ApplySnapshot`, perde silenciosamente.
- **`UWorldSubsystem` não tem `GetGameInstance()` direto.** Use `GetWorld()->GetGameInstance()`.

---

## Glossário (termos do projeto)

- **POP** — agrupamento populacional por stratum em uma província
- **Stratum** — classe social: Laborer / Artisan / FactoryWorker / Bourgeoisie (ativos) + Aristocracy / Soldier / Clergy (stub)
- **PM** — Production Method (`UProductionMethodAsset`), "carta" trocável que define inputs/outputs/emprego de um prédio
- **Modifier** — `UProductionModifierAsset`, buff/debuff aplicável a um prédio (3 mutex groups)
- **MTTH** — Mean Time To Happen, gate probabilístico de eventos
- **Strategic Indices** — 3 floats em [0.5, 1.5] (MilitaryReadiness, CivilianMorale, IndustrialCapacity)
- **NAP** — Non-Aggression Pact (status diplomático)
- **Arquétipo** — personalidade de líder/IA: Militarist / Diplomat / Merchant / Pragmatist

---

## Como abordar uma nova sessão (Claude Code)

1. **Leia este arquivo** (você está fazendo isso agora — é automático)
2. **Verifique se o usuário colou um `PROJECT_STATE.md`** no início da mensagem. Se sim, esse é o "onde paramos hoje" — leia.
3. **Se a tarefa envolve subsistema específico**, leia o doc correspondente em `docs/architecture/` antes de codar.
4. **Antes de criar algo novo**, dê um `grep`/`find` para confirmar que não existe ainda.
5. **Edite arquivos com `Edit`, não `Write`**, exceto pra criar do zero.
6. **Não commite/pushe** sem autorização explícita do usuário (combinado: deploys são responsabilidade do usuário pra economizar tokens, mas ele pode autorizar pontualmente).
7. **Comunique progresso conciso** — uma frase por marco, não narrativa.
8. **Fim da sessão:** se o usuário pedir, atualize/regenere o conteúdo do `PROJECT_STATE.md` pra ele colar no Obsidian.

---

## Roadmap macro

Estágios da arquitetura (todos documentados em `docs/architecture/`):

- **Etapa 0** — Foundation ✅
- **Etapa 1** — MVP (World, Map, Military, Save, AI placeholder) ✅
- **Etapa 2** — Prototype (Economy ✅ + Events ✅ + UI Cards 🚧 + Diplomacy 🚧)
- **Etapa 3** — Sistemas internos (Progress, Politics, Battle, AI Director)
- **Etapa 4** — Escala (Multiplayer, conteúdo, polish)

O detalhe granular do que falta dentro de cada etapa vive no `PROJECT_STATE.md`.
