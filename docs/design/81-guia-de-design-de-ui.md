# 81 — Guia de design de UI: moldes, tipografia, cor e referências

**Status:** DRAFT — contexto de referência; decisões novas marcadas para confirmação
**Data:** 2026-10-09
**Vale para:** o kit de moldes (canvas "Moldes de telas comuns") e todos os jogos que o usarem. As seções marcadas **LGS** são específicas do Le Grand Strategos.
**Documentos relacionados:** `80-interface-e-ux.md`, canvas "LG — Mockups de Interface 2", canvas "Moldes de telas comuns".

Marcações:
- **[FONTE]** — dado verificado em fonte externa (lista no fim).
- **[PROPOSTA]** — recomendação minha, a confirmar.
- **[MEDIDO]** — calculado sobre os tokens atuais.

---

## §1 — A regra-mãe: molde, skin, conteúdo

Toda tela comum (menu, nova partida, salvar, configurações, pausa, tutorial, marco, designer) separa três camadas:

| Camada | O que é | Quem muda | No Godot |
|---|---|---|---|
| **Molde** | Layout e comportamento | Ninguém, por jogo | Cenas num addon compartilhado entre os repositórios |
| **Skin** | Cores, fontes, nome, subtítulo | Uma por jogo | Um `Theme` por jogo, com type variations |
| **Conteúdo** | Facções, eras, passos de tutorial, abas extras, ferramentas de designer | Cada jogo | Um `Resource` que o jogo preenche |

**[PROPOSTA]** Um jogo novo só escreve skin e conteúdo. Se precisar mudar o molde, a mudança vale para todos os jogos, ou não entra.

No Godot, a skin vira um `Theme` de projeto. Variações de um mesmo controle (botão primário, botão de perigo, botão de aba) viram **Theme Type Variations**: um tipo que estende um tipo base, sobrescreve só o que muda, e é escolhido pelo controle na propriedade *Theme Type Variation*. **[FONTE: Godot, Theme type variations]**

---

## §2 — Tipografia

### 2.1 Três funções, três famílias

| Função | Neutro | LGS | Por quê |
|---|---|---|---|
| **Títulos e texto de época** | IBM Plex Sans 700 | **Crimson Pro** 600–700 | Dá identidade sem custo de arte |
| **Interface** | IBM Plex Sans | **IBM Plex Sans** | Legível em tamanho pequeno, muitos pesos |
| **Números** | IBM Plex Mono | **IBM Plex Mono** | Algarismos de largura fixa: colunas alinham e valores não "pulam" quando mudam |

Licenças: Crimson Pro é **SIL Open Font License**, desenho de Jacques Le Bailly. **[FONTE: CTAN]** IBM Plex também é **SIL OFL**. **[FONTE: arquivo de licença do Plex]** As duas permitem embutir no jogo e vender. Mantenha o arquivo de licença junto das fontes no repositório.

**[PROPOSTA]** Fonte de acessibilidade opcional: **Atkinson Hyperlegible**, da Braille Institute, sob SIL OFL. **[FONTE: Wikipedia]** A versão *Next* saiu em 2025; confira a licença dela antes de usar.

### 2.2 Tamanhos — aqui os mockups precisam mudar

As Xbox Accessibility Guidelines pedem, a 1080p, **altura de corpo mínima de 18 px no PC** e **26 px no console**. Altura de corpo vai do descendente mais baixo ao ascendente mais alto. **[FONTE: XAG 101]** A Game Accessibility Guidelines cita **28 px a 1080p** como piso para TV e recomenda ir além. **[FONTE: GAG]**

**[MEDIDO]** Os mockups v1 e v2 usam muito texto de 11 a 13 px. Isso fica **abaixo** do mínimo de PC. É o mesmo problema dos jogos da Paradox, que compensam com escala de interface.

**[PROPOSTA]** Escala a 1920×1080, escala de interface 100%:

