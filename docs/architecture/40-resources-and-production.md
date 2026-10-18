# 40 — Sistema de Recursos e Produção

**Status:** Em execução. Estende Economy v1.
**Sessão 1 (geografia de província): ✅ implementada** — `FProvinceGeography` (topografia, clima, vegetação, hidrografia), persistida na **SaveVersion 6**. Classificação via `StrategosWorldGen`.
**Pendentes:** Sessões 2–8 (slots, elegibilidade, recurso principal, tech tier, integração, modificadores, UI) — ver `docs/prompts/_plano-sessoes-recursos.md`.

---

## Decisões travadas

Essas quatro decisões fecharam o desenho. Não reabrir sem motivo forte.

1. **Estende Economy v1, não substitui.** O motor de simulação continua sendo o que já existe: `UProductionMethodAsset`, precificação dinâmica, mercado local, tick de 9 fases, auto-investimento da Bourgeoisie. Este sistema adiciona **quatro camadas em cima**: geografia da província, slots, elegibilidade de recursos e determinação do recurso principal. Não toca na lógica de produção/preço/mercado já implementada.

2. **Vocabulário industrial, não medieval.** População usa os strata do projeto: `Laborer`, `Artisan`, `FactoryWorker`, `Bourgeoisie` (ativos) + `Aristocracy`, `Soldier`, `Clergy` (stub). Nada de peasants/burghers/nobles/clerics/slaves — são anacrônicos para 1820–1880.

3. **Tech tier gating é o coração do tema.** O jogo simula a transição napoleônica → vitoriana com desenvolvimento desigual. Cada nação tem um tier (`Developed` / `Developing` / `Underdeveloped`) que destrava quais métodos de produção pode usar. Os bens "primitivos" do desenho original (ferro de forja, tear manual, cavalaria irregular) **não são anacronismos** — são os métodos de produção de tier baixo do mesmo bem. Mali em 1850 roda `PM_BloomeryIron`; Inglaterra roda `PM_BessemerSteel`. Mesmo bem `G_Steel`, métodos diferentes, gated por tier + pesquisa (Progress v1).

4. **Execução em etapas.** Sessão 1 entrega só geografia de província. As demais (slots, elegibilidade, primary resource, tech tier, integração, modificadores, UI) vêm uma a uma. Ver `_plano-sessoes-recursos.md`.

---

## Visão geral

Cada província ganha **geografia estática** (relevo, clima, hidrografia, vegetação) que determina:

- Quais **recursos brutos** ela pode extrair (elegibilidade geográfica).
- Qual é seu **recurso principal** (o RGO padrão, determinístico por seed).
- Quantos **slots** ela tem para edifícios adicionais.
- **Modificadores** de produção, crescimento populacional e movimento militar.

Sobre essa base, o **tech tier da nação** determina *como* os bens são produzidos — qual método de produção está disponível. A geografia diz *o que* o lugar pode dar; o tier diz *com que tecnologia* você transforma isso.

```
Geografia (província)  ──>  o QUE pode ser extraído/produzido aqui
Tech tier (nação)      ──>  COM QUE método (primitivo vs industrial)
Economy v1 (motor)     ──>  preço, mercado, mão de obra, lucro, auto-invest
```

---

## Camada 1 — Geografia da província (estática)

Quatro eixos independentes. Decompostos de propósito: relevo ≠ vegetação ≠ clima ≠ água. Isso evita combinações impossíveis e dá granularidade pra elegibilidade.

### `ETerrainTopography`
`Flatland`, `Hills`, `Mountains`, `Plateau`, `Wetlands`, `Coastal`

### `EClimateZone`
`Tropical`, `Mediterranean`, `Continental`, `Arid`, `Arctic`, `Oceanic`

### `EVegetationCover`
`DenseForest`, `LightForest`, `Grassland`, `Wetland`, `Desert`, `Tundra`

### Hidrografia — flags (não enum, podem coexistir)
`bHasMinorRiver`, `bHasNavigableRiver`, `bHasLake`, `bHasAquifer`, `bIsCoastal`

> Estática = definida no mapa (handcrafted ou geração), não muda em runtime. Limpar floresta muda `VegetationCover` (ação de jogador, fora da Sessão 1).

---

## Camada 2 — Slots

```
SlotsTotal = SlotsBase(Topography) + HydroBonus + DevelopmentBonus
```

- `SlotsBase`: tabela de relevo (Flatland=3, Hills=2, Mountains=1, …).
- `HydroBonus`: +1 se `bHasNavigableRiver` ou `bIsCoastal` (slot de porto/comércio).
- `DevelopmentBonus`: +1 a cada 25 de desenvolvimento (máx +4).

Slots são consumidos por edifícios de exploração (recurso alternativo) e de processamento. **Para o Alpha, pool único** — não separar rural/urbano (overengineering pra esta fase).

---

## Camada 3 — Elegibilidade de recursos

Função central, no padrão plugin que o projeto já usa para Conditions/Effects:

```cpp
// UResourceEligibilityRule (UObject derivável)
// Cada UGoodAsset raw aponta para uma regra.
virtual bool CanProduce(const UProvince* Province) const;
```

