# 94 — Recon: Economy v1

**Status:** REPORT — recon only, nothing changed. A dated snapshot (`00` §11): decisions taken on it go to `82`, `93` and the port.
**Date:** 2026-10-16
**Task:** `93` S1-06 (questions 1–8)
**Ran from:** a claude.ai cloud session, read-only, on the GitHub repo `Monkius-Maximus/LeGrandStrategos`: `main` at `eaac6ed` (2026-06-07) and the latest branch `claude/event-schema-debug-kqxid8` at `4dcfb53` (2026-08-26). The economy code is identical on both (`git diff` touches only Events and Save).
**Related:** `93` (S1-06, S1-07 … S1-09), `95` (Part C preview), `82` (names), `92` (ADR-0001)

All paths are relative to `Source/StrategosCore/`. "cpp:N" means `Private/Economy/EconomySubsystem.cpp` line N.

---

## Summary

1. **The tick is small and portable.** One file, 1,263 lines; nine monthly phases, a daily construction countdown, then Private Investment. Nothing in the logic needs Unreal; the Unreal parts are containers, asset loading and delegates (§4).
2. **The economy has no randomness.** Private Investment builds an RNG and never uses it (cpp:1172, 1261). Determinism depends only on iteration order, and three loops rely on `TMap` order (§6).
3. **`UActiveModifierRegistry` does not exist in code.** It is a design in the Unreal `30-events.md` §13. Every doc that says "through the existing modifier registry" (`33`, `34`, `42`, `52`, `61`) points at nothing; the port builds it new (§5).
4. **Loyalty** is one per-pop number that three unrelated things move: whether the pop's basket was met, the Tax Rate on its stratum, and building policies. The economy never reads it; only one event and the UI do (§7).
5. **The automatic loan is a game rule, not a code fallback.** Victoria 3 does the same: past the gold reserve, every pound spent beyond income becomes debt. Nothing in v1 reacts to Bankruptcy yet (§8).
6. **Money is not conserved.** Wages, state profits and taxes are created from nothing; pops eat for free; construction money disappears. The economy is a valuation model, not a closed money loop (§9.1). This is the biggest design question the port inherits.
7. **The World Market of `82` §4.17 is not in the code.** The code has one closed national stockpile, no imports and no exports. Only unused scaffolding names a Global Market. `82` §4.17 is corrected: the World Market is new design.

---

## §1 — The tick, phase by phase (question 1)

Binding: `Initialize` subscribes `OnDayTick` and `OnMonthTick` of the time subsystem (cpp:17-28). `HandleMonthTick` iterates the nations and runs `RunMonthlyTickForNation` for each (cpp:172-187), then broadcasts `OnEconomyTickComplete`.