| Papel | Tamanho | Fonte |
|---|---|---|
| Título de tela | 32–40 px | Crimson Pro 700 |
| Título de painel | 24–28 px | Crimson Pro 700 |
| Texto de leitura (gazeta, descrições) | 18 px | Crimson Pro ou Plex Sans |
| Corpo da interface | 16 px | Plex Sans 400–500 |
| Secundário | 14 px | Plex Sans, cor `text-mute` |
| Rótulo de seção em caixa alta | 13 px, espaçamento 0,12 em | Plex Sans 600 |
| Números | 16 px; destaque 22–28 px | Plex Mono 500–600 |

Texto menor que isso só num modo "Compacto" que o jogador liga por conta própria. As XAG aceitam texto menor quando o jogador escolhe. **[FONTE: XAG 101]**

### 2.3 Outras regras de texto [FONTE: XAG 101]

- O jogador deve poder aumentar o texto até **200%** sem perder conteúdo.
- Ofereça pelo menos uma opção **sem serifa**. Fontes estilizadas precisam de alternativa. **[PROPOSTA]** No LGS, isso significa um ajuste "Gazeta em fonte sem serifa".
- No máximo **80 caracteres por linha**.
- Espaçamento entre linhas de pelo menos **1,5×** em parágrafos.
- Caixa alta só em rótulos de uma ou duas palavras.
- Alinhamento à esquerda; nunca centralizado nem justificado em parágrafos.

### 2.4 Fontes no Godot [FONTE: Godot, ResourceImporterDynamicFont]

- Títulos grandes e textos que escalam: ligue `multichannel_signed_distance_field` (MSDF). O texto fica nítido em qualquer tamanho sem rerasterizar.
- Texto pequeno de interface: deixe MSDF desligado e use `subpixel_positioning` e `hinting` adequados.
- Use `fallbacks` para glifos que faltem, como travessão e símbolos de moeda.
- Use `generate_mipmaps` quando a fonte for reduzida.

---

## §3 — Cor

### 3.1 Cor com significado (LGS)

| Significado | Token | Uso |
|---|---|---|
| Ação, seleção, foco | latão `#c9a55a` | Botão primário, item ativo, borda de foco |
| Ganho | `#8fbf9c` | Números positivos |
| Perda, perigo | `#d9827a` | Números negativos, botão de apagar |
| Informação, progresso | cobalto `#7da3c6` | Barras de construção, links neutros |
| Nação | pastéis do token | Só para identificar nação |
| Tier | escala de aço `#3b4351` → `#d9dee2` | Só para Tier, sempre com o numeral |

**Regra:** cada cor tem um significado e cada significado tem uma cor. Latão nunca indica nação; pastel de nação nunca indica ação.

### 3.2 Contraste [FONTE: XAG 102] e medição dos tokens [MEDIDO]

As XAG pedem **4,5:1** para texto normal, **3:1** para texto grande, elementos grandes e texto desabilitado, e **≥ 7:1** no modo de alto contraste.

| Par | Contraste | Veredito |
|---|---|---|
| `text` `#ece4d2` sobre `bg-2` | 12,0:1 | ok |
| `text-mute` `#a59f8d` sobre `bg-2` | 5,8:1 | ok |
| `text-mute` sobre `bg-3` `#2a3140` | 4,9:1 | ok, no limite |
| `text-dim` `#6f6a5e` sobre `bg-2` | 2,8:1 | **falha** — só decorativo, nunca texto |
| latão sobre `bg-2` | 6,5:1 | ok |
| tinta `#17130a` sobre latão | 8,0:1 | ok |
| oxblood `#b25048` sobre `bg-2` | 3,0:1 | **falha como texto** — use `#d9827a` (5,4:1) |
| `line` `#353d4d` sobre `bg-2` | 1,4:1 | só divisor, nunca borda de controle |
| `line-strong` `#4e5867` sobre `bg-2` | 2,1:1 | idem |

**[PROPOSTA]** Novo token **`line-control` = `#717b8b`** para bordas de controles interativos (campos, botões secundários). Mede 3,6:1 sobre `bg-2` e 3,05:1 sobre `bg-3`. ⚠ RECONCILIAR com `StrategosStyleTokens.h`.

### 3.3 Nunca só cor

