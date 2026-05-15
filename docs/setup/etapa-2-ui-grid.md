# Setup do Editor — Etapa 2 UI Grid

Contrato visual para cartas de unidade, painéis e identidade nacional. Tudo deriva de um **módulo de 8px** em base 1920×1080. Cabe a arte respeitar esse grid; o código só consome os tamanhos definidos aqui.

> Decisões travadas: **Rarity descartada** (unidades não têm tier de raridade). **Stance system** fora do MVP — entra junto com BattleResolver.

---

## 1. Princípios

- **Mapa 2D puro** (estilo Vic2 / Civ3 / Humankind sem mapa 3D).
- **Hitbox sempre retangular**, mesmo quando a arte arredonda cantos ou "vaza" visualmente (Vic2 faz isso com bandeiras circulares).
- **Cartas são views**, não entidades. A entidade é o `UArmy` + `UUnitTypeAsset` + `FArmyLoadout`. A UI escolhe qual variante (micro / compact / detalhada) renderizar.
- **MVP usa a variante compact** na HUD lateral + a micro em painéis comparativos. A detalhada e o painel de customização entram depois.
- Todo tamanho, padding e fonte é múltiplo de 8.

---

## 2. Módulo e proporções

| Constante | Valor | Notas |
|---|---|---|
| Base resolution | 1920×1080 | UE5 DPI Scaling = 1.0 nesse alvo; escala automática pra 4K |
| Módulo (M) | 8 px | Tudo é múltiplo |
| Padding interno padrão | 8 px (1M) | Borda → conteúdo |
| Padding entre cartas | 16 px (2M) | Gap de grid |
| Raio de canto padrão | 8 px (1M) | Cards e botões |
| Espessura de borda | 2 px | Borda colorida da nação |

A escala dinâmica para suportar 1440p / 4K / ultrawide é **pós-MVP**. Por enquanto, target fixo 1080p.

---

## 3. Variantes de carta

| Variante | Tamanho (px) | Proporção | Uso | MVP? |
|---|---|---|---|---|
| **Micro** | 144×320 | 9:20 | Comparativa, enciclopédia, deck-builder | ✅ |
| **Compact** | 192×288 | 2:3 | HUD lateral de roster de exércitos | ✅ |
| **Detalhada** | 720×1024 | ~3:4 | Modal de inspeção (click na compact) | ❌ |
| **Painel customização** | 320×800 | 2:5 | Slide-out direito da detalhada | ❌ |

Todas as variantes consomem o **mesmo** `UArmy` + `UUnitTypeAsset`. Só muda o que cada variante mostra.

---

## 4. Anatomia da carta Compact (192×288)

```
┌──────────────────────────────────┐  192
│ [Flag32]  Nome unidade    [Cost] │  header 24
├──────────────────────────────────┤
│                                  │
│         Portrait                 │  160
│         (160×160 área útil)      │
│                                  │
├──────────────────────────────────┤
│  ATQ  DEF  MOB  MOR              │  stats 24
├──────────────────────────────────┤
│  Traço primário (1 linha)        │  trait 32
├──────────────────────────────────┤
│  ████░░░░░░  XP    ↑rank         │  footer 24
└──────────────────────────────────┘  288
```

- **Header (24px):** bandeira (hitbox 32×32, arte 32×24) + nome (Title Small) + custo de manutenção.
- **Portrait (160px):** ilustração da unidade. Usa proporção 1:1 dentro do espaço. Tinta de borda = `Nation.Color`.
- **Stats (24px):** 4 ícones-stats inline com valor numérico. ATQ/DEF/MOB/MOR sempre — os demais só na detalhada.
- **Trait (32px):** 1 linha curta com o traço passivo principal (`UnitType.PrimaryTrait`).
- **Footer (24px):** XP bar (8px alta) + nível atual (estrelas ou número).

Clique em qualquer área da carta → abre a **detalhada** (modal).

---

## 5. Anatomia da carta Micro (144×320)

Variante "encyclopedia" / comparativa. Stack vertical denso:

```
┌────────────────────────┐  144
│ [Flag] Nação           │  header 24
├────────────────────────┤
│                        │
│   Portrait 144×160     │  160
│                        │
├────────────────────────┤
│ ATQ DEF MOB MOR (inline)│  stats 24
├────────────────────────┤
│  Habilidade 1          │  trait 1 — 32
│  Habilidade 2          │  trait 2 — 32
├────────────────────────┤
│  Linha de estilo       │  style 24
│  (Defensiva / Cautelosa)│
└────────────────────────┘  320
```

Sem ações, sem XP — é leitura pura. Usada em telas tipo "Doutrina militar" / "Catálogo".

---

