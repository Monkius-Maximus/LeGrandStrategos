# 42 — Política e Grupos de Interesse

**Status:** LOCKED v1.0
**Data:** 2026-07-18
**Documentos relacionados:** `40-resources-and-production.md`, `41-population-and-social-strata.md`, `51-seed-tecnologias.md`
**Escopo:** Interest Groups, leis com ladder ordinal, engine de enactment, capital político.

---

## §1 — Filosofia e decisão arquitetural

### 1.1 Decisão

O sistema político do LGS usa o **modelo bottom-up de Interest Groups do Victoria 3 como base estrutural**, combinado com **capital político escasso no estilo EU4 como recurso tático do jogador**.

- **Vic3 fornece o "quem":** o poder político emerge da composição demográfica e econômica da nação. Não existe política desconectada da simulação de pops e economia.
- **EU4 fornece o "como":** o jogador age gastando um recurso escasso e acumulável, criando decisões táticas de timing e priorização.

### 1.2 Racional

O pilar central do projeto é o desenvolvimento assimétrico expresso em produção (`eficácia = tier_conceito × PM.Tier`). A política precisa ser a camada que **explica e restringe** por que Albion adota o Bessemer antes de Galia — não um minigame paralelo. O modelo bottom-up garante isso: a estrutura de classes produzida pela economia determina quem tem poder, e quem tem poder determina quais leis (e portanto quais modifiers) estão disponíveis.

### 1.3 Rejeitado explicitamente

- **Mana genérico de EU4 como sistema completo** (ADM/DIP/MIL): rejeitado. Aproveitamos apenas o conceito de recurso político escasso, não a tríade nem a geração via monarca.
- **Eventos de partido / focus trees de HOI4**: rejeitados. Nenhum conteúdo político scriptado por nação; tudo emerge de dados.
- **"Divine push" político**: se nenhum IG apoia uma lei, ela é inalcançável. Não existe mecanismo de força bruta que ignore a estrutura social. Consistente com a proibição geral de auto-fallbacks do projeto.

---

## §2 — Interest Groups

### 2.1 Princípio: derivados, não paralelos

Interest Groups são **derivados do modelo de pops do doc 41** (profissão × classe social × estrato). Não existe sistema de filiação próprio, nem grammar paralela de pertencimento. A política é **consumidora** dos sistemas existentes — mesmo princípio aplicado ao tech emergence.

### 2.2 Estrutura de dados

```
UInterestGroupAsset : UPrimaryDataAsset
├── FName Id                          // ex.: IG_Industrialists
├── FText DisplayName
├── TArray<FStratumAffinity> Affinities   // mapeamento estrato → este IG
└── TArray<FLawStance> Stances            // posição por categoria de lei (ver §4.2)
```

```
FStratumAffinity
├── FName StratumId      // referencia estratos do doc 41
└── float Weight         // 0.0–1.0, fração do estrato que este IG representa
```

**Regra de integridade:** para cada `StratumId`, a soma dos `Weight` entre todos os IGs deve ser exatamente 1.0. Validado no commandlet de seed — falha de validação é erro fatal, não warning (fail-fast).

### 2.3 Clout

O clout de um IG é calculado mensalmente na fase de tick apropriada, consumindo dados que o Economy v1 já produz:

```
Clout(IG) = Σ sobre estratos [ Pop(estrato) × Weight × WealthFactor(estrato) ]
```

- `Pop(estrato)`: população do estrato, do sistema do doc 41.
- `WealthFactor(estrato)`: derivado da riqueza per capita do estrato no Economy v1, normalizado. Fórmula exata definida na implementação, mas o contrato é: **riqueza amplifica voz política de forma sublinear** (log ou raiz), para que massa populacional continue relevante.

Clout é normalizado para somar 1.0 entre todos os IGs da nação. Nenhum valor absoluto é exposto ao jogador — apenas frações.

### 2.4 IGs no poder

Os IGs com maior clout formam o **bloco governante** (v1: os IGs necessários para acumular ≥ 50% de clout, em ordem decrescente). O bloco governante determina:

1. Quais leis são propostas com custo normal vs. custo penalizado (§4).
2. A geração de capital político (§5).

Sem coalizões manuais na v1 — o bloco é determinístico a partir do clout. (Manipulação de coalizões: fora de escopo, §7.)

### 2.5 Seed inicial

Seis IGs canônicos na v1, cobrindo o arco 1800–1950:

| Id | Nome | Base (estratos, indicativo) |
|---|---|---|
| `IG_Landowners` | Latifundiários | Aristocracia rural, alto clero |
| `IG_Industrialists` | Industrialistas | Burguesia industrial e financeira |
| `IG_PettyBourgeoisie` | Pequena Burguesia | Artesãos, comerciantes, profissionais liberais |
| `IG_Laborers` | Trabalhadores | Operários, mineiros, camponeses assalariados |
| `IG_Military` | Forças Armadas | Officers e Soldiers (profissões do doc 41) |
| `IG_Clergy` | Clero | Clero (nota: religião é territorial per EU4; o IG representa a instituição, não crenças per-pop) |

