# 11 — Sistema de Unidades Militares

Sistema de unidades baseado em **composição em cinco eixos ortogonais**, em vez de hierarquia de classes. Cada unidade jogável é a resolução em runtime de: Classe (função tática) + Equipamento (tecnologia) + Doutrina (escolha estratégica) + Experiência (estado evolutivo) + Modificadores (efeitos ativos).

---

## 1. Princípio Arquitetural: Composição em vez de Hierarquia

O instinto inicial é fazer:

```
URegiment → URegimentInfantry → URegimentLineInfantry → URegimentRiflemen1860 → ...
```

**Não faça isso.** Em poucos meses você terá 200 classes e qualquer mudança de regra propaga em cascata.

A solução é **composição via DataAssets**. Uma unidade é a **soma de cinco eixos ortogonais**:

```
Unit = UnitClass (função tática)
     + EquipmentLoadout (tecnologia)
     + Doctrine (doutrina nacional/comandante)
     + Experience (estado evolutivo)
     + Modifiers (efeitos ativos: moral, suprimento, terreno)
```

Cada eixo é um DataAsset separado. A "unidade jogável" é a **resolução em runtime** desses cinco DataAssets em um `FRegimentRuntimeProfile`.

> ⚠️ Por que isso importa: quando aparecer "Rifle de Ferrolho" como nova tecnologia, você adiciona **um EquipmentAsset** e ele se aplica automaticamente a toda classe compatível. Sem refatorar nada.

---

## 2. Os Cinco Eixos da Unidade

### Eixo 1 — `UUnitClassAsset` (Função Tática)

Define **o que a unidade faz**, não com o quê.

```cpp
UCLASS(BlueprintType)
class UUnitClassAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly) FName ClassId;            // "Infantry.Line", "Cavalry.Heavy"
    UPROPERTY(EditDefaultsOnly) EUnitDomain Domain;       // Land, Naval, Air
    UPROPERTY(EditDefaultsOnly) EUnitRole Role;           // LineHolder, Skirmisher, Shock, Support, Recon, Siege, Logistics
    UPROPERTY(EditDefaultsOnly) FUnitBaseStats BaseStats;

    // Slots de equipamento que esta classe aceita
    UPROPERTY(EditDefaultsOnly) TArray<EEquipmentSlot> AvailableSlots;
                                // Primary, Sidearm, Mount, Armor, Support, Mobility

    // Slots de carta que esta classe pode preencher (independente da tecnologia)
    UPROPERTY(EditDefaultsOnly) int32 CardSlotCount;      // ex: 3
    UPROPERTY(EditDefaultsOnly) TArray<ECardCategory> AcceptedCategories;

    // Cartas inerentes — sempre disponíveis pela função, mesmo sem tech
    UPROPERTY(EditDefaultsOnly) TArray<UBattleCardAsset*> InherentCards;

    UPROPERTY(EditDefaultsOnly) FUnitMatchupTable Matchups; // bônus/penalidade vs outras Roles
};
```

**Exemplos de classes**:

| ClassId | Domain | Role |
|---|---|---|
| `Infantry.Irregular` | Land | Skirmisher |
| `Infantry.Line` | Land | LineHolder |
| `Infantry.Light` | Land | Skirmisher |
| `Infantry.Shock` | Land | Shock |
| `Infantry.Guard` | Land | LineHolder (elite) |
| `Cavalry.Light` | Land | Recon |
| `Cavalry.Heavy` | Land | Shock |
| `Cavalry.Dragoon` | Land | Hybrid |
| `Artillery.Field` | Land | Support |
| `Artillery.Heavy` | Land | Siege |
| `Artillery.Mortar` | Land | Siege |
| `Engineer.Sapper` | Land | Support |
| `Machinegun.Crew` | Land | Suppression |
| `Armor.Tankette` | Land | Shock |
| `Logistics.Train` | Land | Logistics |
| `Naval.ShipOfTheLine` | Naval | LineHolder |
| `Naval.Ironclad` | Naval | LineHolder |
| `Naval.Cruiser` | Naval | Recon |
| `Naval.Destroyer` | Naval | Skirmisher |
| `Naval.Submarine` | Naval | Ambush |
| `Air.Balloon` | Air | Recon |
| `Air.Airship` | Air | Recon |
| `Air.Plane.Scout` | Air | Recon |

> Note: **Role é o que importa para o sistema de cartas e matchup**, não Domain. "Skirmisher" se comporta de forma similar tendo cavalo, fuzil ou submarino — o que muda é a **tecnologia**.

---

### Eixo 2 — `UEquipmentAsset` (Tecnologia)

