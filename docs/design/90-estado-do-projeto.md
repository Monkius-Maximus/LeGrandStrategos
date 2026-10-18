# 90 — Estado do Projeto: Consolidação, Descartes, Reaproveitamento e Pendências

**Status:** LIVING DOCUMENT (não travar)
**Data:** 2026-09-06
**Escopo:** Consolidação transversal de todas as sessões de design do LGS. Não substitui nenhum doc numerado; aponta para eles.
**Documentos cobertos:** `33`, `40`, `41`, `42a` (depósitos), `42b` (política), `50`, `51`, `52`, `60`, `70`

---

## §0 — Como ler este documento

Este é o mapa do território, não o território. Cada afirmação aqui remete a um documento travado
onde a decisão vive em detalhe. Quando houver divergência, **o documento numerado vence**.

Três marcações são usadas ao longo do texto:

- **[TRAVADO]** — decisão confirmada explicitamente, com documento LOCKED correspondente.
- **[VIGENTE]** — prática corrente estabelecida, sem documento formal de travamento.
- **[INFERIDO]** — leitura consolidada a partir do histórico, **não confirmada** por você. Trate como proposta.

**Limitação de fonte:** esta consolidação foi montada a partir do doc `42-politica-e-grupos-de-interesse.md`
(íntegra) e do registro acumulado de decisões do projeto. Os transcritos brutos das conversas não foram
relidos. Itens marcados [INFERIDO] são exatamente os que precisam da sua confirmação antes de virarem base
para qualquer coisa.

---

## §1 — O QUE ESTAMOS USANDO

### 1.1 Visão e escopo do produto

| Item | Estado | Detalhe |
|---|---|---|
| Gênero | [TRAVADO] | Grand strategy, mundo fictício em industrialização |
| Período | [TRAVADO] | **1800–1950**. Markdowns que ainda citam 1820–1880 estão desatualizados — dívida de documentação |
| Fim de campanha | [TRAVADO] | Soft end estilo Civilization: ao atingir a data, o jogador pode continuar livremente. Sem lock de data final |
| Referência primária | [TRAVADO] | Victoria 3 (arquitetura) + Europa Universalis 4 (conteúdo e capital político) |
| Nações canônicas | [TRAVADO] | Albion, Galia, Norden |
| Pilar central | [TRAVADO] | **Acesso assimétrico a recursos e métodos de produção** entre nações |
| Expressão mecânica do pilar | [TRAVADO] | Mesma mercadoria produzida por PM tiers diferentes por nação (framing canônico: bloomery vs. conversor Bessemer para aço) |
| Interlock militar | [TRAVADO] | `eficácia = tier_conceito × PM.Tier` — poder militar é função direta do tier industrial |

### 1.2 Stack e estrutura

| Camada | Escolha | Estado |
|---|---|---|
| Engine | Unreal Engine 5.5, C++ | [VIGENTE] — **sob revisão**, ver §4.1 |
| Módulos | `StrategosCore` (backend) / `StrategosUI` (frontend) | [TRAVADO] |
| Dados | DataAsset com identificadores `FName` | [TRAVADO] |
| Mapa-mundo | Azgaar's Fantasy Map Generator → export JSON → `UStrategosImportWorldCommandlet` | [TRAVADO] (desenhado, **não executado**) |
| Vault de design | Obsidian | [VIGENTE] — estrutura ainda pendente |
| Interação com código | Claude Code (repo montado lá) | [VIGENTE] |
| Design e documentos | Claude.ai (chat) | [VIGENTE] |
| Arte (exploratório) | Leonardo AI, Tensor.art | [INFERIDO] — anotados como ponto de partida gratuito, sem direção travada |

### 1.3 Sistemas travados (o que já é base sólida)

**Economy v1 — OPERACIONAL.** Tick de 9 fases, precificação dinâmica, auto-investimento da Burguesia,
DataAssets `UGoodAsset` / `UProductionMethodAsset` / `UBuildingTypeAsset`, sistema de modifiers via
`UActiveModifierRegistry`, estratos populacionais industriais. **É o único sistema com código rodando.**
Todo o resto do design é consumidor dele.

