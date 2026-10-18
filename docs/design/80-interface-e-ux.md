# 80 — Interface e UX: Layout, Telas e Sistemas Atrelados

**Status:** DRAFT — proposta para confirmação seção a seção
**Data:** 2026-10-07
**Documentos relacionados:** `33-resources.md`, `41-population-and-social-strata.md`, `42-politica-e-grupos-de-interesse.md`, `43-depositos-e-descoberta.md`, `52-seed-leis.md`, `60-sistema-de-batalha.md`, `70-arquitetura-modular-e-godot.md`, `91-direcionamentos.md`, `92-adr-0001-fronteira-sim-godot.md`
**Escopo:** como a tela do LGS se organiza, que painéis existem, que sistema do `Strategos.Sim` alimenta cada um, e em que ordem isso entra nos sprints.

---

## §0 — Como ler este documento

Tudo aqui é **proposta minha** até você confirmar. Não há decisão travada neste documento ainda.

Três marcações:

- **[REF]** — consequência direta de um documento já LOCKED. Não é decisão nova; é a UI obedecendo a regra que já existe.
- **[PROPOSTA]** — decisão nova de UI/UX, a confirmar.
- **[ABERTO]** — pergunta que eu não consigo responder sozinho. Listadas de novo no §16.

Nomes de componentes e tipos (`TierLadder`, `ShareBar` etc.) são **provisórios**. O `Strategos.Game` ainda não tem código, então não há gramática para colidir — mas os nomes finais saem do passe de implementação, não deste documento. Qualquer referência a identificador já existente no Sim vai marcada `⚠ RECONCILIAR`.

Origem dos padrões: análise do mod MEIOU and Taxes para EU4 (conversa de 2026-10-07), filtrada pelos princípios do projeto. Do mod veio o "o quê"; dos princípios veio o "o que não copiar".

---

## §1 — Princípios de UI

Sete regras. Toda tela abaixo é consequência delas.

1. **O mapa é a interface.** O jogador deve conseguir entender o estado do mundo pelos modos de mapa antes de abrir qualquer painel. Painel é para agir e para detalhar, não para descobrir. **[PROPOSTA]**

2. **Todo número se explica.** Qualquer valor numérico na tela tem um tooltip que decompõe o valor em suas fontes. Como o `UActiveModifierRegistry` ⚠ RECONCILIAR é o único canal de efeito do jogo, o tooltip é só uma consulta ao registry. **[REF — `42` §3.2]**

3. **A UI só lê o Sim e só escreve por comando.** A UI nunca calcula regra de jogo, nunca guarda estado de simulação e nunca duplica enum. Ela consulta, exibe e envia comandos. **[REF — ADR-0001, `70`]**

4. **Uma forma só, também na UI.** Cada conceito visual tem um componente, e todos os sistemas usam o mesmo. A escada de PMs, a escada de leis e os tiers militares são o mesmo componente, porque `Tier` é o mesmo primitivo. **[REF — princípio 4 do `90` §1.4]**

5. **Divulgação progressiva.** Resumo → detalhe → tabela. Tabela nunca é a visão padrão. **[PROPOSTA]**

6. **Fail-fast também na UI.** Chave de localização ausente, fonte de modifier sem `DisplayName`, modo de mapa apontando para consulta inexistente: tudo isso quebra teste ou build, não aparece como texto cru na tela. **[PROPOSTA]**

7. **O pilar é visível sem clique.** A assimetria de Tier entre nações tem que aparecer no mapa e na barra superior desde o primeiro minuto, não escondida num submenu. **[PROPOSTA]**

---

## §2 — Arquitetura: como a UI conversa com o Sim

### 2.1 Os três canais

Os mesmos três canais do doc `70`, e nada além deles. **[REF]**

| Canal | Direção | Uso na UI |
|---|---|---|
| **Query** | UI → Sim (leitura) | Tudo que é exibido. Retorna dados somente-leitura. |
| **Command** | UI → Sim (escrita) | Toda ação do jogador: construir, iniciar enactment, mover exército, mudar velocidade. |
| **Event** | Sim → UI | Avisos de mudança: tick concluído, enactment resolvido, depósito descoberto, guerra declarada. |

### 2.2 Regras de fluxo

