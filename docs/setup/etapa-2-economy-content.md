# Setup do Editor — Etapa 2 v1 (Economia)

Lista de DataAssets que precisam ser criados no editor para ativar a economia. O sistema funciona sem isso (sandbox programático cobre o caso de "nenhum DataAsset"), mas para customizar bens, prédios e métodos de produção, é por aqui.

---

## 0. Pré-requisitos

- Etapa 1 já configurada (ver `etapa-1-editor-setup.md`)
- Build do C++ atualizado para incluir os novos UClass/USTRUCT da Economy
- (Opcional) Pasta `Content/Economy/` criada

---

## 1. Estrutura de pastas recomendada

```
Content/Economy/
  Goods/         ← UGoodAsset (12 itens)
  Methods/       ← UProductionMethodAsset (~19 itens)
  Modifiers/     ← UProductionModifierAsset (7 itens)
  Buildings/     ← UBuildingTypeAsset (8 itens)
  DA_EconomyRegistry  ← UEconomyContentRegistry agregando tudo
```

---

## 2. UGoodAsset — 12 bens

Para cada um: *Add → Miscellaneous → Data Asset → GoodAsset*. Use prefixo `G_`.

| Asset | Id | Tier | BasePrice | Category |
|---|---|---|---|---|
| `G_Grain` | `Grain` | 0 | 1.0 | Raw |
| `G_IronOre` | `IronOre` | 0 | 1.5 | Raw |
| `G_Coal` | `Coal` | 0 | 1.2 | Raw |
| `G_Wood` | `Wood` | 0 | 1.0 | Raw |
| `G_Cotton` | `Cotton` | 0 | 1.3 | Raw |
| `G_Iron` | `Iron` | 1 | 4.0 | Industrial |
| `G_Lumber` | `Lumber` | 1 | 2.5 | Industrial |
| `G_Cloth` | `Cloth` | 1 | 3.5 | Industrial |
| `G_Flour` | `Flour` | 1 | 2.0 | Staple |
| `G_Bread` | `Bread` | 2 | 3.0 | Staple |
| `G_Garments` | `Garments` | 2 | 6.0 | Luxury |
| `G_Tools` | `Tools` | 2 | 8.0 | Industrial |

`Icon` é opcional na v1 — adicione conforme arte fica disponível.

> Os ids precisam **bater exatamente** com os strings hardcoded no código (ex.: `Bread`, `Coal`). Mudar o id quebra o consumption basket e os strategic indices.

---

## 3. UProductionMethodAsset — ~19 PMs

Padrão de nomenclatura: `PM_<Building>_<Variant>`. Tier conforme o output mais alto.

### Tier 0 (Raw extraction)

| Asset | Inputs | Outputs/slot | Employment/slot | RawResource | Tier |
|---|---|---|---|---|---|
| `PM_Grain_Subsistence` | — | Grain × 5 | 5 Laborer @ 0.5 | bRequiresRawResource=true, Grain | 0 |
| `PM_Cotton_Plantation` | — | Cotton × 4 | 8 Laborer @ 0.5 | true, Cotton | 0 |
| `PM_IronMine_Manual` | — | IronOre × 3 | 6 Laborer @ 0.6 | true, IronOre | 0 |
| `PM_IronMine_Mechanized` | Coal × 2 | IronOre × 8 | 2 Engineer + 4 FactoryWorker | true, IronOre; tech: SteamPower | 0 |
| `PM_CoalMine_Manual` | — | Coal × 3 | 6 Laborer @ 0.5 | true, Coal | 0 |
| `PM_CoalMine_Mechanized` | — | Coal × 8 | 4 FactoryWorker | true, Coal; tech: SteamPower | 0 |
| `PM_Forester_Pure` | — | Wood × 4 | 4 Laborer | true, Wood | 0 |

### Tier 1 (Processed)

| Asset | Inputs | Outputs/slot | Employment/slot |
|---|---|---|---|
| `PM_Sawmill_Lumber` | Wood × 3 | Lumber × 4 | 3 Artisan |
| `PM_Mill_Flour` | Grain × 3 | Flour × 4 | 2 Artisan + 2 Laborer |
| `PM_Smelter_Bloomery` | IronOre × 2, Coal × 1 | Iron × 2 | 4 Artisan |
| `PM_Smelter_BlastFurnace` | IronOre × 3, Coal × 2 | Iron × 5 | 3 Engineer + 5 FactoryWorker (tech: Metallurgy) |
| `PM_Textile_HandLoom` | Cotton × 2 | Cloth × 2 | 4 Artisan |
| `PM_Textile_PowerLoom` | Cotton × 3, Coal × 1 | Cloth × 5 | 6 FactoryWorker (tech: SteamPower) |