**População e Estratos Sociais** (`41`, v1.1). Modelo multidimensional que separa **profissão**
(o que a pop consome na produção) de **classe social** (peso político e tier de consumo). Soldier e
Officer são profissões (estratos Lower e Middle). Religião segue o modelo territorial de EU4 —
por província, **sem campo de religião por pop**. Cultura permanece por pop.

**Recursos e Produção** (`33`). Filosofia híbrida EU4/Vic3, classificação via `UGoodAsset`, hierarquia
de mercado em três camadas, consumo de POP com mecânica de substituição, recursos estratégicos como
portões geopolíticos. **31 goods travados** em raw / intermediate / final. Ladders de PM definidas para
Aço (5 tiers), Tecido, Explosivos, Ferramentas, Artilharia, Armas Leves, Motores, Aeronaves.
Goods tardios incluem petróleo com duas gerações descobríveis (poços rasos exaurindo ~1920–40, campos
profundos destravados por tech) e desvalorização estratégica do salitre quando a química sintética
torna os depósitos obsoletos.

**Depósitos e Descoberta** (`42`-depósitos). Seeding determinístico via commandlet executado uma vez no
import do mundo, camada de override autoral para as nações canônicas, validação fail-fast, **sem
fallbacks auto-plantados**. Descoberta em runtime por um único passe mensal de resolução alimentado pelo
`UActiveModifierRegistry`, com odds escalando temporalmente e **sem pity timer separado**.

**Tecnologia** (`50`, `51`). Emergência tecnológica desenhada como **consumidora** de sistemas existentes,
não como motor novo. Sete esferas civilizacionais mapeadas ao enum `ETechSphere`; mecânica de
spark / difusão / adoção; 15 entradas `UTechnologyDataAsset` seedadas.

**Sistema de Batalha** (`60`). Tático baseado em cartas. Command points vindos de Logística do comandante
+ doutrina; economia de ações da unidade é separada. Manpower drena pops militares primeiro — conscrição
de civis dispara penalidades de atributo e dreno econômico. Rodadas variáveis por cenário/terreno
(campo aberto é a primeira fatia; urbano/cerco/nativo diferidos). Deck = cartas pessoais do comandante +
cartas nacionais destravadas por difusão de tech. Combate naval é auto-resolve até o terrestre validar.
O auto-resolver roda **o mesmo engine em headless**. IA usa utility-scoring limitado com lookahead de
1 ply e **não trapaceia com informação**. Supersede o Protocolo B.

**Política e Grupos de Interesse** (`42`-política, LOCKED v1.0). IGs derivados dos estratos via tabela de
afinidade (`FStratumAffinity`, soma de pesos por estrato = 1.0, validado fail-fast). `UInterestGroupAsset`.
Clout mensal derivado de pop × peso × riqueza (amplificação **sublinear** da riqueza, para que massa
populacional continue importando). Bloco governante determinístico: IGs em ordem decrescente até ≥ 50%
de clout. Leis em ladder ordinal por categoria, um slot ativo por categoria, movimento de ±1 tier por
enactment, sem saltos. Efeitos emitidos **exclusivamente** pelo `UActiveModifierRegistry`. Custo pago
integralmente no início, piso de 25% do `BaseCost`. Resolução por rolagem única determinística via seed
do save; só o clout **dentro** do bloco governante decide, IGs de fora afetam apenas o custo. Falha
consome o capital integralmente. Capital político é escalar único, com teto de 24× a geração base.

**Seed de Leis** (`52`, LOCKED v1.0). Cinco categorias como ladders ordinais: `LAWCAT_Economy`,
`LAWCAT_Labor`, `LAWCAT_Military`, `LAWCAT_Governance`, `LAWCAT_Religion`. `LAWCAT_Military` **absorve
a mecânica civil/militar existente** sem criar comportamento novo. `LAWCAT_Governance` define a base de
geração de capital político, com Alignment como contrapeso. `LAWCAT_Religion` limitada à satisfação do
`IG_Clergy` na v1. Todos os IDs de modifier e tech gate que tocam código existente estão marcados
`⚠ RECONCILIAR`.