- **Comandos são aplicados na fronteira do tick**, nunca no meio dele. É o que preserva o determinismo exigido pelo `42` §4.4 e pelo ADR-0001. **[PROPOSTA]**
- **A UI atualiza só o que está visível**, ao receber o evento de tick concluído. Painel fechado não consulta nada. **[PROPOSTA]**
- **Thread única na v1.** O tick roda, termina, a UI lê. Paralelizar simulação e renderização fica para quando houver medição mostrando necessidade. **[PROPOSTA]**
- **A UI nunca exibe uma mecânica que o Sim não implementa.** Nada de botão desabilitado com "em breve", nada de modal com valores falsos. Foi exatamente o que aconteceu com os modais de guerra e diplomacia do `StrategosUI`. **[PROPOSTA]**

### 2.3 Por que isso elimina a dívida do `StrategosUI`

O `StrategosUI` quebrou porque redeclarava enums a partir dos documentos de design (`ELeaderArchetype`, `EDiplomaticStatus::Truce`, `EPopStratum`). Em Godot, o `Strategos.Game` **referencia** o `Strategos.Sim` e usa os tipos dele diretamente. Um enum renomeado no Sim vira erro de compilação na UI no mesmo instante. Essa classe de bug deixa de existir por construção. **[REF — ADR-0001]**

---

## §3 — Layout por zonas

```
┌──────────────────────────────────────────────────────────────────────┐
│ BARRA SUPERIOR: nação · tesouro · capital político · manpower ·      │
│                 Tier de Aço · data · velocidade · pausa              │
├──────────────────────────────────────────────────────────────────────┤
│ faixa de alertas (só aparece quando há alerta)                       │
├──────────────┬─────────────────────────────────────────┬─────────────┤
│              │                                         │             │
│   PAINEL     │                                         │  OUTLINER   │
│  CONTEXTUAL  │               MAPA                      │             │
│  (um por     │                                         │  processos  │
│   vez)       │                                         │  em curso   │
│              │                                         │             │
│  + 1 nível   │                                         │             │
│  de detalhe  │                                         ├─────────────┤
│              │                                         │ modos de    │
│              │      [ legenda do modo de mapa ]        │ mapa +      │
│              │                                         │ minimapa    │
└──────────────┴─────────────────────────────────────────┴─────────────┘
          Sobreposições centrais: gazeta de evento · modal de confirmação
```

**Regras de zona:** **[PROPOSTA]**

- **Um painel contextual por vez**, mais no máximo um nível de detalhe empilhado ao lado dele. Abrir outro painel substitui o atual. `Esc` desempilha.
- O painel contextual e o outliner são **colunas fixas**, não janelas flutuantes. Nada se sobrepõe ao mapa além das sobreposições centrais, que são modais e curtas.
- A **legenda do modo de mapa** está sempre visível quando o modo não é o político.
- O outliner pode ser recolhido; o painel contextual fecha. O mapa nunca fica coberto por mais de ~40% da largura.

---

## §4 — Mapa e modos de mapa

### 4.1 Um renderizador, N fontes

Cada modo de mapa é **dado**, não código: **[PROPOSTA]**

```
Modo de mapa
├── Id
├── DisplayName (chave de localização)
├── Fonte            → uma consulta do Sim que devolve um valor por província
├── Escala           → categórica | sequencial | divergente
├── Legenda          → derivada da escala
└── Foco             → escurece tudo fora da nação do jogador? (sim/não)
```

Adicionar um modo novo é adicionar uma entrada e, se preciso, uma consulta no Sim. O renderizador não muda. Se a fonte declarada não existir, o carregamento falha com erro nomeando o modo. **[REF — fail-fast]**

### 4.2 Modos da v1

| Modo | Fonte (sistema) | Escala | Por que existe |
|---|---|---|---|
| **Político** | World / Nações | categórica | Padrão. Quem é dono do quê. |
| **Tier de Aço** | Economia (`33`) | sequencial, T1–T5 | **O pilar.** Mostra o melhor PM de Aço ativo por província. Albion clara, Norden escura, no primeiro minuto. |
| **Mercado** | Economia (`33`) | divergente | Preço ou déficit de um good escolhido na legenda. |
| **Depósitos** | Depósitos (`43`) | categórica | Depósitos **conhecidos** pelo jogador, por tipo. |
| **População** | População (`41`) | categórica | Estrato ou profissão dominante na província. |
| **Base política** | Política (`42`) + População (`41`) | categórica | IG com maior base local. **Derivado**: pops da província × afinidade por estrato. Não é dado novo; o clout continua nacional. |
| **Militar** | Militar | categórica | Exércitos, frentes e ocupação durante guerra. |

