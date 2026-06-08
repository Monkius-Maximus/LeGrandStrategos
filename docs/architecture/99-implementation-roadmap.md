# 99 — Roadmap de Implementação

Plano consolidado por etapas, organizando os planos individuais de cada subsistema em um cronograma global de MVP → Protótipo → Vertical Slice → Versão Completa.

---

## Etapa 0 — Fundação (1–2 semanas)

Objetivo: criar o esqueleto sobre o qual tudo será construído.

- `GameInstance`, `GameMode`, `GameState`
- `UGameFlowSubsystem` com FSM e os 7 estados (vazios, com logs de transição)
- `UTimeSubsystem` + UI de play/pause/velocidade
- `UEventBusSubsystem`
- Setup de C++/módulos (`StrategosCore`, `StrategosUI`, `StrategosBattle`, `StrategosAI`, `StrategosData`)

**Critério de pronto**: jogo compila, abre menu, transita para mapa vazio, pausa funciona.

---

## Etapa 1 — MVP (4–6 semanas)

Objetivo: mapa com províncias, nações coloridas, tempo passa, jogador move exército.

- `UWorldState`, `UNation`, `UProvince`, `UArmy` (sem economia ainda)
- `UMapSubsystem` (mesh por província, picking)
- `UMilitarySubsystem` v1 (movimento por adjacência, sem suprimento)
- `USaveSubsystem` mínimo (carrega/salva mundo)
- 1 nação jogador, 2 nações IA com IA placeholder

**Critério de pronto**: jogador seleciona província, move exército por dias simulados, salva e recarrega.

---

## Etapa 2 — Protótipo (6–8 semanas)

Objetivo: simulação viva. Nações têm vida própria, eventos disparam, batalhas resolvem.

- `UEconomySubsystem` v1 (produção + consumo + tesouraria; sem mercado complexo)
- `UDiplomacySubsystem` v1 (paz/guerra/aliança apenas)
- `UEventSubsystem` + 5 eventos de exemplo
- `UAIDirectorSubsystem` v1 (utility AI simples, 1 Advisor)
- `UBattleResolverService` (auto-resolve sem UI tática)
- `UProgressSubsystem` mínima (3 nós encadeados liberando 1 equipamento)

**Critério de pronto**: rodar 50 anos simulados sem intervenção e ver economia, guerra e diplomacia evoluindo.

---

## Etapa 3 — Vertical Slice (8–10 semanas)

Objetivo: uma campanha jogável de ponta a ponta com o pilar de combate funcionando.

- `UCommander` + `UCommanderDeck` + 20 cartas
- `UBattleSubsystem` com mapa tático completo
- Auto-resolve usando o mesmo solver
- Mercados regionais (`UMarketRegion`)
- 3 governos diferentes, leis, facções (`UPoliticsSubsystem` completo)
- Tech tree v1 (3 ramos × 5 nós)
- Casus Belli + Tratados compostos
- UI completa para 1 cenário (1836 vitoriano com 5–10 nações)

**Critério de pronto**: jogador termina uma campanha de 30 anos com batalhas táticas, política interna, diplomacia e tecnologia funcionando juntos.

---

## Etapa 4 — Versão Completa

Objetivo: escala, profundidade e polimento.

