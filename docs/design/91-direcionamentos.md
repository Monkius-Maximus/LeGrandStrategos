# 91 — Direcionamentos: Roteiros Possíveis para Destravar o LGS

**Status:** PROPOSTA — para leitura, comparação e decisão
**Data:** 2026-09-06
**Premissa fixada nesta sessão:** engine = **Godot (última estável: 4.7.2, jun/2026)**, C#/.NET. Não é mais uma pendência.
**Objetivo declarado:** ter algo jogável e testável em breve. Cada mês sem build é um mês em que o mercado se move sem nós.

---

## §0 — Decisões tomadas para fechar as pendências do doc 90

Você pediu que eu decidisse o que ficou em aberto. Decidi. Cada uma é reversível até você dizer "confirmado".

| Pendência (doc 90) | Decisão | Racional |
|---|---|---|
| Engine | **Godot 4.7.x estável**. Não perseguir 4.8-dev. Subir de minor só quando houver ganho concreto | Você já opera em 4.7 nos outros projetos; 4.7.2 é a estável corrente. Minor releases saem a cada 3–6 meses — perseguir a ponta custa retrabalho sem retorno |
| Dívida UI↔Core do `StrategosUI` | **Morta.** Não reconciliar. O módulo UE5 não é portado | Com Godot decidido, é trabalho em código que será abandonado |
| Branches de Diplomacia não mergeadas | **Descartar.** Diplomacia é desenhada do zero, em documento, antes de qualquer código | Código UE5 não porta; o que porta é contrato. Não há contrato — há código sem doc. É exatamente o cenário de "fonte de verdade duplicada" |
| Migração de população | **Não decidida — e fica marcada como tal.** Entra como item explícito no doc `41` v1.2 | Não invento uma decisão que não encontrei. Fica como pendência nomeada, não como buraco |
| Colisão do `42` | **`42-depositos-e-descoberta.md` → `43-depositos-e-descoberta.md`** | Política está LOCKED e referenciada no projeto; depósitos é o candidato mais barato a renumerar |
| Referência `40-resources-and-production.md` vs `33-resources.md` | **Padronizar em `33`** e corrigir a citação no doc `42` | Um nome só |
| Período 1820–1880 nos markdowns | Corrigir para 1800–1950 na primeira passada de higiene | Dívida pequena, custo zero de decidir |

---

## §1 — Sobre a preocupação com a concorrência

Li as duas páginas da Steam. Minha leitura, sem suavizar:

**Sword & Banner** (Bitrot Games) é RTS medieval-fantasia com goblins e mortos-vivos, batalhas táticas em tempo real com micro de unidades, pixel art, previsto para Q1/2027, ainda em playtest. Não é concorrente do LGS. Compartilha "estratégia" e "economia procedural" como palavras; não compartilha público. Quem compra RTS de formação tática não é quem passa 200 horas gerenciando estratos sociais.

**Imperial Ambitions** (aoiti) é o caso real. Grand strategy 4X **por turnos**, Era dos Descobrimentos, 40+ recursos com cadeias de produção, 5 classes sociais, traços de pop reagindo a fome/doença/desigualdade, batalhas táticas com formações, pixel art. Lançado em 13/jul/2026 a US$19,99. **95 reviews, 81% positivo.** Isso é a informação mais importante da página: dois meses após o lançamento, um jogo tematicamente próximo do seu, com escopo comparável, feito por um time pequeno, tem menos de 100 reviews. Pela regra de bolso usual da Steam (reviews × 30–50 ≈ vendas), estamos falando de algo entre 3 e 5 mil cópias.

O que isso significa para o LGS, em duas frases: **o nicho não está saturado — está subatendido.** Um jogo próximo do seu conceito chegou primeiro e não capturou o público; a demanda que você identificou continua lá. O que Imperial Ambitions prova não é que a janela fechou, é que **um dev solo consegue lançar nesse nicho com pixel art e escopo fechado**, e que o teto de um lançamento sem diferenciação clara é modesto.

Onde o LGS é estruturalmente diferente de Imperial Ambitions, e isso precisa ser visível na primeira build pública:

- **Tempo real com pausa**, não turnos. Muda o público (Paradox-like vs. Civ-like).
- **1800–1950**, não Descobrimentos. Industrialização é o tema, não colonização.
- **Assimetria de PM tiers como pilar.** Imperial Ambitions tem cadeias de produção; não tem "a mesma mercadoria produzida por tecnologias diferentes em nações diferentes, e isso decide guerras". Esse é o seu gancho de uma frase.
- **Batalha por cartas com command points.** Nenhum dos dois faz isso.

