# 35 — Economy Port Analysis: Money, Scarce Goods, Method Order

**Status:** ANALYSIS · [LATEST] — options for three open port decisions; nothing here is decided until Diogo confirms.
**Date:** 2026-10-18
**Decisions covered:** `94` §10 questions 3 (money), 4 (scarce goods), 5 (method execution order). Recorded as open in `00` §10 and `93` §5.
**Related:** `94` (Economy v1 recon), `82` (names), `83` and the canvas "LGS — Interface Mockups 2" (PDF export of 2026-10-17, taken as the latest UI vision), `93` S1-08/S1-09
**Numbering:** `35` sits in the 30s family (resources and economy), next to `33` and `34`.

Each decision follows the same order: what it is, the options and the proposal, the theory behind it, games that work this way, the formulas, how it combines with other systems, and how it shows on screen. My recommendation comes first in every proposal; Diogo decides.

---

## Summary

| # | Decision | Recommendation | One-line reason |
|---|---|---|---|
| 3 | Money model | **C — Accounted money:** every unit of money moves through one ledger with named sources and sinks; not fully conserved | Closes the "pops eat for free" hole and makes every number explainable, without the cost of a full closed economy. Victoria 3 works this way. |
| 4 | Scarce goods | **C — Pro-rata:** every buyer gets the same fill ratio of what it asked for | No order dependence, deterministic, and it is already what the mockup shows (Steelworks receives 83% of its Coal). |
| 5 | Method order | **C — Simultaneous, one-month lag:** everyone consumes from the stock at the start of the month; output lands for next month | The first-build goods graph has a cycle (Steel → Tools → Bessemer Process → Steel), so a derived order fails on day one. |

The three are linked: the fill ratio of 4 needs a supply figure, which 5 defines (the opening stock), and buyers pay for what they receive, which 3 defines. Section 4 shows the combined monthly tick.

---

## 1 — Decision 3: the money model

### 1.1 What it is

Economy v1 is a **valuation model**, not a money economy (`94` §9.1):

- Wages are added to pop Wealth, but no building pays them.
- State profit is the value of goods that moved into the stockpile.
- Taxes are added to the Treasury without leaving pop Wealth.
- Pops take their basket from the stockpile for free.
- Construction money disappears.

Only the Capitalists' Wealth matters, because it funds Private Investment. The question: port this as it is, close the loop, or something in between.

Decision 1 (Needs Met) and decision 2 (the player handles a negative Treasury) make this sharper. If pops eat for free, Needs Met measures only physical supply, never purchasing power. If the Treasury can go negative and the player must act, the player has to see where the money went.

### 1.2 Options and proposal

| | A — Port as is | B — Closed loop | **C — Accounted money (recommended)** |
|---|---|---|---|
| Idea | Keep the v1 valuation model; note the loop as design debt | Every Crown has an owner; money only enters or leaves through trade with the outside world | Every money change is a transaction between named stocks, or an explicit source or sink in one ledger |
| Pops pay for goods | No | Yes, from Wealth | Yes, from Wealth |
| Buildings pay wages | No (created) | Yes, from their cash | Yes, from their cash |
| Taxes leave pop Wealth | No | Yes | Yes |
| Total money | Drifts silently | Constant, except trade | Changes only through listed sources and sinks, and each change is logged |
| Effort for S1-09 | Lowest | Highest (and needs a money supply policy) | Medium |
| Risk | Numbers on screen cannot be explained; Needs Met ignores money | Deflation or hoarding if money has no tap; hard to tune at 5 h/week | Sources and sinks need tuning, but each is visible |

**Proposal: C.** It keeps "one source of truth" (`90` §1.4) for money: a single ledger that every system writes to. The market difference between what buyers pay and sellers receive is a named line, as it is in Victoria 3, instead of a silent leak. S1-09 can still port v1's phases and constants; what changes is that each phase books its money through the ledger.

### 1.3 The theory behind it

- **Circular flow of income.** Households sell labor to firms for wages and spend wages on firms' goods; the state taxes both. A closed loop (option B) is this model with nothing leaking.
- **Stock-flow consistency.** Every flow comes out of one stock and goes into another, so the stocks always add up. Option C applies this as a test: the change in all money stocks equals the ledger's sources minus its sinks, every tick.
- **Faucets and sinks.** Game economies (MMOs, city builders) rarely conserve money. They name where money enters (faucets) and leaves (sinks) and tune the two. Option C is a faucet-and-sink design with an audit.

### 1.4 Games that work this way