### 1.4 Princípios de engenharia em vigor

1. **Uma fonte de verdade** — nenhuma gramática paralela de regras.
2. **Sem fallbacks / fail-fast** — o sistema falha alto e explícito, nunca degrada em silêncio.
3. **Sistemas como consumidores, não motores** — tech e política consomem Economia e População.
   Foi isso que baixou o custo de implementação dos dois.
4. **`Tier` como primitivo universal** — mesmo campo ordinal em PMs, conceitos militares, esferas de
   tech e ladders de lei.
5. **Sem identificadores fabricados** — documentos novos nunca inventam IDs que possam colidir com a
   gramática do código; tudo vai marcado `⚠ RECONCILIAR`.
6. **Derivar parâmetros de sistemas, não escolher arbitrariamente** — a lista de profissões vem do que
   os PMs consomem; o clout vem dos dados que a Economia já produz.
7. **Recon antes de implementação** — o Claude Code sempre roda um passe de reconhecimento com
   relatório escrito antes de qualquer passe de implementação, e só mediante aprovação explícita.

### 1.5 Convenções de trabalho em vigor

- **Propor-então-confirmar**: decisões irreversíveis são sinalizadas para confirmação explícita antes de travar.
- **Documentos numerados, travados por promoção**: DRAFT → LOCKED v1.0 via `str_replace` cirúrgico, nunca reescrita completa (evita drift do conteúdo já confirmado).
- **Fluxo de documento**: rascunho completo via `create_file` → apresentação → confirmação ou correção → `str_replace` para ajustes.
- **Dois turnos no Claude Code**: recon (só auditoria) → aprovação → implementação.
- **Mudanças cirúrgicas**: implementação em estágios, bump de SaveVersion por sessão, sem scope creep entre sessões.
- **Idiomas**: conversa em português, código em inglês, documentação e comentários em PT-BR.
- **Commits**: Conventional Commits.
- **Determinismo**: `FRandomStream`.
- **Sessão por sistema**: cada sistema maior tem seu próprio arco de conversa, com prompt de handoff para continuidade.

---

## §2 — O QUE FOI DESCARTADO

Registrar o descarte é tão importante quanto registrar a escolha: evita reabrir discussões fechadas e
protege contra a tentação de "só um fallbackzinho".

### 2.1 Descartes de design

| Descartado | Motivo | Onde |
|---|---|---|
| **Mana ADM/DIP/MIL de EU4** como sistema completo | Aproveitou-se só o conceito de recurso político escasso, não a tríade nem a geração via monarca. Tríade fragmenta a decisão sem ganho de profundidade | `42` §1.3 |
| **Eventos de partido e focus trees de HOI4** | Nenhum conteúdo político scriptado por nação; tudo emerge de dados. Focus tree é conteúdo autoral caro que congela a rejogabilidade | `42` §1.3 |
| **"Divine push" político** | Se nenhum IG apoia uma lei, ela é inalcançável. Sem mecanismo de força bruta que ignore a estrutura social | `42` §1.3 |
| **"Divine push" tecnológico** | Mesmo princípio aplicado ao tech emergence: sem atalho que ignore as pré-condições | princípio geral |
| **Reembolso ou degradação em falha de enactment** | Falha custa e ensina. O jogador aprende a mudar a estrutura de clout antes de tentar de novo | `42` §4.4 |
| **Coalizões manuais / montagem de bloco pelo jogador** | O bloco governante emerge do clout. Manipulação de coalizão seria uma gramática paralela | `42` §2.4, §7 |
| **Eleições, partidos e ciclos eleitorais** | Fora de escopo v1 | `42` §7 |
| **Revoluções e guerras civis** | Falha de enactment não escala para conflito na v1 | `42` §7 |
| **Radicalização de pops** | Sem eixo de radicalismo per-pop na v1 | `42` §7 |
| **IGs opinando sobre diplomacia** | Fora de escopo até o sistema de diplomacia existir | `42` §7 |
| **Sufrágio como simulação de voto** | `LAWCAT_Governance` afeta geração de capital e pesos de clout; não simula votação | `42` §7 |
| **Religião por pop** | Modelo territorial de EU4 (por província) adotado. Cultura continua per-pop | `41` |
| **Pity timer separado na descoberta de depósitos** | Odds escalando temporalmente já cobrem o caso; um timer paralelo seria segunda fonte de verdade | `42`-depósitos |
| **Fallbacks auto-plantados de depósito** | Validação fail-fast; se o mundo não tem o recurso, o mundo não tem o recurso | `42`-depósitos |
| **IA que trapaceia com informação** | A IA de batalha usa só o que veria | `60` |