| # | Phase | Def. | Reads | Writes | Notes |
|---|---|---|---|---|---|
| daily | `TickConstruction` | cpp:230 | building `ConstructionDaysRemaining` | same; `OnBuildingCompleted` | A finished building produces from the next monthly tick. Cost is paid in full when ordered. |
| pre | reset | cpp:193-215 | — | stockpile Demand/Supply = 0; the seven treasury lines = 0; pop `WageEarnedLastMonth` = 0 | Stocks (the goods themselves) are **not** reset: they carry over. |
| 1 | `Phase_PopGrowth` | cpp:281 | pop `Population` | pop `Population` | +0.1 %/month, `FloorToInt`: a pop under 1,000 never grows. No migration. |
| 2 | `Phase_AssignEmployment` | cpp:310 | method `EmploymentPerSlot` × building `Level`; pop `Population` | pop `EmployedThisMonth`; building `LastTickEmployment` | First come, first served in building order; later buildings get what is left. Unemployment is implicit. |
| 3 | `Phase_RunProduction` | cpp:369 | employment, province `RawResourcePotential`, building policies, stockpile stocks | stocks (−inputs, +outputs), Demand, Supply, building `LastTickInputs/Outputs` | Loops method `ProductionTier` 0→3. Scale = Level × employment ratio × raw potential × throughput; cut down to the scarcest input. Outputs of a lower tier feed higher tiers in the same month. |
| 4 | `Phase_PopConsumption` | cpp:506 | hardcoded basket per stratum (cpp:479-495), stocks | stocks, Demand, pop `Loyalty` | Pops take goods from the stockpile **for free**. Basket met (≥ 95 % of each item) → Loyalty +0.005, else −0.02. |
| 5 | `Phase_PaywagesAndProfits` | cpp:560 | last outputs/inputs × price, wages, maintenance | pop `Wealth`, `WageEarnedLastMonth`; treasury `Income_StateProfits`, `Expense_Maintenance`; owner pop `Wealth`; Loyalty (policies) | Profit = revenue − input cost − wages − maintenance, all valued at the current price. State-owned → treasury; Private → the Bourgeoisie pop of the owner province (clamped at 0). |
| 6 | `Phase_CollectTaxes` | cpp:673 | pop `Population`, base tax per stratum (cpp:654-664), Tax Rate | treasury `Income_Taxes`; pop `Loyalty` | Tax = population × base × multiplier. Pop Wealth is **not** debited. |
| 7 | `Phase_PayExpenses` | cpp:709 | armies' `ManpowerCount`, province count, debt | `Expense_ArmyUpkeep`, `Expense_AdminCost`, `Expense_DebtInterest` | Upkeep 0.001 per man; administration 10 + 0.5 per province; interest = debt × annual rate / 12. |
| 8 | `Phase_SettleTreasury` | cpp:740 | income, expenses | `Balance`, `DebtBalance`; `OnBankruptcyImminent` | Negative balance becomes debt (§8). |
| 9 | `Phase_ComputeStrategicIndices` | cpp:772 | Supply/Demand ratios of Tools, Iron, Bread, Garments, Coal | the three indices | Military Readiness = mean(Tools, Iron); Civilian Morale = mean(Bread, Garments); Industrial Capacity = Coal. Clamp 0.5–1.5. |
| post | `RunBourgeoisieAutoInvestment` | cpp:1163 | see §3 | new Private buildings, Bourgeoisie Wealth, treasury `Balance`, stocks | — |

The comment in `EconomySubsystem.h:56` ("deficit derruba loyalty") means a goods **shortage**, not money (`82` §4.7).

---

## §2 — Price (question 2)

`GetDynamicPrice` (cpp:138-156):

```
price = BasePrice × clamp(1 + (D − S) / D, 0.5, 2.0)      if D > 0
price = BasePrice                                          if D = 0
```

- **D** is this month's demand: production inputs wanted (phase 3) plus pop baskets (phase 4). **S** is this month's production only (phase 3). Goods already in stock do not count as supply, so a good with a full warehouse and no production this month is priced at ×2.
- Prices are read within the same month (phase 5, Private Investment). There is no smoothing and no memory of last month's price.
- An unknown good id returns 1.0 silently (cpp:141).
- The supply ratio used by the indices is a separate formula: `clamp(S / D, 0, 2)`, 1.0 when D = 0 (`NationalStockpile.h`).

**Where the numbers live:**

| Constant | Value | Where |
|---|---|---|
| Base price per good | 1.0–8.0 | `UGoodAsset::BasePrice`, set in the editor (`docs/setup/etapa-2-economy-content.md` §2) |
| Price clamp | 0.5–2.0 | cpp:152, hardcoded |
| Pop growth | 0.1 %/month | cpp:287 |
| Baskets | Bread 1.0 per pop; Garments 0.3 (Artisan, Factory Worker), 0.8 (Bourgeoisie) | cpp:482-485 |
| Basket met threshold, Loyalty deltas | 95 %; +0.005 / −0.02 | cpp:532, 539 |
| Base tax per pop | Laborer 0.05, Artisan 0.15, Factory Worker 0.20, Bourgeoisie 1.00 | cpp:654-664 |
| Tax Rate multiplier, Loyalty delta | 0.5 / 0.75 / 1.0 / 1.5 / 2.0; +0.01 / +0.005 / 0 / −0.01 / −0.03 | `TaxLevel.h` |
| Army upkeep, administration | 0.001 per man; 10 + 0.5 per province | cpp:719, 724 |
| Interest | 5 %/year | `FTreasury::AnnualInterestRate`, seeded in `WorldBootstrapper.cpp:143` |
| Debt ceiling | max(50, 5 × monthly income) | cpp:758 |
| Indices clamp | 0.5–1.5 | cpp:780-782 |
| Private Investment minimum Wealth | 200 | cpp:1184 |
| Demolition refund / privatize / nationalize | 50 % / 70 % / 120 % of cost × Level | cpp:898, 1063, 1095 |
| Start values | Treasury 500; stocks of Bread, Grain, Garments, Iron, Coal, Lumber; pops per province by capital/non-capital | `Private/Bootstrap/WorldBootstrapper.cpp:113-155` |