Os perfis iniciais de Albion, Galia e Norden (distribuição de clout em 1800) são definidos pelos overrides autorais das nações canônicas, no mesmo mecanismo usado para depósitos — não aqui.

---

## §3 — Leis: categorias, slots e ladder ordinal

### 3.1 Decisão: ordinal em tudo

**Toda lei pertence a uma categoria, e cada categoria é uma ladder ordinal com `Tier`.** Uma forma só, sem exceções — consistente com o primitivo de progressão do projeto (`Tier` em PMs, conceitos militares, esferas de tech). Onde a progressão semântica é menos óbvia (ex.: leis religiosas), o ordinal representa o eixo tradicional → moderno/secular, que o período 1800–1950 sempre oferece.

**Regra estrutural:** dentro de uma categoria, exatamente **uma lei está ativa** por vez (um slot por categoria). Mudar de lei é mover-se na ladder — para cima ou para baixo, um tier por enactment. Sem saltos.

### 3.2 Estrutura de dados

```
ULawAsset : UPrimaryDataAsset
├── FName Id                     // ex.: LAW_ECO_LaissezFaire
├── FName CategoryId             // ex.: LAWCAT_Economy
├── int32 Tier                   // posição na ladder da categoria
├── FText DisplayName
├── TArray<FName> ModifierIds    // modifiers emitidos via UActiveModifierRegistry
└── TArray<FName> TechGateIds    // techs necessárias para a lei existir (opcional)
```

Leis emitem seus efeitos **exclusivamente** através do `UActiveModifierRegistry`. Nenhum sistema lê leis diretamente; todos leem modifiers. Uma fonte de verdade, um caminho.

### 3.3 Categorias v1

Cinco categorias — deliberadamente poucas, expansão é barata depois:

1. **`LAWCAT_Economy`** — de mercantilismo fechado a laissez-faire a economia planejada. Interage com auto-investimento da Burguesia.
2. **`LAWCAT_Labor`** — de servidão a jornadas reguladas a direitos plenos. Interage com eficiência de PMs e crescimento de estratos.
3. **`LAWCAT_Military`** — **absorve a mecânica civil/militar já existente** como tag + lei no `UActiveModifierRegistry`. A mecânica atual não muda; ela ganha formalmente uma categoria e uma ladder (ex.: milícia → conscrição limitada → mobilização total).
4. **`LAWCAT_Governance`** — de monarquia absoluta a parlamentarismo censitário a sufrágio amplo. Afeta geração de capital político (§5).
5. **`LAWCAT_Religion`** — de teocracia/igreja estatal a tolerância a secularismo pleno. Interage com o modelo territorial de religião; mecânicas detalhadas ficam para o doc de religião pendente.

Cada categoria terá 4–6 tiers na seed. Conteúdo exato das ladders (nomes, tiers, modifiers) será produzido como documento de seed separado (`52-seed-leis.md`), no mesmo padrão do `51-seed-tecnologias.md`.

### 3.4 Gates tecnológicos

Uma lei pode exigir techs (`TechGateIds`). Sem a tech, a lei **não aparece** — nó inerte, sem versão degradada. O tech emergence já fornece o mecanismo; a política apenas consome.

---

## §4 — Engine de enactment

### 4.1 Fluxo

1. Jogador seleciona uma lei adjacente (tier ±1) na ladder de uma categoria, com gates tecnológicos satisfeitos.
2. O custo em capital político é exibido (§4.3).
3. Jogador paga o custo integral **no início** do enactment.
4. O enactment corre por N meses (v1: 6 meses base, modificável por modifiers).
5. Ao fim do período, resolve-se sucesso ou falha (§4.4).

Um enactment ativo por vez, nação inteira. Sem filas.

### 4.2 Posição dos IGs

Cada IG tem, por categoria de lei, uma **direção preferida na ladder** — não um script por lei individual:

```
FLawStance
├── FName CategoryId
├── int32 PreferredTier      // tier ideal do IG nesta categoria
```

A aprovação de um IG a um enactment é derivada: mover **em direção** ao `PreferredTier` = apoio; mover **para longe** = oposição; magnitude proporcional à distância. Nenhuma lei precisa declarar quem a apoia — isso emerge da geometria da ladder. Zero scripting por lei, zero grammar paralela.

### 4.3 Custo

```
Custo = BaseCost × (1 + OppositionClout − SupportClout)
```

- `BaseCost`: constante por categoria (tuning).
- `OppositionClout` / `SupportClout`: somas de clout dos IGs contra/a favor, do **bloco governante e de fora dele** — todos os IGs pesam, mas ver §4.4 para o papel especial do bloco.
- Piso de custo: 25% do `BaseCost`. Sem custo zero, mesmo com apoio unânime.

### 4.4 Resolução

Ao fim do período de enactment:

```
P(sucesso) = clamp(SupportClout_governante / (SupportClout_governante + OppositionClout_governante), 0.05, 0.95)
```

Apenas o clout **dentro do bloco governante** decide o resultado; IGs fora do bloco afetam só o custo (§4.3). Rolagem única, determinística via seed do save (reload não reroda).