Define **com o quê** a unidade luta. É o eixo da evolução vitoriana.

```cpp
UCLASS(BlueprintType)
class UEquipmentAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly) FName EquipmentId;        // "Rifle.BoltAction.M1888"
    UPROPERTY(EditDefaultsOnly) EEquipmentSlot Slot;
    UPROPERTY(EditDefaultsOnly) FName Family;             // "Rifle", "Cannon", "Engine", "Armor"
    UPROPERTY(EditDefaultsOnly) int32 TechEra;            // 0..N progressão temporal

    // Pré-requisito tecnológico (na tech tree)
    UPROPERTY(EditDefaultsOnly) FName RequiredTechNode;

    // Modificadores aplicados aos stats da unidade
    UPROPERTY(EditDefaultsOnly) FStatModifierSet StatModifiers;

    // Cartas que esta peça desbloqueia
    UPROPERTY(EditDefaultsOnly) TArray<UBattleCardAsset*> UnlockedCards;

    // Cartas que esta peça remove (ex: "Carga de Sabre" some quando vira pistola)
    UPROPERTY(EditDefaultsOnly) TArray<UBattleCardAsset*> RemovedCards;

    // Compatibilidade — em quais classes pode ser equipado
    UPROPERTY(EditDefaultsOnly) TArray<EUnitRole> CompatibleRoles;

    // Custo industrial (para economia)
    UPROPERTY(EditDefaultsOnly) FIndustrialCost Cost;
};
```

**Famílias de equipamento e progressão**:

| Família | Progressão |
|---|---|
| Firearm | Mosquete → Rifle de Avancarga → Rifle de Retrocarga → Rifle de Ferrolho → Rifle de Repetição → Carregador |
| Sidearm | Sabre → Pistola → Revólver |
| Cannon | Liso/Avancarga → Raiado/Avancarga → Raiado/Retrocarga → Recuo Hidráulico |
| Propulsion | Vela → Vapor (Roda) → Vapor (Hélice) → Turbina |
| Hull | Madeira → Compósito → Ferro → Aço/Casemate → Aço/Torres |
| Engine (terrestre) | — → Vapor → Combustão Interna |
| Armor | Nenhum → Couraça (cavalaria) → Blindagem (veicular) |
| Communication | Mensageiro → Heliógrafo → Telégrafo → Rádio |

> O **mesmo sistema** descreve "infantaria troca de mosquete para rifle" e "fragata troca de vela para vapor". Reuso máximo.

---

### Eixo 3 — `UDoctrineAsset` (Doutrina)

Doutrina é **modificador de comportamento**, não equipamento. Vive na **nação** e/ou no **comandante**.

```cpp
UCLASS(BlueprintType)
class UDoctrineAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly) FName DoctrineId;         // "Doctrine.Napoleonic.Line"
    UPROPERTY(EditDefaultsOnly) FText DisplayName;
    UPROPERTY(EditDefaultsOnly) EDoctrineScope Scope;     // National, Commander, Both

    // Aplica modificadores condicionais (só quando role X está engajada, etc)
    UPROPERTY(EditDefaultsOnly) TArray<FConditionalModifier> Modifiers;

    // Adiciona cartas específicas de doutrina ao deck
    UPROPERTY(EditDefaultsOnly) TArray<UBattleCardAsset*> GrantedCards;

    // Bloqueia cartas de outras doutrinas
    UPROPERTY(EditDefaultsOnly) TArray<UBattleCardAsset*> ForbiddenCards;

    // Mutuamente exclusivas
    UPROPERTY(EditDefaultsOnly) TArray<UDoctrineAsset*> Conflicts;
};
```

**Doutrinas vitorianas exemplares**:

| Doutrina | Efeito |
|---|---|
| Linha Napoleônica | Bônus em formação fechada, dano de volley aumentado, penalidade em terreno acidentado |
| Guerra de Movimento | Bônus de mobilidade estratégica, cartas de manobra mais baratas |
| Guerra de Trincheiras | Bônus defensivo absurdo, mas mobilidade reduzida; libera "Entrincheirar" |
| Fogo e Movimento | Sinergia infantaria+metralhadora; cartas de supressão acessíveis |
| Élan / Ofensiva à Outrance | Bônus ofensivo + penalidade defensiva; força engajamento |
| Doutrina Naval Mahaniana | Bônus em frota concentrada de couraçados |
| Jeune École | Bônus em destroyers e submarinos vs frota grande |

---

### Eixo 4 — `FRegimentExperience` (Estado Evolutivo)

Não é DataAsset — é **estado runtime persistido no save**.

