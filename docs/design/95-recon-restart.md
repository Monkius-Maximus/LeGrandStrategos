# 95 — Recon: Sprint 01 state and terminology

**Status:** REPORT — recon only, nothing changed
**Date:** 2026-10-15
**Prompt:** `PROMPT-RECON-RESTART.md` (2026-10-11)
**Ran from:** a claude.ai cloud session, against the GitHub repo `Monkius-Maximus/LeGrandStrategos` (public). Read-only: no build, no edit, no commit.
**Related:** `00-start-here.md`, `82-terminology-and-localization.md`, `93-sprint-backlog-01.md`, `52-seed-leis.md` §7

---

## Summary

1. **What runs:** unknown. The Godot repo (`Strategos.Sim` / `Strategos.Game`) is not on GitHub and this session is not linked to Diogo's computer, so Part A could not be checked. The GitHub repo is the **old Unreal 5.5 / C++ project** (latest branch `claude/event-schema-debug-kqxid8`, 2026-08-26).
2. **First blocker:** the Godot repo is not reachable. Push it to GitHub (private is fine) or run Part A in Claude Code on the machine that has it.
3. **Restart list total:** not estimated (needs Part A). One input is settled: a full S1-06 recon of Economy v1 **is feasible from the GitHub repo** — the whole tick is in one file and matches the 9-phase description.
4. **Retired-word hits:** 177 in the Unreal repo (102 without `Resource` inside identifiers). Zero hits for NAÇ/LEI/ECO/SOC/EXÉ/CLE, molde, galianos, "Chance to Pass", votação, 42-depositos.
5. **⚠ RECONCILIAR rows answered:** 16 of 16 items of Part C have an evidence-backed answer; 3 are "not defined anywhere", 7 exist only in a design doc or an unused header. **M1 = 0**: the Unreal Core has no civil/military split mechanic.

---

## Blockers

| Blocker | Effect | How to clear it |
|---|---|---|
| Godot repo not on GitHub | Part A (S1-01 … S1-09 status, build, tests, restart list) not run | Push the repo, or run Part A in Claude Code locally |
| Design docs `33`, `41`, `43`, `50`, `51`, `52`, `60`, `70` are not in the GitHub repo nor in the Project (`21` and `61` are in the Project) | Rows that wait for those docs stay open; Part C answered from the older repo docs instead | Add them to the Project or to the repo `docs/` |
| No .NET SDK in this session | `dotnet build` / `dotnet test` not run | Not needed until the Godot repo is reachable |

The repo docs that do exist are older equivalents: `40-resources-and-production.md` (old name of `33`), `20-economy.md` (economy and population), `21-progress.md` (technology, eras), `31-politics.md` (laws), `10-battle.md`, `11-units.md`, `41-military.md` (military, not population), `98-estado-implementacao.md` (on branch `claude/friendly-ride-GyJpH`), `99-implementation-roadmap.md`.

---

## Part A — Where Sprint 01 stopped

**Not run** (blocker above). What can be said from evidence:

| Item | Status | Evidence |
|---|---|---|
| S1-01 Vault hygiene | Partial, outside the repo | In the Project: `00` §3 records 1820–1880 → 1800–1950 and `42-depositos` → `43`. The Unreal repo still says 1820–1880 (`CLAUDE.md:9`). |
| S1-02 … S1-05, S1-07 … S1-09 | Unknown | Godot repo not reachable. Diogo stated on 2026-10-10: the repo exists, the Economy v1 port stalled, nothing runs (`00` §2). |
| S1-06 Economy v1 recon | Not started (`docs/94-recon-economy-v1.md` not found in the GitHub repo) | **The Unreal repo is reachable**, and a preview shows the recon is feasible (below). |

**Economy v1 preview (input for S1-06):**

| Question | Answer | Evidence |
|---|---|---|
| Where the tick lives | One file | `Source/StrategosCore/Private/Economy/EconomySubsystem.cpp` |
| Phase order | Daily construction countdown; monthly: PopGrowth → AssignEmployment → RunProduction (tier 0 → 3) → PopConsumption → PayWagesAndProfits → CollectTaxes → PayExpenses → SettleTreasury → ComputeStrategicIndices; then Bourgeoisie auto-investment | `EconomySubsystem.cpp:217-227`; matches `98-estado-implementacao.md:52` |
| Price formula | `BasePrice × clamp(1 + (D − S)/D, 0.5, 2)` | `GetDynamicPrice`, `EconomySubsystem.cpp:138-156` |
| Private Investment | `RunBourgeoisieAutoInvestment` | `EconomySubsystem.cpp:1163-1263` |
| Caveat | `20-economy.md` describes a weekly, regional model that was never built. For the port, trust the code. Several market and logistics headers are unused scaffolding. | Part C |

**The restart question (A.10)** cannot be answered without the Godot repo. See `93` §5 for the proposed restart sequence.

---

## Part B — Terminology inventory (Unreal repo)

Scope: 28 doc files and 197 code files (`ThirdParty/` excluded) from the latest branch, plus the files only present on `claude/friendly-ride-GyJpH` and `claude/new-session-T5bjg`. Headings below keep the numbering of the recon prompt.