### 2.2 Descartes estruturais e de método

| Descartado | Motivo |
|---|---|
| **EU4 como backbone arquitetural** (um good por província) | Modelo estruturalmente errado para o pilar do jogo. EU4 permanece **mina de conteúdo** — quais goods e modificadores estratégicos existem na era — mas a arquitetura correta é a de production methods do Victoria 3 |
| **HOI4 como referência arquitetural** | Estudado e majoritariamente rejeitado para o escopo deste projeto |
| **Diagramas Mermaid para topologia de sistemas** | Mermaid serve estruturas lineares. Diagramas com cross-linking denso ficaram mais confusos que texto puro |
| **Reescrita completa de documento para atualizar status** | Introduz drift em relação ao conteúdo confirmado. Usar `str_replace` cirúrgico |
| **Naval terrestre-primeiro** | Combate naval fica em auto-resolve até o terrestre validar o engine |
| **Rodadas fixas de batalha** | Rodadas variam por cenário e terreno; urbano/cerco/nativo diferidos |

> **Nota de risco:** vários desses descartes são "diferidos", não "mortos" (eleições, revoluções,
> radicalização, diplomacia via IG). O perigo é que a v1 fique tão limpa que a camada política não
> gere pressão narrativa. Vale observar isso na primeira sessão jogável — **é uma hipótese a testar,
> não um defeito comprovado.**

---

## §3 — O QUE É REAPROVEITÁVEL EM OUTRO PROJETO

