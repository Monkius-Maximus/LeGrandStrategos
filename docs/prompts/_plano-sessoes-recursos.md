# Plano de Sessões — Sistema de Recursos e Produção

> Só escopo e ordem. O prompt completo de cada sessão é montado quando você for executá-la, com base no que a anterior deixou pronto. Não escrever prompts adiantados — cada sessão depende do estado real deixada pela anterior.

Referência de desenho: `docs/architecture/40-resources-and-production.md`.

---

## ✅ Sessão 1 — Geografia da província
Enums Topography/Climate/Vegetation + struct Hydrography em `UProvince`. Persistência, SaveVersion 6. Sem comportamento. **Prompt pronto** em `sessao-1-terreno-provincia.md`.

## Sessão 2 — Slots
`SlotsTotal = base(topografia) + hydroBonus + developmentBonus`. Cálculo, contagem de ocupação, validação ao construir. Pool único (sem rural/urbano). Tabela de slots base por topografia em DataAsset de constantes. SaveVersion bump (slots ocupados são estado).

## Sessão 3 — Elegibilidade de recursos
`UResourceEligibilityRule` no padrão plugin (igual Conditions/Effects). Metadata de requisito geográfico em cada `UGoodAsset` bruto. Função `CanProduce(Province)`. Sem fallback — falhou, não aparece. Ainda sem UI.

## Sessão 4 — Recurso principal
Determinação determinística via `FRandomStream` seeded por `(LocationId, WorldSeed)`. Prioridade vegetação → relevo+clima → sustento. Grava `PrimaryResource` na província. SaveVersion bump.

## Sessão 5 — Tech tier da nação
`ENationTechTier { Underdeveloped, Developing, Developed }`. Campo na nação (simples no Alpha) + `RequiredTier` em cada `UProductionMethodAsset`. Decidir aqui se tier é campo direto ou derivado de Progress v1. Gating: edifício só oferece PMs com tier ≤ tier da nação.

## Sessão 6 — Integração com Economy v1
Edifício vira construível só se: elegível (S3) + slot livre (S2) + PM disponível pro tier (S5) + mão de obra. Liga as camadas no fluxo de construção existente. Produção/preço/mercado **inalterados**.

## Sessão 7 — Modificadores de produção
Modificadores de terreno/clima/hidrografia aplicados via `UActiveModifierRegistry` (mesmo canal dos modifiers de evento). Tabelas de balanceamento em DataAsset de constantes. Sem caminho novo de aplicação.

## Sessão 8 — UI
Lista de slots, recursos elegíveis (com cadeado + motivo do bloqueio), e PMs disponíveis por tier. Cartas no grid 8px existente.

---

## Trilha paralela (não bloqueia as acima, mas depende da S1)

**Visual de relevo no mapa** — câmera inclinada, heightmap shading, mesh decorations instanciados, camada de água separada. Consome a geografia da Sessão 1. Pode rodar a qualquer momento depois da S1, em paralelo às sessões de mecânica.

---

## Princípio de execução

Uma sessão por vez. Mecânica antes de visual. Cada SaveVersion bump confere todos os pontos de comparação de versão. Nada de adiantar escopo de sessão futura "já que estou aqui".