**Falha:** o capital político gasto está perdido. A lei vigente permanece. **Sem versão parcial, sem degradação, sem reembolso.** Falha custa e ensina — o jogador aprende a mudar a estrutura de clout antes de tentar de novo.

**Sucesso:** a lei nova entra em vigor no tick seguinte; modifiers antigos da categoria são removidos e os novos registrados atomicamente no `UActiveModifierRegistry`.

---

## §5 — Capital político

### 5.1 Natureza

Recurso escalar único por nação, acumulável, com teto. É o **único** recurso do sistema político — sem tipos, sem cores, sem tríades.

### 5.2 Geração

Gerado mensalmente, **derivado — não arbitrário**:

```
Geração = Base(LAWCAT_Governance.Tier) × Alignment
Alignment = Σ sobre IGs do bloco governante [ Clout(IG) × Satisfação(IG) ]
Satisfação(IG) = 1 − média_normalizada(|Tier_vigente − PreferredTier|) sobre as categorias
```

Em prosa: um governo cujas leis vigentes estão próximas das preferências dos IGs que o sustentam gera capital rápido; um governo em contradição com sua própria base gera pouco. A lei de Governança define o multiplicador base (regimes diferentes têm capacidades de mobilização política diferentes — o conteúdo exato fica na seed).

### 5.3 Gastos

Na v1, capital político tem **exatamente um uso: enactment de leis** (§4.3). Sem micro-gastos, sem botões de "pressionar IG". Um recurso, um dreno — expandir usos depois é barato; remover usos depois é impossível.

### 5.4 Teto

`Teto = 24 × Geração_base_do_tier_de_Governança` (dois anos de acumulação em alinhamento perfeito). Excedente é descartado. Força o jogador a gastar, não a estocar indefinidamente.

---

## §6 — Interação com sistemas existentes

Contratos explícitos, todos na direção **política consome, não inventa**:

| Sistema | Contrato |
|---|---|
| **Doc 41 (pops/estratos)** | IGs derivam de estratos via `FStratumAffinity`. Política nunca cria dados demográficos próprios. |
| **Economy v1** | `WealthFactor` do clout lê riqueza per capita de estrato já calculada no tick econômico. Sem cálculo duplicado. |
| **`UActiveModifierRegistry`** | Único canal de efeito das leis. A mecânica civil/militar existente é absorvida como `LAWCAT_Military` sem mudança de comportamento. |
| **Tech emergence** | `TechGateIds` em `ULawAsset` referencia `UTechnologyDataAsset` por `FName`. Adicionalmente, leis podem (via modifiers) alterar taxas de adoção de tech — fechando o loop: tech habilita lei, lei acelera tech. |
| **Nações canônicas** | Perfis políticos de 1800 (clout inicial, leis vigentes iniciais, `PreferredTier` ajustados se necessário) entram nos overrides autorais, junto aos de depósitos. |
| **Tick** | Clout e geração de capital calculados em fase mensal própria, **após** as fases econômicas (dependem de riqueza atualizada). Posição exata na ordem do tick: decisão de implementação, flag para o passe de recon. |

---

## §7 — Fora de escopo v1

Nomeados explicitamente para não haver ambiguidade:

- **Eleições e partidos** — o bloco governante é derivado de clout, sem ciclos eleitorais.
- **Revoluções e guerras civis** — falha de enactment não escala para conflito na v1.
- **Movimentos radicais / radicalização de pops** — sem eixo de radicalismo per-pop.
- **Manipulação de coalizões** — o jogador não monta blocos; eles emergem.
- **Política externa via IGs** — IGs não opinam sobre diplomacia na v1 (revisitar quando o sistema de diplomacia for desenhado).
- **Sufrágio como mecânica de voto** — `LAWCAT_Governance` afeta geração de capital e (via modifiers futuros) pesos de clout, mas não simula votação.

---

## §8 — Impacto em save e migração

- **SaveVersion bump** obrigatório na sessão de implementação.
- Novos dados persistidos por nação: capital político acumulado, enactment ativo (lei alvo, meses restantes, custo pago), lei vigente por categoria.
- Clout **não é persistido** — é derivado e recalculado no primeiro tick pós-load. Uma fonte de verdade.
- Saves anteriores migram com: capital = 0, sem enactment ativo, leis vigentes = tier definido como default de cada categoria na seed (campo `bIsDefault` ou tier 0 — decisão de implementação, flag para o recon).

---

## Apêndice — Decisões tomadas nesta sessão

1. IGs **derivados dos estratos** via tabela de afinidade; sem sistema de filiação próprio. *(Confirmado por Diogo.)*
2. Leis com **`Tier` ordinal em todas as categorias**; uma forma só. *(Confirmado por Diogo.)*
3. Bloco governante determinístico por clout; sem coalizões manuais na v1.
4. Falha de enactment consome capital integralmente, sem degradação nem reembolso.
5. Capital político com uso único (enactment) na v1.
6. Conteúdo das ladders vai para documento de seed separado (`52-seed-leis.md`).