Cada `UGoodAsset` bruto ganha metadata de requisito geográfico (topografia/clima/vegetação/hidro permitidos). A UI lista recursos elegíveis com cadeado + motivo do bloqueio quando algum requisito falta. **Sem fallback**: se a regra não passa, o recurso simplesmente não aparece como construível.

---

## Camada 4 — Recurso principal (Primary Resource)

Determinístico. Seed = `(LocationId, WorldSeed)` via `FRandomStream`. Mesmo mapa + mesma seed = mesmo resultado sempre (regra de determinismo do projeto).

Prioridade: vegetação manda primeiro (DenseForest→Timber), depois relevo+clima (Hills+Continental→Iron), depois fallback de sustento (Grain/Cattle). A escolha sai da interseção de elegíveis, ponderada.

---

## Camada 5 — Tech tier e gating

```cpp
enum class ENationTechTier : uint8 { Underdeveloped, Developing, Developed };
```

Cada `UProductionMethodAsset` ganha `RequiredTier` (e opcionalmente `RequiredTech`, integrando com Progress v1). Um edifício oferece os PMs cujo tier ≤ tier da nação **e** cuja pesquisa esteja destravada.

Exemplo — edifício `BT_Foundry`, bem `G_Steel`:

| Production Method | Tier mínimo | Inputs | Eficiência |
| :--- | :--- | :--- | :--- |
| `PM_BloomeryIron` | Underdeveloped | 1 IronOre + 1 Charcoal | baixa |
| `PM_PuddledSteel` | Developing | 1 IronOre + 1 Coal | média |
| `PM_BessemerSteel` | Developed | 1 IronOre + 1 Coal | alta (Bessemer, 1856) |

O tier **não** é um rótulo fixo no save — emerge do nível de desenvolvimento/pesquisa da nação. Para o Alpha pode começar como campo simples na nação e depois virar derivado de Progress. **Decisão de quando derivar fica para a Sessão 5.**

---

## Bens — revisão industrial (1820–1880)

A lista do desenho original foi reescrita para a era. Bens brutos quase não mudam (são geográficos); manufaturados ganham espectro de tier via métodos de produção, não via bens separados.

### Brutos (raw) — geográficos
Alimento: `Grain`, `Cattle`, `Fish`, `Fruit`, `Sugar`*, `Coffee`*, `Tea`*
Construção: `Timber`, `Stone`, `Clay`
Estratégico: `IronOre`, `Coal`, `Saltpeter`, `Oil`** (emergente, ~1860+)
Fibras: `Cotton`, `Wool`, `Silk`
Riqueza: `Gold`, `Gems`
Colonial/luxo: `Spices`, `Tobacco`, `Rubber`*

\* bens coloniais — relevantes pro tema de desenvolvimento desigual
\** Oil entra tarde; gating por período + tech

### Manufaturados (refined) — com PMs por tier
`Flour`, `CannedFood` (conserva, ~1810+), `Steel` (bloomery→puddled→Bessemer), `Tools`, `Machinery` (developed), `Textiles` (tear manual→mecânico→fábrica), `Firearms`, `Artillery`, `Glass`, `Chemicals` (fertilizante/corantes, emergente), `RailEquipment` (developed only), `Ships` (madeira/vela → ferro/vapor → aço/vapor), `LuxuryGoods`.

> Esta tabela é **referência de direção**, não escopo de implementação imediata. O balanceamento detalhado de cada PM fica para as sessões de integração (5–7), centralizado em DataAssets, nunca hardcoded.

---

## Modificadores (referência)

Tabelas do desenho original mantidas como base de balanceamento, centralizadas em DataAsset de constantes. Resumo dos eixos:

- **Topografia** → slots base, custo de construção, prod. vegetal/mineral/aquífera, defensividade, movimento militar.
- **Clima** → crescimento e capacidade populacional, atrito por doença, inverno, recursos vegetais permitidos.
- **Hidrografia** → fertilidade (crescimento pop), slots de comércio, irrigação, energia hidráulica (+prod. manufaturas).
- **Vegetação** → custo de limpeza, slots vegetais iniciais, recurso principal padrão.

Valores numéricos exatos: ver desenho original (preservados) e ajustar no DataAsset de constantes durante balanceamento.

---

## Integração com Economy v1 (resumo)

- Edifício só vira **construível** se: recurso elegível (Camada 3) + slot livre (Camada 2) + PM disponível pro tier (Camada 5) + mão de obra suficiente.
- Produção, preço, mercado e auto-invest = **inalterados**, rodam como já rodam.
- Modificadores de terreno/clima entram via `UActiveModifierRegistry` (mesmo canal dos modifiers de evento), não via caminho novo.

---

## Fora de escopo (este sistema)

- ❌ Renderização visual de relevo (heightmap, mesh decorations) — trilha de arte/mapa separada, mas depende da Camada 1.
- ❌ Cadeias produtivas com 3+ etapas — máximo 2 (raw → refined) no Alpha.
- ❌ Separação rural/urbano de slots — pool único no Alpha.
- ❌ Derivar tech tier de Progress automaticamente — fica para Sessão 5+.