- **Victoria 3** does not conserve money. Buyers pay and sellers receive at the market price, and the difference between the two totals is created or destroyed by the simulation ([Vic3 Wiki — Market](https://vic3.paradoxwikis.com/Market)). An analysis of its engine lists sources (sales revenue, gold mines, minting, tariffs), sinks (consumption, inputs, taxes above administrative capacity, gold reserves above a soft cap) and "sponges" that store money (building reserves, state debt or reserve, the investment pool, pop standard of living) ([Dragon Tributary](https://riverlimburg.substack.com/p/exploring-the-economic-engine-of)). Pops buy with their money and Wealth, and purchasing power limits how much of their needs they meet ([Vic3 Dev Diary #9](https://forum.paradoxplaza.com/forum/threads/victoria-3-dev-diary-9-national-markets.1484917/)). This is option C.
- **Victoria 3, government side.** Surpluses pay down debt or build a gold reserve; deficits draw on the reserve or add debt (Dragon Tributary). Decision 2 keeps the player in charge of that last step.
- **Economy v1** itself is option A.

### 1.5 Formulas

Stocks of money: Treasury, each building's Cash, each pop's Wealth, the Investment Pool. Per month:

```
Pop.Wealth      += Wages + Dividends − Purchases − Taxes
Building.Cash   += Sales − InputCost − Wages − Maintenance − Dividends
Treasury        += Taxes + StateProfits + TariffIncome − Expenses − Interest
InvestmentPool  += share of Dividends (Capitalists) − ConstructionSpending
```

Market line (Victoria 3's form, with S and D in money):

```
MarketBalance = P × (Sold − Bought)      // > 0 money created, < 0 money destroyed
```

Ledger invariant, checked every tick by a test:

```
Σ ΔStocks  ==  Σ Sources − Σ Sinks
Sources = MarketBalance(+), Exports, Loans taken
Sinks   = MarketBalance(−), Imports, Debt repaid, Construction money
```

Purchasing power feeds Needs Met (decision 1):

```
Affordable_g = min(Requested_g, Pop.Wealth_available / Price_g)   // per needs group, in order Life → Everyday → Luxury
NeedsMet     = Σ Received_g × weight_g / Σ Requested_g × weight_g
```

v1's constants (wage per worker, base tax per pop, upkeep, administration, 5 % interest) port unchanged as data (`94` §2); only their accounting changes.

### 1.6 How it combines with other systems

| System | Effect |
|---|---|
| Needs Met (decision 1) | Pops that cannot afford their basket get less of it. Poverty shows up as unmet needs, not as a separate number. |
| Treasury and Loan (decision 2) | The Treasury can only go negative through ledger lines the player can read. The alert links to them. |
| Private Investment | Funded from the Investment Pool, filled by Capitalist dividends, instead of from Wealth that appears from nothing. |
| Taxes and Tax Rate | Taxes leave pop Wealth, so a high Tax Rate lowers Needs Met. That is the hook Politics v1 needs for Approval (Sprint 05). |
| World Market | Imports are money leaving the nation; exports bring it in. It is the only true external flow. |
| Scarce goods (decision 4) | Buyers pay for what they receive, not for what they asked for. |
| Bankruptcy | Defined on the ledger: Treasury below zero and no Loan taken. |

### 1.7 How it shows on screen

From the mockups (Economy · F3, Treasury tab, PDF page 24):

- **Treasury tab.** The Income and Expenses lists stay. Each line gets a Provenance Tooltip that opens the ledger entries behind it. A new line, **Market Balance**, shows money created or destroyed by the market. The current `⚠ RECONCILIAR` notes on Taxes by Stratum become real lines once the Tax Rate data exists.
- **Negative Treasury (decision 2).**
  - The Alert Strip shows "Treasury below zero", with actions: take a Loan, sell goods, cut expenses.
  - The Treasury tab gets the **"Take Loans automatically"** checkbox.
  - With it off, the game keeps running (decision 2: alert without pause). The effect of unpaid expenses is still to be designed.
- **Selected building** (Steelworks, page 5). Profit already shows "+62/month". Its tooltip splits Sales, Inputs, Wages, Maintenance and Dividends.
- **Society panel** (page 18) shows pop Wealth next to Needs Met. The mockup still labels the needs groups Basic / Comfort / Luxury; the `82` names are Life / Everyday / Luxury Needs (`83` §3.1).
- **Designer Mode** shows the ledger invariant as a pass or fail line each month.

---

## 2 — Decision 4: scarce goods

### 2.1 What it is

When buyers in a National Market request more of a good than exists, someone gets less. In v1, whoever is iterated first takes the stock, so pop and building order decides who starves (`94` §9.2). Order in v1 depends on container order, which is not deterministic across runs (`94` §6). The port must decide who gets what.

### 2.2 Options and proposal

| | A — First-come by SimId | B — Priority classes | **C — Pro-rata (recommended)** | D — Price only |
|---|---|---|---|---|
| Idea | v1's rule, with a fixed order | Some buyers first (state, military, then buildings, then pops) | Every buyer receives the same share: available ÷ requested | Everyone buys; the price rises and a shortage penalty hits users of the good |
| Fair? | Arbitrary: low SimIds win | By design, unequal | Equal share for everyone | Equal, but abstract |
| Deterministic | Yes | Yes | Yes | Yes |
| Order-independent | No | Within a class, no | Yes | Yes |
| New rules needed | None | Who ranks where (a law?) | None | A threshold and a penalty curve |

**Proposal: C, pro-rata.** It is the simplest rule that does not depend on order. The mockup already uses it: on page 25 the Steelworks "receives 83% of the Coal it requests", and every 10% missing costs 0.08 Efficiency. Priority classes (B) can come later as a law effect, for example Military law giving armies first claim on Small Arms. That would be a modifier on top of C, not a second rule.

### 2.3 The theory behind it

- **Rationing.** When a resource falls short of claims, the proportional rule (each claimant gets the same fraction of its claim) is the oldest and most studied solution of the "bankruptcy problem" in economics. It treats every claim alike and cannot be gamed by ordering.
- **Market clearing.** In a pure market the price rises until demand falls to supply. Option D relies on this, but prices adjust slowly, so a quantity rule is still needed inside the month.
- **Leontief production.** A building with several inputs is limited by its scarcest one. v1 already cuts output to the scarcest input (`94` §1, phase 3).

### 2.4 Games that work this way

- **Victoria 2** sells world-market goods by country ranking: higher-ranked powers buy first ([Vic2 Wiki — markets guide](https://vic2.paradoxwikis.com/A_Complete_Understanding_of_Victoria_2's_Markets_and_the_Power_of_Foreign_Investment)). Players know the effect well: small nations starve while great powers are served. That is option B.
- **Europa Universalis V** distributes a short good by Trade Advantage, which also orders who fills exports first ([EU5 Wiki — Market](https://eu5.paradoxwikis.com/Market)). Also option B, with the ranking as a stat the player can raise.
- **Victoria 3** lets every buyer and seller transact; mismatches change the price instead of the volume. A shortage starts when buy orders reach double the sell orders. Buildings that use the good then lose throughput: −5 % at first, growing 1 % per day to −50 %, or −75 % if the good is missing locally ([Vic3 Wiki — Market](https://vic3.paradoxwikis.com/Market)). Its developers rejected sequential priority because it caused cascading failures and could not run in parallel ([Dev Diary #9](https://forum.paradoxplaza.com/forum/threads/victoria-3-dev-diary-9-national-markets.1484917/)). That is option D.

### 2.5 Formulas

Per good g, per month, in one National Market:

```
Requested_g = Σ over buyers i of request_i,g          // buildings, pops, state, armies
Available_g = OpeningStock_g + Imports_g              // decision 5 defines OpeningStock
f_g         = min(1, Available_g / Requested_g)       // fill ratio; 1 when nothing is short
Received_i,g = f_g × request_i,g
Shortage_g  = max(0, Requested_g − Available_g)       // the "Shortage" term of 82
```

Buildings, with the mockup's penalty (page 25):

```
InputFill_b   = min over inputs k of f_k                       // scarcest input (Leontief)
Efficiency_b  = Base × EmploymentFactor − 0.8 × (1 − InputFill_b) + modifiers
Output_b      = Level × OutputPerLevel × Efficiency_b
```

The mockup's own numbers check out: 100 of 120 t is 83 %, so 17 % missing × 0.8 = 0.133, shown as −0.14. Round the display from the exact value; 0.8 is a placeholder for balance.

Pops: `Received_i,g` feeds Needs Met, together with purchasing power (decision 3).

Rounding: goods are fractional tons, so `f × request` needs no remainder rule. If a good is ever counted in whole units, the remainder goes by SimId order. That is the only place order could matter, and it is explicit.

### 2.6 How it combines with other systems

| System | Effect |
|---|---|
| Efficiency | The fill ratio of the scarcest input enters the Efficiency breakdown, as in the mockup tooltip. |
| Needs Met (decision 1) | The fill ratio of each needs good is the physical half of Needs Met; money is the other half (decision 3). |
| Price | The price formula keeps D = requested and S = available, so a shortage raises the price, and the higher price lowers next month's requests. |
| Military Readiness | Defined over Small Arms and Artillery (`94` §10.6, still open as decision 6). Under pro-rata, armies feel shortages at the same rate as everyone else. |
| World Market and Trade Capacity | Imports raise Available; the World Market tab already shows "Shortage of 240 t/month even with Imports" (page 23). |
| Private Investment | Profitability uses the expected fill ratio, so investors avoid industries whose inputs are short. |
| Determinism | No order dependence is left in consumption (`94` §6, risk 2). |
| Laws (later) | A priority for military buyers can be a law modifier on the request weight, without a new rule. |

### 2.7 How it shows on screen

- **Nested tooltip** (page 25): Efficiency → Input Shortage: Coal → "receives 83% of the Coal it requests", Requested and Received → Coal in Norden (Supply, Demand, Shortage). This is the full explanation chain, already designed.
- **Top Bar.** "Shortages · tons/month: Coal −240, Tools −18". Same numbers as `Shortage_g`.
- **National Market tab** (page 7). The ▼ marker on a good is a shortage; the good's detail adds a "Fill ratio" line next to the price history.
- **Alert Strip.** "Strategic good in shortage" when the fill ratio of a Strategic Good stays below 1 for N months (`80` §5.2).
- **Society panel and province Population tab.** Needs bars show the fill ratio per needs group.

---

## 3 — Decision 5: method execution order

### 3.1 What it is

Inside one month, a Steelworks needs Iron Ore and Coal, and a Tool Workshop needs Steel. If the Steelworks runs first, the workshop can use this month's Steel; if the workshop runs first, it cannot. v1 solves this with a hand-set `ProductionTier` on each method and runs tiers 0 → 3 (`94` §1, phase 3). The port must choose how order is set.

**A finding that limits the options.** The first-build goods form a cycle:

- Tools are made from Steel.
- Bessemer Process (Steel T3) and Open Hearth (T4) need Tools (`82` Goods; `pm_bessemer`, `pm_open_hearth`).

So Steel → Tools → Steel. S1-08 seeds T1–T4, which puts the cycle in the data from day one. Any rule that sorts the recipe graph and fails on a cycle stops the game at load.

### 3.2 Options and proposal

| | A — Hand-set order | B — Derived from the graph | **C — Simultaneous, one-month lag (recommended)** | D — Iterate until stable |
|---|---|---|---|---|
| Idea | v1's `ProductionTier` field | Topological sort of goods; fail on a cycle | Everyone consumes from the opening stock; output lands in next month's stock | Re-run production until supplies stop changing |
| Handles the Steel–Tools cycle | Only with a hand-picked order that hides it | **No: throws at load** | Yes | Yes, if it converges |
| Extra data | One number per method | None | Initial stocks (already seeded by v1) | None |
| Order-independent | No | Yes | Yes | Mostly |
| Cost | Maintenance of a number per method | Cycle breaking rules | One month of delay per chain step | Unbounded loops; hard to test |

**Proposal: C.** It is the only option that needs no order at all, survives the cycle, and keeps every buyer of a month on equal terms (it is what pro-rata in decision 4 needs). The price is a one-month delay per step: Iron Ore mined in March becomes Steel in April and Small Arms in May. At a monthly tick that reads as a supply pipeline, and the stock on hand becomes something the player manages.

### 3.3 The theory behind it

- **Jacobi vs Gauss-Seidel updates.** Updating every element from the previous state (Jacobi) is order-free and parallel. Updating in sequence from values already changed this step (Gauss-Seidel) converges faster but depends on order. Option C is Jacobi; v1 is Gauss-Seidel with a fixed order.
- **Double buffering.** Simulations read from last step's buffer and write to the next one, so no element sees a half-updated world.
- **Input-output tables (Leontief).** An economy's goods graph has cycles as a rule (steel makes tools, tools make steel). Input-output models solve the whole system at once instead of ordering it.

### 3.4 Games that work this way

- **Victoria 3** has no stockpiles: a unit is sold at the moment it is produced, and all purchases resolve on their own and add up into one price ([Dev Diary #9](https://forum.paradoxplaza.com/forum/threads/victoria-3-dev-diary-9-national-markets.1484917/)). No production order exists at all. LGS keeps a stockpile (the National Market is the v1 stockpile, `82` §4.17), so its form of "no order" is the opening stock.
- **Economy v1** is option A.

### 3.5 Formulas

Per month t, per good g:

```
OpeningStock_g(t) = ClosingStock_g(t−1)
Available_g       = OpeningStock_g + Imports_g − Exports_g
Received_i,g      = f_g × request_i,g                  // decision 4
Produced_b,g      = Level_b × OutputPerLevel_b,g × Efficiency_b
ClosingStock_g(t) = Available_g − Σ Received_i,g + Σ Produced_b,g
```

`ClosingStock` can never go negative, because `Σ Received ≤ Available` by construction. The loader requires a seeded opening stock for every good, and fails fast if one is missing.

The price formula of v1 counted only this month's production as supply. Under C, supply for the price is `Available_g`, the stock that buyers can actually draw on. This changes v1's pricing, so it is flagged for S1-09's "done when" check (no Coal → Steel price rises still holds).

### 3.6 How it combines with other systems

| System | Effect |
|---|---|
| Scarce goods (decision 4) | Pro-rata needs a fixed `Available` before anyone buys; the opening stock is exactly that. |
| Money (decision 3) | Sales are booked when goods are received, so the ledger has one moment per month to settle. |
| Price | Supply becomes the stock on hand, not this month's output; prices react a month later to a mine closing. |
| Construction | Construction goods are drawn from the opening stock like any other request. |
| Military | Equipment made this month reaches armies next month. Fits the "industry decides wars" pillar: a nation must build up stock before a war. |
| Determinism and threads | No order, so the tick can later run in parallel. |
| Tier ladder | The data needs no `ProductionTier`; Tier stays reserved for ladder rungs (`82` §4.18). |

### 3.7 How it shows on screen

- **National Market tab** (page 7). Each good's tile gains **Stock** next to price: the opening stock this month.
- **Provenance tooltip for Supply** (page 25, last layer). "Coal in Norden: Supply 1,180 t/month" splits into **Opening stock**, **Imports**, and **Produced this month (available next month)**. The player sees why output from a new mine arrives a month later.
- **Selected building** (page 5). Output "41 t/month" gets a note in its tooltip: available on the National Market next month.
- **Construction** (page 20). "Steel for Construction 30 t" draws from stock; the build picker warns when stock is below the requirement.
- **Designer Mode** draws the goods graph and marks cycles (Steel ↔ Tools) as information, not errors.

---

## 4 — The monthly tick with all three recommendations

```
1. Open month: OpeningStock = last ClosingStock; reset money flow lines.
2. Requests: every building, pop, army and the state states what it wants.
3. Fill: f_g = min(1, Available_g / Requested_g) per good; Received = f × request.
4. Pay: buyers pay P × Received from Cash, Wealth or Treasury; the ledger books it.
5. Produce: Efficiency uses the scarcest input fill; output goes to ClosingStock.
6. Wages, dividends, taxes, expenses: booked in the ledger.
7. Treasury check: below zero → auto-Loan if the checkbox is on; otherwise Alert.
8. Prices for next month from Requested and Available.
9. Ledger invariant test; indices; Private Investment from the Investment Pool.
```

Phases 1–9 of v1 map onto this; the constants port as data (`94` §2).

---

## 5 — Questions for Diogo

1. **Money:** C (accounted money) — or A or B?
2. **Scarce goods:** C (pro-rata) — or A, B or D? If C: keep the mockup's penalty, 0.8 Efficiency per 100 % missing, as the placeholder?
3. **Method order:** C (simultaneous, one-month lag) — or another? If C: accept that supply for the price becomes the stock on hand?
4. **Unpaid expenses** with the Treasury below zero and auto-Loan off: what happens until the player acts? Proposal for the finance doc: unpaid lines stop working in order (construction pauses, then maintenance lapses and Efficiency drops), each shown in the Alert.

---

## Appendix — Decisions taken

- Needs Met replaces pop Loyalty. *(Confirmed by Diogo, 2026-10-17.)* Used here as a given.
- Negative Treasury: the player decides; a "Take Loans automatically" checkbox exists; with it off, an alert without pause. *(Confirmed by Diogo, 2026-10-17.)* Used here as a given.
- The three recommendations above. *(My proposals.)*
- The mockup PDF of 2026-10-17 is the reference for screen descriptions. *(Diogo, 2026-10-18.)*
- The Steel–Tools cycle in the first-build data. *(Fact from `82` Goods and the Steel ladder.)*