```cpp
USTRUCT()
struct FRegimentExperience
{
    GENERATED_BODY()

    int32 BattlesFought;
    int32 BattlesWon;
    float ExperiencePoints;
    EExperienceTier Tier;          // Green, Trained, Veteran, Elite, Legendary

    // Traços conquistados em batalha
    TArray<FName> AcquiredTraits;  // "Trait.SteadyUnderFire", "Trait.MountainBorn"
};
```

Tier multiplica stats e libera **slots de carta extras** (veteranos têm mais opções táticas).

---

### Eixo 5 — `FActiveModifierStack` (Modificadores Vivos)

Estado momentâneo: moral, suprimento, terreno, clima, efeitos de cartas. Vive no `FRegimentBattleState` que já desenhamos no `BattleSubsystem`.

---

## 3. Resolução em Runtime — `FRegimentRuntimeProfile`

Quando uma batalha começa (ou para auto-resolve), o `RegimentResolver` compõe os cinco eixos:

```cpp
USTRUCT()
struct FRegimentRuntimeProfile
{
    GENERATED_BODY()

    FName ClassId;
    FUnitFinalStats FinalStats;              // resultado da composição
    TArray<UBattleCardAsset*> AvailableCards;// união filtrada das fontes
    FUnitMatchupTable EffectiveMatchups;
    EExperienceTier Tier;
};
```

### Pipeline do resolver

```cpp
FRegimentRuntimeProfile URegimentResolver::Resolve(const URegiment* Reg,
                                                   const UNation* Nation,
                                                   const UCommander* Cmdr)
{
    FRegimentRuntimeProfile Profile;

    // 1. Base da classe
    Profile.FinalStats = Reg->Class->BaseStats;
    Profile.AvailableCards.Append(Reg->Class->InherentCards);

    // 2. Equipamentos (somam, alguns substituem cartas)
    for (UEquipmentAsset* Eq : Reg->EquippedItems)
    {
        Profile.FinalStats.Apply(Eq->StatModifiers);
        Profile.AvailableCards.Append(Eq->UnlockedCards);
        for (auto* Removed : Eq->RemovedCards)
            Profile.AvailableCards.Remove(Removed);
    }

    // 3. Doutrinas (nacional + comandante)
    ApplyDoctrine(Profile, Nation->NationalDoctrine);
    if (Cmdr) ApplyDoctrine(Profile, Cmdr->PersonalDoctrine);

    // 4. Experiência
    Profile.Tier = Reg->Experience.Tier;
    Profile.FinalStats.Multiply(GetTierMultiplier(Profile.Tier));
    if (Profile.Tier >= EExperienceTier::Veteran)
        Profile.AvailableCards.Append(Reg->Experience.UnlockedTraitCards());

    // 5. Deduplicação e validação final
    Profile.AvailableCards = DedupAndFilter(Profile.AvailableCards);

    return Profile;
}
```

> **Cache este resultado**. Recalcule só quando equipamento, doutrina ou tier mudar. Em batalha, o profile é congelado no início.

---

## 4. Integração com a Tech Tree

Já planejamos `UTechTreeAsset` na arquitetura geral. Agora ele fica concreto:

```cpp
USTRUCT()
struct FTechNode
{
    FName NodeId;                 // "Tech.Firearms.BoltAction"
    FName Category;               // "Firearms", "Naval", "Industry", "Doctrine"
    int32 Era;
    TArray<FName> Prerequisites;
    int32 ResearchCost;

    // Efeitos ao desbloquear
    TArray<UEquipmentAsset*> UnlocksEquipment;
    TArray<UDoctrineAsset*> UnlocksDoctrine;
    TArray<UUnitClassAsset*> UnlocksClasses; // ex: "Tank" desbloqueia classe inteira
    FStatModifierSet PassiveBonuses;
};
```

Quando `UProgressSubsystem` completa um nó, emite `OnTechResearched(NodeId)`. O `MilitarySubsystem` ouve e:
- Marca novos equipamentos como produzíveis
- Permite que regimentos existentes sejam **re-equipados** (custo industrial + tempo)
- Notifica IA para considerar atualização

> ⚠️ **Não troque equipamento automaticamente**. Trocar é decisão do jogador (custo, tempo, prioridade). É exatamente isso que cria a tensão "minha cavalaria está obsoleta mas custa caro modernizar" — o coração de Victoria.

---

## 5. Conexão com o `BattleSubsystem`

Voltando ao desenho do BattleSubsystem, o `FRegimentBattleState` ganha uma referência ao profile resolvido:

```cpp
USTRUCT()
struct FRegimentBattleState
{
    FGuid RegimentId;
    FRegimentRuntimeProfile Profile;       // resolvido no início da batalha
    int32 CurrentStrength;
    float Morale;
    float OrganizationLeft;
    EBattleStance Stance;
};
```

E o **deck do lado** na batalha agora é construído como:

```
Deck = Commander.PersonalDeck
     + ⋃ (Regimento.Profile.AvailableCards)  para cada regimento presente
     + Nation.DoctrineCards
     - Forbidden por doutrina/conflito
```

> **Consequência de design importante**: o deck deixa de ser estático. Cada batalha tem deck diferente conforme **a composição da força engajada**. Isso é o que torna decisão de composição no mapa estratégica significativa.

---

## 6. Modificadores de Stats — Sistema Unificado

Para evitar bagunça, todos os modificadores passam por uma estrutura única:

```cpp
USTRUCT()
struct FStatModifier
{
    EStatTarget Target;       // Attack, Defense, Morale, Mobility, Range, Reload, Supply
    EStatOp Op;               // Add, Multiply, Override
    float Value;
    EModifierStacking Stacking; // Stacks, Overrides, MaxOnly
    FName Source;             // para debug e UI
    FConditionalContext Conditions; // só ativo se terreno=Forest, etc
};

USTRUCT()
struct FStatModifierSet
{
    TArray<FStatModifier> Modifiers;
};
```

Aplicação:

```cpp
void FUnitFinalStats::Apply(const FStatModifierSet& Set,
                            const FBattleContext* Ctx = nullptr)
{
    for (const FStatModifier& M : Set.Modifiers)
    {
        if (Ctx && !M.Conditions.IsSatisfied(*Ctx)) continue;
        ApplyOne(M);
    }
}
```

Ordem fixa: **Add → Multiply → Override**, em três passes. Garante determinismo.

---

## 7. Cartas Específicas do Período Vitoriano

Para concretizar, deixo aqui o catálogo inicial mapeado ao sistema:

### Cartas inerentes por Role

| Role | Cartas inerentes |
|---|---|
| LineHolder | Hold the Line, Volley Fire |
| Skirmisher | Loose Formation, Withdraw |
| Shock | Charge, Press the Attack |
| Support | Concentrate Fire |
| Recon | Scout Ahead, Report Position |
| Siege | Bombardment |
| Logistics | Resupply, Forage |

### Cartas desbloqueadas por equipamento

| Equipment | Carta(s) |
|---|---|
| Rifle de Ferrolho | Aimed Fire (alta precisão, lento) |
| Rifle de Repetição | Rapid Volley (dano sustentado) |
| Metralhadora | Suppression (reduz moral, trava avanço) |
| Canhão Raiado | Counter-Battery |
| Canhão Retrocarga | Sustained Bombardment |
| Couraça naval | Ramming Speed (curto período histórico!) |
| Torpedo | Torpedo Run |
| Vapor (naval) | Steam Maneuver (ignora vento) |
| Telégrafo | Coordinate Strike (sinergia entre regimentos) |
| Trincheira (engenharia) | Entrench |

### Cartas por Doutrina

| Doutrina | Carta exclusiva |
|---|---|
| Napoleônica | Grand Battery, Squares vs Cavalry |
| Trincheira | Defense in Depth |
| Movimento | Forced March |
| Élan | Bayonet Charge |
| Mahaniana | Crossing the T |
| Jeune École | Wolfpack |

---

## 8. Mapeamento Completo Unreal

| Conceito | Tipo Unreal | Lifetime |
|---|---|---|
| `UUnitClassAsset` | PrimaryDataAsset | estático |
| `UEquipmentAsset` | PrimaryDataAsset | estático |
| `UDoctrineAsset` | PrimaryDataAsset | estático |
| `UBattleCardAsset` | PrimaryDataAsset | estático |
| `UTechTreeAsset` | PrimaryDataAsset | estático |
| `URegiment` (estado estratégico) | UObject puro, dentro de `UWorldState` | save |
| `FRegimentExperience` | USTRUCT | save |
| `FRegimentRuntimeProfile` | USTRUCT | runtime, cacheado |
| `FRegimentBattleState` | USTRUCT | só durante batalha |
| `URegimentResolver` | UWorldSubsystem | por mapa |
| `UMilitarySubsystem` | UWorldSubsystem | por mapa |

---

## 9. Diagrama de Composição