Adiados, porque o sistema ainda não existe: religião territorial, diplomacia, infraestrutura de mercado (camadas do `33`). **[REF — `93` §4]**

### 4.3 Regras visuais do mapa

- **Foco:** nos modos analíticos, tudo fora da nação do jogador fica escurecido, como no MEIOU. Nos modos político e militar, não. **[PROPOSTA]**
- **Legenda interativa:** clicar numa categoria da legenda destaca só as províncias dela. **[PROPOSTA]**
- **Agregação por zoom:** com o zoom afastado, exércitos e marcadores se agregam por região. O mapa coberto de bandeiras do MEIOU é o anti-exemplo. **[PROPOSTA]**
- **Cor nunca sozinha:** o modo Tier de Aço mostra também o numeral do tier no centro da província quando o zoom permite. Escalas sequenciais perceptualmente uniformes, seguras para daltonismo. **[PROPOSTA]**

### 4.4 Visão do designer

O toggle da `previa-depositos-lgs.html` vira um **modo de mapa de desenvolvimento**: revela depósitos ocultos e o que cada um exige para ser descoberto. Existe só em build de desenvolvimento, excluído do build de release por flag de compilação. Nunca vira trapaça para o jogador. **[PROPOSTA]**

---

## §5 — Barra superior e alertas

### 5.1 Barra superior

| Campo | Fonte | Observação |
|---|---|---|
| Bandeira e nome | Nações | Clique abre o painel da nação. |
| Tesouro + variação mensal | Economia | Tooltip decompõe receitas e despesas. |
| **Capital político: atual / teto** | Política (`42` §5) | Mostrar o teto é obrigatório: o excedente é descartado (`42` §5.4). Fica âmbar a partir de 90% do teto. Tooltip decompõe `Base(Governança) × Alignment`. |
| Manpower | População / Militar | Separado em pops militares e reserva civil, porque conscrição civil tem custo (`60`). |
| **Tier de Aço nacional** | Economia | O melhor PM de Aço em operação. É o pilar na barra superior. |
| Data, velocidade, pausa | Orquestrador de tempo | Ver §10. |

**[PROPOSTA]**, exceto o teto de capital político, que é **[REF]**.

### 5.2 Faixa de alertas

Um alerta só existe se o jogador **pode agir** sobre ele. Clique leva à tela da ação. **[PROPOSTA]**

| Alerta | Condição | Clique leva a |
|---|---|---|
| Capital perto do teto | ≥ 90% do teto | Painel de política |
| Nenhum enactment ativo | capital disponível ≥ menor custo de lei adjacente | Painel de política |
| Good estratégico em déficit | demanda > oferta por N meses | Painel de mercado, good selecionado |
| Depósito descoberto | evento de descoberta | Província no mapa |
| Pops em conscrição civil | manpower militar esgotado | Painel militar |

---

## §6 — Outliner

Lista de **processos em andamento**, cada linha com relógio ou progresso. Clique numa linha foca o mapa e abre o painel correspondente. **[PROPOSTA]**

| Seção | Cada linha mostra | Fonte |
|---|---|---|
| **Enactment** | lei alvo · meses restantes · custo já pago · P(sucesso) | Política (`42` §4) |
| **Construções** | edifício · província · progresso | Economia |
| **Prospecção** | região · recurso · tendência das odds | Depósitos (`43`) |
| **Exércitos** | nome · tamanho · ordem atual | Militar |
| **Guerras** | adversário · situação | Militar / Diplomacia mínima |

Sobre o **P(sucesso) do enactment**: o `42` §4.4 resolve com o clout do bloco governante **no fim** do período. Durante o enactment, o clout pode mudar. Então o outliner mostra "P(sucesso) se resolvesse hoje", recalculado a cada tick, nunca o resultado da rolagem. A rolagem é única e determinística e não deve vazar antes da hora. **[PROPOSTA, derivada de `42` §4.4]**

Seção Enactment fica sempre no topo e sempre visível, mesmo vazia ("Nenhuma lei em votação"). Com um enactment por vez na nação inteira (`42` §4.1), é a decisão de maior peso do jogo. **[PROPOSTA]**

---

## §7 — Painéis contextuais

### 7.1 Província

Seções recolhíveis, não abas. A ordem reflete o pilar. **[PROPOSTA]**