A conclusão prática: a preocupação correta não é "alguém fez antes". É **"eu ainda não tenho nada que alguém possa jogar"**. Os roteiros abaixo existem para resolver isso, não para vencer uma corrida que, pelos números, ninguém está ganhando.

---

## §2 — Os roteiros

Quatro caminhos. Cada um responde "o que é a primeira build jogável e o que se corta para chegar nela". Estimativas de tempo assumem dev solo, tempo parcial, com Claude Code como par de implementação — e são chutes calibrados, não compromissos.

### Roteiro A — "Fatia Vertical da Economia"

**Tese:** O único sistema com código rodando é o Economy v1. Portar ele para Godot como assembly C# puro, plugar um mapa importado do FMG, e ter uma **caixa de areia econômica de uma nação** jogável antes de qualquer outra coisa.

**Primeira build:** uma nação, mapa real importado, ~10 dos 31 goods, 2 ladders de PM (Aço e Tecido), tick mensal, preços dinâmicos, auto-investimento da Burguesia. Sem política, sem batalha, sem diplomacia. UI mínima: mapa, painel de província, painel de mercado.

**O que se corta:** tudo que não seja economia + mapa. Tech, leis, batalha e IGs ficam em documento.

**Marcos:**
1. `Strategos.Sim` compila como assembly C# sem dependência de Godot; Economy v1 portado com testes de tick determinístico. *(4–6 semanas)*
2. Import do FMG rodando (o commandlet vira um tool script Godot ou um CLI .NET separado); depósitos seedados; mapa renderizado. *(3–4 semanas)*
3. Loop jogável: jogador constrói edifícios, observa preços, vê a Burguesia investir. Build para 5 testadores. *(3–4 semanas)*

**Total até build testável:** ~3 meses.

**Riscos:** a economia sozinha é um "aquário", não um jogo — testadores vão dizer "bonito, e daí?". O pilar (assimetria entre nações) não aparece com uma nação só. Risco de polir o aquário por meses.

**Reaproveita:** 100% do design travado, o código do Economy v1 como referência linha a linha, o playbook do doc `70`.

**Sinal de sucesso:** um testador consegue explicar, sem você, por que o preço do aço caiu.

---

### Roteiro B — "LGP como Teste de Mercado"

**Tese:** O LGP já é Godot, já tem escopo travado (10 goods, 3 facções, vitória territorial, sessões de 2–4h, combate auto-resolvido), já validou o cluster território→produção. Ele é o **caminho mais curto até um humano estranho jogar algo seu**. O LGS vira o "jogo grande" que herda o que o LGP provar.

**Primeira build:** o protótipo do LGP como já especificado. Nenhum redesign.

**O que se corta:** o LGS inteiro pausa. Sem exceções, sem "só portar a economia enquanto isso".

**Marcos:**
1. Executar os 6 prompts de pesquisa técnica pendentes (API Godot 4.7, dependências). Resolver a tensão `Resource`/`StringName` vs. assembly C# puro — **decidir: assembly puro, com adaptadores de carga**. *(2 semanas)*
2. Fases 1–3 do mapa de execução de 9 workstreams: mapa, produção, facções, combate blob. *(6–8 semanas)*
3. Diplomacia bilateral mínima (o cluster ainda não desenhado) + condição de vitória + build itch.io/Steam playtest. *(4–6 semanas)*

**Total até build testável:** ~3–4 meses. Até uma página na Steam com demo: ~6.

**Riscos:** dois jogos, um dev. O LGP pode "virar o jogo" e o LGS nunca voltar — o que não é necessariamente ruim, mas precisa ser uma escolha consciente. O LGP não testa o pilar do LGS (assimetria de PM tiers é rasa com 2 intermediários e 3 finais). O público de 4X-sessão não é o público de grand-strategy-200-horas.

**Reaproveita:** a arquitetura engine-agnóstica (que é o objetivo declarado do LGP), padrões do doc 90 §3, o `FactionDefinition` como molde para as nações do LGS.

**Sinal de sucesso:** 20 pessoas terminam uma partida e 5 pedem uma segunda.

---

### Roteiro C — "Batalha Primeiro"

**Tese:** O sistema de batalha por cartas (doc `60`) é o **diferencial mais visível** do LGS contra tudo que existe no nicho, e é o único sistema que funciona como jogo isolado. Um "autobattler tático de cartas com command points" é testável em semanas, não meses, e gera material de divulgação (GIFs, clipes) que uma tela de mercado nunca gera.

