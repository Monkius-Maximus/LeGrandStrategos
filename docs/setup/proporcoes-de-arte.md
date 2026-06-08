# Proporções de Arte — Referência de Assets

> Catálogo único de **dimensões de todas as artes** a inserir no projeto.
> Consolida os tamanhos travados em `Source/.../Style/StrategosStyleTokens.h`
> e em `etapa-2-ui-grid.md` (cartas), e adiciona mapa/mundo, identidade
> nacional e as **configurações de import da UE5** que você precisa marcar.
>
> Regra mãe: **módulo de 8 px**, base **1920×1080**. Todo tamanho de UI é
> múltiplo de 8. Para cartas de unidade em detalhe, ver `etapa-2-ui-grid.md`
> (este doc não duplica a anatomia interna delas).

---

## 0. Regras de ouro para texturas na UE5

Antes da tabela de tamanhos, o que importa **no import** (um clique errado aqui
estraga a arte):

| Situação | Configuração na UE | Por quê |
|---|---|---|
| Ícone / sprite de UI | **Texture Group = UI**, **sRGB = ON**, **Mip Gen = NoMipmaps** | UI não usa mipmap; UI group preserva nitidez |
| Bandeira / brasão / portrait | sRGB = ON, Mip = padrão | São cor, não dados |
| Máscara / heightmap / dados | **sRGB = OFF**, Compression = *Masks* ou *Grayscale* | Dados não podem sofrer gamma |
| Normal map | Compression = *Normalmap*, sRGB = OFF | — |
| Tudo | Preferir **potência de 2** (256, 512, 1024…) | Habilita compressão DXT e mipmaps |

- **Potência de 2 não é obrigatória para UI** (a UE aceita 144×320 etc.), mas
  texturas de mundo/material rendem melhor em PoT. Quando a arte não for PoT,
  use Compression = *UserInterface2D (RGBA)* para evitar artefato de bloco.
- **Hitbox é sempre retangular** (regra Vic2), mesmo que a arte tenha cantos
  arredondados ou forma orgânica. A arte pode "vazar"; a área clicável é o
  retângulo nominal.
- Formato de origem recomendado: **PNG** (com alpha) para UI/ícones; **TGA**
  também serve.

---

## 1. Layout da HUD (valores travados em código)

Fonte: `StrategosStyleTokens.h`. São tamanhos de **container**, não de arte —
mas ditam o espaço útil que a arte de fundo/moldura precisa preencher.

| Elemento | Dimensão | Múltiplo |
|---|---|---|
| Top bar (barra superior) | altura **64** | 8M |
| Left rail (trilho esquerdo) | largura **56** | 7M |
| Right outliner (painel direito) | largura **320** | 40M |
| Province dock (colapsada) | altura **44** | — |
| Province dock (expandida) | **760 × 560** | — |
| Padding interno padrão | 8 (1M) | — |
| Gap entre cartas | 16 (2M) | — |
| Espessura de borda colorida | 2 px | — |
| Raio de canto | 0–8 px | — |

> Texturas de fundo/painel da HUD: produza em **tiles** ou **9-slice** (bordas
> + centro) para escalar sem distorcer. Um painel 320 de largura não precisa de
> arte 320 de largura se for 9-slice; um tile de 32–64 px basta.

---

## 2. Cartas de unidade (resumo — detalhe em `etapa-2-ui-grid.md`)

| Variante | Tamanho (px) | Proporção | MVP? |
|---|---|---|---|
| **Micro** | 144 × 320 | 9:20 | ✅ |
| **Compact** | 192 × 288 | 2:3 | ✅ |
| **Detalhada** | 720 × 1024 | ~3:4 | ❌ pós-MVP |
| **Painel customização** | 320 × 800 | 2:5 | ❌ pós-MVP |

Áreas internas de arte dentro das cartas:

| Arte | Dimensão | Onde |
|---|---|---|
| Portrait (compact) | **160 × 160** (1:1) | corpo da compact |
| Portrait (micro) | **144 × 160** | corpo da micro |
| Portrait (detalhada) | **432 × 320** | centro da detalhada |

---

## 3. Ícones e elementos pequenos

| Elemento | Hitbox | Arte recomendada | Notas |
|---|---|---|---|
| Ícone de stat (ATQ/DEF/...) | 24 × 24 | **48 × 48** ou 96×96 (downscale) | 11 ícones canônicos — ver §8 abaixo |
| Bandeira no header de carta | 32 × 32 | arte 32 × 24 (cropada) | — |
| Brasão / coat of arms | 48×48 ou 64×64 | **128 × 128** circular | medalhão decorativo |
| Botão de ação | 64 × 64 | **128 × 128** | MOVER/ATACAR/etc. |
| Slot de equipamento | 56 × 56 | 112 × 112 | pós-MVP |
| Slot de opção (customização) | 56 × 56 | 112 × 112 | pós-MVP |
| Chip de modificador | 88 × 56 | 9-slice | buff/debuff na detalhada |

> Desenhe ícones em **2× ou 4×** do tamanho de hitbox e deixe a UE fazer o
> downscale — fica nítido em 1080p e já serve para 4K depois.