1. **Cabeçalho** — nome, dono, terreno, camada de mercado a que pertence.
2. **Produção** — edifícios, cada um com seu PM ativo exibido num `TierLadder` compacto: os tiers existentes, o ativo marcado, os bloqueados apagados. Tooltip no PM: insumos, produtos, profissões consumidas.
3. **População** — barra por estrato (Lower / Middle / Upper) com satisfação de necessidades (`NeedsBar`, §8). Detalhe sob demanda: tabela por profissão (`Peasant`, `Laborer`, `Machinist`, `Artisan`, `Clerk`, `Capitalist`, `Aristocrat`, `Soldier`, `Officer`) ⚠ RECONCILIAR contra o `41` final. Os níveis da cesta de necessidades ainda não estão travados no `41`; o componente mostra o que o Sim expuser.
4. **Recursos** — depósitos conhecidos e o que falta para descobrir os prospectáveis. Nada dos ocultos.
5. **Militar** — manpower local, guarnição.

### 7.2 Nação

Visão geral com quatro blocos lado a lado: economia, política, militar, tecnologia. Cada bloco tem um número-síntese e abre o painel próprio. É a "home" do jogador. **[PROPOSTA]**

### 7.3 Mercado

**[PROPOSTA]**

- **Lista de goods** com preço atual, minigráfico dos últimos 24 meses e saldo oferta/demanda. Ordenável. Filtro por categoria raw / intermediate / final (`33`).
- **Detalhe de um good:** quem produz, com qual PM, em qual Tier, por nação. **É aqui que o pilar fica explícito no painel:** "Aço — Albion T4, Galia T3, Norden T2". Gráfico de preço com os eventos relevantes marcados.
- Quanto o jogador sabe dos Tiers das outras nações é **[ABERTO]** (ver §16).

### 7.4 Política

O painel que mais ganha com o padrão do MEIOU, e que pode ser melhor do que ele. **[PROPOSTA, sobre regras [REF] do `42`]**

**Bloco 1 — Quem manda.** Uma `ShareBar` horizontal com o clout dos seis IGs, ordenada do maior para o menor, com uma **linha vertical em 50%**. Os IGs à esquerda da linha são o bloco governante, sem conta nenhuma. Só frações, nunca valor absoluto. **[REF — `42` §2.3, §2.4]** Cada IG mostra também a satisfação. Tooltip decompõe o clout por estrato de origem.

**Bloco 2 — A matriz de leis.** Linhas = as cinco categorias. Colunas = tiers. Em cada linha:

- um marcador sólido no **tier vigente**;
- um **ponto por IG** no seu `PreferredTier`, com tamanho proporcional ao clout e contorno reforçado se o IG está no bloco governante;
- as células adjacentes ao vigente (±1) são clicáveis, se os gates de tech estiverem satisfeitos.

Isso traduz direto a regra do `42` §4.2: apoio e oposição **emergem da distância** ao `PreferredTier`. O jogador vê na hora quem puxa para cada lado, sem tabela de apoio por lei.

**Bloco 3 — Prévia do enactment.** Ao clicar numa célula adjacente:

- **Custo**, decomposto: `BaseCost`, oposição, apoio, e o piso de 25% quando aplicável. **[REF — `42` §4.3]**
- **P(sucesso) hoje**, com a regra de que só o bloco governante decide. **[REF — `42` §4.4]**
- **Duração** em meses.
- **Diff de modifiers**: o que sai e o que entra se a lei passar, lido do registry.
- Aviso explícito: "Falha consome o capital integralmente." **[REF — `42` §4.4]**
- Botão de confirmar → modal → comando.

**Bloco 4 — Geração de capital.** `Base(Governança) × Alignment`, com a contribuição de cada IG do bloco. **[REF — `42` §5.2]**

Leis com gate de tech não satisfeito **não aparecem** — nó inerte, sem versão degradada (`42` §3.4). Como isso interage com os pontos de `PreferredTier` que caem num tier invisível é **[ABERTO]**.

### 7.5 Militar

**[PROPOSTA]**

- Lista de exércitos e comandantes.
- **Interlock decomposto** por tipo de unidade: `eficácia = tier_conceito × PM.Tier`, mostrando os dois fatores e de onde vem cada um. O jogador de Norden precisa ver que o problema é o PM, não o conceito. **[REF — fórmula do `90` §1.1]**
- Manpower: pops militares disponíveis, e aviso de quando a próxima convocação passa a drenar pops civis. **[REF — `60`]**
- **Relatório de batalha** auto-resolvida: forças, Tiers envolvidos, perdas, e por que o resultado saiu assim. Na fase 1 do Roteiro D, a guerra é auto-resolve; o relatório é a única tela de combate.