### Tier 2 (Consumer / Industrial Light)

| Asset | Inputs | Outputs/slot | Employment/slot |
|---|---|---|---|
| `PM_Bakery_FullBake` | Flour × 2 | Bread × 4 | 3 Artisan |
| `PM_Textile_GarmentMill` | Cloth × 3 | Garments × 4 | 4 Artisan |
| `PM_Textile_ModernMill` | Cloth × 4, Coal × 1 | Garments × 8 | 2 Engineer + 6 FactoryWorker (tech: Industrial) |
| `PM_Workshop_Artisanal` | Iron × 1, Lumber × 1 | Tools × 1 | 5 Artisan |
| `PM_Workshop_Standard` | Iron × 2, Lumber × 1 | Tools × 2 | 3 Artisan + 5 Laborer (tech: Workshops) |
| `PM_Workshop_Mechanized` | Iron × 4, Coal × 1 | Tools × 4 | 2 Engineer + 8 FactoryWorker (tech: SteamPower) |

`MaintenancePerSlot` sugerido: 1.0 para PMs Tier 0/1, 2.0 para Tier 2 mecanizadas.

---

## 4. UProductionModifierAsset — 7 modifiers

Os IDs são consultados por nome pelo AIPlaceholderSubsystem (`PushForOutput`, `ModernizationDrive`). Mantenha esses dois com esses Ids exatos.

| Asset | Id | MutexGroup | Throughput | Input | Wage | Maint | LoyaltyΔ | Tech |
|---|---|---|---|---|---|---|---|---|
| `M_PushForOutput` | `PushForOutput` | `Pace` | ×1.25 | ×1.50 | ×1.20 | ×1.00 | -0.02 | — |
| `M_Conservative` | `Conservative` | `Pace` | ×0.85 | ×0.70 | ×0.90 | ×1.00 | +0.01 | — |
| `M_SkilledLabor` | `SkilledLabor` | `Labor` | ×1.20 | ×1.00 | ×1.40 | ×1.00 | +0.01 | — |
| `M_CheapLabor` | `CheapLabor` | `Labor` | ×0.90 | ×1.00 | ×0.70 | ×1.00 | -0.03 | — |
| `M_Premium` | `Premium` | `Quality` | ×1.15 | ×1.25 | ×1.00 | ×1.00 | 0 | — |
| `M_Volume` | `Volume` | `Quality` | ×0.95 | ×0.85 | ×1.00 | ×1.00 | 0 | — |
| `M_ModernizationDrive` | `ModernizationDrive` | (vazio) | ×1.20 | ×1.00 | ×1.20 | ×1.30 | 0 | SteamPower |

---

## 5. UBuildingTypeAsset — 8 prédios

Padrão: `BT_<Name>`. ConstructionDays sugerido 30-60 conforme tier do output.

| Asset | Category | RequiresRaw | Resource | Methods | DefaultMethod | ConstructionDays | MonetaryCost | ConstructionGoods |
|---|---|---|---|---|---|---|---|---|
| `BT_Farm` | Farm | true | Grain | `PM_Grain_Subsistence`, `PM_Cotton_Plantation` | Grain | 30 | 50 | Lumber × 5 |
| `BT_IronMine` | Mine | true | IronOre | `PM_IronMine_Manual`, `PM_IronMine_Mechanized` | Manual | 45 | 100 | Lumber × 10 |
| `BT_CoalMine` | Mine | true | Coal | `PM_CoalMine_Manual`, `PM_CoalMine_Mechanized` | Manual | 45 | 100 | Lumber × 10 |
| `BT_Sawmill` | Forester | true | Wood | `PM_Forester_Pure`, `PM_Sawmill_Lumber` | Sawmill | 30 | 80 | Iron × 3 |
| `BT_Smelter` | Refinery | false | — | `PM_Smelter_Bloomery`, `PM_Smelter_BlastFurnace` | Bloomery | 60 | 200 | Iron × 5, Lumber × 10 |
| `BT_Bakery` | Workshop | false | — | `PM_Mill_Flour`, `PM_Bakery_FullBake` | FullBake | 30 | 60 | Lumber × 5 |
| `PM_TextileMill` é PM, não BT — corrigindo: | | | | | | | | |
| `BT_TextileMill` | Factory | false | — | `PM_Textile_HandLoom`, `PM_Textile_PowerLoom`, `PM_Textile_GarmentMill`, `PM_Textile_ModernMill` | HandLoom | 60 | 250 | Iron × 10, Lumber × 5 |
| `BT_ToolWorkshop` | Workshop | false | — | `PM_Workshop_Artisanal`, `PM_Workshop_Standard`, `PM_Workshop_Mechanized` | Artisanal | 45 | 150 | Iron × 5 |