Esta seção existe porque o reaproveitamento **já está acontecendo**: o *Le Grand Polemarkos* (LGP,
Godot 4.3+/C#) é um spin-off que compartilha DNA de design com o LGS sem compartilhar código — e um dos
seus dois objetivos declarados é justamente **validar a arquitetura de subsistemas engine-agnóstica**.
Então o que está abaixo não é teoria: é uma aposta arquitetural em teste ativo.

### 3.1 Padrões arquiteturais portáteis (independem de engine)

1. **`Tier` como primitivo ordinal universal.** Um único campo inteiro que expressa progressão em
   domínios não relacionados (produção, militar, pesquisa, lei). O ganho não é economia de código — é
   que qualquer fórmula de interlock entre domínios vira multiplicação de tiers.
2. **Fórmula de interlock** (`eficácia = tier_conceito × PM.Tier`). Amarra o poder militar à profundidade
   industrial em uma linha, sem tabela de conversão. **Já reaproveitada no LGP**, onde só os
   intermediários carregam tiers e os finais herdam via interlock.
3. **Derivação em vez de sistema paralelo.** IGs derivados de estratos por tabela de afinidade é o
   padrão-mãe: sempre que um sistema novo precisar de "membros", derive-os de uma população que já
   existe em vez de criar filiação própria. Aplica-se a facções, guildas, seitas, sindicatos, dinastias.
4. **Registry de modifiers como único canal de efeito.** Nenhum sistema lê leis; todos leem modifiers.
   É o que permite adicionar uma fonte de efeito nova (lei, tech, evento, edifício) sem tocar em nenhum
   consumidor. Portável para qualquer engine — é só um dicionário com ciclo de vida.
5. **Ladder ordinal com movimento ±1 e um slot ativo por categoria.** Modelo genérico de progressão
   política/institucional. Elimina scripting por lei: quem apoia o quê emerge da **geometria da ladder**
   (distância até o `PreferredTier` do IG).
6. **Seeding determinístico + camada de override autoral.** Gerar por commandlet uma vez, permitir
   sobrescrita curada para entidades canônicas, validar fail-fast. Serve para depósitos, perfis políticos
   iniciais, elencos, geografia — qualquer conteúdo "procedural com exceções autorais".
7. **Auto-resolver rodando o mesmo engine em headless.** Garante que o resultado automático e o
   resultado jogado nunca divergem por implementações separadas. Uma fonte de verdade aplicada a combate.
8. **Clout normalizado com amplificação sublinear da riqueza.** Padrão geral para "poder derivado de
   massa × recurso" sem que o recurso domine completamente.

### 3.2 Método de trabalho portátil

Isto é reaproveitável em **qualquer** projeto seu, inclusive os que não são jogos:

- Ciclo **recon → aprovação → implementação** no Claude Code.
- **Propor-então-confirmar** com marcação explícita de decisões irreversíveis.
- **Documentos numerados** com promoção DRAFT → LOCKED e edição cirúrgica.
- **`⚠ RECONCILIAR`** como marcador de identificador não verificado contra o código — evita a classe
  inteira de bug "documento inventou um ID que já existe com outra semântica".
- **Apêndice "Decisões tomadas nesta sessão"** ao fim de cada documento, com marcação de quais foram
  confirmadas por você. É o que torna esta consolidação possível.
- **Prompt de handoff** ao fim de cada arco de sessão.
- Você já mantém um **template genérico de `CLAUDE.md`** com esses princípios — o LGS é um dos alvos
  declarados de aplicação.

### 3.3 O que **não** é reaproveitável

- Tudo que é `U*Asset : UPrimaryDataAsset`, `FName`, `FText`, commandlets, `FRandomStream` — é
  vocabulário de Unreal. Os **contratos** portam; as **assinaturas** não.
- O pipeline de import do FMG está acoplado ao commandlet do UE5.
- A lista de 31 goods e as ladders de PM são conteúdo do mundo do LGS. O LGP deliberadamente refez a sua
  com 10 itens — o que confirma que a **forma** porta e o **conteúdo** não.

---

## §4 — O QUE FICOU PENDENTE

Organizado por poder de bloqueio, não por ordem cronológica.

### 4.1 O bloqueador-mor: decisão de engine (UE5 vs. Godot)

**Estado:** [VIGENTE] em UE5.5. Migração para Godot **sinalizada como provavelmente o encaixe mais
correto**, dadas as restrições de hardware e o fato de o jogo ser fundamentalmente uma simulação 2D.
Um playbook de migração faseada e um mapa de subsistemas engine-agnóstico (`70-arquitetura-modular-e-godot.md`)
já foram produzidos. **Nenhuma decisão final foi tomada.**

Por que isto trava tudo: cada hora gasta na dívida de reconciliação do `StrategosUI` (§4.2) é uma hora
potencialmente jogada fora se a migração acontecer. É a única pendência do projeto que **muda o valor de
todas as outras**. Enquanto ela estiver aberta, qualquer trabalho de UI em UE5 é uma aposta.

**Sinal a favor de decidir agora:** o LGP existe justamente para validar a arquitetura engine-agnóstica
em Godot. Ele é o experimento cujo resultado responde esta pergunta. **[INFERIDO]** — vale explicitar se
a decisão do LGS está formalmente aguardando o resultado do LGP ou se são trilhas independentes.

### 4.2 Dívida de reconciliação UI ↔ Core

`StrategosUI` **não compila**. Mismatches de enum identificados:

- `ELeaderArchetype`
- `EDiplomaticStatus::Truce`
- valores de `EPopStratum`
- mismatch de tipo em `CategoryColors`

A ordem recomendada de reconciliação já está documentada. Falta um passe limpo de compilação depois das
correções de enum. **Bloqueia:** qualquer validação visual, qualquer sessão de playtest.

### 4.3 Recon de política (`PROMPT-RECON-POLITICA.md`) — enfileirado, não executado

Prompt pronto para o Claude Code auditar o Core contra os **cinco itens abertos de reconciliação** do
`52-seed-leis.md §7`. O item bloqueante é **M1**: quantos estados a mecânica civil/militar existente
expõe hoje — isso define a contagem de tiers de `LAWCAT_Military`. Sem M1, a seed de leis não pode ser
implementada.

Além disso, `42` §6 e §8 deixam dois itens explicitamente marcados como "decisão de implementação,
sinalizar para o recon":

- posição exata da fase política na ordem do tick (só se sabe que é **após** as fases econômicas);
- como saves antigos definem a lei vigente inicial por categoria (`bIsDefault` ou tier 0).

### 4.4 Pipeline de import do mundo — desenhado, nunca executado

O commandlet FMG está desenhado. Nunca rodou. Sem mundo importado não há depósitos seedados, e sem
depósitos seedados a Economia roda no vazio. **Bloqueia:** qualquer teste de sistema em escala real.

### 4.5 Diplomacia — não desenhada

Confirmado como o próximo tópico maior de design depois de recursos. Já existe um `EDiplomaticStatus`
no código (com o `Truce` quebrado) e o registro indica um **MVP de Diplomacia** anterior, além de
**branches de Diplomacia não mergeadas das quais o versionamento de SaveVersion depende**. Ou seja:
há código de diplomacia flutuando fora do design. Isso é exatamente a situação que o princípio de
"uma fonte de verdade" existe para prevenir.

**[INFERIDO]** — recomendo tratar o merge/descarte dessas branches como pré-requisito do design de
diplomacia, não como consequência dele.

### 4.6 Pendências de design

| Pendência | Estado |
|---|---|
| Documento de religião | Efeitos territoriais diferidos da seed de leis v1; doc não existe |
| Migração de população | Decisão pendente de reconciliação — **não encontrada no arquivo de conversas** |
| Estrutura do vault Obsidian | Pendente; a pipeline de design não tem casa organizada |
| Markdowns com período 1820–1880 | Desatualizados; período correto é 1800–1950 |
| **Colisão de numeração: dois docs `42`** | `42-depositos-e-descoberta.md` e `42-politica-e-grupos-de-interesse.md` ocupam o mesmo número. Ambiguidade real em referências cruzadas |

### 4.7 Caminho crítico sugerido — [INFERIDO]

Isto é **proposta minha**, não decisão registrada. Ordenado para maximizar destravamento por unidade de esforço:

1. **Decidir a engine.** Ou trave UE5 explicitamente, ou trave Godot, ou defina o critério objetivo e a
   data em que o resultado do LGP decide. Sem isso, os passos 2 e 4 são apostas.
2. **Rodar o recon de política.** É barato (auditoria, zero código), destrava M1 e responde as duas
   decisões de implementação de `42` §6/§8. Independe da engine — é leitura do Core.
3. **Executar o import do FMG.** Transforma "sistemas desenhados" em "sistemas com dados". Provavelmente
   o maior salto de confiança disponível hoje.
4. **Reconciliar a UI** — mas só depois de (1). Se for Godot, essa dívida evapora.
5. **Auditar as branches de Diplomacia**: mergear ou descartar, e só então desenhar diplomacia.
6. Higiene de documentação em paralelo: resolver a colisão do `42`, corrigir 1820–1880 → 1800–1950,
   montar o vault Obsidian.

---

## §5 — GLOSSÁRIO

### 5.1 Siglas e nomes próprios

| Sigla / Nome | Significado |
|---|---|
| **LGS** | *Le Grand Strategos* — o projeto principal. Grand strategy, UE5/C++, 1800–1950 |
| **LGP** | *Le Grand Polemarkos* — spin-off 4X real-time-with-pause em Godot 4.3+/C#. Compartilha DNA de design, **não compartilha código** |
| **LGE** | Referência ao projeto Unreal anterior do qual o LGP é spin-off (na prática, o LGS) |
| **StrategosCore** | Módulo de backend — simulação, dados, regras |
| **StrategosUI** | Módulo de frontend — apresentação. Atualmente não compila |
| **FMG** | *Azgaar's Fantasy Map Generator* — gerador do mapa-mundo; exporta JSON consumido pelo commandlet de import |
| **Albion / Galia / Norden** | As três nações canônicas. Albion = potência industrial; Norden = fronteira marcial com recursos abundantes e metalurgia primitiva; Galia = estado centralizado da pólvora (identidades explicitadas no LGP, coerentes com o LGS) |
| **PM** | *Production Method* — método de produção. A unidade que carrega `Tier` e define a assimetria entre nações |
| **IG** | *Interest Group* — grupo de interesse. Derivado dos estratos, nunca um sistema de filiação próprio |
| **Vic2 / Vic3** | Victoria 2 / Victoria 3. Vic3 é o **modelo arquitetural** do projeto |
| **EU4** | Europa Universalis 4. **Mina de conteúdo** e origem do conceito de capital político escasso — não é backbone arquitetural |
| **HOI4** | Hearts of Iron 4. Estudado, majoritariamente **rejeitado** para este escopo |
| **Protocolo B** | Sistema de batalha anterior, **superseded** pelo doc `60` |
| **ADR** | *Architecture Decision Record* — registro de decisão arquitetural |

### 5.2 Conceitos do domínio

| Termo | Definição operacional |
|---|---|
| **Tier** | Primitivo ordinal universal de progressão. Mesmo campo em PMs, conceitos militares, esferas de tech e ladders de lei |
| **Interlock** | A fórmula que amarra domínios via tier: `eficácia = tier_conceito × PM.Tier` |
| **Estrato (stratum)** | Camada social da população. Eixo de **classe**: peso político e tier de consumo |
| **Profissão** | O que a pop faz na produção; determina consumo produtivo. Eixo **independente** da classe. Soldier e Officer são profissões |
| **Clout** | Fração normalizada (soma 1.0) do poder político de um IG. `Σ [Pop(estrato) × Weight × WealthFactor(estrato)]`. Nunca exposto em valor absoluto ao jogador |
| **WealthFactor** | Normalização da riqueza per capita do estrato. Contrato: amplifica voz política de forma **sublinear** (log ou raiz) |
| **Bloco governante** | Os IGs que, em ordem decrescente de clout, acumulam ≥ 50%. Determinístico, sem coalizão manual |
| **Alignment** | `Σ [Clout(IG) × Satisfação(IG)]` sobre o bloco governante. Multiplicador da geração de capital político |
| **Satisfação (IG)** | `1 − média_normalizada(|Tier_vigente − PreferredTier|)` sobre as categorias |
| **Ladder ordinal** | Categoria de lei como escada de tiers. Uma lei ativa por categoria, movimento de ±1 por enactment, sem saltos |
| **Enactment** | O processo de mudar de lei. Custo pago integral no início, N meses de duração (v1: 6 base), rolagem única no fim |
| **Capital político** | Recurso escalar único por nação, acumulável, com teto de 24× a geração base. Na v1 tem **exatamente um uso**: enactment |
| **Stance / PreferredTier** | A posição de um IG numa categoria de lei. Apoio e oposição **emergem da distância** até esse tier — zero scripting por lei |
| **Spark / difusão / adoção** | As três fases da emergência tecnológica |
| **Esfera tecnológica** | Uma das sete divisões civilizacionais do tech tree (`ETechSphere`) |
| **Command points** | Recurso tático de batalha: Logística do comandante + doutrina. Separado da economia de ações da unidade |
| **Manpower** | Reserva humana militar. Drena pops militares primeiro; conscrição de civis gera penalidade de atributo e dreno econômico |
| **Override autoral** | Camada de dados curados que sobrescreve o seeding procedural para entidades canônicas |
| **Recurso estratégico** | Good cuja distribuição funciona como portão geopolítico |
| **Soft end** | Fim de campanha estilo Civilization: ao atingir a data, o jogador escolhe continuar |

### 5.3 Convenções de código

| Convenção | Regra |
|---|---|
| `U*Asset` | Prefixo Unreal de `UObject`. Todos os assets de dados herdam de `UPrimaryDataAsset` |
| `F*` | Prefixo Unreal de struct (`FName`, `FStratumAffinity`, `FLawStance`, `FRandomStream`) |
| `E*` | Prefixo Unreal de enum (`ETechSphere`, `EPopStratum`, `EDiplomaticStatus`, `ELeaderArchetype`) |
| `IG_*` | Identificador de Interest Group — `IG_Landowners`, `IG_Industrialists`, `IG_PettyBourgeoisie`, `IG_Laborers`, `IG_Military`, `IG_Clergy` |
| `LAWCAT_*` | Identificador de categoria de lei — `LAWCAT_Economy`, `LAWCAT_Labor`, `LAWCAT_Military`, `LAWCAT_Governance`, `LAWCAT_Religion` |
| `LAW_<CAT>_*` | Identificador de lei individual — ex.: `LAW_ECO_LaissezFaire` |
| `FName` como chave | Todo identificador de dado é `FName`. Nunca string crua, nunca índice numérico |
| `FRandomStream` | Única fonte de aleatoriedade. Garante determinismo e resistência a save-scumming |
| Conventional Commits | Formato obrigatório de mensagem de commit |
| DataAsset sobre hardcode | Valores de tuning vivem em DataAssets, nunca no código |

### 5.4 Convenções de documentação

| Convenção | Regra |
|---|---|
| **Numeração** | Documentos numerados por família: 30s = recursos/economia, 40s = população/política/depósitos, 50s = tecnologia/leis, 60s = batalha, 70s = arquitetura. ⚠ Há colisão ativa em `42` |
| **Seed doc** | Documento de **conteúdo** separado do documento de **sistema**. `51-seed-tecnologias.md` para `50`, `52-seed-leis.md` para `42` |
| **Status** | `DRAFT` → `LOCKED v1.0`. Promoção por `str_replace` cirúrgico, nunca reescrita |
| **`⚠ RECONCILIAR`** | Marca um identificador citado em documento que **ainda não foi verificado** contra o código. Nada com essa marca pode ser implementado antes do recon |
| **Apêndice de decisões** | Todo documento fecha com "Decisões tomadas nesta sessão", marcando quais foram *(Confirmado por Diogo.)* |
| **Fora de escopo** | Seção explícita nomeando o que **não** entra na versão, para eliminar ambiguidade |
| **Idioma** | Documento e comentários em PT-BR; identificadores e código em inglês |
| **SaveVersion bump** | Obrigatório na sessão de implementação de qualquer sistema que persista dados novos |

---

## §6 — Perguntas em aberto para você

Estas são as lacunas que impedem esta consolidação de ser 100% confiável:

1. **Engine.** A decisão UE5 vs. Godot está formalmente aguardando o resultado do LGP, ou são trilhas
   independentes? (§4.1)
2. **Branches de Diplomacia.** O código de diplomacia não mergeado deve ser mergeado, descartado, ou
   redesenhado do zero junto com o doc de diplomacia? (§4.5)
3. **Migração de população.** A decisão não foi encontrada no arquivo. Foi tomada em alguma sessão e
   não registrada, ou nunca foi tomada? (§4.6)
4. **Colisão do `42`.** Quer renumerar depósitos ou política? (§4.6)
5. **Ordem do caminho crítico.** O §4.7 é proposta minha — concorda com a ordem?