### 7.6 Tecnologia

**Adiado.** Na fase 1 do Roteiro D, Tiers avançam por evento datado (`93` §4), então não há sistema para exibir. Quando o tech emergence (`50`) entrar, o painel mostra as sete esferas e as fases spark / difusão / adoção. Até lá, o avanço de Tier aparece como gazeta (§10) e no modo de mapa Tier de Aço. **[REF]**

### 7.7 Batalha por cartas (fase 2)

Fora da primeira build. Quando entrar: mão de cartas embaixo, campo no centro, command points e log de rodadas à direita. Mesmo motor do auto-resolve (`60`). Fica registrado só para que o layout geral não colida com ele: a batalha é **tela cheia**, não um painel contextual. **[PROPOSTA]**

---

## §8 — Catálogo de componentes compartilhados

Cada conceito visual tem **um** componente. Variante ad hoc não entra. **[PROPOSTA]**

| Componente | O que é | Quem usa |
|---|---|---|
| `TierLadder` | Escada ordinal horizontal: tiers, ativo marcado, bloqueados apagados, numeral sempre visível | PMs, leis (linha da matriz), conceitos militares, tech (futuro) |
| `ShareBar` | Barra empilhada de frações que somam 1.0, com linhas de limiar opcionais | Clout dos IGs (limiar 50%), composição de estratos, participação de mercado |
| `ProvenanceTooltip` | Tooltip que decompõe um valor em fontes: base + lista de (fonte, valor) | Qualquer número da UI |
| `OutlinerRow` | Linha com ícone, título, progresso/relógio e ação de foco | Outliner inteiro |
| `NeedsBar` | Barra segmentada por nível de necessidade de um estrato | Província, nação |
| `Sparkline` | Minigráfico de série temporal | Mercado, tesouro |
| `EventGazette` | Sobreposição de evento em formato de jornal de época | Eventos maiores (§10) |
| `ConfirmModal` | Confirmação de ação irreversível com resumo do custo | Enactment, declaração de guerra, demolição |

O `TierLadder` é o mais importante: ele é o `Tier` como primitivo visual, do mesmo jeito que `Tier` é o primitivo de dado.

---

## §9 — Tooltips de proveniência

**[PROPOSTA, sobre o registry [REF]]**

- Todo valor numérico exibido é ligado a uma consulta que sabe devolver sua decomposição.
- A decomposição de um modifier vem do registry: lista de (fonte, `DisplayName`, contribuição).
- **Tooltips aninhados:** um item dentro do tooltip pode ter seu próprio tooltip. Para não sumir quando o mouse se move, o tooltip **trava** depois de um instante parado sobre ele.
- Fonte de modifier sem `DisplayName` é erro de carregamento. Nunca aparece o id cru, como os `Stat_FLVal_25.6` e `PH_154` das capturas do MEIOU.

---

## §10 — Tempo, pausa e eventos

**[PROPOSTA]**

- **Velocidades 1 a 5 e pausa** (barra de espaço). Pausa é inconfundível: faixa central "PAUSADO", relógio parado.
- **Comandos funcionam com o jogo pausado** e são aplicados na próxima fronteira de tick (§2.2).
- **Dois tamanhos de evento:**
  - **Menor** — vai para a faixa de alertas ou para um log. Não pausa.
  - **Maior** — abre uma `EventGazette` e pausa: enactment resolvido, guerra declarada, depósito de grande porte descoberto, avanço de Tier de Aço de qualquer nação. A gazeta é o formato da `previa-depositos-lgs.html`.
- Quais eventos pausam é configurável pelo jogador, com os acima como padrão.
- **Fim suave** em 1950 é uma gazeta própria com a escolha de continuar. **[REF — `90` §1.1]**

---

## §11 — Direção visual da v1

**[PROPOSTA]**

- **Plana e tipográfica.** Sem molduras ornamentadas, sem texturas pintadas. Ornamento é custo de arte que não cabe em ~5h/semana, e pode entrar depois sem mudar layout nenhum.
- **Sabor de época concentrado em poucos lugares:** a gazeta (tipografia serifada, papel), a paleta das nações, os ícones de goods. O resto é legibilidade.
- **Escala de UI ajustável** pelo jogador.
- **Acessibilidade de cor:** nenhum significado carregado só por cor; escalas seguras para daltonismo; numerais de Tier sempre visíveis.
- Fontes: só com licença que permita distribuição comercial. Escolha fica para o primeiro passe visual.