## 6. Anatomia da carta Detalhada (720×1024) — pós-MVP

Layout em 3 colunas:

| Coluna | Largura | Conteúdo |
|---|---|---|
| Esquerda (anotações) | 144 px | Legendas 1-9 (cabeçalho, custo, ilustração, hard stats, traços, modificadores, geral, ações, customização) |
| Centro (carta) | 432 px | Header, portrait grande (432×320), stats grid (6+5), traços expandidos, modificadores ativos, info geral, ações rápidas |
| Direita (legenda) | 144 px | Legenda de ícones, estado da unidade |

Ações rápidas (footer central): `MOVER · ATACAR · DEFENDER · FORTIFICAR · DESCANSAR · DETALHES`. Cada ação = 64×64.

Variantes do mesmo tipo (ex.: `Guarda Imperial`, `Caçadores à Pé`, `Zouaves`) aparecem em strip horizontal na parte inferior da modal — usa a Compact (192×288).

---

## 7. Painel de Customização (320×800) — pós-MVP

Slide-out à direita quando o player clica "Personalizar" na detalhada.

Tabs verticais (esquerda do painel, 64px de largura):

| Tab | Edita | Asset Type |
|---|---|---|
| Armamento | Arma principal + munição | `UWeaponAsset` + `UAmmunitionAsset` |
| Equipamento | Mochila/utilities | `UEquipmentAsset` |
| Doutrina | Bônus de ataque/defesa | `UDoctrineAsset` |
| Treinamento | XP/moral inicial | `UTrainingAsset` |
| Aparência | Uniforme/cor | `UAppearanceAsset` |
| Papéis | Tags táticas | `TArray<FName>` |

Conteúdo do painel (direita, 256px): preview do item atual + grid de opções disponíveis (cada opção = 56×56 slot).

---

## 8. Vocabulário de stats

**Decisão travada.** 6 hard stats + 5 secundárias. Substitui o vocabulário pendente do PROJECT_STATE.

| Sigla | Stat | Range esperado | Onde aparece |
|---|---|---|---|
| ATQ | Ataque | 0–100 | Todas |
| DEF | Defesa | 0–100 | Todas |
| MOB | Mobilidade | 0–100 | Todas |
| MOR | Moral | 0–100 | Todas |
| ORG | Organização | 0–100 | Compact + Detalhada |
| SUP | Suprimento | 0–100 | Compact + Detalhada |
| ALC | Alcance | 0–10 (tiles) | Detalhada |
| PREC | Precisão | 0.0–1.0 (%) | Detalhada |
| SUPR | Supressão | 0–100 | Detalhada |
| REC | Reconhecimento | 0–100 | Detalhada |
| CST | Custo manutenção | inteiro (gold/mês) | Header de todas |

Ícones canônicos (a desenhar): espadas cruzadas (ATQ), escudo (DEF), bota (MOB), bandeira (MOR), pilares (ORG), caixote (SUP), alvo (ALC), mira (PREC), olho (REC), corrente (SUPR), moeda (CST).

---

## 9. Identidade nacional

Três camadas, sempre presentes:

1. **Bandeira** — `Nation.FlagTexture`, arte 256×192 (proporção 4:3 real), exibida cropada/centralizada em hitbox quadrada (32×32 no header da carta).
2. **Cor primária** — `Nation.Color` (já existe). Usada como tint na borda da carta, marcador no mapa, highlights de UI.
3. **Cor secundária** — `Nation.SecondaryColor` (**a adicionar**). Trim/detalhe, gradientes.
4. **Brasão** — `Nation.CoatOfArmsIcon`, 128×128, ícone circular. Usado no medalhão decorativo do header da Detalhada.