### Ícones de stat canônicos (a desenhar)

espadas cruzadas (ATQ) · escudo (DEF) · bota (MOB) · bandeira (MOR) · pilares
(ORG) · caixote (SUP) · alvo (ALC) · mira (PREC) · olho (REC) · corrente (SUPR)
· moeda (CST). Todos em 48×48 (hitbox 24×24).

---

## 4. Identidade nacional

| Asset | Dimensão | Proporção | Campo em `Nation` |
|---|---|---|---|
| **Bandeira** | **256 × 192** | 4:3 | `FlagTexture` |
| **Brasão** | **128 × 128** | 1:1 circular | `CoatOfArmsIcon` |
| Cor primária | — | — | `Color` (já existe) |
| Cor secundária | — | — | `SecondaryColor` (a adicionar) |

3 nações canônicas: **Albion, Galia, Norden** (cores pastel "Belle
Cartographie" já definidas em `StrategosStyleTokens.h::Graphite`).

---

## 5. Mapa estratégico e mundo (2D puro)

O mapa é **2D puro** (sem terreno 3D). Hoje cada província é um *plane* com
material colorido (`AStrategosProvinceVisualActor`) e cada exército é um *plane*
com textura placeholder (`AStrategosArmyVisualActor`). Escala de mundo:
**`WorldUnitsPerMapCell = 1000`** (1 célula de mapa = 1000 uu no mundo).

| Asset | Dimensão recomendada | Notas |
|---|---|---|
| Marcador de exército (sprite) | **128 × 128** ou 256 × 256 | aplicado no material via param `BaseTexture`; alpha = silhueta |
| Ícone de exército na UI | 64 × 64 | listas/roster |
| Textura de província (opcional) | tile 256 × 256 | hoje é cor sólida; textura entra se quiser padrão/hachura |
| Pinos de cidade / capital | 64 × 64 | overlay no mapa |
| Background do mapa (se usar) | PoT grande (2048/4096) ou tiles | depende do estilo final |

### Saída do worldgen (módulo experimental)

`StrategosWorldGen` gera um **mapa procedural 1024 × 1024** (default
`MapSize`). O PNG de debug exportado pelo `AWorldGenDebugActor` sai nesse
tamanho em `Saved/WorldGen/`. Se você for usar essa textura como base de mapa,
ela já é quadrada e quase-PoT (1024 é PoT) — bom para material de fundo.

---

## 6. Modais e telas (recomendações)

Não há dimensão travada em código ainda; recomendações alinhadas ao grid:

| Tela / modal | Dimensão | Notas |
|---|---|---|
| Ilustração de evento | **768 × 432** (16:9) | banner no topo do `EventModalWidget` |
| Modal de evento (container) | ~720 de largura | múltiplo de 8 |
| Country card / War room | usar 720 × 1024 como teto | reaproveita grid da detalhada |
| Main menu — logo | 512 × 256 ou maior, PoT | alpha |
| Main menu — background | 1920 × 1080 | tela cheia, pode ser arte ou render do mapa |

---

## 7. Tipografia

Tamanhos de fonte também em múltiplos de 8 (ou 4 para corpo). Sugestão
inicial (ajustável quando a fonte final entrar):

| Papel | Tamanho |
|---|---|
| Título grande (modal) | 32 |
| Título de carta | 16–20 |
| Corpo / labels | 12–14 |
| Numérico de stat | 14–16 |
| Legenda / micro | 10–12 |

Use no máximo 2 famílias: uma display (com serifa, século XIX) para títulos e
uma sans/legível para dados.

---

## 8. Checklist de produção (ordem sugerida para o MVP visual)

- [ ] 11 ícones de stat (48×48) — ATQ DEF MOB MOR ORG SUP + ALC PREC SUPR REC CST
- [ ] 3 bandeiras (256×192) — Albion, Galia, Norden
- [ ] 3 brasões (128×128)
- [ ] Portraits de unidade (160×160) por tipo de unidade do MVP
- [ ] Marcador de exército no mapa (128×128, alpha)
- [ ] Molduras 9-slice da carta compact (192×288) e da HUD
- [ ] (Opcional) 1 ilustração de evento (768×432) para testar o modal
- [ ] Logo + background do main menu

Tudo acima respeitando: **PNG, sRGB ON, Texture Group = UI, sem mipmap** para os
elementos de interface.

---

## 9. Onde os assets moram no projeto

Convenção sugerida (criar conforme produzir):

```
Content/
  UI/
    Icons/Stats/        ic_atq, ic_def, ...
    Cards/              molduras, fundos
    Flags/              flag_albion, flag_galia, flag_norden
    CoatOfArms/         coa_albion, ...
  Map/
    Army/               marcador de exército
    Provinces/          texturas/padrões (se houver)
  Portraits/Units/      retratos de unidade
  Events/               ilustrações de eventos
  Menu/                 logo, background
```

> Lembrete (ver `onboarding-ue5-e-workflow.md`): assets `.uasset` são binários.
> Antes de criar muitos, configure **Git LFS** para `*.uasset`/`*.umap`. E para
> validar visual comigo, **anexe o PNG/screenshot** — eu não leio `.uasset`.