**Summary**
1. Rows with evidence found: **138 / 158** (some word for the concept appears in docs or code; many in a different word than 82's name, see Note).
2. Rows not found: **20**. interlock, export, prospecting, discovered_undiscovered, unemployed, ig_landowners, ig_industrialists, ig_petty_bourgeoisie, ig_military, outside_bloc, effectiveness, autosave, gazette, mapmode_interest_groups, mapmode_steel_tier, mapmode_deposits, mapmode_population, panel_society, tooltip_lock, ui_template.
3. New player-visible concepts not in 82: **~120**, in 9 groups (§2). The biggest gaps are diplomacy, factions/ideology/elections, battle cards, unit stats and province geography.
4. New collisions not covered by 82 §4: **18** (§3, C1–C18).
5. Retired-word hits: **177** in total (102 if `Resource` inside identifiers is left out). Labels `NAÇ/LEI/ECO/SOC/EXÉ/CLE`, `molde`, `galianos`, "Chance to Pass", `votação` and `42-depositos` all have 0 hits (§4).

---

### Scope and method

- **Source:** `/home/claude/lgs-recon` (read-only). The **docs set** has 28 files: `CLAUDE.md`, `README.md`, `Source/StrategosWorldGen/README.md` and `docs/**/*.md`. The **code set** has 197 files: `Source/**/*.{h,cpp,cs}` and `Config/*.ini`, with `ThirdParty/` left out.
- **Search:** Python regexes. Docs are matched case-insensitively on whole words, with PT and EN variants. Code is matched case-sensitively on identifier substrings. Counts are matches, not lines.
- **How to read the cells:** "Docs say" gives `label ×count (top files)`. "Code says" gives the top identifiers, the total, and the first two `file:line` hits. A cell written by hand after I checked the context replaces the automatic count when the count was noise. I did this for: building_level, good_raw/intermediate/final, shortage, input, export, import, trade_capacity, deposit, strategic_good, discovered_undiscovered, profession, needs_met, interest_group and the IG rows, governing_bloc, capital_gain/cap, support_opposition, preferred_tier, manpower, design_tier, equipment_tier, doctrine, journal, alert, panel_society, tooltip_lock, ui_rail, ui_template, ui_skin and steel_tier.
- **What "not found" means:** both sets came back empty for the searches listed in the cell.
- **Retired words:** counted over **all** text files (docs and code), case-insensitive. The three-letter labels were counted case-sensitive in UPPER case, because lowercase `lei`/`eco` are ordinary words.

### Repo facts the recon asked for

- **Period the repo states: 1820–1880.**
  - `CLAUDE.md:9`: "ambientado na transição industrial (1820–1880)".
  - The same range appears in `docs/prompts/sessao-1-terreno-provincia.md:9`, `docs/architecture/40-resources-and-production.md:118` and `Source/StrategosCore/Public/World/ProvinceGeographyTypes.h:33`.
  - **The code starts in 1836:** `TimeSubsystem.cpp:8` `FDateTime(1836, 1, 1)`, `WorldBootstrapAsset.h:27` `StartYear = 1836`, `StrategosGameMode.cpp:40`, and the `DA_Sandbox1836` asset in `etapa-1-editor-setup.md:189`.
  - **The era table runs to 1925:** `21-progress.md:75-79`.
  - 82 §4.14 says 1800–1950.
- **Language rule that conflicts with 82 §1:**
  - `CLAUDE.md:18`: "**Idioma primário: Português (PT-BR).** Código, comentários, commits, UI — tudo PT exceto identificadores C++ (que ficam em inglês por convenção UE)."
  - Supporting lines: `CLAUDE.md:61` "Mensagem em inglês (convenção git), mas docs e código em PT". `CLAUDE.md:109` "Localização para outras línguas (PT-BR só, no MVP)", which also conflicts with 82 §4.15 (EN only in the first build).
- **The glossary itself conflicts with 82:**
  - `CLAUDE.md:127` "Stratum — classe social: Laborer / Artisan / FactoryWorker / Bourgeoisie …". It uses a retired word, and it treats strata as professions.
  - `CLAUDE.md:95` "Bourgeoisie auto-investe".
- **Code drift seen while searching:**
  - `StrategosProvinceDockWidget.cpp:234-236` uses `EPopStratum::Lower/Middle/Upper`.
  - `StrategosCommanderCardWidget.cpp:92-95` uses `ELeaderArchetype::Martial/Diplomatic/Scholar`.
  - Neither set of values exists in the enums (`PopStratum.h:15`, `LeaderArchetype.h:19`). The branch mixes two vocabularies for the same enums.

---

### 1. Per-row inventory (82 §5, same IDs)

#### Core and progression

| ID | EN (from 82) | Docs say (word ×count, files) | Code says (identifiers, file:line) | Note |
|---|---|---|---|---|
| `tier` | Tier | tier ×63 (40-resources-and-production, 11-units, etapa-2-economy-content +8); nível ×17 (00-overview, onboarding-ue5-e-workflow, 31-politics +6) | `Tier`, `GetWaterAccessTier`, `ProductionTier` ×17 (EconomySubsystem.cpp:374, GoodAsset.h:12) | Used for at least 5 ladders: PM tier 0–5 (`ProductionTier`, ProductionMethodAsset.h:53), good tier 0–2 (`UGoodAsset::Tier`, GoodAsset.h:50), nation tech tier Underdeveloped/Developing/Developed (40-resources:101), needs tiers (20-economy:180), XP tier (11-units:204), AI "Tiers de complexidade" (40-ai-director:586). See collisions C1. |
| `steel_tier` | Steel Tier | aço/steel ×11 (20-economy, 11-units, 40-resources-and-production +2) | `TEXT("steel")` ×1 (StrategosProvinceDockWidget.cpp:160, building-category color key); no `G_Steel` in code | No Steel Tier concept. Steel appears as good `G_Steel` and PM ladder bloomery→puddled→Bessemer (40-resources:106-112); gating is by nation-wide tech tier, not a per-province Steel Tier. Code `"steel"` is only a building-category color key. |
| `interlock` | Interlock | not found (interlock) | not found (`\w*Interlock\w*`) | Concept absent; closest is 11-units "Equipment × tech" composition (doc only). |
| `modifier` | Modifier | modifier(s) ×37 (10-battle, 11-units, 98-estado-implementacao +10); modificador(es) ×56 (30-events, 21-progress, 11-units +13) | `UProductionModifierAsset`, `Modifier`, `ActiveProductionModifiers` ×131 (AIPlaceholderSubsystem.cpp:324, BattleResolverService.cpp:96) | Two meanings: generic effect (`FStatModifierSet`, `UActiveModifierRegistry`, docs) and player-toggled building policy `UProductionModifierAsset` (M_PushForOutput etc., 3 mutex groups). See C6. |
| `odds` | Chance | chance ×3 (etapa-2-events-content, 32-diplomacy); probabilidade ×6 (30-events, 10-battle) | `GetAcceptanceProbability`, `GetAIAcceptanceProbability`, `LeaderTurnoverChance` ×13 (AIPlaceholderSubsystem.cpp:100, AIPlaceholderSubsystem.h:70) | Diplomacy uses verdict text "Provável aceitação / Resultado incerto / Provável recusa" (StrategosDiplomacyActionWidget.cpp:226-228), no %. |

#### Economy

| ID | EN (from 82) | Docs say (word ×count, files) | Code says (identifiers, file:line) | Note |
|---|---|---|---|---|
| `production_method` | Production Method | production method ×2 (CLAUDE, 40-resources-and-production); método(s) de produção ×6 (40-resources-and-production, CLAUDE, etapa-2-economy-content); PM ×16 (etapa-2-economy-content, 40-resources-and-production, _plano-sessoes-recursos +2) | `UProductionMethodAsset`, `CurrentProductionMethod`, `ProductionMethodAsset` ×47 (EconomySubsystem.cpp:5, SaveSubsystem.cpp:11) | Matches 82. CLAUDE.md glossary calls a PM a "carta" (card) (CLAUDE:128), which collides with battle cards. |
| `building_level` | Level | level ×29 (31-politics, etapa-1-editor-setup, onboarding +5; mostly law ProgressionLevel and UE map "Level"); `MaxLevel` ×1 (etapa-2-economy-content:127); nível ×17 (mixed) | `MaxLevel` (BuildingTypeAsset.h:52), `FProvinceBuildingRow::Level` (StrategosProvinceDockWidget.h:33), `FBuildingHUDRow::Level` (StrategosHUDWidget.h:33); noise: `ETaxLevel`, `FInfrastructureLevel`, XP level | Docs "level" mostly law `ProgressionLevel` (31-politics:156) and UE map "Level"; building level is `MaxLevel` / `Level`. Also XP level, infrastructure level, `ETaxLevel`. See C2. |
| `good` | Good | good(s) ×25 (20-economy, 21-progress, etapa-2-economy-content +1); bem/bens ×52 (20-economy, 40-resources-and-production, etapa-2-economy-content +11) | `GoodId`, `FGoodAmount`, `UGoodAsset` ×194 (EconomySubsystem.cpp:4, EventConditions.cpp:57) | Matches. Docs also use "recurso" for raw goods (40-resources, §4 retired list). |
| `good_raw` | Raw | raw ×9 (etapa-2-economy-content, 40-resources-and-production); bruto(s) ×6 (40-resources-and-production, 30-events, _plano-sessoes-recursos) | `EGoodCategory::Raw` (GoodAsset.h:20); `RawResourcePotential` (Province.h:62), `bRequiresRawResource` (BuildingTypeAsset.h:55) ×50 total | Matches as `EGoodCategory::Raw`; also called "Tier 0" (GoodAsset.h:12). |
| `good_intermediate` | Intermediate | intermediate ×3 (onboarding-ue5-e-workflow); refined/processed ×3 (40-resources-and-production, etapa-2-economy-content); manufaturado(s) ×2 (40-resources-and-production); Industrial (category) ×30 (21-progress, etapa-2-economy-content, 20-economy +7) | `EGoodCategory::Industrial` (GoodAsset.h:22) — the intermediate category | Code/docs call this category **Industrial** (Iron, Lumber, Cloth, Tools), docs 40 call it "refined"/"manufaturados". 82 name not used. |
| `good_final` | Final | final ×18 (11-units, 20-economy, proporcoes-de-arte +11); consumer ×1 (etapa-2-economy-content); Staple ×3 (etapa-2-economy-content, 20-economy); Luxury ×3 (20-economy, 31-politics, etapa-2-economy-content) | `EGoodCategory::Staple` (GoodAsset.h:21), `EGoodCategory::Luxury` (GoodAsset.h:23) | Final is split into **Staple** and **Luxury** in code; doc 20 has 6 categories (Staple, Industrial, Manufactured, Military, Luxury, Strategic, 20-economy:149-157). |
| `supply` | Supply | supply ×20 (41-military, 20-economy, 10-battle +3); oferta ×2 (20-economy, 40-ai-director) | `Supply`, `GetSupplyRatio`, `SupplyConsumed` ×51 (BattleAIController.cpp:89, BattleCardAsset.cpp:59) | Collides with military Supply (SUP stat, `USupplyNetwork`, `GetSupplyRatio`). See C3. |
| `demand` | Demand | demand ×11 (20-economy, 41-military, 98-estado-implementacao +1); demanda ×9 (31-politics, 00-overview, 20-economy +3) | `Demand`, `EDemandFulfillmentSource`, `RecordDemand` ×40 (EconomySubsystem.cpp:146, EconomySubsystem.h:95) | Matches (`FNationalStockpile::Demand`, `FPopDemandOrder`). |
| `market` | Market | market(s) ×3 (docs/README, 21-progress); mercado(s) ×46 (20-economy, 40-resources-and-production, README +7) | `GlobalMarket`, `LocalMarket`, `Market` ×20 (LogisticsInfrastructure.h:59, MarketDestination.h:4) | Docs: regional markets `UMarketRegion` (20-economy:207). Code: Local Market → Trade Hub → Global Market (MarketDestination.h:10-13, TradeHub.h:21). |
| `national_market` | National Market | mercado nacional ×2 (20-economy); local market/mercado local ×1 (40-resources-and-production); Market Region ×17 (20-economy, 99-implementation-roadmap, README +2) | `LocalMarket`, `UTradeHub`, `FNationalStockpile` ×18 (LogisticsInfrastructure.h:60, MarketDestination.h:10) | No "National Market" anywhere. Code layers: Local Market (province), `UTradeHub` (regional), `FNationalStockpile` (state stock). Docs: Market Region. Feeds the *waits for recon* row. |
| `world_market` | World Market | global market/mercado global ×2 (20-economy); internacional ×4 (20-economy, 21-progress, 31-politics +1) | `GlobalMarket`, `ShareToGlobalMarket` ×7 (LogisticsInfrastructure.h:59, MarketDestination.h:12) | Code says **Global Market** (`EMarketDestination::GlobalMarket`, DisplayName "Global Market", MarketDestination.h:20; "Global Market (Imported)", PopDemandOrder.h:19). |
| `shortage` | Shortage | `GetTopShortfalls()` ×1 (etapa-2-economy-content:167); déficit ×3 (41-military:393-397, army supply, not goods); "falta" ×7 = false positives | `FShortfallEntry`, `GetTopShortfalls`, `ShortfallAmount` (StrategosHUDWidget.h:17,23,129); `EDemandFulfillmentSource::Unmet` (PopDemandOrder.h:20); comment "deficit derruba loyalty" (EconomySubsystem.h:55) | Code word is **Shortfall** (HUD) and `Unmet` (demand source). All 7 doc "falta" hits mean "is missing", not the concept. Doc "déficit" hits are army supply. |
| `surplus` | Surplus | sobra ×1 (20-economy) | not found (`\b\w*Surplus\w*\b`) | Only "sobra acumulada" = market stockpile (20-economy:222). No surplus concept. |
| `price` | Price | price ×8 (20-economy, 30-events); preço ×21 (20-economy, 40-resources-and-production, 98-estado-implementacao +3) | `GetDynamicPrice`, `SalePrice`, `BasePrice` ×22 (EconomySubsystem.cpp:138, EconomySubsystem.h:95) | Matches (`BasePrice`, `GetDynamicPrice`). |
| `input` | Input | input(s) ×27 (etapa-1-editor-setup, etapa-2-economy-content, 98-estado-implementacao +6) | `InputsPerSlot` (ProductionMethodAsset.h:56), `LastTickInputs` (Building.h:60), `InputCostMultiplier` (ProductionModifierAsset.h:45); noise: UE `UInputAction` etc. | Matches in economy (`InputsPerSlot`). Doc/code counts are dominated by UE input (Enhanced Input). |
| `output` | Output | output(s) ×25 (21-progress, etapa-2-economy-content, onboarding-ue5-e-workflow +6) | `LastTickOutputs`, `Output`, `OutputsPerSlot` ×12 (AIPlaceholderSubsystem.cpp:306, EconomySubsystem.cpp:418) | Matches (`OutputsPerSlot`, `LastTickOutputs`). Doc count includes UE "Output Log". |
| `efficiency` | Efficiency | eficiência ×5 (21-progress, 20-economy, 40-resources-and-production); throughput ×15 (20-economy, 41-military, 21-progress +1) | `Throughput`, `ThroughputMultiplier`, `WorkEfficiencyPenalty` ×10 (EconomySubsystem.cpp:254, GoodConsumerProfile.h:73) | Code word is **Throughput** (`ThroughputMultiplier`), also used for TradeHub capacity (`ThroughputPerMonth`). See C7. |
| `building` | Building | building(s) ×12 (21-progress, 98-estado-implementacao, etapa-2-economy-content +2); edifício(s) ×7 (40-resources-and-production, _plano-sessoes-recursos); prédio(s) ×14 (etapa-2-economy-content, CLAUDE, 98-estado-implementacao +4); indústria/industry ×36 (20-economy, 21-progress, 40-ai-director +3) | `UBuilding`, `BuildingId`, `Building` ×364 (AIPlaceholderSubsystem.cpp:10, WorldBootstrapper.cpp:70); `SponsorPrivateIndustry`, `GetIndustryIndex` ×4 (EconomySubsystem.cpp:1120, EconomySubsystem.h:140) | Code: Building; docs: Industry (`UIndustry`, 20-economy), "prédio" (CLAUDE), "edifício". Two+ words. UI tab "Edifícios" (StrategosProvinceDockWidget.h:19). |
| `building_slot` | Building Slot | slot(s) ×40 (40-resources-and-production, etapa-2-economy-content, 11-units +5) | `BuildingSlots` (StrategosProvinceDockWidget.h:65), `GetFreeBuildingSlots` (…h:121), `Rejected_NoSlot` (EconomySubsystem.h:30); noise: UE save `SlotName` | Matches in code (`BuildingSlots`, `GetFreeBuildingSlots`). Docs "slot" also = card slots, equipment slots (11-units). |
| `construction` | Construction | construction ×2 (20-economy, 40-ai-director); construção ×7 (40-ai-director, 40-resources-and-production, 21-progress +2); construir ×5 (40-resources-and-production, _plano-sessoes-recursos, 40-ai-director); obra(s) ×4 (40-resources-and-production, _plano-sessoes-recursos, etapa-2-events-content) | `EBuildResult`, `Build`, `Construction` ×104 (DefaultGame.ini:10, LeGrandStrategos.Target.cs:1) | Matches; state `UnderConstruction` "Under Construction" (BuildingState.h:22). |
| `employment` | Employment | employment ×4 (etapa-2-economy-content); emprego ×3 (98-estado-implementacao, CLAUDE); workforce ×3 (20-economy); mão de obra ×3 (40-resources-and-production, _plano-sessoes-recursos) | `EmploymentPerSlot`, `FStratumEmployment`, `LastTickEmployment` ×28 (EconomySubsystem.cpp:218, Building.h:66) | Matches; docs 20 use Workforce. |
| `profit` | Profit | profit ×1 (CLAUDE); lucro ×7 (20-economy, 30-events, 40-resources-and-production +1) | `LastTickProfit`, `Profit`, `Income_StateProfits` ×29 (EconomySubsystem.cpp:197, Building.h:72) | Matches (`LastTickProfit`). |
| `private_investment` | Private Investment | auto-invest* ×8 (40-resources-and-production, etapa-2-economy-content, CLAUDE +2) | `RunBourgeoisieAutoInvestment` ×3 (EconomySubsystem.cpp:227, EconomySubsystem.h:144); `EBuildingOwnerKind::Private` ×7 (EconomySubsystem.cpp:866, StrategosHUDWidget.cpp:315) | Retired "auto-invest*" is the only doc word (CLAUDE:95, 40-resources:13,37,156). Owner kind displays "Private (Bourgeoisie)" (BuildingOwnerKind.h:10). |
| `treasury` | Treasury | treasury ×8 (etapa-2-events-content, docs/README, 00-overview +4); tesouro ×13 (20-economy, docs/README, 30-events +1) | `Treasury`, `FTreasury`, `Phase_SettleTreasury` ×101 (WorldBootstrapper.cpp:142, EconomySubsystem.cpp:195) | Matches; money stock field is `FTreasury::Balance` (Treasury.h:21) and doc field `Gold` (20-economy:127). |
| `income` | Income | income ×1 (etapa-2-events-content); receita(s) ×2 (20-economy, 98-estado-implementacao) | `Income_StateProfits`, `Income`, `GetMonthlyIncome` ×26 (EconomySubsystem.cpp:195, EconomySubsystem.h:57) | Matches; split `Income_Taxes`, `Income_Tariffs`, `Income_StateProfits` (Treasury.h:25-31). |
| `expenses` | Expenses | despesa(s) ×2 (98-estado-implementacao); gasto(s) ×6 (31-politics, 20-economy, 40-ai-director) | `Expense_Maintenance`, `GetMonthlyExpenses`, `Expense_ArmyUpkeep` ×31 (EconomySubsystem.cpp:198, EconomySubsystem.h:58) | Matches; split Maintenance, ArmyUpkeep, AdminCost, DebtInterest (Treasury.h:34-43). |
| `balance` | Balance | not found (balance \| saldo) | `FTreasury::Balance` (Treasury.h:21), `GetTreasuryBalance` (StrategosHUDWidget.h:100), `DebtBalance` (Treasury.h:46) ×41 | `FTreasury::Balance` is the **money stock** (= 82 Treasury), not income minus expenses. See C8. |
| `deficit` | Deficit | deficit/déficit ×3 (41-military) | `OnBankruptcyImminent`, `HandleBankruptcyImminent`, `EconomyBankruptcyImminent` ×15 (EconomySubsystem.cpp:736, EventSubsystem.cpp:26) | No money deficit; code models **Bankruptcy** (`OnBankruptcyImminent`). Doc "déficit" = army supply. |
| `pm_bloomery` | Bloomery | bloomery ×2 (40-resources-and-production, etapa-2-economy-content) | not found (`\b\w*Bloomery\w*\b`) | Docs: `PM_BloomeryIron` (40-resources:110, outputs Steel) and `PM_Smelter_Bloomery` (etapa-2-economy-content:75, outputs Iron, Tier 1). Same name, two different outputs. |
| `pm_puddling` | Puddling | puddled/puddling ×1 (40-resources-and-production) | not found (`\b\w*Puddl\w*\b`) | Docs: `PM_PuddledSteel`, tier Developing (40-resources:111). "Puddled", not "Puddling". |
| `pm_bessemer` | Bessemer Process | bessemer ×5 (21-progress, 40-resources-and-production, 30-events) | not found (`\b\w*Bessemer\w*\b`) | Docs: `PM_BessemerSteel`, tier Developed, "Bessemer, 1856" (40-resources:112); "Aço Bessemer" key tech of era High Industrial 1870–1895 (21-progress:77). |

#### Trade

| ID | EN (from 82) | Docs say (word ×count, files) | Code says (identifiers, file:line) | Note |
|---|---|---|---|---|
| `tariff` | Tariff | tarifa(s) ×3 (32-diplomacy, 40-ai-director) | `Income_Tariffs`, `TariffRate` ×4 (EconomySubsystem.cpp:196, TradeHub.h:50) | Matches (`TradeHub::TariffRate`). |
| `tariff_income` | Tariff Income | not found (customs \| alfândega \| Income_Tariffs) | `Income_Tariffs` ×3 (EconomySubsystem.cpp:196, Treasury.h:28) | Matches as `Income_Tariffs`; no "customs" anywhere. |
| `import` | Import | not found in the trade sense (`import*`/`importa*` hits are "importante", UE asset import, `#include`) | `Imported` ×1 (PopDemandOrder.h:19) | All doc hits are "importante"/UE import (false positives). Only code: DisplayName "Global Market (Imported)". |
| `export` | Export | not found in the trade sense (hits are WorldGen PNG export) | not found in trade sense; `OutgoingCapacity` (LogisticsInfrastructure.h:72) is the export side | All hits are worldgen PNG export (false positives); code has `OutgoingCapacity` "exportado a hubs / GlobalMarket" (LogisticsInfrastructure.h:58-59). |
| `trade_capacity` | Trade Capacity | capacidade de comércio/importação ×1 (20-economy); capacity ×3 (41-military, 20-economy) | `FLogisticsBandwidth` (LogisticsInfrastructure.h:64), `IncomingCapacity`/`OutgoingCapacity` (…h:69,72), `CapacityPerMonth` (…h:47), `BandwidthCostPerUnit` (GoodConsumerProfile.h:19), `ThroughputPerMonth` (TradeHub.h:43) | Code calls it **Bandwidth** ("Largura de Banda"): `FLogisticsBandwidth` Incoming/OutgoingCapacity; infrastructure `CapacityPerMonth`; hub `ThroughputPerMonth`. |

#### Resources and deposits

| ID | EN (from 82) | Docs say (word ×count, files) | Code says (identifiers, file:line) | Note |
|---|---|---|---|---|
| `deposit` | Deposit | recurso(s) ×28 (40-resources ×14, 98 ×4, _plano ×4 +4); recurso principal / primary resource ×10 (40-resources, 98, _plano +1); RGO ×1 (40-resources:28) | `RawResourcePotential`, `bRequiresRawResource`, `RGO` ×67 (WorldBootstrapper.cpp:63, EconomySubsystem.cpp:408) | No "deposit". Docs say **recurso** / primary resource / RGO; code says RawResource, PrincipalResource, RGO. All retired under §4.12. |
| `discovery` | Discovery | discover* ×3 (32-diplomacy, 31-politics); descoberta/descoberto ×9 (21-progress, 32-diplomacy, 30-events +1) | not found (`\b\w*Discover\w*\b`) | Doc hits are tech discovery (21-progress) and diplomacy; no deposit discovery mechanic. |
| `prospecting` | Prospecting | not found (prospect* \| prospecção) | not found (`\b\w*Prospect\w*\b`) | Concept absent. |
| `strategic_good` | Strategic Good | `Strategic` category / `bStrategic` ×2 (20-economy:156,174); "Estratégico:" goods group ×1 (40-resources:125); other "estratégico" ×26 = strategic map/layer | not found (`bStrategic` / `EGoodCategory::Strategic`); `FStrategicIndices` is unrelated | Doc 20 has category `Strategic` + `bStrategic` (20-economy:156,174); 40-resources groups IronOre/Coal/Saltpeter/Oil as "Estratégico" (40-resources:125). Most "estratégico" hits mean "strategic map". |
| `discovered_undiscovered` | Discovered / Undiscovered | not found for deposits (hidden ×4 = hidden techs/info, 21-progress, 10-battle) | not found for deposits (`GetKnownCounterparts` is diplomacy) | Absent for deposits. "hidden techs" (21-progress:386) and `GetKnownCounterparts` (diplomacy) are other concepts. |

#### Population

| ID | EN (from 82) | Docs say (word ×count, files) | Code says (identifiers, file:line) | Note |
|---|---|---|---|---|
| `pop` | Pop | POP(s) ×126 (31-politics, 20-economy, 30-events +9) | `EPopStratum`, `Pops`, `Population` ×261 (AIPlaceholderSubsystem.cpp:12, BattleEffects.cpp:166) | Matches; docs write "POP" in caps. |
| `stratum` | Stratum | stratum/strata ×10 (CLAUDE, 98-estado-implementacao, etapa-2-events-content +1); estrato(s) ×9 (20-economy, docs/README); classe social ×1 (CLAUDE) | `EPopStratum`, `Stratum`, `PopStratum` ×157 (AIPlaceholderSubsystem.cpp:13, BattleAIController.cpp:103) | `EPopStratum` holds **professions** (Laborer, Artisan, FactoryWorker, Bourgeoisie, Aristocracy, Soldier, Clergy; PopStratum.h:15); CLAUDE glossary: "Stratum — classe social: Laborer / …" (CLAUDE:127). See C4. |
| `stratum_lower` | Lower Stratum | lower ×1 (20-economy) | `Lower` ×1 (StrategosProvinceDockWidget.cpp:234) | UI code references `EPopStratum::Lower/Middle/Upper` (StrategosProvinceDockWidget.cpp:234-236) but the enum has no such values (stale code). Doc 20 tax categories "Aristocrat, Middle, Lower" (20-economy:134). |
| `stratum_middle` | Middle Stratum | middle ×1 (20-economy) | `Middle` ×1 (StrategosProvinceDockWidget.cpp:235) | Same stale reference (StrategosProvinceDockWidget.cpp:235). |
| `stratum_upper` | Upper Stratum | estrato alto/elite ×2 (11-units) | `Upper` ×1 (StrategosProvinceDockWidget.cpp:236) | Same stale reference (…cpp:236). Doc "elite" hits are unit/era usage. |
| `profession` | Profession | pop type / EPopType ×13 (20-economy, 31-politics, 21-progress) | not found (`Profession`/`PopType`); the profession list lives in `EPopStratum` (PopStratum.h:15) | No word "profession". Docs: `EPopType` (20-economy:39); code: `EPopStratum` carries the profession list. |
| `prof_peasant` | Peasant | peasant(s) ×1 (40-resources-and-production); Farmer ×2 (20-economy) | not found (`\b\w*(?:Peasant\|Farmer)\w*\b`) | 40-resources:15 rejects "peasants" as anachronistic. Doc 20 has `Farmer`; code has `Laborer` ("peão"). |
| `prof_laborer` | Laborer | laborer(s) ×11 (etapa-2-economy-content, CLAUDE, 40-resources-and-production +2) | `Laborer` ×20 (AIPlaceholderSubsystem.cpp:338, WorldBootstrapper.cpp:133) | Matches; code also displays it as **"Trabalhadores"** (EventEffects.cpp:113), the PT word 82 uses for IG Workers. |
| `prof_machinist` | Machinist | FactoryWorker/factory worker ×9 (etapa-2-economy-content, CLAUDE, 40-resources-and-production +1); Worker(s) ×13 (31-politics, 20-economy, etapa-2-events-content +1) | `FactoryWorker` ×17 (AIPlaceholderSubsystem.cpp:338, WorldBootstrapper.cpp:135) | Code `FactoryWorker` "Factory Worker" (PopStratum.h:19), PT "Operários" (EventEffects.cpp:115); doc 20/31 "Worker". |
| `prof_artisan` | Artisan | artisan(s) ×14 (etapa-2-economy-content, 20-economy, CLAUDE +3) | `Artisan` ×11 (WorldBootstrapper.cpp:134, EconomySubsystem.cpp:483) | Matches. |
| `prof_clerk` | Clerk | bureaucrat/burocrata ×2 (20-economy) | not found (`\b\w*(?:Clerk\|Bureaucrat)\w*\b`) | Doc 20 `Bureaucrat` only (20-economy:39). |
| `prof_capitalist` | Capitalist | capitalist(s) ×1 (40-ai-director); capitalista(s) ×6 (20-economy, 40-ai-director); Bourgeoisie/Bourgeois ×15 (CLAUDE, 31-politics, etapa-2-economy-content +4); burguesia ×2 (31-politics, etapa-2-events-content) | `Bourgeoisie`, `RunBourgeoisieAutoInvestment` ×37 (AIPlaceholderSubsystem.cpp:338, WorldBootstrapper.cpp:136) | Code/docs say **Bourgeoisie** / Bourgeois; doc 20 "capitalistas" for the AI investors (20-economy:374 heading). |
| `prof_aristocrat` | Aristocrat | aristocrat/aristocracy ×12 (31-politics, 20-economy, CLAUDE +2); aristocrata ×2 (31-politics) | `Aristocracy`, `Aristocracia` ×5 (EventEffects.cpp:117, PopStratum.h:10) | Code `Aristocracy` (stub); docs Aristocrat. |
| `prof_soldier` | Soldier | soldier(s) ×6 (CLAUDE, 20-economy, 40-resources-and-production +2) | `Soldier` ×4 (EventEffects.cpp:118, PopStratum.h:10) | Matches (`EPopStratum::Soldier`, stub). |
| `prof_officer` | Officer | officer(s) ×1 (20-economy) | not found (`\b\w*Officer\w*\b`) | Doc 20 `Officer` in EPopType only. |
| `needs` | Needs | needs ×1 (20-economy); necessidades ×8 (20-economy, 31-politics) | `Need`, `TotalNeeded` ×14 (EconomySubsystem.cpp:341) | Docs ladder: LifeNeeds / EverydayNeeds / LuxuryNeeds (20-economy:178-186). Code: consumption basket + `FGoodLuxuryProfile`. Feeds the *waits for recon* row. |
| `needs_met` | Needs Met | satisfaction/NeedsSatisfaction ×9 (20-economy, 31-politics); satisfação ×4 (20-economy); atendid* ×4 (20-economy, 31-politics) | `Satisfaction` (StrategosProvinceDockWidget.h:51, filled from Loyalty), `SatisfactionBonus`/`SatisfactionBonusEarly` (GoodConsumerProfile.h:65,90), `bAllSatisfied` (EconomySubsystem.cpp:524), `EDemandFulfillmentSource` (PopDemandOrder.h:15) | Docs use `NeedsSatisfaction` / "satisfação" (retired). UI `FProvincePopRow::Satisfaction` is filled from **Loyalty** (StrategosProvinceDockWidget.cpp:245). |
| `literacy` | Literacy | literacy ×3 (20-economy, 21-progress, 31-politics); letrado(s) ×1 (31-politics) | `LiteracyFloor` ×1 (TechnologyDataAsset.h:50) | Doc 20 pop field; code only `LiteracyFloor` (tech gate). |
| `promotion_demotion` | Promotion / Demotion | promoção/rebaixamento ×1 (20-economy); mobility/mobilidade (social) ×3 (11-units, 20-economy) | `Mobility`, `FPopMobilityTracker`, `PromotionThreshold` ×15 (PopMobilityTracker.h:4) | Code `FPopMobilityTracker` with `PromotionThreshold`/`DemotionThreshold` — uses retired "mobility". Doc 20 "PopPromotion → mobilidade entre estratos" (20-economy:330). |
| `unemployed` | Unemployed | not found (unemploy* \| desempreg*) | not found (`\b\w*Unemploy\w*\b`) | Concept absent. |
| `migration` | Migration | migration ×1 (20-economy); migração/migra* ×17 (20-economy, sessao-1-terreno-provincia, 31-politics +4) | not found (`\b\w*Migrat\w*\b`) | Doc only (20-economy PopMigration, 31-politics). Some "migra*" hits are ideology drift ("POPs migram entre ideologias"). |
| `culture` | Culture | culture ×5 (21-progress, 20-economy, 31-politics); cultura ×8 (32-diplomacy, 40-ai-director, 00-overview +2) | `CultureWorldview`, `Culture` ×2 (TechSphere.h:17) | Doc 20 `ECultureId`; code only tech sphere "Culture & Worldview". |
| `religion` | Religion | religion ×2 (20-economy, 31-politics); religião ×1 (99-implementation-roadmap) | `Religion` ×1 (PopStratum.h:11) | Pop `EReligionId` (doc 20); code: no religion field (only a comment, PopStratum.h:11). |
| `state_religion` | State Religion | Estado Confessional ×1 (31-politics) | not found (`\b\w*StateReligion\w*\b`) | Closest: law rung "Estado Confessional" (31-politics:190). |

#### Politics

| ID | EN (from 82) | Docs say (word ×count, files) | Code says (identifiers, file:line) | Note |
|---|---|---|---|---|
| `interest_group` | Interest Group | faction/facção ×26 (31-politics, 40-ai-director, 00-overview +4); party/partido ×15 (31-politics, WorldGen/README) | not found (`InterestGroup`/`Faction`/`Party`/`IG_`) | No IGs. Nearest is **Faction** (`UFaction`, `EFactionType`: Court, Military, Religious, Industrial, Labor, Nationalist, Colonial, Reformist; 31-politics:518-530) plus Parties/ideologies. |
| `ig_landowners` | Landowners | not found (landowner(s) \| latifundiário) | not found (`\b\w*Landowner\w*\b`) | Absent; nearest faction type `Court` (31-politics:523). |
| `ig_industrialists` | Industrialists | not found as a group ("industriais" ×4 = adjective); faction type `Industrial` (31-politics:523) | not found (`\b\w*Industrialist\w*\b`) | Nearest faction type `Industrial` (31-politics:523). "industriais" hits are the adjective. |
| `ig_petty_bourgeoisie` | Petty Bourgeoisie | not found (petty bourgeoisie \| pequena burguesia) | not found (`\b\w*PettyBourgeois\w*\b`) | Absent. |
| `ig_laborers` | Workers | workers ×8 (31-politics, etapa-2-events-content, 41-military); Labor (faction) ×5 (31-politics, etapa-2-economy-content, CLAUDE) | not found; `"Trabalhadores"` is the PT label of `EPopStratum::Laborer` (EventEffects.cpp:113) | No IG; nearest faction type `Labor` (31-politics:524). Doc "Workers" = pop type; "Labor" also = PM mutex group (CLAUDE:96). |
| `ig_military` | Armed Forces | not found (IG_Military / armed forces / forças armadas); faction type `Military` (31-politics:523) | not found (`\bIG_Military\b\|ArmedForces`) | Nearest faction type `Military` (31-politics:523); military coup (31-politics:545). |
| `ig_clergy` | Clergy | clergy ×5 (CLAUDE, 20-economy, 40-resources-and-production +1); clero ×2 (31-politics) | `Clergy` ×4 (EventEffects.cpp:119, PopStratum.h:10) | Nearest faction type `Religious` (31-politics:523). Code `EPopStratum::Clergy` is a stratum/profession, not an IG. |
| `clout` | Clout | influência/influence ×25 (32-diplomacy, 31-politics, docs/README +4) | `InfluenceTaxCost` ×1 (GoodConsumerProfile.h:46) | Nearest: faction `Influence` "0..1, peso político" (31-politics:525). Most "influência" hits are Sphere of Influence (32-diplomacy); `InfluenceTaxCost` is a goods field. |
| `governing_bloc` | Governing Bloc | ruling party / partido governista ×9 (31-politics); coalizão/coalition ×5 (31, 32, 40-ai-director); "bloco" ×5 = unrelated | not found (`\b\w*(?:GoverningBloc\|RulingParty\|Coalition)\w*\b`) | Old design: ruling party + coalitions in parliament (31-politics). |
| `outside_bloc` | Outside the Bloc | not found (outside the bloc \| fora do bloco) | not found (`\b\w*OutsideBloc\w*\b`) | Absent. |
| `ig_approval` | Approval | aprovação ×4 (31-politics, CLAUDE); loyalty ×9 (31-politics, etapa-2-economy-content, etapa-2-events-content); lealdade ×4 (20-economy, 31-politics, 41-military +1) | `Loyalty`, `MonthlyLoyaltyDelta`, `MakeEffect_AddLoyalty` ×56 (WorldBootstrapper.cpp:129, EconomySubsystem.cpp:258) | Docs: faction `Loyalty` "-1..1, alinhamento com governo" (31-politics:526). Code: **Loyalty** per pop (`FPopGroup::Loyalty`, PopGroup.h:32), shown as "Lealdade" (EventEffects.cpp:107) and as a "Satisfação" bar (C9). |
| `alignment` | Alignment | alignment ×1 (40-ai-director); alinhamento ×2 (31-politics, 32-diplomacy) | not found (`\b\w*Alignment\w*\b`) | Not a mechanic. "alinhamento" appears only as the meaning of faction Loyalty (31-politics:526) and diplomatic alignments (32-diplomacy:24). |
| `preferred_tier` | Preferred Tier | preferência/preference ×9 (31-politics, 10-battle); stance ×10 (etapa-2-ui-grid, 10-battle, 41-military +2) | not found (`EQuitPreference` is the quit dialog) | Docs: `FLawPreferenceCurve` / `IdeologySupport` per ideology (31-politics:172,202). "stance" hits are army/battle stances. |
| `law` | Law | law(s) ×5 (30-events, 31-politics, docs/README); lei(s) ×56 (31-politics, 21-progress, 30-events +7) | not found (`\b\w*Law(?!yer)\w*\b`) | Docs only (`ULawAsset`, 31-politics:149). No law code. |
| `law_category` | Law Category | categoria de lei ×2 (31-politics); ELawCategory ×5 (31-politics) | not found (`\b\w*LawCategory\w*\b\|LAWCAT_\w+`) | `ELawCategory` has **11** categories (31-politics:180-193) vs 82's 5. |
| `lawcat_economy` | Economy | Trade/Taxation (ELawCategory) ×2 (31-politics) | not found (`LAWCAT_Economy`) | Old categories Trade + Taxation. |
| `lawcat_labor` | Labor | LaborRights ×2 (31-politics) | not found (`LAWCAT_Labor\|LaborRights`) | Old category `LaborRights`. |
| `lawcat_military` | Military | Conscription (ELawCategory) ×1 (31-politics) | not found (`LAWCAT_Military`) | Old category `Conscription`. |
| `lawcat_governance` | Governance | governance/governança ×1 (31-politics); Government (ELawCategory) ×1 (31-politics) | not found (`LAWCAT_Governance\|\bGovernance\b`) | Old category `Government`. |
| `lawcat_religion` | Religion | Religion (ELawCategory) ×1 (31-politics) | not found (`LAWCAT_Religion`) | Old category `Religion` (Confessional → Tolerance → Secularism). |
| `enactment` | Enactment | enact* ×5 (31-politics); reform/reforma ×28 (31-politics, 21-progress, 40-ai-director +3); votação/vote ×4 (31-politics) | not found (`\b\w*(?:Enact\|Reform\|Vote)\w*\b`) | Docs say **Reform** (`UReformEngine`, `FReformProposal`), "EnactmentDays"/"EnactmentPeriod" for the waiting time (31-politics:165,433); vote only in elections. |
| `enactment_enacted` | Enacted | passed/LawPassed ×7 (31-politics, 30-events, 03-event-bus); aprovada(s)/aprovado ×3 (30-events, 31-politics) | not found (`\b\w*(?:Enacted\|Passed)\w*\b`) | Docs: `EReformPhase::Passed`, `OnLawPassed` (31-politics:435,451). |
| `enactment_failed` | Failed | failed (reform) ×1 (31-politics); rejeitado ×2 (31-politics, 40-ai-director) | not found (`\b\w*ReformFailed\w*\b`) | Docs: `EReformPhase::Failed` (31-politics:451). |
| `political_capital` | Political Capital | political capital ×10 (31-politics); capital político ×8 (31-politics, docs/README, 21-progress +1); PC ×13 (31-politics, CLAUDE, 40-ai-director) | `PoliticalCapital` ×2 (StrategosDiplomacyActionWidget.cpp:145, StrategosDiplomacyActionWidget.h:26) | Docs abbreviate **PC** (31-politics:399-405). Code: diplomatic actions cost political capital (StrategosDiplomacyActionWidget.h:26). |
| `capital_gain` | Monthly Gain | `MonthlyGain` ×1 (31-politics:372); `ComputePoliticalCapitalGain` (31-politics:379) | not found (`\b\w*MonthlyGain\w*\b`) | Docs field `MonthlyGain` (31-politics:372) and `ComputePoliticalCapitalGain` — same word as 82. |
| `capital_cap` | Capital Cap | `MaxCapacity` "teto" ×1 (31-politics:371); other "teto" hits unrelated | not found (`\b\w*(?:MaxCapacity\|CapitalCap)\w*\b`) | Docs `MaxCapacity` "teto" (31-politics:371). |
| `enactment_chance` | Chance of Success | CurrentSupport ×1 (31-politics) | not found (`\b\w*(?:SuccessChance\|ChanceOfSuccess\|PassChance)\w*\b`) | Docs: `CurrentSupport` 0..1 in parliament (31-politics:452); no success chance. |
| `support_opposition` | Support / Opposition | support ×11 (11-units, 10-battle, onboarding-ue5-e-workflow); apoio ×2 (30-events, 31-politics); opposition/oposição ×3 (31-politics) | not found in the political sense (`ECardCategory::Support`, `DoesSupportWorldType` are other concepts) | Docs: parliament support/opposition (31-politics:430-432). Code "Support" = card category / unit role. |
| `tech_gate` | Requires | RequiredTech* ×9 (20-economy, 31-politics, 98-estado-implementacao +3); requer/exige ×4 (CLAUDE, 03-event-bus, 32-diplomacy +1); gate/gating ×9 (40-resources-and-production, etapa-2-events-content, CLAUDE +2) | `RequiredTechId`, `RequiredPrereqTech` ×3 (ProductionMethodAsset.h:66, ProductionModifierAsset.h:57) | Matches as dev term (`RequiredTechId`); UI text "Requer: 500 Ouro" (etapa-2-events-content:143). |

#### Military

| ID | EN (from 82) | Docs say (word ×count, files) | Code says (identifiers, file:line) | Note |
|---|---|---|---|---|
| `unit` | Unit | unit(s) ×12 (docs/README, 00-overview, 41-military +3); unidade(s) ×49 (11-units, etapa-2-ui-grid, CLAUDE +13); regiment(s)/regimento(s) ×36 (10-battle, 11-units, 41-military +4) | `UUnitTypeAsset`, `UnitType`, `EUnitState` ×56 (SaveSubsystem.cpp:8, GoodConsumerProfile.h:19); `FRegimentBattleState`, `Regiments`, `UEffect_DamageRegiment` ×56 (BattleEffects.cpp:11, BattleResolverService.cpp:32) | Code: `UUnitTypeAsset` (one army = one unit type); docs: Regiment (`URegiment`) inside armies. Two words. |
| `army` | Army | army/armies ×16 (etapa-1-editor-setup, 00-overview, CLAUDE +5); exército(s) ×46 (41-military, proporcoes-de-arte, 20-economy +11) | `Army`, `UArmy`, `ArmyId` ×527 (AIPlaceholderSubsystem.cpp:6, AIPlaceholderSubsystem.h:9) | Matches. |
| `battle` | Battle | battle(s) ×34 (docs/README, 01-game-flow, 10-battle +8); batalha(s) ×69 (10-battle, 41-military, 11-units +13) | `FBattleSide`, `UBattleSubsystem`, `EBattlePosition` ×873 (LeGrandStrategos.Target.cs:15, LeGrandStrategosEditor.Target.cs:15) | Matches. |
| `war` | War | war ×34 (32-diplomacy, 40-ai-director, 21-progress +8); guerra(s) ×59 (32-diplomacy, 30-events, 40-ai-director +8) | `WarRoom`, `UStrategosWarRoomWidget`, `EWarRoomTab` ×57 (DiplomacySubsystem.cpp:131, DiplomaticStatus.h:17) | Matches (`EDiplomaticStatus::War` "Guerra"). |
| `peace` | Peace | peace ×7 (21-progress, 32-diplomacy, 98-estado-implementacao +1); paz ×19 (32-diplomacy, 41-military, 40-ai-director +3) | `Peace`, `UStrategosPeaceNegotiationWidget`, `FPeaceTermRow` ×58 (WorldBootstrapper.cpp:465, DiplomacySubsystem.cpp:117) | Matches; also peace negotiation widget and peace terms. |
| `manpower` | Manpower | not found (manpower/reserva/contingente); 41-military:133 uses "pop disponível" | `ManpowerCount`, `Manpower`, `GetManpower` ×20 (BattleResolverService.cpp:36, BattleSubsystem.cpp:828) | Code only (`Manpower`, army card). Docs: "pop disponível" (41-military:133). |
| `conscription` | Conscription | conscription ×1 (31-politics); conscrição ×10 (31-politics, 41-military); mobilização/mobilization ×14 (41-military, docs/README, 98-estado-implementacao +2) | `MobilizationDays` ×1 (UnitTypeAsset.h:68) | Docs: law category + mobilization (`UMobilizationEngine`, 41-military:567). Code `MobilizationDays`. |
| `recruitment` | Recruitment | recruit* ×4 (41-military, 21-progress, 40-ai-director); recrutar/recrutamento ×7 (41-military, docs/README); reforço/reposição ×3 (41-military) | `RequestRecruitArmy` ×3 (StrategosProvinceDockWidget.cpp:295, StrategosProvinceDockWidget.h:151) | Matches (`RequestRecruitArmy`, `URecruitmentEngine`). "Reforço/Reposição" (refill) defined in 41-military:123 — the concept 82 said has no doc. |
| `design_tier` | Design Tier | concept/conceito ×3 (11-units, 21-progress, 41-military); unit class/classe ×16 (11-units, 21-progress, 41-military) | not found (`Concept`/`UnitClass`/`DesignTier`); nearest `UUnitTypeAsset::Role` (UnitTypeAsset.h:39) | Docs: unit class `UUnitClassAsset` (functional role) + doctrine; no ordinal design ladder. Code: `UUnitTypeAsset` + `Role` text. |
| `equipment_tier` | Equipment Tier | equipment ×11 (11-units, docs/README, 20-economy +4); equipamento(s) ×26 (11-units, 21-progress, 41-military +5) | not found (only a comment, Army.h:15) | Docs: `UEquipmentAsset` axis (11-units:97). Code: only a comment (Army.h:15). |
| `effectiveness` | Effectiveness | not found (effectiveness \| eficácia) | not found (`\b\w*Effectiveness\w*\b`) | Absent. |
| `doctrine` | Doctrine | doctrine ×17 (11-units, docs/README, 20-economy +4); doutrina(s) ×37 (11-units, 40-ai-director, 21-progress +2) | not found (only a comment, Army.h:15) | Docs: `UDoctrineAsset`, scope National/Commander, named doctrines (11-units:149-190, class at :155); AI also uses "Doutrina Nacional" for AI themes (40-ai-director:153). No four-axis model. Code: comment only. |
| `commander` | Commander | commander ×18 (11-units, 10-battle, 30-events +4); comandante(s) ×18 (11-units, 00-overview, 10-battle +6); general(is) ×5 (10-battle, 00-overview, 30-events +1); leader/líder ×20 (40-ai-director, 31-politics, 21-progress +5) | `UStrategosCommanderCardWidget`, `CommanderCard`, `FCommanderStatRow` ×59 (BattleProposal.h:42, BattleTypes.h:247); `ELeaderArchetype`, `CurrentLeader`, `Leader` ×168 (AIPlaceholderSubsystem.cpp:7, AIPlaceholderSubsystem.h:60) | Code: Commander card + `ULeader` (national leader with archetype). Stats Tática/Carisma/Logística/Ousadia (StrategosCommanderCardWidget.cpp:100-103). |
| `morale` | Morale | morale ×13 (10-battle, 41-military, 11-units +1); moral ×17 (10-battle, 11-units, 41-military +3); MOR ×9 (etapa-2-ui-grid, proporcoes-de-arte, CLAUDE) | `Morale`, `MOR`, `UEffect_MoraleShift` ×123 (BattleAIController.cpp:83, BattleCardAsset.cpp:56) | Unit stat MOR / `Morale` (battle). Also country card "Moral" = `CivilianMoraleIndex` (StrategosCountryCardWidget.cpp:148). See C5. |
| `command_points` | Command Points | command points ×2 (10-battle, 98-estado-implementacao) | `CommandPoints`, `RefreshCommandPoints`, `MaxCommandPoints` ×16 (BattleAIController.cpp:22, BattleCardAsset.cpp:62) | Matches (battle). |
| `auto_resolve` | Auto-resolve | auto-resolve ×16 (10-battle, 98-estado-implementacao, 99-implementation-roadmap +3); resolução automática ×2 (10-battle); ResolveQuick ×5 (10-battle, 98-estado-implementacao) | `UBattleResolverService`, `ResolveQuick`, `EnqueueOrAutoResolve` ×15 (BattleResolverService.cpp:1, BattleProposal.h:10) | Matches; code `UBattleResolverService::ResolveQuick`. |
| `battle_report` | Battle Report | battle log/replay ×24 (10-battle, 98-estado-implementacao, 00-overview +6) | `EBattleLogType`, `Replay`, `FBattleLogEntry` ×43 (BattleReplayService.cpp:1, BattleSubsystem.cpp:260) | Code: battle log + replay (`FBattleLogEntry`, `UBattleReplayService`). |
| `occupation` | Occupation | ocupação/ocupad* ×18 (41-military, 32-diplomacy, _plano-sessoes-recursos +1) | not found (`\b\w*Occup\w*\b`) | Docs only (`UOccupationEngine`, 41-military:528). |
| `line_infantry` | Line Infantry | Infantry.Line / U_Infantry_Line ×3 (11-units, CLAUDE) | `U_Infantry_Line` ×1 (UnitTypeAsset.h:27) | Matches as `U_Infantry_Line` (naming convention in UnitTypeAsset.h:27 comment and CLAUDE:43; no asset in code); battle `ERegimentType` still has Archer/Pike. |

#### Technology (phase 2)

| ID | EN (from 82) | Docs say (word ×count, files) | Code says (identifiers, file:line) | Note |
|---|---|---|---|---|
| `technology` | Technology | technology/tech ×82 (21-progress, etapa-2-economy-content, 40-ai-director +11); tecnologia(s) ×16 (11-units, CLAUDE, README +7) | `Technology`, `ETechSphere`, `ETechTrack` ×33 (TechnologyDataAsset.cpp:1, ProductionMethodAsset.h:66) | Matches; code `UTechnologyDataAsset`. |
| `tech_sphere` | Sphere | sphere ×5 (docs/README, 20-economy, 32-diplomacy); esfera(s) ×31 (32-diplomacy, 40-ai-director, 20-economy +6) | `ETechSphere`, `TechSphere`, `Sphere` ×7 (TechSphere.h:4, TechnologyDataAsset.h:5) | Code has the seven: Political & Social, Production & Energy, Economy, Military, Communication, Culture & Worldview, Natural Science (TechSphere.h:12-18). Docs "esfera" = Sphere of Influence (C10). |
| `spark_diffusion_adoption` | Spark / Diffusion / Adoption | diffusion/difusão ×8 (21-progress, docs/README) | `BaseSparkChance`, `DiffusionBase` ×2 (TechnologyDataAsset.h:54) | Code: `BaseSparkChance`, `DiffusionBase` (TechnologyDataAsset.h:54,58). Docs 21: difusão/espionagem. No "adoption". |

#### Time and campaign

| ID | EN (from 82) | Docs say (word ×count, files) | Code says (identifiers, file:line) | Note |
|---|---|---|---|---|
| `era` | Era | era(s) ×39 (21-progress, docs/README, 11-units +7) | not found (`\b\w*Era(?:s\|Id\|Index\|Asset)?\b`) | Docs eras: Late Mercantile 1820–1840, Steam Age 1840–1870, High Industrial 1870–1895, Late Imperial 1895–1910, Early Modern 1910–1925 (21-progress:75-79). No era code. |
| `tick` | Tick | tick(s) ×59 (20-economy, 30-events, 40-ai-director +15) | `OnMonthTick`, `HandleMonthTick`, `OnDayTick` ×151 (AIPlaceholderSubsystem.cpp:26, AIPlaceholderSubsystem.h:23) | Matches (dev). |
| `game_speed` | Speed | speed ×3 (41-military, 11-units); velocidade ×5 (00-overview, 02-time, 31-politics +2); Slow/Normal/Fast/Fastest ×3 (etapa-1-editor-setup) | `ETimeSpeed`, `CurrentSpeed`, `NewSpeed` ×79 (BattleCameraPawn.cpp:46, BattleCameraPawn.h:32) | Code `ETimeSpeed`: Paused, Slow (1x), Normal (2x), Fast (4x), Fastest (8x) — four speeds, not 1–5 (TimeSpeed.h:9-13). |
| `pause` | Pause | pause/paused ×15 (01-game-flow, 02-time, 99-implementation-roadmap +4); pausa/pausar/pausado ×30 (01-game-flow, 30-events, 02-time +5) | `Paused`, `LastNonPausedSpeed`, `PauseGame` ×27 (GameFlowSubsystem.cpp:25, TimeSubsystem.cpp:9) | Matches. |
| `soft_end` | Continue Playing | GameOver ×6 (01-game-flow, docs/README, 00-overview +1); vitória/victory ×18 (21-progress, docs/README, 00-overview +5) | `DefenderVictory`, `AttackerVictory`, `GameOver` ×21 (BattleResolverService.cpp:250, BattleSubsystem.cpp:539) | Code: `EGameFlowState::GameOver` "Game Over"; docs: victory conditions (21-progress §9). No soft end. |
| `campaign` | Campaign | campaign ×1 (31-politics); campanha ×7 (40-ai-director, 41-military, 99-implementation-roadmap +1); partida ×1 (etapa-2-events-content) | `GetTotalCampaigns` ×2 (StrategosCommanderCardWidget.cpp:130, StrategosCommanderCardWidget.h:95) | "campanha" = military/election campaign; `GetTotalCampaigns` = commander's campaigns. "partida" ×1 = playthrough (etapa-2-events-content). See C11. |

#### Meta

| ID | EN (from 82) | Docs say (word ×count, files) | Code says (identifiers, file:line) | Note |
|---|---|---|---|---|
| `autosave` | Autosave | not found (autosave) | not found (`\b\w*Auto[Ss]ave\w*\b`) | Absent. |
| `game_title` | Le Grand Strategos | Le Grand Strategos ×9 (CLAUDE, docs/README, README +4) | `LeGrandStrategos`, `LeGrandStrategosTarget`, `LeGrandStrategosEditorTarget` ×6 (DefaultEngine.ini:7, LeGrandStrategos.Target.cs:4) | Matches. |

#### Interface

| ID | EN (from 82) | Docs say (word ×count, files) | Code says (identifiers, file:line) | Note |
|---|---|---|---|---|
| `outliner` | Outliner | outliner ×1 (proporcoes-de-arte) | `UStrategosRightOutlinerWidget`, `Outliner`, `EOutlinerTab` ×68 (StrategosRightOutlinerWidget.cpp:1, StrategosRightOutlinerWidget.h:6) | Matches (`UStrategosRightOutlinerWidget`). |
| `journal` | Journal | not found as UI (diário ×5 all mean "daily") | `Diary`, `"Diário"` ×7 (StrategosRightOutlinerWidget.cpp:214, StrategosRightOutlinerWidget.h:15) | Code: `EOutlinerTab::Diary` "Diário" (StrategosRightOutlinerWidget.h:15). All doc "diário" hits mean "daily". |
| `gazette` | Gazette | not found (gazette/gazeta \| jornal) | not found (`\b\w*Gazette\w*\b`) | Absent. |
| `alert` | Alert | alerta(s) ×6 (20-economy, 21-progress, 31-politics +2) | `"alert"` category string (StrategosRightOutlinerWidget.h:79); `FOutlinerNotificationRow` (…h:71); `EEventType::Notification` | Outliner row category `"alert"` (StrategosRightOutlinerWidget.h:79); notifications (`OnNotificationFired`). |
| `map_mode` | Map Mode | modo(s) de mapa ×2 (98-estado-implementacao, 99-implementation-roadmap) | not found (`\b\w*MapMode\w*\b`) | Planned only: 99-roadmap:123; 98 says "sem modos de mapa" (98:35). |
| `mapmode_political` | Political | modo político / político (map mode) ×1 (99-implementation-roadmap) | not found (`\b\w*(?:MapMode::Political\|PoliticalMap)\w*\b`) | Roadmap list: político, econômico, militar, religioso, cultural, ferroviário (99-roadmap:123). |
| `mapmode_interest_groups` | Interest Groups | not found (base política / political base) | not found (`\b\w*PoliticalBase\w*\b`) | Absent. |
| `mapmode_steel_tier` | Steel Tier | not found (steel tier map mode) | not found (`\b\w*SteelTier\w*\b`) | Absent. |
| `mapmode_market` | Market | modo econômico (map mode) ×1 (99-implementation-roadmap) | not found | Roadmap "econômico" map mode only. |
| `mapmode_deposits` | Deposits | not found (deposits map mode) | not found (`\b\w*DepositMap\w*\b`) | Absent. |
| `mapmode_population` | Population | not found (population map mode) | not found (`\b\w*PopulationMap\w*\b`) | Absent. |
| `mapmode_military` | Military | militar (map mode list) ×1 (99-implementation-roadmap) | not found (`\b\w*MilitaryMap\w*\b`) | Roadmap "militar" map mode only. |
| `panel_nation` | Nation | country card / carta de país ×2 (98-estado-implementacao, proporcoes-de-arte) | `CountryCard`, `UStrategosCountryCardWidget`, `StrategosCountryCardWidget` ×52 (StrategosCountryCardWidget.cpp:1, StrategosCountryCardWidget.h:6) | Code: **Country Card** modal (`UStrategosCountryCardWidget`). |
| `panel_politics` | Politics | painel de leis / politics panel ×1 (31-politics) | not found (`\b\w*PoliticsPanel\w*\b`) | Doc 31 polish: "painel de leis" (31-politics:812). |
| `panel_economy` | Economy | painel(éis) de mercado / economy panel ×1 (20-economy) | not found (`\b\w*EconomyPanel\w*\b`) | Doc 20 polish: "painéis de mercado" (20-economy:629). |
| `panel_society` | Society | not found as a panel (society/sociedade ×4 generic) | not found (`\b\w*(?:Society\|SocietyPanel)\w*\b`) | Doc hits are generic "sociedade". |
| `panel_military` | Military | war room ×2 (98-estado-implementacao, proporcoes-de-arte) | `WarRoom`, `UStrategosWarRoomWidget`, `EWarRoomTab` ×44 (StrategosWarRoomWidget.cpp:1, StrategosWarRoomWidget.h:5) | Code: **War Room** modal, tabs Visão Geral / Objetivos / Batalhas (StrategosWarRoomWidget.h:12-14). |
| `tooltip_lock` | Lock | not found (fixa*/fixar hits mean "fixed") | not found (`\b\w*(?:TooltipLock\|LockTooltip\|PinTooltip)\w*\b`) | Absent ("fixa*" hits mean "fixed"). |
| `designer_mode` | Designer Mode | debug UI / painel de debug ×7 (etapa-2-events-content, 40-ai-director) | `UEventDebugSubsystem`, `ToggleDebugUI`, `RefreshDebugUI` ×43 (EventDebugSubsystem.cpp:1, EventDebugSubsystem.h:6) | Code: Event Debug overlay (`UEventDebugSubsystem`, `ToggleDebugUI`); AI debug panel planned (40-ai-director:744). |
| `ui_rail` | Rail | left rail/trilho ×2 (proporcoes-de-arte) | `LeftRailWidth` (StrategosStyleTokens.h:31); `Railway` is unrelated | "Left rail (trilho esquerdo)" width 56 (proporcoes-de-arte:46); `LeftRailWidth` (StrategosStyleTokens.h:31). |
| `ui_system_panel` | System Panel | painel lateral ×1 (etapa-1-editor-setup) | not found (`\b\w*SystemPanel\w*\b`) | Absent; "painel lateral" for selected province (etapa-1-editor-setup:150). |
| `ui_selection_dock` | Selection Dock | dock / province dock ×2 (proporcoes-de-arte) | `UStrategosProvinceDockWidget`, `ProvinceDock`, `EProvinceDockTab` ×58 (StrategosProvinceDockWidget.cpp:1, StrategosProvinceDockWidget.h:5) | Code: **Province Dock** 760×560 (proporcoes-de-arte:49; `UStrategosProvinceDockWidget`). |
| `ui_template` | Template | not found as UI (template ×4 = worldgen Template param) | not found (`EWorldTemplate` is worldgen) | Absent as a UI concept ("Template" = worldgen `EWorldTemplate`). |
| `ui_skin` | Skin | style tokens ×3 (proporcoes-de-arte) | `StrategosStyle` namespace (StrategosStyleTokens.h:19) | Code: `namespace StrategosStyle` tokens (StrategosStyleTokens.h:19); docs "style tokens". |

#### World and names

| ID | EN (from 82) | Docs say (word ×count, files) | Code says (identifiers, file:line) | Note |
|---|---|---|---|---|
| `nation` | Nation | nation(s) ×81 (21-progress, etapa-2-ui-grid, 11-units +11); nação/nações ×152 (40-ai-director, 30-events, 32-diplomacy +17); country/país ×4 (etapa-2-events-content, 40-ai-director, proporcoes-de-arte) | `Nation`, `UNation`, `NationId` ×1040 (AIPlaceholderSubsystem.cpp:4, AIPlaceholderSubsystem.h:8); `CountryCard`, `UStrategosCountryCardWidget`, `FCountryStatRow` ×61 (StrategosCountryCardWidget.cpp:1, StrategosCountryCardWidget.h:6) | Matches; modal says Country ("Country Card"). |
| `province` | Province | province(s) ×23 (etapa-1-editor-setup, 00-overview, 30-events +9); província(s) ×92 (41-military, 20-economy, sessao-1-terreno-provincia +17) | `UProvince`, `ProvinceId`, `Province` ×768 (AIPlaceholderSubsystem.cpp:5, BattleResolverService.cpp:181) | Matches. |
| `nation_albion` | Albion | albion ×10 (etapa-1-editor-setup, etapa-2-events-content, proporcoes-de-arte +2) | `Albion`, `"Albion"`, `AlbionCenter` ×21 (WorldBootstrapper.cpp:340, EventDebugSubsystem.cpp:173) | Matches. No demonym anywhere. |
| `nation_galia` | Galia | galia ×7 (etapa-1-editor-setup, proporcoes-de-arte, CLAUDE +2) | `Galia`, `GaliaCenter`, `GaliaSouth` ×19 (WorldBootstrapper.cpp:358, WorldBootstrapper.h:16) | Matches. No demonym; "galianos" ×0. |
| `nation_norden` | Norden | norden ×6 (etapa-1-editor-setup, proporcoes-de-arte, CLAUDE +1) | `Norden`, `NordenCenter`, `NordenSouth` ×15 (WorldBootstrapper.cpp:363, WorldBootstrapper.h:16) | Matches. No demonym. |

---

### 2. Concepts not in 82 (player-visible)

Shown to the player or meant for a player screen in the docs or code, and with no row in 82 §5. One line each.

#### Economy and trade
| Doc / file | Word used | Meaning |
|---|---|---|
| `Treasury.h:53`, `TaxLevel.h:7-13` | Tax Level (`ETaxLevel` Very Low 0.5x … Very High 2.0x), per stratum | Tax rate set per stratum. 82 left taxes out "because no doc defines them", but the code defines them. |
| `Treasury.h:43-49`, `StrategosHUDWidget.h` `GetDebtBalance` | Debt, Interest (`DebtBalance`, `AnnualInterestRate`, `Expense_DebtInterest`) | State debt and interest. Shown in the HUD. |
| `Treasury.h:34-40` | Maintenance, Army Upkeep, Admin Cost | Expense lines. |
| `Treasury.h:31` | State Profits (`Income_StateProfits`) | Income from state-owned buildings. |
| `EconomySubsystem.h` `OnBankruptcyImminent` | Bankruptcy | Fires when the treasury is about to run out. |
| `PopGroup.h:38`, `ProductionMethodAsset.h:23`, ProvinceDock Wage column | Wage | Pay per worker. 82 left wages out on purpose. |
| `MarketDestination.h:18-21` | State Stockpile, Local Market, Global Market, Private Warehouse | Four destinations a building's output can go to. |
| `PopDemandOrder.h:17` | City Warehouse | A source for pop demand. |
| `NationalStockpile.h`, HUD `GetGoodStock` | National Stockpile / Stock | The state's stock of goods. |
| `TradeHub.h:21` | Trade Hub | Regional node that links provinces to the Global Market. |
| `LogisticsInfrastructure.h:20-25` | Dirt Road, Paved Road, Railway, River Port, Sea Port, Canal | Infrastructure types. |
| `GoodConsumerProfile.h:7`, `LogisticsInfrastructure.h:52` | Bandwidth ("Largura de Banda") | Logistics throughput of a province. |
| `GoodAsset.h:20-23` | Good categories Staple, Industrial, Luxury | 82 uses Raw / Intermediate / Final instead. |
| `GoodAsset.h:50` | Good Tier 0–2 | How many processing steps a good is from raw. |
| `40-resources:101` | Nation tech tier: Underdeveloped / Developing / Developed | Gates which production methods a nation can use. |
| `BuildingOwnerKind.h:14` | Building categories: Farm, Mine, Plantation, Forester, Workshop, Factory, Refinery | Building type groups. |
| `BuildingState.h:22-26` | Under Construction, Operational, Survival Mode, Ruined, Nationalized | Building states. |
| `BuildingOwnerKind.h:9-10` | Ownership: Government / Private (Bourgeoisie) | Who owns a building. |
| `ProductionModifierAsset.h`, `etapa-2-economy-content:95-107` | Production Modifier (Push for Output, Conservative, Skilled/Cheap Labor, Premium, Volume, Modernization Drive), mutex groups Pace / Labor / Quality | Policy toggles the player sets per building. |
| `GoodConsumerProfile.h:52-99` | Luxury access, Addiction, Withdrawal | The "opium effect" on pops. |
| `GoodConsumerProfile.h:39-46` | Top/Fifth Producer Prestige | Prestige for being a leading producer. |
| `StrategicIndices.h:24-32`, CountryCard "Cap. Militar / Cap. Industrial / Moral" | Military Readiness, Industrial Capacity, Civilian Morale | Three national indices, 0.5–1.5. |
| `20-economy:134-141` | Budget lines (Military, Navy, Education, Administration, Construction, Subsidies, DebtService, Diplomacy), Credit Rating | Budget allocation. |
| `20-economy` §8 | Trade Agreement, Trade Route, Embargo, Naval Blockade | Trade between markets. |
| ProvinceDock `"Sugestão: explorar {0} (pot. {1}%)"` (`.cpp:211`) | Raw resource potential | Per-province yield potential of a raw good. |

#### Province and geography
| Doc / file | Word used | Meaning |
|---|---|---|
| `ProvinceGeographyTypes.h:23-55`, `40-resources:46-56` | Topography, Climate Zone, Vegetation Cover, Hydrography (river, lake, aquifer, coastal) | Static province geography. The display names are in PT. |
| `TerrainType.h`, `ClimateType.h`, `WaterAccessType.h` | Terrain (Plains…Water), Climate (Freezing…Tropical), Water Access | A second, older geography vocabulary. |
| `ResourceScope.h:18-35` | Resource Scope (Vegetal/Mineral/Aquifer), Principal / Alternative Resource | Classes of deposit-like sources. |
| `ProvinceGeography.h:42` | Fertility | Province fertility. |
| `40-resources:70` | Development (DevelopmentBonus) | Province development, which adds slots. |
| ProvinceDock `GetProvinceStability`, `GetProvinceMilitancy` | Stability, Militancy (province) | Shown in the province dock. |

#### Population
| Doc / file | Word used | Meaning |
|---|---|---|
| `etapa-2-economy-content:62-90` | Engineer | A profession used in PM staffing. Not in `EPopStratum` and not in 82. |
| `20-economy:39` | Slave, Bureaucrat, Clergyman, Farmer, Worker | Pop types in the old doc. |
| `PopGroup.h:29`, `20-economy:42` | Wealth | Wealth the pop has saved up. |
| `PopGroup.h:32` | Loyalty ("Lealdade") | Pop loyalty to the state. |
| `20-economy:44-45` | Political Awareness, Militancy | Pop political state. |
| `PopMobilityTracker.h:24` | Quality of Life points | What drives promotion and demotion. |
| `20-economy:72-73` | Infrastructure, Urbanization | Province fields. |
| `20-economy:182-186` | Life / Everyday / Luxury Needs | The needs ladder 82 is waiting on. |

#### Politics
| Doc / file | Word used | Meaning |
|---|---|---|
| `31-politics:63-102` | Ideology (Reactionary, Conservative, Liberal, Radical, Socialist, Communist, Anarchist, Nationalist, Fascist) | A pop's distribution across ideologies. |
| `31-politics:225-280`, CountryCard `:92-95` | Government type (Absolute / Constitutional / Military Monarchy, Census Republic) | Shown on the Country Card. |
| `31-politics:284-360` | Party, Parliament, Ruling Party, Upper House | Party politics. |
| `31-politics:461-510` | Election, Suffrage, Electoral Fraud | Elections. |
| `31-politics:512-545` | Faction (Court, Military, Religious, Industrial, Labor, Nationalist, Colonial, Reformist), Faction Demand, Coup | The old stand-in for IGs. |
| `31-politics:564-685` | Militancy, Revolt, Repression, Concession | The instability loop. |
| `31-politics:180-193` | Law categories Suffrage, Slavery, Trade, Taxation, Education, Press, Colonial | Categories that 82 does not have. |
| EventModal "-10 Estabilidade" (`StrategosEventModalWidget.h:20`) | Stability (national) | Effect of events. |

#### Diplomacy
| Doc / file | Word used | Meaning |
|---|---|---|
| `DiplomaticStatus.h:14-17`, CountryCard `:204-207` | Peace, Non-Aggression Pact, Alliance, War, Truce ("Trégua") | Diplomatic status. Truce is in the UI but missing from the enum. |
| `DiplomaticRelation.h:57`, DiploAction `:188` | Opinion | Bilateral opinion. |
| DiploAction `:27-58` | Propose Alliance, Propose NAP, Trade Embargo, Diplomatic Gift, Military Passage, Fabricate CB, Declare War (no CB) | Diplomatic actions. |
| DiploAction `:226-228` | Likely accept / Uncertain / Likely refuse | AI verdict. |
| `32-diplomacy:195-286`, WarRoom `:50` | Casus Belli (CB) | Justification for war. |
| `32-diplomacy:379-471`, DiploAction `.h:30` | Prestige, Great Power, Infamy, World Opinion | Reputation. |
| `32-diplomacy:328` | Sphere of Influence | Diplomatic sphere. |
| `32-diplomacy:108-190` | Treaty, Clause | Treaties. |
| `32-diplomacy:496-505` | War Score, War Exhaustion | War state. |
| WarRoom `:137-143` | Peace terms: Annex Capital, Reparations (small/large), Liberate Region, War Guilt Clause, Demilitarize Border, White Peace | Peace negotiation. |

#### Military and battle
| Doc / file | Word used | Meaning |
|---|---|---|
| `ArmyStats.h:18-29`, `etapa-2-ui-grid:141-153` | Unit stats ATQ, DEF, MOB, MOR, ORG, SUP, ALC, PREC, SUPR, REC, CST | PT abbreviations shown on unit cards. They conflict with §4.11 "no short labels". |
| `ArmyStats.h:35-39` | Unit state Ready / In Combat / Damaged / Disorganized / Retreated | Shown on the portrait. |
| ArmyCard `.h:82`, `11-units:191-211` | Experience, XP Level, Experience Tier (Green…Legendary) | Unit growth. |
| ArmyCard `GetPrimaryTrait`, CommanderCard `:114` | Trait (e.g. "Veterano") | Unit and commander traits. |
| CommanderCard `:100-103` | Commander stats Tactics, Charisma, Logistics, Boldness | Commander card. |
| `LeaderArchetype.h:21-26` | Leader Archetype (Militarist, Diplomat, Merchant, Religious, Intellectual, Pragmatist) | National leader personality. |
| `UnitTypeAsset.h:39,60` | Role, Variants | Unit type data. |
| `41-military:59-195` | Regiment, Fleet, Posture, Stance | Military structure. |
| `41-military:325-420` | Supply, Attrition | Logistics. |
| `41-military:479-602` | Siege, Mobilization, Front, Mercenaries | Military processes. |
| `BattleTypes.h:13-129` | Battle type (Direct Clash, Border Skirmish, Raid, Siege, Ranged Harass), Phase (Setup, Engagement, Climax, Pursuit, Resolved), Position (Frontline, Flank, Rear, High Ground, Crossing), Outcome (Attacker/Defender Victory, Stalemate), Weather, Battle Terrain, Battle Stance (Aggressive, Hold, Skirmish, Reserve) | Tactical battle. The display names are in PT. |
| `BattleTypes.h:84-90` | Regiment types Infantry, Cavalry, Archer, Pike, Artillery | Archer and Pike are anachronistic. |
| `BattleTypes.h:107-121`, `10-battle:195-296` | Battle Card, Card category (Maneuver, Assault, Support, Stratagem, Reaction), Timing, Deck, Initiative, Round | The card system. |
| `10-battle:356` | Rout, Capture | Battle outcomes per side. |

#### Technology, events, time
| Doc / file | Word used | Meaning |
|---|---|---|
| `TechTrack.h:15-16` | Tech Track: Civil Flow / State Research | Two research channels. |
| `21-progress:81-147,216-310` | Tech Category, Tech Node, Research Points, Catch-up, Frontier, Espionage | The research model. |
| `21-progress:472-560` | Victory, Victory Conditions (e.g. Industrial victory) | Win conditions. |
| `EventType.h:20-22` | Event type Decision / Notification / Silent | Event kinds. |
| `EventRepeatPolicy.h`, `30-events` | MTTH, Event Chain, Choice, Persistent Modifier | Event engine terms (MTTH is dev). |
| `TimeSpeed.h:10-13` | Slow (1x), Normal (2x), Fast (4x), Fastest (8x) | Speed labels. |
| `WorldBootstrapAsset.h:17` | Scenario (Sandbox 1836) | Start setup. |

#### Interface and world
| Doc / file | Word used | Meaning |
|---|---|---|
| `StrategosCountryCardWidget.h` | Country Card | Nation modal (≈ panel F1). |
| `StrategosWarRoomWidget.h:12-14` | War Room, tabs Overview / Objectives / Battles | Military modal. |
| `StrategosWarRoomWidget.h:151` | Peace Negotiation | Modal. |
| `StrategosCommanderCardWidget.h`, `StrategosDiplomacyActionWidget.h`, `StrategosEventModalWidget.h` | Commander Card, Diplomacy Action, Event Modal | Modals. |
| `StrategosArmyCardWidget.h:14-15`, `etapa-2-ui-grid:34-135` | Army Card Compact / Micro / Detailed, Customization Panel (Armament/Equipment/Doctrine/Training/Appearance) | Unit cards. |
| `StrategosProvinceDockWidget.h:18-21` | Province Dock tabs Overview / Buildings / Population / Information | The selection dock. |
| `StrategosRightOutlinerWidget.h:14-15,79` | Outliner tabs Outline / Diary; row categories alert / diplo / econ / pol | The `pol`/`econ` short labels conflict with §4.11. |
| `etapa-2-ui-grid:159-176`, `Nation.h:34-38` | Flag, Coat of Arms, Primary/Secondary Color | National identity. |
| `Nation.h:49`, WarRoom "Anexar Capital" | Capital (province) | Capital province. |
| `Nation.h:62` | National Ideas | Nation traits. |
| `StrategosMainMenuWidget.h` | Main Menu, Save/Load slots | Menu. |
| `EventEffects.cpp:97`, `EventSubsystem.cpp:247`, DiploAction `:43` | Gold / "Ouro" / "g" / "£" | Currency. It has four notations and no term. |

---

### 3. New collisions (not covered by 82 §4)

| # | Kind | Word(s) | Concepts | Evidence |
|---|---|---|---|---|
| C1 | one word, many concepts | **Tier** | PM tier 0–5, good tier 0–2 (processing depth), nation tech tier (Developed…), needs tiers (Life/Everyday/Luxury), XP tier, AI complexity tier | `ProductionMethodAsset.h:53`, `GoodAsset.h:50`, `40-resources:101`, `20-economy:180`, `11-units:204`, `40-ai-director:586`. §4.1 covers only Tier vs building Level. |
| C2 | one word, many concepts | **Level** | Building level, law rung (`ProgressionLevel`, "Level 0 (Nenhum)"), XP level, infrastructure level, tax level | `31-politics:156,206-218`, `BuildingTypeAsset.h:52`, ArmyCard `.h:82`, `LogisticsInfrastructure.h:44`, `TaxLevel.h`. Law rungs as "Level" break §4.1. |
| C3 | one word, two concepts | **Supply** / Suprimento | Market supply of a good vs military supply (SUP stat, `USupplyNetwork`, attrition) | `NationalStockpile.h:26` vs `ArmyStats.h:23`, `41-military:325`. |
| C4 | one word, two concepts | **Stratum** | 82: Lower/Middle/Upper. Code: `EPopStratum` = the profession list (Laborer…Clergy). UI also references nonexistent `Lower/Middle/Upper`. | `PopStratum.h:15`, `CLAUDE:127`, `StrategosProvinceDockWidget.cpp:234`. |
| C5 | one word, two concepts | **Morale** / "Moral" | Unit morale (MOR) vs national Civilian Morale index (Country Card label "Moral") | `ArmyStats.h:21` vs `StrategosCountryCardWidget.cpp:148`, `StrategicIndices.h:28`. |
| C6 | one word, two concepts | **Modifier** | Named effect (82) vs player-chosen building policy `UProductionModifierAsset` with mutex groups | `ProductionModifierAsset.h`, `CLAUDE:41,129`. |
| C7 | one word, two concepts | **Throughput** | Building output multiplier (≈ Efficiency) vs Trade Hub capacity (≈ Trade Capacity) | `ProductionModifierAsset.h:42` vs `TradeHub.h:43`. |
| C8 | one word, two concepts | **Balance** | 82: income − expenses. Code: `FTreasury::Balance` = money in the treasury. | `Treasury.h:21`, `StrategosHUDWidget.h:100`. |
| C9 | two words, one concept | **Loyalty / "Satisfação" / "humor"** | The same pop value is named Loyalty in code, shown as "Lealdade" in event text and drawn as a "Satisfação"/mood bar | `PopGroup.h:32`, `EventEffects.cpp:107`, `StrategosProvinceDockWidget.h:50-51`, `.cpp:245`. |
| C10 | one word, two concepts | **Sphere / Esfera** | Tech Sphere (`ETechSphere`) vs Sphere of Influence (diplomacy) | `TechSphere.h:10` vs `32-diplomacy:328`, `CLAUDE:106`. |
| C11 | one word, two concepts | **Campaign / Campanha** | Playthrough (82) vs a commander's military campaigns (`GetTotalCampaigns`) vs an election campaign phase | `StrategosCommanderCardWidget.h:95`, `31-politics:470`, `41-military`. |
| C12 | one word, two concepts | **Capital** | Political Capital vs capital province ("Anexar Capital", `CapitalProvinceId`) | `Nation.h:49`, `StrategosWarRoomWidget.cpp:137`, `31-politics:364`. 82's mockups already count "capital ×20". |
| C13 | one word, two concepts | **Reserve / Reserva** | Retired manpower word (82) vs battle stance Reserve "Reserva" vs State Stockpile ("reserva estratégica") | `BattleTypes.h:99`, `MarketDestination.h:10`. |
| C14 | one word, two concepts | **Labor** | Law category Labor vs PM modifier mutex group "Labor" vs faction type Labor | `CLAUDE:96`, `etapa-2-economy-content:103-104`, `31-politics:524`. |
| C15 | one word, two concepts | **Doctrine / Doutrina** | Military doctrine (`UDoctrineAsset`) vs AI "Doutrina Nacional" (strategy theme: Industrializer, Conqueror…) | `11-units:155` vs `40-ai-director:153`. |
| C16 | one word, two concepts | **Stance** | 82 dev term for IG law preference (`FLawStance`) vs army stance, battle stance (`EBattleStance`), faction stance | `BattleTypes.h:94`, `41-military:100`, `31-politics:453`. |
| C17 | one word, two concepts | **Government** | Building owner "Government" vs government type vs law category Government | `BuildingOwnerKind.h:9`, `31-politics:182,225`. |
| C18 | two words, one concept (and the reverse) | **Gold / Ouro / g / £**; **Diário** | Currency has four notations and no term. "Diário" is both the Journal tab and the everyday word for "daily" (every doc hit is "daily"). | `EventEffects.cpp:97`, `EventSubsystem.cpp:247`, `StrategosCountryCardWidget.cpp:143`; `StrategosRightOutlinerWidget.h:15`. |

Also seen, and close to existing §4 items:
- **§4.2 extended.** The profession Laborer is displayed as "Trabalhadores" (`EventEffects.cpp:113`), which is the PT word 82 gives to IG Workers.
- **§4.9 extended.** "Military" and "Economy" are also tech sphere names (`TechSphere.h:14-15`) and good-category names in doc 20. Neither is on the §3 shared-names list.
- **§4.4 extended.** Doc 31 calls enactment **Reform**. 82 has no row or retired variant for "reform".
- **"Industrial"** is a good category, an index (`IndustrialCapacityIndex`), a victory type (`21-progress:541`), a faction type and part of an era name.

---

### 4. Retired words still present

Counted over all docs and code (`.md .h .cpp .cs .ini .uproject`, `ThirdParty/` left out). Lines are in parentheses.

| Word | Total | Per file |
|---|---|---|
| votação | 0 | — |
| vote (vote/votes/voting) | 4 | `docs/architecture/31-politics.md` ×4 (474, 495, 501, 502). Election votes only, not enactment. |
| satisfação | 5 | `20-economy.md` ×4 (329, 332, 572, 620); `Source/StrategosUI/Public/HUD/StrategosProvinceDockWidget.h` ×1 (50) |
| satisfaction (including identifiers) | 15 | `20-economy.md` ×6 (52, 320, 495, 496); `31-politics.md` ×3 (576-578); `GoodConsumerProfile.h` ×4 (53, 65, 71, 90); `StrategosProvinceDockWidget.h` ×1 (51); `StrategosProvinceDockWidget.cpp` ×1 (245) |
| classe social | 1 | `CLAUDE.md` ×1 (127) |
| social class | 0 | — |
| déficit / deficit | 4 | `41-military.md` ×3 (393, 394, 397): **army supply**, neither goods nor money. `EconomySubsystem.h` ×1 (55) "POPs comem; deficit derruba loyalty": **goods** (pop consumption). Money: 0. |
| recurso(s) | 39 | `40-resources-and-production.md` ×14; `98-estado-implementacao.md` ×4; `_plano-sessoes-recursos.md` ×4; `sessao-1-terreno-provincia.md` ×3; `00-overview.md`, `10-battle.md`, `41-military.md` ×1 each; code comments: `Province.h` ×2, `ProvinceGeographyTypes.h` ×2, `GoodAsset.h`, `ProductionMethodAsset.h`, `ProvinceGeography.h`, `ResourceScope.h`, `StrategosSaveData.h`, `StrategosProvinceDockWidget.h`, `ProvinceGeographyClassifier.h` ×1 each. Used as a player term for raw goods and deposits. |
| resource(s), whole word | 12 | `ResourceScope.h` ×3 (7, 34, 35: DisplayNames "Principal Resource", "Alternative Resource"); `40-resources…md` ×2; `EconomySubsystem.h` ×1 (31: "province lacks required raw resource"); `EconomySubsystem.cpp`, `ProvinceGeography.h`, `StrategosProvinceDockWidget.cpp`, `_plano…`, `sessao-1…`, `etapa-2-economy-content.md` ×1 each |
| `Resource` inside identifiers | 75 | `WorldBootstrapper.cpp` ×16, `EconomySubsystem.cpp` ×13, `RGOTemplateAsset.h` ×6, `ResourceScope.h` ×5, `StrategosProvinceDockWidget.cpp` ×5, `SaveSubsystem.cpp` ×4, `BuildingTypeAsset.h` ×4, `ProductionMethodAsset.h` ×3, and others (see §5) |
| auto-investment / auto-invest* | 10 | `40-resources-and-production.md` ×3 (13, 37, 156); `CLAUDE.md` ×1 (95); `98-estado-implementacao.md` ×1 (53); `sessao-1…md` ×1 (9); `etapa-2-economy-content.md` ×1 (184); `EconomySubsystem.h` ×1 (143); `EconomySubsystem.cpp` ×1 (1127); `PopGroup.h` ×1 (11) |
| `AutoInvest` inside identifiers | 4 | `EconomySubsystem.cpp` ×2 (227, 1163); `EconomySubsystem.h` ×1 (144); `etapa-2-economy-content.md` ×1 (180) |
| auto-investimento | 2 (subset of auto-invest*) | `40-resources-and-production.md` (13); `EconomySubsystem.h` (143) |
| molde | 0 | — |
| galianos | 0 | — (no demonyms at all) |
| "Chance to Pass" | 0 | — |
| NAÇ, LEI, ECO, SOC, EXÉ, CLE (UPPER case) | 0 each | — (the code's short labels are `pol`/`econ`/`diplo`/`alert`, `StrategosRightOutlinerWidget.h:79`, and ATQ/DEF/MOB/MOR…) |
| 1820 | 6 | `CLAUDE.md` (9); `40-resources-and-production.md` ×2 (15, 118); `21-progress.md` (75); `sessao-1-terreno-provincia.md` (9); `ProvinceGeographyTypes.h` (33) |
| 42-depositos | 0 | — |
| 40-resources | 2 | `docs/prompts/_plano-sessoes-recursos.md` (5); `docs/prompts/sessao-1-terreno-provincia.md` (9). File `docs/architecture/40-resources-and-production.md` exists, and so does a second `40-ai-director.md`, so number 40 is used twice. |

**Total: 177 hits** (4 + 5 + 15 + 1 + 4 + 39 + 12 + 75 + 10 + 4 + 6 + 2). Without identifier substrings it is 102.

Other retired variants from 82 §5, checked in passing:
- `mobility`: `FPopMobilityTracker`, plus 3 doc hits.
- `reserva`: `EBattleStance::Reserve` "Reserva".
- `partida`: 1 (`etapa-2-events-content`), plus an `EventRepeatPolicy.h` comment.
- `influência`: about 25, mostly Sphere of Influence.
- `oculto`, `conhecido`, `customs`, `alfândega`, `convocação`, `resolução automática` ×2 (`10-battle`), `Labour`, and British spellings (`Colour`, `Centre`, `-ise`): 0, except resolução automática ×2.
- `PM` in player text: 0 in code. Docs use it as a dev abbreviation.

---

### 5. Code identifiers with a retired or Portuguese word (no renames done)

#### Retired words
| File:line | Identifier | 82 term it maps to |
|---|---|---|
| `Source/StrategosCore/Public/Economy/ResourceScope.h` (file), `:18` | `ResourceScope.h`, `EResourceScope` (Vegetal/Mineral/Aquifer) | `deposit` (kind of deposit) |
| `ResourceScope.h:32-35` | `EResourceRole::Principal/Alternative`, DisplayNames "Principal Resource" / "Alternative Resource" | `deposit` |
| `ProvinceGeography.h:46` | `PrincipalResourceId` | `deposit` |
| `Province.h:62` | `RawResourcePotential` | `deposit` (yield) / `good_raw` |
| `BuildingTypeAsset.h:55,58` | `bRequiresRawResource`, `RequiredResourceId` | `deposit` |
| `ProductionMethodAsset.h:74,77` | `bRequiresRawResource`, `RawResourceGoodId` | `deposit` / `good_raw` |
| `EconomySubsystem.h:31` | `EBuildResult::Rejected_NoRawResource` | `deposit` |
| `StrategosProvinceDockWidget.h:142,145` | `GetRawResourceIds`, `GetRawResourcePotential` | `deposit` |
| `RGOTemplateAsset.h` (file) | `URGOTemplateAsset`, `FRGOYieldRule` (RGO = Victoria "resource gathering operation") | `deposit` |
| `EconomySubsystem.h:144` | `RunBourgeoisieAutoInvestment` | `private_investment` (§4.8 keeps code IDs) |
| `BuildingOwnerKind.h:10` | `EBuildingOwnerKind::Private`, DisplayName "Private (Bourgeoisie)" | `private_investment` |
| `StrategosProvinceDockWidget.h:51` | `FProvincePopRow::Satisfaction` (filled from Loyalty) | `needs_met` / pop loyalty (no row) |
| `GoodConsumerProfile.h:65,90` | `SatisfactionBonus`, `SatisfactionBonusEarly` | `needs_met` |
| `EconomySubsystem.cpp:524` | `bAllSatisfied` | `needs_met` |
| `PopMobilityTracker.h` (file), `:24-42` | `FPopMobilityTracker` | `promotion_demotion` (retired 'mobility') |
| `BattleTypes.h:99` | `EBattleStance::Reserve` "Reserva" | none (retired 'reserva' belongs to `manpower`; a different concept, C13) |
| `StrategosRightOutlinerWidget.h:15` | `EOutlinerTab::Diary` "Diário" | `journal` |
| `PopStratum.h:15` | `EPopStratum` (values are professions) | `profession` (not `stratum`) |
| `PopStratum.h:19` | `EPopStratum::FactoryWorker` | `prof_machinist` |
| `PopStratum.h:20` | `EPopStratum::Bourgeoisie` | `prof_capitalist` |
| `PopStratum.h:21` | `EPopStratum::Aristocracy` | `prof_aristocrat` |
| `Treasury.h:21` | `FTreasury::Balance` | `treasury` (not `balance`) |
| `StrategosHUDWidget.h:17,129` | `FShortfallEntry`, `GetTopShortfalls` | `shortage` |
| `MarketDestination.h:20`, `PopDemandOrder.h:19` | `GlobalMarket` | `world_market` |
| `EventEffects.h:11` | `UEffect_AddGold` "Add Gold" | `treasury` (money; no currency term) |
| `StrategosCountryCardWidget.h` (file) | `UStrategosCountryCardWidget` | `panel_nation` |
| `StrategosWarRoomWidget.h` (file) | `UStrategosWarRoomWidget` | `panel_military` |
| `StrategosProvinceDockWidget.h` (file) | `UStrategosProvinceDockWidget` | `ui_selection_dock` |

#### Portuguese words in identifiers
| File:line | Identifier | 82 term it maps to |
|---|---|---|
| `Source/StrategosCore/Public/World/ArmyStats.h:18-29` | `FArmyStats::ATQ, DEF, MOB, MOR, ORG, SUP, ALC, PREC, SUPR, REC, CST` (PT abbreviations: Ataque, Defesa, Mobilidade, Moral, Organização, Suprimento, Alcance, Precisão, Supressão, Reconhecimento, Custo) | `MOR` → `morale`. The others have no row (unit stats). `CST` ≈ maintenance. |
| `StrategosArmyCardWidget.cpp:92-95` | `TEXT("ATQ")`, `"DEF"`, `"MOB"`, `"MOR"` (labels) | as above. Short labels conflict with §4.11. |
| `Source/StrategosUI/Private/Modals/StrategosCommanderCardWidget.cpp:86-96` | locals `Tatica`, `Carisma`, `Logistica`, `Ousadia` | `commander` (stats; 82 names only "Logistics") |
| `StrategosCommanderCardWidget.cpp:100-103` | NSLOCTEXT keys `Commander/Tatica`, `Carisma`, `Logistica`, `Ousadia` | `commander` |
| `StrategosCountryCardWidget.cpp:92` | NSLOCTEXT key `Gov/Militar` | none (government type) |
| `StrategosRightOutlinerWidget.h:79` | category keys `"alert" / "diplo" / "econ" / "pol"` | `alert`, `panel_economy`, `panel_politics` (short labels, §4.11) |

No file names are in Portuguese. All PT is in identifiers listed above, in comments, and in display strings. PT display strings, which are not identifiers but are where localization will hurt, sit in: `BattleTypes.h` (all UMETA), `BattleEffects.cpp/.h`, `ProvinceGeographyTypes.h`, `DiplomaticStatus.h`, `ArmyStats.h:35-39`, `StrategosWarRoomWidget.h/.cpp`, `StrategosProvinceDockWidget.h/.cpp`, `StrategosRightOutlinerWidget.h`, `StrategosCountryCardWidget.cpp`, `StrategosDiplomacyActionWidget.cpp`, `StrategosCommanderCardWidget.cpp`, `EventEffects.cpp` ("Ouro", "Lealdade", stratum labels), `EventConditions.h`, `EventDebugSubsystem.cpp`. EN display strings sit in `EconomySubsystem.h`, `PopStratum.h`, `MarketDestination.h`, `BuildingState.h`, `TaxLevel.h`, `TimeSpeed.h`, `TechSphere.h` and `EventSubsystem.cpp`. The code mixes both languages, sometimes inside one enum family.

---

## Part C — Rows marked ⚠ RECONCILIAR, M1, Economy v1


Source: `/home/claude/lgs-recon` (Unreal 5.5 / C++ tree). Read-only. All paths below are relative to that root.

### Summary

- **16 items** (14 terms + M1 + Economy v1). **All 16 answered with evidence** (file:line, or the grep that came back empty).
- **3 are "not defined anywhere in this repo":**
  - tariff ranges per Economy law;
  - "concept tier" and the four doctrine axes;
  - a civil/military split mechanic in Core (M1).
- **7 exist only in a design doc or as an unused header, with no working code:**
  - Steel ladder;
  - trade_capacity;
  - needs ladder;
  - literacy;
  - migration;
  - eras;
  - law categories.
- **Fully defined in code:** morale, tech spheres, the stratum enum, and the Economy v1 tick.
- **Docs that do not exist in this repo:** 33 resources, 41 population, 43 deposits, 50/51 technology, 52 laws, 60 battle. `docs/architecture/41-military.md` is the *military* doc, not population. The closest equivalents present are:

  | Topic | Doc present |
  |---|---|
  | resources | `40-resources-and-production.md` (old name of 33) |
  | economy and population | `20-economy.md` |
  | technology | `21-progress.md` |
  | laws | `31-politics.md` |
  | battle | `10-battle.md` |
  | units | `11-units.md` |
  | military | `41-military.md` |

- **Main structural conflict:** `20-economy.md` is an early, aspirational Victoria-style design. Its model differs from the code in four ways:

  | Topic | `20-economy.md` | Code |
  |---|---|---|
  | Pop types | `EPopType`, 11 types | `EPopStratum`, 7 values |
  | Markets | regional `UMarketRegion` | one national stockpile |
  | Tick | weekly + monthly | monthly only |
  | Needs | 3 tiers | 1 hardcoded basket |

  The code also carries unused scaffolding headers that cite a "doc §3–§7". That doc is not in the repo. The scaffolding uses a Local/Global market vocabulary, which contradicts `20-economy.md`. Both are flagged below and not reconciled.

---

### Term table

#### market — what are the market layers called?

- **Code (implemented):** there is one market per nation.
  - `Source/StrategosCore/Public/Economy/NationalStockpile.h:9` reads "v1: tudo nacional (sem mercados regionais)" ("v1: everything national, no regional markets").
  - Prices are computed from `Nation->Stockpile.Demand/Supply` (`Private/Economy/EconomySubsystem.cpp:138-156`).
- **Code (declared, never used; grep shows no references outside its own header):**
  - `EMarketDestination` (`Source/StrategosCore/Public/Economy/MarketDestination.h:16-22`) has four values: `StateStockpile`, `LocalMarket`, `GlobalMarket`, `PrivateWarehouse`.
  - `EDemandFulfillmentSource` (`Public/Economy/PopDemandOrder.h:15-21`) defines a demand cascade: `CityWarehouse → LocalMarket → GlobalMarket → Unmet`.
- **`40-resources-and-production.md`:** names only a "mercado local" (local market), in its list of Economy v1 features (`:13`). It defines no layers.
- **`20-economy.md`:** uses "Mercados Regionais" (`UMarketRegion`, `:203-226`). A nation has at least one market at its capital. Unconnected provinces become a "mercado isolado" (isolated market) (`:230-233`).
- **Ambiguity:** there are three different vocabularies (national stockpile; Local/Global/Warehouse/Stockpile; regional markets). No doc says which one is canonical.

#### world_market — what is the layer outside the nation called?

- **Code scaffolding:** `GlobalMarket`.
  - `MarketDestination.h:12,20` ("escoa via hub de comércio" — flows out through a trade hub).
  - `PopDemandOrder.h:19`, display name "Global Market (Imported)".
  - Access is through `UTradeHub` (`Public/Economy/TradeHub.h:21`; comment at `:9-11` "para alcançar o Mercado Global" — to reach the Global Market).
- **`20-economy.md:205`** rejects a global market outright: "Mercado global = irreal e sem peso espacial" ("a global market is unrealistic and has no spatial weight"). Outside trade is market-to-market via `FTradeRoute` (`:262-289`).
- **Ambiguity:** the code name is "GlobalMarket", but the design doc forbids that concept. Neither is implemented.

#### Steel ladder — Steel production methods and their tiers

- **Doc only.** `docs/architecture/40-resources-and-production.md:106-112`, building `BT_Foundry`, good `G_Steel`. "Tier mínimo" means minimum tier; the doc's efficiency words are baixa / média / alta:

  | Method | Tier mínimo | Inputs | Efficiency |
  |---|---|---|---|
  | `PM_BloomeryIron` | `Underdeveloped` | 1 IronOre + 1 Charcoal | low |
  | `PM_PuddledSteel` | `Developing` | 1 IronOre + 1 Coal | medium |
  | `PM_BessemerSteel` | `Developed` | 1 IronOre + 1 Coal | high ("Bessemer, 1856") |

- The tier enum `ENationTechTier { Underdeveloped, Developing, Developed }` appears only in the doc (`:101`) and in `docs/prompts/_plano-sessoes-recursos.md:22` (Session 5, not done). Grepping `ENationTechTier` / `RequiredTier` in `Source/` finds nothing.
- `:134` gives the same ladder in short form: "Steel (bloomery→puddled→Bessemer)".
- **Code:** no `PM_*` data exists. `UProductionMethodAsset` has `int32 ProductionTier` 0..5 (`Public/Economy/ProductionMethodAsset.h:52-53`). That is a *DAG execution order* (raw → processed → consumer), not a tech tier. There is also a `RequiredTechId` (`:66`).
- **Related but different:** the Iron (not Steel) ladder in the editor-setup doc, `docs/setup/etapa-2-economy-content.md:75-76`:
  - `PM_Smelter_Bloomery`: ProductionTier 1, no tech.
  - `PM_Smelter_BlastFurnace`: ProductionTier 1, tech `Metallurgy`.
  - Both output `Iron`. There is no `G_Steel` among the 12 goods (`:32-45`).
- **Ambiguity:** "Tier" means a nation tech tier in doc 40 and a DAG production tier in code and setup.

#### tariff — are tariff ranges per Economy law defined?

**Not defined anywhere in this repo.**

- There is no "Economy" law category. The closest is `ELawCategory::Trade` in `docs/architecture/31-politics.md:185`, a ladder "Mercantilismo → Protecionismo → Livre Comércio" (mercantilism → protectionism → free trade), with no tariff numbers.
- The only tariff values:
  - `UTradeHub::TariffRate = 0.05f` (`Public/Economy/TradeHub.h:50`, per hub; unused).
  - `FTradeAgreement::TariffRate // 0..1` (`20-economy.md:451`).
  - `FTreasury::Income_Tariffs` (`Public/Economy/Treasury.h:28`). It is reset each month (`EconomySubsystem.cpp:196`) but never credited.
- Grepping `tarif` across docs and Source finds nothing else.

#### trade_capacity — how are imports gated by infrastructure, and what is the quantity called?

- **Code scaffolding (unused):** "Largura de Banda" (bandwidth), struct `FLogisticsBandwidth` (`Public/Economy/LogisticsInfrastructure.h:64`).
  - `IncomingCapacity` (`:69`) is what can arrive at the local market without a "bandwidth overflow" penalty.
  - `OutgoingCapacity` (`:72`) is what can be exported to hubs / `GlobalMarket`.
  - Capacity comes from `FInfrastructureLevel.CapacityPerMonth` (`:55`). `EInfrastructureType` (`:18`) has six values: DirtRoad, PavedRoad, Railway, RiverPort, SeaPort, Canal.
  - Each good has a weight, `FGoodLogisticsProfile::BandwidthCostPerUnit` (`Public/Economy/GoodConsumerProfile.h:14,19`).
  - Each hub has a cap, `UTradeHub::ThroughputPerMonth` (`TradeHub.h:43`).
  - Overflow rule: a share routed to `GlobalMarket` without the needed infrastructure falls back to `LocalMarket` (`MarketDestination.h:32-35`).
- **`20-economy.md`:**
  - `UProvinceEconomy::Infrastructure // 0..1, afeta capacidade de comércio` (affects trade capacity) (`:72`).
  - Below a threshold, the province becomes an isolated market (`:232`).
  - Inter-market volume is capped by `FTradeRoute.Capacity` and reduced by `TransportCost` (`:277-282`).
- **Ambiguity:** the quantity is called "Largura de Banda / Capacity" in code, "Infrastructure" in doc 20, and `Route.Capacity` for routes. No import gate is implemented.

#### professions — full profession list, with each one's stratum

- **No `EPopProfession` exists.** Grepping `Profession|profiss` in Source finds nothing.
- **Code enum `EPopStratum`** (`Public/Economy/PopStratum.h:15-23`):
  - Active: `Laborer`, `Artisan`, `FactoryWorker`, `Bourgeoisie`.
  - Stubs: `Aristocracy`, `Soldier`, `Clergy`.
  - The same split is stated in `CLAUDE.md` (Glossário), `40-resources-and-production.md:15` (which explicitly bans peasants/burghers/nobles/clerics/slaves) and `98-estado-implementacao.md:54`.
  - The stratum *is* the pop type. There is no separate profession → stratum mapping.
- **`20-economy.md:39-40`** lists `EPopType` with 11 values: Aristocrat, Bourgeois, Bureaucrat, Officer, Clergyman, Engineer, Artisan, Worker, Farmer, Soldier, Slave. **No stratum is given for any of them.**
- **Inconsistency:** the setup doc's PMs employ `Engineer` (`etapa-2-economy-content.md:64,76,86,89`), but `Engineer` is not in `EPopStratum`.
- **Profession → stratum mapping: not defined anywhere.**

#### needs — is a needs ladder defined? Levels and names

- **Doc:** `20-economy.md:178-198` defines three tiers:
  - `LifeNeeds` (food, water, shelter). If unmet: the POP dies or migrates.
  - `EverydayNeeds` (clothes, furniture, coffee). If unmet: militancy grows.
  - `LuxuryNeeds` (wine, jewels, tobacco). If unmet: militancy grows and wealth stops growing.
  - They live in `UPopNeedsAsset` per `EPopType`. `31-politics.md:575-578` reuses them as Life/Everyday/Luxury weights for militancy.
- **Code:** no ladder. There is one flat, hardcoded basket per stratum in `EconomySubsystem.cpp:479-495`:

  | Stratum | Basket |
  |---|---|
  | Laborer | Bread 1.0 |
  | Artisan | Bread 1.0, Garments 0.3 |
  | FactoryWorker | Bread 1.0, Garments 0.3 |
  | Bourgeoisie | Bread 1.0, Garments 0.8 |

  The outcome is binary: +0.5% or −2% loyalty (`:524-540`).
  - There is an unused `FGoodLuxuryProfile` (`GoodConsumerProfile.h:57`).

#### literacy and mobility — defined anywhere?

- **Literacy:**
  - Docs: `FPopGroup.Literacy // 0..1` (`20-economy.md:43`); ideology attraction (`31-politics.md:112,128`); research (`21-progress.md:232,375,533`); `UCond_LiteracyAbove` (`30-events.md:247`).
  - Code: only `UTechnologyDataAsset::LiteracyFloor` (`Public/Progress/TechnologyDataAsset.h:48-50`), commented "Usado a partir do TE-2" (used starting at TE-2). The code `FPopGroup` has no literacy field (`Public/Economy/PopGroup.h:17-39`).
- **Mobility:**
  - Social mobility, code: `FPopMobilityTracker` (`Public/Economy/PopMobilityTracker.h:19`). It uses QualityOfLifePoints, PromotionThreshold 100, DemotionThreshold −50, MinMonthsAtThreshold 12, PromotionFraction 0.07. **Unused**: grep finds no references outside its header.
  - Social mobility, doc: `PopPromotion` (`20-economy.md:330,627`).
  - Geographic or labour mobility: **not defined**.
  - "MOB" (`docs/setup/etapa-2-ui-grid.md:145`) is the unit *mobility stat*. It is not related.

#### migration — does any doc or code decide population migration?

- **Not decided in any detail.**
- `20-economy.md` mentions it in four places:
  - monthly phase `3. PopMigration → POPs migram entre províncias (wealth diferencial)` (POPs move between provinces by wealth differential) (`:331`);
  - a performance note "O(N²) … limite a vizinhos" (limit to neighbours) (`:559`);
  - roadmap step 12 (`:626`);
  - as an unmet-LifeNeeds outcome (`:184`).
- `99-implementation-roadmap.md:72` defers "Religião, cultura, migração detalhadas" (detailed religion, culture, migration) to Etapa 4.
- `02-time.md:31` places it on `OnMonth`.
- Code explicitly excludes it: `EconomySubsystem.cpp:279` "PopGrowth (placeholder — … sem migração)" (no migration). `ClimateType.h:10` mentions "penalidades de mortalidade/migração" (mortality/migration penalties) only in a comment.
- `31-politics.md:104-134` "migram entre ideologias" refers to *ideology drift*, not population migration.

#### doctrine vs unit concept — how is doctrine modelled? Concept tier? The four axes?

- **Doctrine** (`docs/architecture/11-units.md:149-187`):
  - A discrete `UDoctrineAsset`. `Scope` is National / Commander / Both (`:161`).
  - It carries conditional `Modifiers`, `GrantedCards` and `ForbiddenCards`.
  - It has mutually exclusive `Conflicts` (`:173`). This is **lateral choice among named doctrines, not a ladder.**
  - Example doctrines (`:181-187`, also `:509`): Linha Napoleônica, Guerra de Movimento, Guerra de Trincheiras, Fogo e Movimento, Élan, Mahaniana, Jeune École.
  - Doctrines are unlocked by tech nodes, `FTechNode.UnlocksDoctrine` (`:298`). Laws can gate them: "Conscrição Universal libera doutrinas de massa" (Universal Conscription unlocks mass doctrines) (`31-politics.md:43`).
  - Not in code: grepping `Doctrine` in Source finds nothing. `98-estado-implementacao.md:147` lists 5-axis unit composition as not done.
- **"Concept tier": not found.** Grep for `concept tier|ConceptTier|concept_tier` found nothing.
- **Four axes: not found.**
  - "Maneuver" appears only as a battle card category (`Source/StrategosBattle/Public/BattleTypes.h:109`; `10-battle.md:210`).
  - "Shock" appears only as a unit role (`11-units.md:47,72,75`).
  - "Attrition" appears only as supply attrition (`41-military.md:397,408`).
  - "Discipline" appears only as an `ENationAttribute` (`21-progress.md:95`) and a UI stat (`11-units.md:483`).
  - "Firepower", "Planning" and "Mobilization & Mass" do not appear as axes.

#### morale — is it defined?

- **Yes, in both docs and code.**
- **Doc:** `10-battle.md:80` "Morale 0..100, abaixo de 25 = rota" (below 25 = rout). §6 "Moral, Rota e Captura" (`:356-385`) lists what lowers and raises it. Battle phase exit at morale < 30 (`:176`). Card effect `UEffect_MoraleShift` (`:271`).
- **Code:**
  - `FRegimentBattleState.Morale = 100` and `MOR = 50` (`Source/StrategosBattle/Public/BattleTypes.h:177,183`).
  - Side morale (`:252`).
  - `RoutThreshold = 25`, `EarlyExitMoraleThresh = 30` (`Public/BattleSubsystem.h:164,166`).
  - `MoraleCoeff = clamp(0.5 + M/200, 0.25, 1.0)` (`Private/BattleSubsystem.cpp:767-770`).
  - Unit stat `FArmyStats::MOR` (`Source/StrategosCore/Public/World/ArmyStats.h:21`).
- **Not a morale term:** `CivilianMoraleIndex` (`StrategicIndices.h:28`) is a national economic index.

#### era — are eras defined?

- **Doc only, five eras** (`docs/architecture/21-progress.md:71-79`), via `UEraAsset` with `EraId` (e.g. "Era.SteamAge", `:52`):

  | Era | Years |
  |---|---|
  | Late Mercantile | 1820–1840 |
  | Steam Age | 1840–1870 |
  | High Industrial | 1870–1895 |
  | Late Imperial | 1895–1910 |
  | Early Modern | 1910–1925 |

- Era advancement is described in §8 (`:423-466`).
- **Not in code.** Grepping `EEra|UEraAsset` in Source finds nothing.
- **Conflicts:**
  - `CLAUDE.md` and `40-resources-and-production.md` fix the setting at 1820–1880.
  - The setting given to me is 1800–1950.
  - The eras in the table above end in 1925.

#### law names — any law names or law categories (LAW_*, LAWCAT_*)?

- Grepping `LAW_|LAWCAT_|ELawCategory|ULawAsset` in Source finds **nothing in code**.
- **Doc:** `31-politics.md:179-193`, `ELawCategory`, 11 categories, each with a level ladder:

  | Category | Ladder |
  |---|---|
  | Government | Monarquia Absoluta → Constitucional → Parlamentarismo → República |
  | Suffrage | Nenhum → Censitário → Universal Masculino → Universal |
  | Slavery | Legal → Apenas Colônias → Abolido |
  | Trade | Mercantilismo → Protecionismo → Livre Comércio |
  | Taxation | Por Estamento → Proporcional → Progressivo |
  | Conscription | Profissional → Voluntário → Conscrição Limitada → Universal |
  | Education | Privada → Religiosa → Estatal Básica → Universal |
  | LaborRights | Nenhum → Mínimo → Sindicatos → Direitos Plenos |
  | Religion | Estado Confessional → Tolerância → Secularismo |
  | Press | Censura Total → Censura Parcial → Liberdade |
  | Colonial | Exploração → Assimilação → Autonomia |

- Law ID format is `"Law.Suffrage.Universal"` (`:153`). There is one active law per category (`:196`).
- Conscription rate limits are given in `41-military.md:214-219`: 0.5% / 1% / 2% / 5% of population per year.
- The MVP subset is Government, Suffrage and Trade, with 3 levels each (`31-politics.md:797`).
- The only tax control in code is `ETaxLevel` per stratum: VeryLow 0.5×, Low 0.75×, Medium 1.0×, High 1.5×, VeryHigh 2.0× (`Public/Economy/TaxLevel.h:7-13`; `Treasury.h:53`).

#### tech spheres — the seven ETechSphere values

`Source/StrategosCore/Public/Progress/TechSphere.h:10-19`:

| Value | Display name |
|---|---|
| `PoliticalSocial` | "Political & Social" |
| `ProductionEnergy` | "Production & Energy" |
| `Economy` | "Economy" |
| `Military` | "Military" |
| `Communication` | "Communication" |
| `CultureWorldview` | "Culture & Worldview" |
| `NaturalScience` | "Natural Science" |

- Related: `ETechTrack { CivilFlow, StateResearch }` (`Public/Progress/TechTrack.h:13-17`).
- **Conflict:** `21-progress.md:91,101-109` uses a different `ETechBranch` taxonomy (Industry, Military, Culture, Naval, Politics in the enum comment; the table also lists Air and Commerce). The doc never mentions `ETechSphere`.

---

### M1 — How many states does the existing civil/military split expose in Core?

**Zero. There is no civil/military split enum, tag or mechanic in StrategosCore.**

- Grep run: `grep -rni 'civil|militar|stance|mobiliz|conscript|conscri' Source/`.
  - `Mobiliz` and `Conscri`: no hits.
  - `Stance`: hits only in StrategosBattle.
  - `Civil`: only index names and comments.
- Nearest constructs, so the caller can decide whether any of them counts:

| Construct | Values | Location | Civil/military? |
|---|---|---|---|
| `FStrategicIndices` (3 floats, not states) | `MilitaryReadinessIndex`, `CivilianMoraleIndex`, `IndustrialCapacityIndex`; each in [0.5, 1.5] | `Source/StrategosCore/Public/Economy/StrategicIndices.h:24,28,32`; computed at `EconomySubsystem.cpp:772-780` | Economic readout only. It drives nothing. |
| `ETechTrack` | `CivilFlow`, `StateResearch` | `Public/Progress/TechTrack.h:15-16` | Civil vs **state**, not military. |
| `EBuildingOwnerKind` | `Government`, `Private` | `Public/Economy/BuildingOwnerKind.h:9-10` | Ownership, not military. |
| `EDiplomaticStatus` | `Peace`, `NonAggressionPact`, `Alliance`, `War` | `Public/Diplomacy/DiplomaticStatus.h:14-17` | Bilateral relation, not national posture. |
| `EUnitState` | `Ready`, `InCombat`, `Damaged`, `Disorganized`, `Retreated` | `Public/World/ArmyStats.h:35-39` | Per-unit state. |
| `EPopStratum::Soldier` (stub) | — | `Public/Economy/PopStratum.h:22` | Stub stratum, no logic. |
| `EBattleStance` (**StrategosBattle**, not Core) | `Aggressive`, `Hold`, `Skirmish`, `Reserve` | `Source/StrategosBattle/Public/BattleTypes.h:94-100` | Tactical regiment stance. |

- **Doc-only (not implemented):** `41-military.md:581-589` describes `UMobilizationEngine` with 3 states, `Peacetime → PartialMobilization → FullMobilization` (30 / 60 days). `41-military.md:85-86` describes `EArmyPosture` (Defend, Skirmish, Assault, Raid, Garrison) and `EArmyStance` (Aggressive, Cautious, Evasive).
- `CLAUDE.md` (Decisões travadas) defers the unit stance "Carga/Reconhecer/Escaramuça" (charge/recon/skirmish).

---

### Economy v1 — S1-06 preview

Binding: `UEconomySubsystem::Initialize` subscribes `OnDayTick` and `OnMonthTick` (`Source/StrategosCore/Private/Economy/EconomySubsystem.cpp:21-25`).

| # | Phase (as implemented, in order) | Call site | Definition |
|---|---|---|---|
| daily | `TickConstruction` (construction countdown) | `EconomySubsystem.cpp:158-170` | `:230` |
| pre | Reset monthly counters (stockpile D/S, treasury lines, wages) | `:193-215` | — |
| 1 | `Phase_PopGrowth` (flat 0.1%/month, no migration) | `:217` | `:281` |
| 2 | `Phase_AssignEmployment` | `:218` | `:310` |
| 3 | `Phase_RunProduction` (tier 0→3 DAG, national stockpile) | `:219` | `:369` |
| 4 | `Phase_PopConsumption` (hardcoded basket, loyalty ±) | `:220` | `:506` |
| 5 | `Phase_PaywagesAndProfits` (Gov → Treasury; Private → Bourgeoisie Wealth) | `:221` | `:560` |
| 6 | `Phase_CollectTaxes` (per-stratum `ETaxLevel`) | `:222` | `:673` |
| 7 | `Phase_PayExpenses` (army upkeep, admin, interest) | `:223` | `:709` |
| 8 | `Phase_SettleTreasury` (auto-loan, `OnBankruptcyImminent`) | `:224` | `:740` |
| 9 | `Phase_ComputeStrategicIndices` | `:225` | `:772` |
| post | `RunBourgeoisieAutoInvestment` | `:227` | `:1163-1263` |

- **Order of calls:** `HandleMonthTick` (`:172`) iterates nations and calls `RunMonthlyTickForNation` (`:189`) for each.
- **Price formula:** `UEconomySubsystem::GetDynamicPrice`, `EconomySubsystem.cpp:138-156` (declared at `Public/Economy/EconomySubsystem.h:97`):
  - Formula: `BasePrice × clamp(1 + (D − S)/D, 0.5, 2.0)`.
  - D and S come from the nation's last-month stockpile Demand/Supply.
  - This is **not** the doc's elasticity-plus-lerp formula (`20-economy.md:240-257`).
- **Bourgeoisie auto-investment:** `RunBourgeoisieAutoInvestment` (`:1163-1263`), with scoring in `ComputeProfitabilityScore` (`:1130-1161`).
  - Per province, it requires free slots, Bourgeoisie Wealth ≥ 200, and construction goods in the stockpile.
  - It ranks building types by profitability and creates one `EBuildingOwnerKind::Private` building per province per month.
  - It seeds an `FRandomStream` (nation, year, month) but does not use it yet (`:1261`).
- **Feasibility:** a full S1-06 recon **is feasible** from this repo. The tick lives in a single file (`EconomySubsystem.cpp`, 1263 lines) and matches `98-estado-implementacao.md:52`, which describes the 9-phase monthly tick. Two caveats:
  - The design doc `20-economy.md` describes a different, unimplemented weekly/regional model. Trust code over that doc.
  - The market/logistics headers (`MarketDestination.h`, `TradeHub.h`, `LogisticsInfrastructure.h`, `PopDemandOrder.h`, `PopMobilityTracker.h`, `GoodConsumerProfile.h`) are unused scaffolding that cites a design doc ("doc §3–§7") not present in the repo.

---

## Open questions for Diogo

Only what the evidence could not settle.

1. **Godot repo.** Where is it? Push it to GitHub (private is fine) so Part A can run from here, or run Part A in Claude Code on your machine.
2. **Missing design docs.** `33`, `41`, `43`, `50`, `51`, `52`, `60`, `70` are in neither the repo nor the Project (`00` §4 says they live in the repo `docs/`; the GitHub repo does not have them). Where do they live? Until they are added, the rows of `82` that wait for them stay open.
3. **Period and eras.** The repo says 1820–1880 (`CLAUDE.md:9`), starts the game in 1836 (`TimeSubsystem.cpp:8`) and has five eras over 1820–1925 (`21-progress.md:71-79`: Late Mercantile, Steam Age, High Industrial, Late Imperial, Early Modern). `82` §4.14 has five eras over 1800–1950 (Flintlock, Steam, Telegraph, Wireless, Mechanization). `21-campaign-eras.md` in the Project holds the newer structure. Confirm the repo version is superseded.
4. **Market model for the port.** Three models disagree: the code runs one national stockpile; unused headers name Local Market / Trade Hub / Global Market; `20-economy.md` uses regional Market Regions and rejects a global market. Which one does `Strategos.Sim` port? It decides `market`, `national_market` and `world_market` in `82`.
5. **Stratum.** The code's `EPopStratum` holds professions (Laborer, Artisan, FactoryWorker, Bourgeoisie…), not Lower / Middle / Upper. The port follows `41` (stratum and profession separate) — confirm.
6. **Law categories.** The repo has 11 `ELawCategory` ladders (`31-politics.md:179-193`, including Trade); `42` (LOCKED) has five. Confirm the repo's 11 are superseded.
7. **M1 answered: 0.** No civil/military split exists in the Unreal Core, so `LAWCAT_Military` does not absorb an existing mechanic (as `42` §3.3 and `52` §7 assumed); its ladder is new design. A candidate exists in `41-military.md:581-589`: Peacetime / Partial / Full mobilization (not implemented). Decide the ladder size.
8. **Steel ladder.** The repo has three methods (`PM_BloomeryIron` Underdeveloped, `PM_PuddledSteel` Developing, `PM_BessemerSteel` Developed; `40-resources-and-production.md:108-112`). `82` and the mockups assume five tiers (Bloomery T1, Puddling T2, Crucible T3, Bessemer Process T4, Siemens-Martin T5). Confirm the five-tier ladder from `33`.
9. **Terminology follow-up.** Part B found 18 new collisions (§3) and about 120 player-visible concepts with no row (§2). They need a sixth batch in `82` before `PROMPT-APPLY-TERMINOLOGY.md` runs.