**Primeira build:** cenário de campo aberto, dois comandantes, decks de ~15 cartas, unidades com tiers fixos (o interlock `eficácia = tier_conceito × PM.Tier` com PM.Tier hardcoded por cenário — o único hardcode permitido, e temporário). IA de utility-scoring 1-ply. Auto-resolver headless rodando o mesmo engine (que é também o harness de teste).

**O que se corta:** mapa, economia, pops, política. Tudo.

**Marcos:**
1. Engine de batalha como assembly C# puro com auto-resolver e testes de determinismo. *(3–4 semanas)*
2. UI de batalha em Godot: mão de cartas, campo, log. *(3–4 semanas)*
3. 6 cenários autorais mostrando a assimetria (Albion tier 4 vs. Norden tier 2 com mais manpower). Build para testadores. *(2 semanas)*

**Total até build testável:** ~2–2,5 meses. **O mais rápido dos quatro.**

**Riscos:** é o roteiro que **mais se afasta do pilar**. Sem economia por trás, a assimetria de tier é um número que você digitou. Testadores vão avaliar "é um bom jogo de cartas tático?", que é uma pergunta diferente de "é um bom grand strategy?". Risco real de o feedback puxar o projeto para um gênero que não é o seu.

**Reaproveita:** doc `60` integralmente. O engine de batalha entra intacto no LGS depois — é um consumidor de tiers, não sabe de onde os tiers vêm.

**Sinal de sucesso:** um clipe de 30 segundos de uma batalha faz sentido para quem nunca ouviu falar do jogo.

---

### Roteiro D — "Fusão: LGS com Escopo de LGP"

**Tese:** A separação LGS/LGP existia em parte por causa da engine (UE5 vs. Godot). Com a engine unificada, a razão estrutural para dois projetos enfraquece. **Um codebase Godot, o design do LGS, o disciplinamento de escopo do LGP.** Três nações, 1800–1950, tempo real com pausa, mas com a lista de goods cortada para ~12, uma ladder de PM completa (Aço, 5 tiers), política v1 e batalha auto-resolvida — batalha por cartas entra na fase 2.

**Primeira build:** três nações no mapa FMG, economia com 12 goods, ladder de Aço com tiers diferentes por nação desde 1800 (o pilar, visível no minuto 1), IGs + 2 categorias de lei (`Economy`, `Military`), guerra com auto-resolve. Sessão-alvo: 60–90 min por "década".

**O que se corta:** 19 goods, 6 ladders de PM, 3 categorias de lei, batalha por cartas, tech emergence (tiers avançam por evento datado na fase 1 — sim, é um fallback, e é temporário e nomeado), religião, diplomacia além de "guerra/paz".

**Marcos:**
1. `Strategos.Sim` assembly puro + Economy v1 portado + import FMG. *(6–8 semanas — é o marco 1+2 do Roteiro A)*
2. Política v1 (IGs, clout, enactment) + 3 nações com overrides autorais. *(4–6 semanas)*
3. Guerra auto-resolvida usando o interlock + UI de campanha. Build testável. *(4–6 semanas)*

**Total até build testável:** ~4–5 meses. **O mais lento dos quatro** — mas é o único cuja primeira build **é o jogo**.

**Riscos:** é o roteiro que mais depende de disciplina de escopo, que é exatamente o que o LGS nunca teve (o LGP foi criado para ter). O LGP fica em limbo — ou é absorvido ou é cancelado; precisa ser dito. Cinco meses sem build pública é muito tempo se a ansiedade com o mercado for real.

**Reaproveita:** tudo dos dois projetos. É o roteiro de maior reaproveitamento e menor desperdício de design.

**Sinal de sucesso:** um testador, jogando de Norden, descobre sozinho que precisa do Bessemer para não perder a guerra contra Albion — e entende por que não consegue aprová-lo (os latifundiários bloqueiam a lei).

---

## §3 — Comparação