---

## §12 — Localização

**[PROPOSTA]**

- Todo texto visível vem de chave de localização. Nenhuma string literal na UI.
- Um teste percorre todas as chaves referenciadas por dados e cena e falha se alguma faltar em algum idioma distribuído. Mesmo padrão do teste de fronteira do ADR-0001.
- Quais idiomas a primeira build distribui é **[ABERTO]**.

---

## §13 — Fatiamento por sprint

Alinhado ao Roteiro D (`91`) e ao esboço do `93` §5. **[PROPOSTA]**

| Sprint | Entra na UI | Sistemas consumidos |
|---|---|---|
| **03** | Mapa renderizado; renderizador genérico de modos com **Político, Tier de Aço e Mercado**; painel de Província (cabeçalho, Produção, População resumida); `ProvenanceTooltip`; barra superior com data, velocidade e pausa | World (FMG), Economia |
| **04** | Painel de Mercado; Outliner (Construções); faixa de alertas; modo Depósitos | Economia, Depósitos |
| **Política v1** | Painel de Política completo (§7.4); capital político na barra; seção Enactment no Outliner; modo Base política | Política, População |
| **Guerra v1** | Painel Militar; relatório de auto-resolve; modo Militar; `EventGazette` | Militar |
| **Fase 2** | Batalha por cartas; painel de Tecnologia | Batalha (`60`), Tech (`50`) |

Critério de pronto da fatia do Sprint 03, para manter o padrão do `93`: **um testador abre o jogo, troca para o modo Tier de Aço e diz, sem ajuda, qual nação está mais atrasada na siderurgia.**

---

## §14 — Fora de escopo da v1

- Molduras e arte ornamental.
- Janelas flutuantes, múltiplos painéis abertos, layouts customizáveis pelo jogador.
- Gráficos de pizza.
- Tela de diplomacia (o sistema não existe).
- Painel de religião (doc não existe).
- Painel de tecnologia (§7.6).
- Batalha por cartas (§7.7).
- Suporte a controle/gamepad.

---

## §15 — O que não repetir do `StrategosUI`

| Problema no UE5 | Regra aqui |
|---|---|
| Enums redeclarados a partir dos docs | A UI usa os tipos do Sim diretamente (§2.3) |
| Modais de guerra e diplomacia exibindo mecânicas que o Core não tinha | A UI nunca exibe mecânica não implementada (§2.2) |
| UI desenhada antes do Core estabilizar | Cada painel entra no sprint em que seu sistema entra (§13) |

---

## §16 — Decisões em aberto

1. **Numeração.** Usei `80` porque a família 80s está livre e UI é documento de sistema, não de gestão (90s). Confirma?
2. **Tiers invisíveis por gate de tech.** O `42` §3.4 diz que a lei sem tech não aparece. Mas o `PreferredTier` de um IG pode cair num tier ainda invisível. Mostrar o ponto do IG "além da borda" da ladder revela que existe algo lá. Opções: (a) mostrar uma seta "quer ir além" sem revelar o tier; (b) não mostrar nada; (c) revelar o tier como nó inerte apagado, o que contradiria o `42` §3.4.
3. **Informação sobre outras nações.** O jogador vê os Tiers de PM das rivais no painel de mercado e no modo Tier de Aço? Se sim, o pilar fica explícito desde o início. Se não, abre espaço para espionagem como mecânica futura, mas o modo de mapa precisa de "desconhecido" como categoria.
4. **P(sucesso) "se resolvesse hoje"** no Outliner (§6). Confirma que é aceitável exibir, dado que a rolagem é única e determinística?
5. **Idiomas da primeira build.** PT-BR, EN, ou ambos?
6. **Visão do designer** como modo de mapa só de desenvolvimento (§4.4). Confirma?

---

## Apêndice — Decisões tomadas nesta sessão

- Toda decisão deste documento é **proposta minha**, aguardando confirmação.
- Padrões de origem, extraídos do MEIOU and Taxes: mapa como interface principal com modos analíticos; tooltip de decomposição de valores; outliner de processos em andamento; painel de grupos com tiers visíveis; cadeias horizontais de tiers. *(Proposta minha.)*
- Rejeitados do MEIOU: densidade como padrão, planilhas como visão principal, gráficos de pizza, múltiplos painéis sobrepostos, molduras ornamentadas, chaves de localização vazando para a tela. *(Proposta minha.)*