`MaxLevel` sugerido: 5.

---

## 6. UEconomyContentRegistry

`Add → Miscellaneous → Data Asset → EconomyContentRegistry`. Nomeie `DA_EconomyRegistry`.

Adicione **todos** os assets criados acima nas 4 arrays:
- `Goods` ← arrasta os 12 `G_*`
- `ProductionMethods` ← arrasta os ~19 `PM_*`
- `ProductionModifiers` ← arrasta os 7 `M_*`
- `BuildingTypes` ← arrasta os 8 `BT_*`

---

## 7. Conectar ao GameMode

No `BP_StrategosGameMode`, em `BeginPlay` (override do BP):

1. Pega o `UEconomySubsystem` via `Get World Subsystem`
2. Chama `Set Content Registry` passando `DA_EconomyRegistry`

Ou faça em C++ (próxima rev): adicionar uma `UPROPERTY(EditDefaultsOnly) TSoftObjectPtr<UEconomyContentRegistry> EconomyContent;` ao `AStrategosGameMode` e chamar no `RunBootstrap`. Por agora, BP é mais flexível.

---

## 8. (Opcional) Smoke test

Pressione **Play**:

1. Logs em `Output Log` devem mostrar:
   ```
   EconomySubsystem content: 12 goods, 19 methods, 7 modifiers, 8 building types
   ```
2. Avance o tempo. A cada mês, no log:
   ```
   Bourgeoisie of AlbionCenter invested in BT_Farm (score=12.3, wealth left=750)
   ```
   (Ou similar — depende dos preços dinâmicos e wealth seeded.)
3. Após 6-12 meses simulados, abra a HUD: o `GetTreasuryBalance()` deve estar mudando, `GetTopShortfalls()` deve listar bens em falta, `GetPlayerBuildings()` deve mostrar o que existe.

Se nenhum prédio aparece e a Bourgeoisie nunca investe: provavelmente o `DA_EconomyRegistry` não foi setado. Confirme via log de Initialize do EconomySubsystem.

---

## 9. Tuning rápido

Onde mexer quando algo está estranho:

| Sintoma | Onde olhar |
|---|---|
| Treasury sempre zerada / dívida cresce sem parar | `GetBaseTaxPerPopPerMonth` em `EconomySubsystem.cpp`; tax level default no bootstrap |
| Bourgeoisie nunca investe | `MinWealth` (default 200) em `RunBourgeoisieAutoInvestment` |
| Nenhum bem produzido | Confirme que cada PM tem `OutputsPerSlot` populado e `EmploymentPerSlot` com Headcount > 0 |
| Loyalty derretendo | `MonthlyLoyaltyDelta` em modifiers ativos; cesta de consumo (`GetConsumptionBasket`) |
| Strategic indices sempre 1.0 | Bens-chave (Tools, Iron, Bread, Garments, Coal) com Demand=0; precisa ter consumidores ativos primeiro |
| Auto-invest spam de prédios iguais | Limite "1 por mês por província" já implementado; se crescer demais, adicionar cooldown extra |

---

## 10. Conteúdo modular para o futuro

A estrutura está pronta para expansão sem refactor:

- Adicionar bens novos: criar UGoodAsset → adicionar no DA_EconomyRegistry → criar PMs que produzem/consomem
- Adicionar prédios: criar UBuildingTypeAsset com PMs disponíveis → registrar
- Adicionar arquétipos econômicos: implementar `Behavior_<Archetype>` em `AIPlaceholderSubsystem.cpp`
- Mods: substituir DA_EconomyRegistry por outro asset apontando para conteúdo customizado

Mudanças em **valores numéricos** (preços, employment, wage) não exigem rebuild C++ — são edição de DataAsset, refletida em runtime.