| | A — Economia | B — LGP | C — Batalha | D — Fusão |
|---|---|---|---|---|
| Tempo até build testável | ~3 meses | ~3–4 meses | **~2–2,5 meses** | ~4–5 meses |
| Primeira build mostra o pilar? | Não (uma nação) | Parcialmente | Não (tier hardcoded) | **Sim** |
| Primeira build é "um jogo"? | Não — aquário | **Sim** | Sim, mas de outro gênero | **Sim** |
| Material de divulgação | Fraco | Médio | **Forte** | Médio |
| Risco de desvio de gênero | Baixo | Médio | **Alto** | Baixo |
| Risco de scope creep | Médio | Baixo (escopo travado) | Baixo | **Alto** |
| Reaproveitamento do design travado | Alto | Médio | Médio | **Total** |
| Destino do LGP | Pausado | **É o produto** | Pausado | Absorvido ou cancelado |
| Testa a arquitetura engine-agnóstica | Sim | Sim (objetivo declarado) | Sim | Sim |
| O que se aprende com testadores | "A economia é legível?" | "O loop de 3h prende?" | "O combate é divertido?" | "O pilar é sentido?" |

---

## §4 — Minha recomendação

**Roteiro D, com o marco 1 do Roteiro C rodando em paralelo como assembly separado.**

O raciocínio:

O seu problema declarado é tempo até algo testável, mas o seu problema **estrutural** é que o LGS tem oito documentos travados e zero código Godot. Qualquer roteiro que não seja D deixa a maior parte desse design na gaveta por mais um ciclo. O Roteiro A produz um aquário; o C produz um jogo de outro gênero; o B produz outro jogo. Só o D produz **o LGS**, e a pergunta que os testadores vão responder — "a assimetria de tiers é sentida?" — é a única pergunta que importa para saber se o pilar do projeto funciona.

O custo é dois meses a mais que o roteiro mais rápido. Contra isso pesa o que Imperial Ambitions mostra: chegar primeiro com algo genérico dá ~100 reviews. **Chegar com o pilar visível vale mais que chegar dois meses antes.**

O engine de batalha como assembly paralelo é a concessão à ansiedade de mercado: é o sistema mais "clipável", é 100% independente (consome tiers, não sabe de onde vêm), e o auto-resolver headless que o Roteiro D precisa no marco 3 **é o mesmo código**. Você não gasta duas vezes — só decide se a UI de cartas entra na primeira build ou na segunda. Recomendo segunda.

Quanto ao LGP: recomendo **absorver formalmente**. O `FactionDefinition`, o cluster território→produção resolvido e a disciplina de escopo migram para o LGS. O que não migra — sessão de 2–4h, vitória territorial, 4X — fica documentado como "modo de jogo curto", possível fase 3. Isso precisa ser dito explicitamente num doc, não deixado morrer em silêncio.

**Se você discordar da recomendação**, o segundo melhor é o **B**, porque é o único outro roteiro cuja primeira build é um jogo completo — e o LGP já tem o disciplinamento de escopo que o D exige e o LGS nunca teve.

---

## §5 — As próximas duas semanas, independentemente do roteiro

Isto não depende de qual caminho você escolher. Todo roteiro precisa disso:

1. **Criar o repositório Godot 4.7.x** com a estrutura de dois projetos: `Strategos.Sim` (class library .NET, zero referência a Godot) e `Strategos.Game` (projeto Godot que referencia o Sim). Esta separação é o doc `70` materializado, e é o que o LGP identificou como tensão não resolvida — resolve-se agora, por decreto: **o Sim não conhece `Resource` nem `StringName`**; o Game carrega DataAssets via adaptadores.
2. **Resolver a tensão `Resource` vs. C# puro** em um ADR de uma página. A decisão acima é a minha; você confirma ou muda.
3. **Higiene do vault:** renumerar `42`→`43` depósitos, corrigir `40`→`33`, corrigir período. Meio dia.
4. **Executar o import do FMG como CLI .NET** (não como tool script Godot) — assim ele roda em CI e independe do editor. É o marco mais barato e o de maior salto de confiança: transforma "sistemas desenhados" em "sistemas com dados".
5. **Escrever o `CLAUDE.md` do repo** a partir do seu template genérico, com os princípios do doc 90 §1.4 e o ciclo recon→aprovação→implementação.

Ao fim das duas semanas você tem um repo que compila, um mapa importado com depósitos, e um ADR que fecha a única tensão arquitetural real do lado Godot. Aí a escolha do roteiro vira a escolha do que se constrói **em cima** disso — e nenhum trabalho foi desperdiçado.

---

## §6 — O que preciso de você

1. Qual roteiro (ou qual combinação)?
2. A absorção do LGP no LGS: sim, não, ou "ainda não"?
3. Confirma as decisões do §0? Em especial: descartar as branches de Diplomacia e matar a dívida do `StrategosUI`.
4. Confirma `Strategos.Sim` como assembly C# puro sem tipos Godot?