**Extensão proposta em `Source/StrategosCore/Public/World/Nation.h`:**

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor SecondaryColor;
UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<UTexture2D> FlagTexture;
UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<UTexture2D> CoatOfArmsIcon;
```

Save format: bumpar `FNationRecord` para incluir `SecondaryColor` + paths dos assets. SaveVersion 3 → 4.

---

## 10. Estados de unidade e modificadores

Estados (`EUnitState`) — exibidos como ícone no canto do portrait quando ≠ `Ready`:

| Estado | Ícone | Cor |
|---|---|---|
| Ready | (sem ícone) | — |
| InCombat | ✕ cruzado | vermelho |
| Damaged | ✕ vermelho | vermelho escuro |
| Disorganized | ⚠ | laranja |
| Retreated | bandeira | cinza |

Modificadores ativos (`FArmyModifier`): aparecem como chips 88×56 na Detalhada. Verdes = buff, vermelhos = debuff. Mostram label curto + valor + turnos restantes.

---

## 11. Hitboxes (regra do Vic2)

**Hitbox é sempre retangular**, mesmo quando a arte aparenta forma orgânica.

| Elemento visual | Forma aparente | Hitbox real |
|---|---|---|
| Bandeira no header | Arredondada | 32×32 |
| Brasão no medalhão | Circular | 48×48 ou 64×64 |
| Ícone de stat | Símbolo orgânico | 24×24 |
| Slot de equipamento | Pedra/moldura | 56×56 |
| Botão de ação | Pergaminho | 64×64 |
| Carta compact | Bordada | 192×288 inteira é clicável |

Em UMG: `Image` widget para arte + `Button` invisível por cima ou `OnMouseButtonDown` no container.

---

## 12. Impacto no código (resumo)

Para chegar do estado atual (`UArmy.ManpowerCount` apenas) à compact da HUD:

```cpp
// World/Army.h — expansão necessária para Compact
USTRUCT(BlueprintType) struct FArmyStats {
    int32 ATQ=0, DEF=0, MOB=0, MOR=0, ORG=0, SUP=0;
    int32 ALC=0, SUPR=0, REC=0, CST=0;
    float PREC=0.f;
};

USTRUCT(BlueprintType) struct FArmyModifier {
    FName Id; FText Label; float Value; int32 TurnsRemaining; bool bPositive;
};

UENUM(BlueprintType) enum class EUnitState : uint8 {
    Ready, InCombat, Damaged, Disorganized, Retreated
};

// UArmy ganha:
FArmyStats BaseStats;
TArray<FArmyModifier> ActiveModifiers;
EUnitState State = EUnitState::Ready;
int32 ExperienceXP = 0;
int32 ExperienceLevel = 0;
TSoftObjectPtr<class UUnitTypeAsset> UnitType;
```

`UUnitTypeAsset` é DataAsset novo (padrão idêntico a `UEventAsset`): `Name`, `Description`, `Portrait`, `BaseStats`, `PrimaryTrait`, `AvailableVariants`.

`FArmyLoadout` (Armament/Equipment/Doctrine/Training/Appearance) só entra junto com o **painel de customização** (pós-MVP). Não pré-implementar agora.

Save format: bumpar `FArmyRecord` para incluir os novos campos. SaveVersion 3 → 4 (combinar com bump do Nation).

---

## 13. Roadmap de implementação

Em ordem:

1. **Nation extension** — adicionar `SecondaryColor`, `FlagTexture`, `CoatOfArmsIcon`. Bump save.
2. **Army stats expansion** — `FArmyStats`, `EUnitState`, `FArmyModifier`, `ExperienceXP/Level`, `UnitType`. Bump save.
3. **UUnitTypeAsset** — DataAsset + content registry padrão.
4. **WBP_ArmyCardCompact** — widget UMG 192×288 consumindo `UArmy*`. Slot no `WBP_HUD` como lista lateral.
5. **WBP_ArmyCardMicro** — variante 144×320 para painéis comparativos.

Tudo acima entrega o MVP visual. Detalhada / customização ficam para depois do BattleResolver — sem combate, não há o que customizar de armamento.

---

## 14. O que **não** está no MVP (registro explícito)

- ❌ Rarity / Quality tiers (descartado por decisão).
- ❌ Stance system (Carga/Reconhecer/Escaramuça) — entra com Battle.
- ❌ Evolution timeline da unidade (Cavalaria de Linha → Lança → Carabina → Mecanizada) — é o sistema de Progress (Etapa 3).
- ❌ Stats secundários funcionais (ALC, PREC, SUPR, REC) — só cosméticos zerados.
- ❌ Painel de customização (Armamento/Equipamento/Doutrina/Treinamento/Aparência/Papéis).
- ❌ Ações rápidas Atacar/Defender/Fortificar/Descansar — entram com Battle.
- ❌ Comandante customizável por exército — entra com o sistema de líderes expandido.
- ❌ Animação de portraits / cartas hover effects elaborados.

---

## 15. Referências visuais (mockups)

Quatro mockups foram usados como base para este contrato:

1. **Detalhada anotada** (1024×1280): mostra as 9 zonas funcionais da carta detalhada + painel de customização lateral. Base para a seção 6 e 7.
2. **Detalhada estilizada** (com Grandeiros Imperiais): variante visual mais ornamentada; serve de inspiração de arte, não de layout.
3. **Cavalaria 3-em-1**: compact comparativa mostrando 3 nações + sistema de stance + evolução. Stance e evolução estão fora do MVP.
4. **Infantaria 8 nações**: micro comparativa. Base direta para a carta Micro da seção 5.

Conforme a arte for sendo produzida, atualizar este doc para apontar para os assets finais (`Content/UI/Cards/...`).
