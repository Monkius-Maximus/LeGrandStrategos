# Documentação — Le Grand Strategos

Este diretório contém a documentação técnica do projeto. Atualmente o foco está na **arquitetura de sistemas**, organizada em arquivos separados por subsistema para facilitar navegação, revisão e manutenção.

---

## Como Navegar

Comece pela **visão geral** para entender as camadas e o vocabulário da arquitetura. Depois, mergulhe em qualquer subsistema na ordem que preferir — cada arquivo é autoexplicativo, mas há referências cruzadas quando relevantes.

Se quiser um olhar de planejamento (em vez de design), pule direto para o **roadmap de implementação**.

---

## Índice da Arquitetura

### Fundação

| Documento | Conteúdo |
|---|---|
| [`00-overview.md`](architecture/00-overview.md) | Visão geral: 5 camadas (Apresentação, Orquestração, Subsistemas, Domínio, Dados/Serviços), mapeamento Unreal de cada camada, regras de dependência, modelo de classes sugerido, save/load e diagrama macro. |
| [`01-game-flow.md`](architecture/01-game-flow.md) | `UGameFlowSubsystem` — FSM principal com 7 estados (MainMenu, Loading, Running, Paused, Battle, Event, GameOver), transições permitidas e fluxos típicos. |
| [`02-time.md`](architecture/02-time.md) | `UTimeSubsystem` — metrônomo da simulação, granularidade por subsistema (OnDay/OnMonth/OnYear), determinismo para replay. |
| [`03-event-bus.md`](architecture/03-event-bus.md) | `UEventBusSubsystem` — infraestrutura pub/sub que desacopla os subsistemas. |

### Combate e Unidades

| Documento | Conteúdo |
|---|---|
| [`10-battle.md`](architecture/10-battle.md) | `UBattleSubsystem` — batalha tática com fases (Setup → Engagement → Climax → Pursuit), cartas, IA tática, auto-resolve. |
| [`11-units.md`](architecture/11-units.md) | Sistema de Unidades com composição em 5 eixos (Class + Equipment + Doctrine + Experience + Modifiers) e o `URegimentResolver`. |

### Simulação Macro

| Documento | Conteúdo |
|---|---|
| [`20-economy.md`](architecture/20-economy.md) | `UEconomySubsystem` — POPs por estrato, indústrias, mercados regionais, tesouro nacional, conexão com produção militar. |
| [`21-progress.md`](architecture/21-progress.md) | `UProgressSubsystem` — tech tree com Eras, `FTechUnlockSet` como contrato único de desbloqueios, mecânicas de difusão, e `UVictorySubsystem` com checks plugáveis. |

### Eventos e Sociedade

| Documento | Conteúdo |
|---|---|
| [`30-events.md`](architecture/30-events.md) | `UEventSubsystem` — eventos narrativos com 5 tipos, indexação por trigger, condições + efeitos plugáveis, encadeamento, modificadores persistentes. |
| [`31-politics.md`](architecture/31-politics.md) | `UPoliticsSubsystem` — ideologias, leis, governos, capital político, eleições, facções, militância, revoltas. |
| [`32-diplomacy.md`](architecture/32-diplomacy.md) | `UDiplomacySubsystem` — Opinion vs Trust, tratados compostos, Casus Belli com fabricação, esferas de influência, prestígio, infâmia, guerra. |

### Inteligência e Estratégia

| Documento | Conteúdo |
|---|---|
| [`40-ai-director.md`](architecture/40-ai-director.md) | `UAIDirectorSubsystem` — Director + Conselheiros + Brain por nação, Strategy + Personality + ContextCache, Advisors por domínio. |
| [`41-military.md`](architecture/41-military.md) | `UMilitarySubsystem` — exércitos, frotas, recrutamento, movimento, suprimento, cerco, ocupação, mobilização, frentes. |

### Planejamento

| Documento | Conteúdo |
|---|---|
| [`99-implementation-roadmap.md`](architecture/99-implementation-roadmap.md) | Roadmap consolidado: Fundação → MVP → Protótipo → Vertical Slice → Versão Completa. Ordem recomendada de implementação. |

---

## Princípios Transversais

Independente do subsistema, alguns princípios atravessam toda a arquitetura:

1. **Composição em vez de hierarquia** — DataAssets compostos em vez de classes derivadas
2. **Comunicação por eventos** — subsistemas nunca chamam outros diretamente
3. **Dados estáticos em DataAssets, estado em UObjects, comportamento global em Subsystems**
4. **Determinismo desde o dia 1** — RNG centralizado, ordem de iteração estável
5. **Personalidade > Otimização** — IA e nações com knobs de comportamento, não apenas heurísticas perfeitas
6. **Fog of war narrativo, não punitivo** — IAs operam sobre estimativas, mas não cegas
7. **Conteúdo plugável** — designers, modders e tradutores trabalham em DataAssets sem tocar C++

---

## Convenções de Nome

- `UXxxSubsystem` — `UWorldSubsystem` ou `UGameInstanceSubsystem`
- `UXxxAsset` — `UPrimaryDataAsset` (dados estáticos)
- `UXxx` (sem sufixo Asset) — `UObject` puro (estado em runtime)
- `FXxx` — `USTRUCT` (DTO ou agregado de valores)
- `EXxx` — `UENUM`

---

## Status de Detalhamento

Todos os 14 subsistemas críticos do gameplay estão **detalhados** em arquitetura. Restam para serem aprofundados quando a implementação começar:

- `USaveSubsystem` — esboçado em `00-overview.md`
- `UMapSubsystem` — esboçado em `00-overview.md`

Esses dois são técnicos/UX e ganham complexidade real só quando os outros sistemas estiverem implementados, então o detalhamento foi adiado de propósito.

---

## Como Contribuir com a Documentação

- Cada arquivo segue o mesmo formato: princípios → estrutura → integrações → diagrama → plano → pontos de atenção.
- Mudanças significativas (novos subsistemas, mudança de paradigma) devem virar **commits separados** por subsistema, mantendo o histórico granular.
- Trechos de código são ilustrativos, não código de produção. Servem para fixar o contrato de cada classe.
- Referências cruzadas usam links relativos (ex: `[Battle](architecture/10-battle.md)`).

---

# Abstract (English)

**Le Grand Strategos** is a grand strategy game in development with the Unreal Engine, inspired by *Victoria 2/3*, *Civilization*, *Humankind* and *Romance of the Three Kingdoms*. It combines deep macro simulation (POPs, regional markets, ideologies, diplomacy with spheres of influence) with a tactical battle layer based on commanders, equipped regiments and a card-based decision system.

This `docs/` directory contains the **system architecture documentation**, written entirely in Portuguese, organized by subsystem under [`architecture/`](architecture/). Each document follows the same structure: architectural principles, data structures, integrations with other subsystems, final diagram, implementation plan and pitfalls to avoid.

### What you will find here

- **Foundation** (`00-03`): layered architecture (Presentation, Orchestration, Subsystems, Domain, Data/Services), the global state machine (`UGameFlowSubsystem`), the simulation tick (`UTimeSubsystem`) and the decoupled communication infrastructure (`UEventBusSubsystem`).
- **Combat and Units** (`10-11`): tactical battle subsystem with phases and card-based command (`UBattleSubsystem`), and the unit composition system that combines Class + Equipment + Doctrine + Experience + Modifiers instead of deep class hierarchies.
- **Macro Simulation** (`20-21`): economy with aggregated POPs, industries, regional markets and national treasury (`UEconomySubsystem`); technology tree with eras, victory checks and dynamic unlocks (`UProgressSubsystem` + `UVictorySubsystem`).
- **Events and Society** (`30-32`): narrative event subsystem with pluggable triggers/conditions/effects (`UEventSubsystem`), internal politics with ideologies, laws and revolts (`UPoliticsSubsystem`), and external diplomacy with treaties, casus belli and spheres of influence (`UDiplomacySubsystem`).
- **Intelligence and Strategy** (`40-41`): national AI orchestrator with Director + Advisors + Brain per nation (`UAIDirectorSubsystem`), and the strategic military layer with armies, fleets, supply, sieges and mobilization (`UMilitarySubsystem`).
- **Planning** (`99`): consolidated implementation roadmap from Foundation through MVP, Prototype, Vertical Slice and Full Release.

### Where to start

If you are a developer joining the project, read [`00-overview.md`](architecture/00-overview.md) first to understand the layering and Unreal mappings, then jump to whichever subsystem interests you. If you want to plan work, go straight to [`99-implementation-roadmap.md`](architecture/99-implementation-roadmap.md). All docs are interlinked, so you can navigate organically.

Code samples in the docs are **illustrative**, not production code — they exist to pin down each class's contract and the data flow between subsystems.