- Tier: numeral sempre visível.
- Ocupação no mapa: hachura.
- Ganho e perda: sinal + ou −, além da cor.
- Pares que precisam ser distinguidos diferem também em luminosidade; prefira azul e laranja a vermelho e verde.
- **[PROPOSTA]** O ajuste "Padrões além de cor" (já no molde de Configurações) liga hachuras extras nos modos de mapa.

---

## §4 — Layout, grade e escala

- **Base 1920×1080, grade de 8 px.** É o contrato do repositório (`etapa-2-ui-grid.md`).
- **Godot:** para jogo de desktop com interface complexa, a documentação recomenda base 1920×1080, *Stretch Mode* `canvas_items` e *Stretch Aspect* `expand`, com âncoras corretas nos Controls. **[FONTE: Godot, Multiple resolutions]**
- **Escala de interface do jogador:** use o *Stretch Scale*, alterável em tempo de execução por `get_tree().root.content_scale_factor`. Exponha-o em Configurações. O *Default Theme Scale* não muda em tempo de execução. **[FONTE: Godot, Multiple resolutions]** É o ajuste "Escala da interface" do molde de Configurações.
- **Zonas do LGS:** barra superior, rail, coluna de sistema, mapa, dock de seleção, outliner. Regras no doc 80 e na prancha "Regras de fusão".
- **Alvos de clique ≥ 44 px.** Inclui os itens do rail.

---

## §5 — Componentes comuns do kit

Cada um existe uma vez, no molde, e é tematizado pela skin.

| Componente | Variações (Theme Type Variations) | Observação |
|---|---|---|
| Botão | primário, secundário, perigo, aba, item de rail | Rótulo é verbo: "Iniciar votação", não "OK" |
| Linha de ajuste | seleção, liga/desliga, controle deslizante, tecla | Base do molde de Configurações |
| Card de seleção | — | Nova partida, saves |
| Confirmação | normal, destrutiva | Mostra o custo e o que se perde |
| Faixa de alerta | aviso, guerra | Só o que pede ação |
| Linha de outliner | progresso, tendência, vazio | Clique foca e abre |
| Tooltip de proveniência | simples, aninhado | Fixa depois de um tempo parado |
| **LGS:** TierLadder, ShareBar, gazeta | — | Ver catálogo do canvas v2 |

**Estados obrigatórios** de todo controle: normal, sob o mouse, **foco** (visível, para teclado e controle), pressionado, desabilitado, selecionado. O foco usa latão.

---

## §6 — Comportamento e boas práticas

1. **Pausa amiga.** Todo comando funciona pausado e entra no próximo tick. Abrir janela não pausa sozinho; só eventos maiores pausam, e o jogador escolhe quais.
2. **Custo antes do clique.** Ação irreversível mostra custo, chance e o que se perde antes de confirmar, como a prévia de votação.
3. **Todo número se explica.** Tooltip com a decomposição, aninhado quando um termo tem a sua própria explicação.
4. **Teclado completo.** `Esc` fecha um nível por vez; atalhos aparecem ao lado do item; `Espaço` pausa.
5. **Saves honestos.** Autosave rotativo (três mais recentes). Versão visível. Save antigo com migração abre; sem migração é recusado com o motivo.
6. **Alertas só com ação.** O resto vai para o Diário.
7. **Animação curta.** Até cerca de 200 ms, com opção de reduzir animações.
8. **Textos de interface.** Português do Brasil, vírgula decimal, datas por extenso no jogo ("14 de setembro de 1851"), números em mono.
9. **Falha cedo.** Chave de texto faltando, nome de exibição ausente ou fonte de dado inexistente quebram o build. O modo designer lista essas verificações.

---

## §7 — O que levar de jogos de sucesso