```
URegiment (estado estratégico, persiste no save)
   │
   ├── ClassRef ─────────► UUnitClassAsset      [função tática]
   ├── EquippedItems[] ──► UEquipmentAsset[]    [tecnologia]
   ├── Experience ───────► FRegimentExperience  [estado evolutivo]
   ├── Modifiers ────────► FStatModifierSet     [efeitos persistentes]
   └── (no engajamento)
          │
          ▼
   URegimentResolver.Resolve(Reg, Nation, Commander)
          │   ┌─► aplica Class.BaseStats
          │   ├─► aplica Equipment.StatModifiers
          │   ├─► aplica Nation.Doctrine
          │   ├─► aplica Commander.Doctrine
          │   ├─► aplica Tier multiplier
          │   └─► coleta AvailableCards (união filtrada)
          ▼
   FRegimentRuntimeProfile
          │
          ▼
   FRegimentBattleState (consumido pelo BattleSubsystem)
          │
          ▼
   Deck final do lado = ⋃ profiles + Commander deck + Doctrine cards
```

---

## 10. Pontos de Atenção Específicos

- **Slot de carta ≠ deck**. Cada regimento *contribui* cartas para o deck do lado. Não há "deck por regimento" no jogo. Isso evitaria caos de UI e mantém a fantasia "comandante orquestra a força".
- **Limite a quantidade de equipamentos simultâneos por classe**. Infantaria: Primary + Sidearm + Support. Cavalaria: Primary + Mount + Sidearm. Sem isso, o jogador empilha tudo e o sistema vira soup.
- **Re-equipar custa tempo e dinheiro**. Crítico para ter sensação de "minha doutrina está atrasada".
- **Obsolescência precisa ter custo emocional**. Cavalaria pesada da era napoleônica vs metralhadora — a unidade não some, ela vira armadilha caso o jogador não modernize. Vitoriano = transição dolorosa.
- **Não exponha 50 stats**. Internamente pode ter; UI mostra 4–5 (Ataque, Defesa, Disciplina, Mobilidade, Alcance). Resto vira "perfil" (ex: "Eficaz contra cavalaria").
- **Performance**: composição é cara se feita a cada tick. Resolva uma vez por mudança real (equip, doutrina, exp tier) e cache.

---

## 11. Plano de Implementação do Sistema de Unidades

1. **Eixo 1 — UnitClass**: criar 5 classes (Line, Light, Cavalry.Heavy, Artillery.Field, Naval.ShipOfTheLine). Sem equipamento ainda.
2. **Composição básica**: `URegimentResolver` aplica só BaseStats. Plugar no `BattleSubsystem`.
3. **Eixo 2 — Equipment**: introduzir 1 família (Firearm: Mosquete, Rifle, Bolt-Action). Cards desbloqueados por equipamento.
4. **TechTree mínima**: 3 nós encadeados que liberam os 3 firearms.
5. **Eixo 3 — Doctrine**: 2 doutrinas opostas (Napoleônica vs Trincheira) para validar conflitos e cartas exclusivas.
6. **Eixo 4 — Experience**: tier system + bônus por tier. Trait acquisition vem depois.
7. **Expansão por domínio**: Naval (Hull + Propulsion), depois Air (Balloon, Airship).
8. **Mecanizadas**: Trens, blindados, tanques — chegam por tech tree no final do jogo.
9. **Polish**: UI de roster, comparador de regimentos, painel de doutrina.

---

## 12. Resumo do Sistema de Unidades

```
Sistema de Unidades
├── DataAssets (estáticos)
│   ├── UUnitClassAsset      [função: Line, Skirmisher, Shock, Recon, Support, Siege, Logistics]
│   ├── UEquipmentAsset      [Firearm, Cannon, Hull, Propulsion, Engine, Armor, Communication]
│   ├── UDoctrineAsset       [Napoleônica, Trincheira, Movimento, Élan, Mahaniana, Jeune École]
│   └── UBattleCardAsset     [inerentes + desbloqueadas + de doutrina]
│
├── Estado persistente (no save)
│   ├── URegiment
│   │   ├── ClassRef
│   │   ├── EquippedItems[]
│   │   ├── FRegimentExperience
│   │   └── ActiveModifiers
│   └── UNation.NationalDoctrine
│
├── Resolução runtime (cacheada)
│   └── URegimentResolver → FRegimentRuntimeProfile
│       ├── FinalStats (Add → Multiply → Override)
│       ├── AvailableCards (união filtrada)
│       └── EffectiveMatchups
│
├── Integração tech
│   └── UProgressSubsystem.OnTechResearched
│       └── desbloqueia Equipment / Doctrine / Class
│
└── Integração combate
    └── BattleSubsystem coleta profiles → monta deck do lado → roda fases
```