Everything except base prices and the interest rate is hardcoded in C++. The port moves them into data (ADR-0002).

---

## §3 — Private Investment (question 3)

`RunBourgeoisieAutoInvestment` (cpp:1163-1263), after phase 9, once per nation per month:

1. For each province of the nation, in order: skip if no free building slot, no Bourgeoisie pop, or its Wealth < 200.
2. For every registered building type (allowed by the province's raw potential), score its **default method**: `ComputeProfitabilityScore` (cpp:1130-1161) = output × current price × raw potential − inputs × price − headcount × wage − maintenance, for one slot. Construction cost is not in the score.
3. Take the best score; stop if it is ≤ 0. Ties go to the first type in `TMap` order (strict `>`).
4. Needed capital = money cost + value of the construction goods at current price. Stop if Wealth is short or the stockpile lacks a construction good.
5. Pay: Bourgeoisie Wealth −= needed capital; the treasury `Balance` += the goods' value (not booked as income); the goods leave the stockpile. The money cost leaves the economy.
6. Build one Level 1 Private building with the default method. **At most one investment per province per month.**

Triggers and thresholds: Wealth ≥ 200 and ≥ total cost, a free slot, positive best score, goods in stock. The RNG seeded from nation, year and month is never used (cpp:1261, "reserved for tie-break").

---

## §4 — Data vs Unreal-only (question 4)

### 4.1 Definitions (the data that ports)

| Asset | Fields that are data | Unreal-only | Port notes |
|---|---|---|---|
| `UGoodAsset` (`GoodAsset.h`) | `Id`, `BasePrice`, `Category` (Raw / Staple / Industrial / Luxury), `Tier` (0–5 processing depth) | `UPrimaryDataAsset`, `FText DisplayName`, `TSoftObjectPtr<UTexture2D> Icon` | Name comes from the termbase key; icon is a `Strategos.Game` concern (ADR-0001). `Category` and `Tier` are replaced by `82` Raw / Intermediate / Final (§4.18). |
| `UProductionMethodAsset` (`ProductionMethodAsset.h`) | `Id`, `ProductionTier`, `InputsPerSlot`, `OutputsPerSlot`, `EmploymentPerSlot` (stratum, headcount, wage per worker), `RequiredTechId`, `bRequiresRawResource` + `RawResourceGoodId`, `MaintenancePerSlot` | `FText`, `Icon` | `ProductionTier` is an **execution order**, not a ladder rung; `82` §4.18 reserves Tier for rungs. The goods doc (`34`) adds `Tier` (rung), `ToolsRequirement` and `UnlockedBy`. Employment is by `EPopStratum`, which holds professions: in the port it is by Profession. |
| `UBuildingTypeAsset` (`BuildingTypeAsset.h`) | `Id`, `Category` (Farm … Refinery), `AvailableMethods`, `DefaultMethod`, `ConstructionCost`, `ConstructionDays`, `ConstructionMonetaryCost`, `MaxLevel`, `bRequiresRawResource` + `RequiredResourceId` | `FText`, soft pointers | References become `SimId`s checked by the loader. |
| `UProductionModifierAsset` | `MutexGroup`, five multipliers, `MonthlyLoyaltyDelta`, `RequiredTechId` | `FText`, `Icon` | Building policies: not in the current design (`82` §4.22). |
| `UEconomyContentRegistry` | lists of the four asset kinds | the whole class | Replaced by the loader reading `data/`. |

### 4.2 Runtime state

| Type | Fields | Port notes |
|---|---|---|
| `UBuilding` (`Building.h`) | `Id`, type, province, `Level`, owner kind and owner province, current method, active policies, `ConstructionDaysRemaining`, last-tick caches | A plain C# class; caches stay for the UI. |
| `FPopGroup` (`PopGroup.h`) | `Stratum`, `Population`, `Wealth`, `Loyalty` (starts at 1.0), employment and wage caches | One pop per stratum per province; the port keys pops by Profession and Stratum (`82` §5 Population). |
| `FNationalStockpile` | `Stocks`, `Demand`, `Supply` | The National Market of `82` §4.17. |
| `FTreasury` | `Balance` (the money held: Treasury in `82` §4.24), seven monthly lines, `DebtBalance`, `AnnualInterestRate`, Tax Rate per stratum | `Income_Tariffs` is reset each month and never written. |
| `FStrategicIndices` | three floats | §9.6. |
| `UProvince` (World) | `RawResourcePotential` (per good, from terrain in `WorldBootstrapper.cpp:60-105`), `BuildingSlots`, `Pops`, `Buildings` | Raw potential becomes Deposits (`43`, Sprint 02). |

### 4.3 What is Unreal-only and dies in the port

`UWorldSubsystem` and its world-type check; resolving state through `AStrategosGameState`; dynamic multicast delegates (`OnEconomyTickComplete`, `OnBuildingCompleted`, `OnBankruptcyImminent`) → plain C# events; `TSoftObjectPtr` and `LoadSynchronous` (called inside the tick, cpp:337, 386, 575, which the repo's own `CLAUDE.md` forbids); `UPROPERTY` metadata; `FName` → `SimId`; `TMap` → ordered collections (§6); `FRandomStream` → `DeterministicRng`; `UE_LOG`. Also the editor-only content path: goods and methods exist only as `.uasset` files described in `docs/setup/etapa-2-economy-content.md`; the port's seed comes from `82` Goods and `34`, not from them.

---

## §5 — The modifier registry (question 5)

**It does not exist in code.** `grep` for `UActiveModifierRegistry`, `FActiveModifier` and `FStatModifierSet` in `Source/` finds nothing. It exists only as a design in `docs/architecture/30-events.md` §13:

- `UActiveModifierRegistry` holds `TArray<FActiveModifier>` with `Add`, `RemoveExpired(CurrentTick)`, `RemoveBySource(SourceId)`, `Query(Target, Scope)`.
- `FActiveModifier`: `ModifierId`, `SourceEventId`, `ScopeId` (nation, province or pop), `ExpiresAtTick` (0 = permanent), `FStatModifierSet Modifiers`, `DisplayText`.
- "Other subsystems query modifiers by target", e.g. the economy asks what affects a province's production.

What the code has instead: building policies (`UProductionModifierAsset`, toggled per building, cpp:261-276); per-army `FArmyModifier` (`World/ArmyStats.h:47`); event effects that write state directly (`UEffect_AddGold`, `UEffect_AddPopLoyalty`). No lifecycle, no expiry, no query.

**Consequence:** the docs that route effects "through the existing `UActiveModifierRegistry`" (goods doc `34`, `33`, `42`, `52`, `61`) depend on a system that has to be built. It is not needed by S1-07 … S1-09; it becomes needed with the first law, event or Deposit effect.

---

## §6 — Randomness and determinism (question 6)

- **Economy: no randomness.** The only RNG (cpp:1169-1172, seeded by nation, year and month) is unused.
- Elsewhere, all seeded `FRandomStream`: events (MTTH, `EventSubsystem.cpp:624`), AI (`AIPlaceholderSubsystem.cpp:76-80`), battle (`BattleSubsystem.cpp:66`), world generation (`WorldGenContext.h`). One unseeded call: `FMath::FRand()` in the UI war room (`StrategosUI/.../StrategosWarRoomWidget.cpp:184`), outside the economy.
- **Determinism risks the port must remove** — results depend on container order:
  1. Nations in `World->Nations` (cpp:179); the comment claims `TMap` order is stable "given the same insertions".
  2. Pops in `Prov->Pops` (phase 4): with scarce goods, whoever iterates first eats. Provinces in `OwnedProvinceIds` order have the same effect.
  3. Building types in `BuildingTypeById` (§3 step 3): ties go to the first.
  The repo's own `CLAUDE.md` lists "TMap iteration order is not stable" as a known pitfall. The port iterates in explicit `SimId` order everywhere.
- Floats: deterministic on one machine and runtime; S1-09's byte-identical test holds there. Cross-platform identity is not promised.

---

## §7 — Pop Loyalty (question 7, for `82` §4.30)

**Who writes it** (always clamped 0–1, starting at 1.0):

| Writer | Change | Where |
|---|---|---|
| Consumption | +0.005 basket met, −0.02 not met | cpp:538-540 |
| Tax Rate | +0.01 … −0.03 per month by rate | cpp:697, `TaxLevel.h` |
| Building policies | `MonthlyLoyaltyDelta` on Laborer, Artisan and Factory Worker of the province | cpp:631-643 |
| Nationalization | −0.05 for the Bourgeoisie | cpp:1109 |
| Events | `UEffect_AddPopLoyalty` | `Private/Events/Effects/EventEffects.cpp:26-46` |

**Who reads it:** the event condition `LoyaltyBelow` (`EventConditions.cpp:36-48`), used by one of the five built-in events (Worker Strike, Factory Worker below 0.6; Foreign Investor and Festival Petition only write Loyalty); the province dock, as "Militancy" = 1 − Loyalty and as the "Satisfação" bar (`StrategosUI/.../StrategosProvinceDockWidget.cpp:134-144, 245`); the save. **The economy itself never reads it**, and neither do the indices.

**What it is:** three different signals summed into one drifting number — needs, tax burden, working conditions. In the current design they belong to different places: needs to Needs Met (`82`), tax burden to Interest Group Approval of the Tax Rate (Politics v1, Sprint 05), working conditions to building policies, which are out (`82` §4.22).

**Options for `82` §4.30:**

- **A — Fold into Needs Met.** The port computes Needs Met each month as the share of the basket received (a ratio, not a drift). Tax effects return with Politics v1 as part of Approval. Events that need unrest wait for a current events design.
- **B — Keep Loyalty** as its own value with the same writers, and give it a row in `82`.

**My recommendation: A.** It removes a value no current doc defines, keeps one source of truth per signal, and S1-09 loses nothing it tests.

---

## §8 — The automatic loan (question 8)

`Phase_SettleTreasury` (cpp:740-763): `Balance += income − expenses`; if `Balance < 0`, the shortfall becomes debt (`DebtBalance += −Balance; Balance = 0`). Interest is charged the next month (phase 7). If debt > max(50, 5 × monthly income), `OnBankruptcyImminent` fires — **every month** while above. `UEventSubsystem` turns it into the trigger `Economy.BankruptcyImminent` (`EventSubsystem.cpp:749-755`), but no built-in event listens to it, and nothing else does. Debt is read only by interest and the HUD.

**Is it a fallback?** No. A code fallback hides a missing precondition; this is a stated rule of the game, with a cost (interest) and a limit (the debt ceiling). Victoria 3 works the same way: "each pound spent in excess of your income will result in automatically taking on debt", up to a debt ceiling where the country defaults ([Victoria 3 Dev Diary #12 — Treasury](https://admin-forum.paradoxplaza.com/forum/developer-diary/victoria-3-dev-diary-12-treasury.1488588)). My note in the batch 6 proposal called it a fallback; that was wrong.

**My recommendation:** port it as an explicit rule using the `82` names (Loan, Debt, Interest, Bankruptcy). What Bankruptcy does stays open (`82` `bankruptcy`). Fire Bankruptcy once when the ceiling is crossed, not every month.

---

## §9 — Other findings that shape the port

1. **Money is not conserved.** Wages are added to pop Wealth and never paid by anyone (cpp:602-603). State profit is the value of goods that only moved into the national stockpile (cpp:616). Taxes are added to the treasury without leaving pop Wealth (cpp:694-695). Pops consume for free (phase 4). Construction money vanishes (cpp:852, 1237). Wealth only matters for the Bourgeoisie, who invest it. The port can reproduce this exactly (S1-09 is a port) and leave a closed money loop to a state-finance design, or close the loop now.
2. **Scarce goods go first-come.** Phase 4 and phase 3 take from the stockpile in iteration order; there is no pro-rata share. With deterministic ordering this is reproducible but arbitrary.
3. **Methods of `ProductionTier` 4–5 never run.** The asset allows 0–5 (`ProductionMethodAsset.h`), the loop runs 0–3 (cpp:374).
4. **Technology gates are not enforced.** `RequiredTechId` exists on methods and policies, but neither `ChangeProductionMethod` nor `ToggleProductionModifier` checks it. The AI's comment that the economy "checks internally" (`AIPlaceholderSubsystem.cpp`, Merchant) is false.
5. **No trade.** No imports, no exports, no tariffs: `Income_Tariffs` is never written. `MarketDestination.h`, `TradeHub.h`, `LogisticsInfrastructure.h`, `PopDemandOrder.h`, `PopMobilityTracker.h`, `GoodConsumerProfile.h` and `BuildingState.h` are declared and used nowhere. The World Market and Trade Capacity of `82` §4.17 are new design.
6. **Hardcoded good ids that `82` retired.** The baskets (Bread, Garments) and the indices (Tools, Iron, Bread, Garments, Coal) name goods by string; Bread, Garments and Iron are not among the 31 goods. Baskets and index formulas must be redefined over the `82` Goods.
7. **Silent defaults everywhere.** Missing registry → empty lookups with a warning (cpp:74-78); unknown good → price 1.0; missing method or policy → skipped; missing Tax Rate → Medium (cpp:692). The repo's `CLAUDE.md` names this "Padrão fallback" as a convention. The port reverses it: the loader throws (`93` S1-08), and runtime lookups of unknown ids throw.
8. **"Engineer" is employed but does not exist.** Four methods in the setup doc employ Engineers; `EPopStratum` has no such value, so those methods cannot be built as written.
9. **Player API beyond the tick:** build, demolish (50 % refund), upgrade Level, change method, toggle policy, set Tax Rate, privatize, nationalize, sponsor private industry (cpp:789-1124). Privatize, nationalize and sponsor have no callers. None is needed by S1-09.

---

## §10 — Questions for Diogo

Each needs an answer before or during S1-09. My recommendation comes first.

1. **Pop Loyalty (`82` §4.30).** A: fold into Needs Met; tax effects return with Politics v1. B: keep it. → **A** (§7).
2. **Automatic loan.** Keep it as an explicit rule (Loan, Debt, Interest), Bankruptcy fired once at the ceiling, its consequence open. → **Keep** (§8).
3. **Money.** Port the valuation model as it is and record the closed loop as a design debt, or close the loop in the port. → **Port as is** for S1-09; a state-finance doc decides the loop. S1-09 is a port, and its "done when" does not depend on it.
4. **Scarce goods.** First-come in id order, or pro-rata by demand. → **Pro-rata**: it removes the order dependency of §6 and costs a few lines.
5. **Execution order of methods.** Keep a hand-set order field, or derive it from the recipe graph and fail on a cycle. → **Derive it**: one less number to maintain, and `82` keeps Tier for ladder rungs.
6. **Baskets and indices over the new goods.** Proposed for the first build: Lower Stratum Grain; Middle Grain + Clothes; Upper Grain + Clothes + Furniture. Military Readiness = mean(Small Arms, Artillery); Industrial Capacity = mean(Coal, Steel); Home Front = mean(Grain, Clothes). Numbers are placeholders for balance.
7. **Building policies, privatize, nationalize, sponsor.** → **Drop** from the port (`82` §4.22); they return only with a design doc.

---

## Appendix — Method

- Read in full: `Private/Economy/EconomySubsystem.cpp`, `Public/Economy/EconomySubsystem.h`, and the Economy headers listed in §4.
- Searched the whole `Source/` tree for: the modifier registry names; every `FRandomStream` and `FMath::Rand*` call; every reader and writer of `Loyalty`; every reader of debt, bankruptcy and the indices; callers of the ownership API.
- Read `WorldBootstrapper.cpp` (terrain potentials, pop seeds, treasury and stock seeds), the event wiring in `EventSubsystem.cpp`, the AI economy behaviors, `docs/setup/etapa-2-economy-content.md` and `docs/architecture/30-events.md` §13.
- Nothing built or run: this session has no .NET or Unreal toolchain, and the recon needs none.