| Jogo | O que ele faz bem | O que levar | Onde entra |
|---|---|---|---|
| **Crusader Kings III / Victoria 3** | Tooltips aninhados: termos destacados abrem outro tooltip. O sistema nasceu no CK3 e foi para o Victoria 3. **[FONTE: PCGamesN]** | Explicar regras dentro da própria interface, sem wiki | Tooltip de proveniência |
| **Victoria 3** | Métodos de produção visíveis no edifício | Ver e trocar o método onde ele acontece | TierLadder no detalhe do edifício |
| **Civilization VI** | "Lentes" que trocam o que o mapa mostra | Um clique muda a leitura do mapa inteiro | Modos de mapa no rail |
| **Europa Universalis IV / Stellaris** | Outliner com tudo que está em andamento | Lista lateral de processos com relógio | Outliner |
| **Football Manager** | Caixa de entrada como fluxo de notícias e decisões; tabelas filtráveis | Histórico separado do que pede ação | Diário; lista de goods no mercado |
| **Factorio** | Sobreposição de informação no mapa com uma tecla; tooltips com números de produção | Mostrar o estado da produção sem abrir painéis | Modo Tier de Aço; tooltip de edifício |
| **RimWorld** | Alertas empilhados à direita; cartas para eventos importantes | Dois níveis de evento: lista discreta e interrupção rara | Faixa de alertas + gazeta |
| **Into the Breach** | Mostra o que vai acontecer antes de você agir | Consequência visível antes de confirmar | Prévia de votação; relatório "por que perdemos" |
| **Anno 1800** | Necessidades por classe, nível a nível | Satisfação por camada, não um número só | NeedsBar por estrato |

Essas referências servem ao "o quê". O "como" segue as regras deste guia e do doc 80. Nenhuma tela copia o visual de outro jogo.

---

## §8 — Checklist rápido para cada tela nova

- [ ] Usa só tokens da skin (nenhuma cor solta).
- [ ] Texto de leitura ≥ 16 px a 100%; nada abaixo de 13 px fora do modo Compacto.
- [ ] Contraste de texto ≥ 4,5:1; bordas de controle ≥ 3:1.
- [ ] Nenhum significado só por cor.
- [ ] Todo número tem tooltip de origem.
- [ ] Toda ação irreversível mostra custo e pede confirmação.
- [ ] Funciona só com teclado; foco visível.
- [ ] Alvos de clique ≥ 44 px.
- [ ] Nenhuma mecânica sem sistema no Sim.
- [ ] Toda string é chave de localização.

---

## §9 — Decisões a confirmar

1. Subir a escala tipográfica dos mockups (§2.2): 16 px de corpo, 13 px mínimo, e um modo Compacto opcional.
2. Criar o token `line-control` `#717b8b` (§3.2).
3. Aposentar `text-dim` como cor de texto (§3.2).
4. Ajuste "Gazeta em fonte sem serifa" e Atkinson Hyperlegible como fonte opcional (§2.1, §2.3).
5. Moldes como addon compartilhado entre os repositórios Godot (§1).

---

## Fontes consultadas

- [Xbox Accessibility Guidelines 101 — Text display](https://devdocs.xbox.com/build/game-principles/accessibility/xag-deep-dives/xag-101-text-display.md)
- [Xbox Accessibility Guidelines 102 — Contrast](https://devdocs.xbox.com/build/game-principles/accessibility/xag-deep-dives/xag-102-contrast.md)
- [Game Accessibility Guidelines — Use an easily readable default font size](https://gameaccessibilityguidelines.com/?p=3285)
- [Godot — Multiple resolutions](https://docs.godotengine.org/en/stable/tutorials/rendering/multiple_resolutions.html)
- [Godot — ResourceImporterDynamicFont](https://docs.godotengine.org/en/stable/classes/class_resourceimporterdynamicfont.html)
- [Godot — Theme type variations](https://docs.godotengine.org/en/stable/tutorials/ui/gui_theme_type_variations.html)
- [CTAN — CrimsonPro](https://ctan.org/pkg/crimsonpro)
- [IBM Plex — licença](https://maxibestof.one/fonts/coding/ibm-plex-mono-LICENSE.txt)
- [Wikipedia — Atkinson Hyperlegible](https://en.wikipedia.org/wiki/Atkinson_Hyperlegible)
- [PCGamesN — Victoria 3 nested tooltip system](https://www.pcgamesn.com/victoria-3/nested-tooltip-system)