- Mais escala (50+ nações, 500+ províncias, 100+ eventos)
- Religião, cultura, migração detalhadas
- Sistemas avançados de comércio (rotas externas, esferas econômicas)
- Mecanização tardia (tanques, aviação)
- Multiplayer (a arquitetura já está pronta porque GameState é replicável e RNG é determinístico)
- Mod tools (expor DataAssets + scripting C# se quiser)

---

## Ordem Recomendada Detalhada

1. **Esqueleto Unreal**: módulos C++ separados (`Core`, `UI`, `Battle`, `AI`, `Data`)
2. **GameFlowSubsystem + 7 estados** vazios, com transições + logs
3. **TimeSubsystem** com play/pause + UI mínima
4. **EventBusSubsystem** (depois de saber os primeiros eventos, não antes)
5. **WorldState + Nation + Province** (UObjects puros, sem lógica)
6. **MapSubsystem** (renderizar províncias, selecionar)
7. **SaveSubsystem** (testar serialização cedo evita dor depois)
8. **MilitarySubsystem** v1 (movimento por adjacência)
9. **EconomySubsystem** v1 (produção/consumo simples)
10. **DiplomacySubsystem** v1 (paz/guerra/aliança)
11. **AIDirectorSubsystem** v1 (utility AI por nação)
12. **EventSubsystem** + primeiros eventos
13. **BattleResolverService** (auto-resolve) — testa o pipeline antes do tático
14. **CommanderDeck + BattleCardAsset**
15. **BattleSubsystem** completo (mapa tático)
16. **ProgressSubsystem** (tech)
17. **Política avançada** (facções, leis, ideologias)
18. **Diplomacia avançada** (CB, esferas, prestígio)
19. **AI Director completo** (todos Advisors, tiers)
20. **Sistemas militares avançados** (suprimento, frentes, naval)
21. **Polish + multiplayer + mod tools**

---

## Pontos de Atenção Transversais

- **Determinismo desde o dia 1**: RNG centralizado, ordem de iteração estável
- **Não use Actors para entidades simuladas**
- **Tudo que é "data" vira DataAsset**. Tudo que é "estado" vira UObject. Tudo que é "comportamento global" vira Subsystem
- **Teste save/load semanalmente**
- **Profile cedo**: rodar 100 anos simulados em < 30 segundos deve ser meta desde o protótipo
- **Telemetria de IA desde o dia 1**: sem logs estruturados, debugar IA é impossível
- **Conteúdo > Mecânica**: 30 eventos bem balanceados são melhores que 1000 mal feitos. Idem para techs, leis, cartas

---

## O Que Resta Detalhar Após a Arquitetura

`USaveSubsystem` e `UMapSubsystem` já estão **implementados em v1** (sem doc dedicado). O que resta neles é evolução técnica/UX, não a base:

- **`USaveSubsystem`** (v1 funcional, SaveVersion 6) — falta: migração entre versões, serializar Brains/History/Treaties/Sieges quando esses sistemas existirem
- **`UMapSubsystem`** (v1 funcional: índice espacial + picking) — falta: modos de mapa (político, econômico, militar, religioso, cultural, ferroviário), fog of war, animações de fronteira
- **`UTimeSubsystem`** — refinamento: pausas automáticas em eventos críticos, calendário, eras visuais
- **Pipeline de UI** — HUD estratégico, painéis de subsistemas, integração com Event Bus para alertas

E o que vem **depois** da arquitetura, quando começar a implementação:

- **Networking/multiplayer** — a arquitetura já está pronta (GameState replicável, RNG determinístico, Brain por nação)
- **Modding** — DataAssets já são extensíveis; falta diretório de mods e loader dinâmico
- **Tooling** — editor de eventos, balanceador de unidades, validador de tech tree
- **Performance profiling** — meta concreta: 1 ano simulado em <2s com 50 nações

---

## Resumo do Inventário de Subsistemas

Coluna "Estado de Implementação" reflete o código em `main`. Detalhe granular em [`98-estado-implementacao.md`](98-estado-implementacao.md).

| Subsistema | Documentação | Status Arquitetural | Estado de Implementação |
|---|---|---|---|
| `UGameFlowSubsystem` | [`01-game-flow.md`](01-game-flow.md) | Definido | ✅ Implementado |
| `UTimeSubsystem` | [`02-time.md`](02-time.md) | Definido | ✅ Implementado |
| `UEventBusSubsystem` | [`03-event-bus.md`](03-event-bus.md) | Definido | ✅ Implementado |
| `UBattleSubsystem` | [`10-battle.md`](10-battle.md) | Detalhado | ✅ Núcleo (sub-etapas 1–10); visualização BP + polish pendentes |
| Sistema de Unidades | [`11-units.md`](11-units.md) | Detalhado | ⚠️ Parcial — `UUnitTypeAsset` + stats de Army; composição em 5 eixos não implementada |
| `UEconomySubsystem` | [`20-economy.md`](20-economy.md) | Detalhado | ✅ Implementado v2 (tick de 9 fases, produção, construção, impostos, auto-invest) |
| `UProgressSubsystem` + Vitória | [`21-progress.md`](21-progress.md) | Detalhado | ❌ Pendente (apenas `RequiredTechId` reservado nos assets) |
| `UEventSubsystem` | [`30-events.md`](30-events.md) | Detalhado | ✅ Implementado v2 (MTTH, conditions/effects plugáveis, fila de decisões) |
| `UPoliticsSubsystem` | [`31-politics.md`](31-politics.md) | Detalhado | ❌ Pendente |
| `UDiplomacySubsystem` | [`32-diplomacy.md`](32-diplomacy.md) | Detalhado | ✅ v1 — matriz Status+Opinion; ações (DeclareWar/Alliance) e tratados pendentes |
| `UAIDirectorSubsystem` | [`40-ai-director.md`](40-ai-director.md) | Detalhado | ❌ Pendente — hoje só `UAIPlaceholderSubsystem` (regras por arquétipo) |
| `UMilitarySubsystem` | [`41-military.md`](41-military.md) | Detalhado | ✅ v1 — movimento por adjacência; sem suprimento/frentes/naval |
| `USaveSubsystem` | — | Esboçado em [`00-overview.md`](00-overview.md) | ✅ v1 implementado (SaveVersion 6); sem migração |
| `UMapSubsystem` | — | Esboçado em [`00-overview.md`](00-overview.md) | ✅ v1 implementado (índice espacial, picking); sem modos de mapa/fog |
