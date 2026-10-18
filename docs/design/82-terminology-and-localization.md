# 82 — Terminology and Localization

**Status:** REVIEWED — §1, §4 and batches 1–6 decided. Three rows marked *waits* stay open: law names, the Military law ladder size, the unit roster.
**Date:** 2026-10-16
**Related:** `00-start-here.md`, `42-politica-e-grupos-de-interesse.md`, `80-interface-e-ux.md` (§12), `81-guia-de-design-de-ui.md` (§6.8), `90-estado-do-projeto.md` (§1.5, §5), `83-ui-artifact-index.md`, `PROMPT-RECON-RESTART.md`, `PROMPT-APPLY-TERMINOLOGY.md`, `93-sprint-backlog-01.md`, `95-recon-restart.md`
**Scope:** one name per concept across code, docs and game text; the termbase that holds those names; how translation plugs in later.

Numbering: `94` is reserved by `93` (S1-06 writes `94-recon-economy-v1.md`), so this doc takes `82`, next to the UI docs that consume it.

---

## §0 — How to read this document

- **[DECIDED]** — confirmed by Diogo.
- **[PROPOSED]** — my proposal, waiting for confirmation.
- **[OPEN]** — a choice only Diogo can make. Listed again in §4.
- **⚠ RECONCILIAR** — a name that must be checked against a doc or code this inventory could not read.
- ***Waits*** — a row whose name or definition waits for a missing doc or a later sprint; the reason follows the marker. (Until batch 6 this marker was *waits for recon*; the recon reported in `95`.)

---

## §1 — The decision

**[DECIDED] English is the source language of the project.** From now on:

- Term names, design docs, data IDs and game text are written in English, in US spelling (§3 rule 9).
- Conversation with Claude stays in Portuguese.
- Brazilian Portuguese becomes the first translation, done later as a localization pass, not mixed into the source.

This replaces:

- `90` §1.5 and §5.4: "documentation and comments in PT-BR".
- `81` §6.8: "interface text in Brazilian Portuguese". The number and date format rules there move to the PT-BR locale style guide when it is written.
- `80` §16.5 (languages of the first build): the source is EN, and the first build ships **EN only** (§4.15).

Existing PT-BR docs are not rewritten in one go. Each doc is converted when it is next edited, using the names in §5.

---

## §2 — Why the names drifted

Three vocabularies grew together and leaked into each other:

| Vocabulary | Example | Language today | Problem |
|---|---|---|---|
| **Code IDs** | `IG_Landowners`, `LAWCAT_Economy` | English | None. These are fine. |
| **Designer jargon** | tier, clout, enactment, PM, interlock | English, from Victoria 3 and EU4 communities | Used inside Portuguese sentences and inside player text. |
| **Player text** | "Tier de Aço", "Machinists", "Modifiers ativos", "votação" | Mixed | Had no owner. Each screen named things on the spot. |

The glossary in `90` §5 defined the jargon but never gave player-facing names, so nothing stopped the mix.

---

## §3 — Rules

**[PROPOSED]** unless marked.

1. **One concept, one ID, one name.** Every concept the player can see has a term ID (`steel_tier`) and one English name ("Steel Tier"). The ID never changes once `data/loc/terms.csv` exists; names can. Before that, IDs may still be renamed here.
2. **One word, one concept.** If two concepts want the same word, one of them gets another word. §4 lists the collisions found.
3. **Dev terms stay in docs and code.** Terms of kind `dev` (rail, dock, tick, interlock) never appear on screen.
4. **Docs use the English name.** When a doc talks about data, it adds the ID in backticks: Steel Tier (`steel_tier`).
5. **No abbreviations on screen** unless the termbase defines them (T1–T5, IG after first use, F-key labels).
6. **Proper names are not translated:** Albion, Galia, Norden, province names, *Le Grand Strategos*.
7. **Retired variants fail a check.** A script searches English docs and UI text for retired words and fails, the same way the ADR-0001 test guards the Sim boundary.
8. **[DECIDED batch 2] Countable terms carry their plural** in the termbase: Pop / Pops, Stratum / Strata, Peasant / Peasants. Screens never build a plural on the spot.
9. **[DECIDED batch 3] US spelling everywhere:** Labor, Center, Color, -ize. Godot and C# use US spelling (`Color`), so code and text match. British variants (Labour, Centre, Colour, -ise) are retired variants under rule 7.

**Shared names — the only exceptions to rule 2.** **[DECIDED, §4.9 and batches 2–5]** A name may sit on more than one row only when every row names the same domain seen from a different place: a panel, a map mode, a law category. The full list: **Military** (`lawcat_military`, `panel_military`, `mapmode_military`, `sphere_military`), **Religion** (`religion`, `lawcat_religion`), **Economy** (`lawcat_economy`, `panel_economy`, `sphere_economy`), **Nation** (`nation`, `panel_nation`), **Steel Tier** (`steel_tier`, `mapmode_steel_tier`), **Market** (`market`, `mapmode_market`). A new shared name needs a decision added here.

### 3.1 Termbase format

**[PROPOSED]** One row per term:

| Column | Content |
|---|---|
| `keys` | Godot string key, UPPER_SNAKE: `TERM_` + the ID in upper case (`TERM_STEEL_TIER`) |
| `en` | approved English name |
| `pt_BR` | empty until the translation pass |
| `_id` | snake_case, permanent (`steel_tier`) |
| `_kind` | `player` · `dev` · `name` |
| `_domain` | the §5 heading the row sits under |
| `_plural_en` | English plural, for countable terms (rule 8) |
| `_definition` | one sentence; this is what translators read |
| `_retired` | variants that must not be used, separated by `;` |
| `_status` | `decided` · `decided_waits` · `waits` · `proposed` |

Godot imports translations from CSV. The first column holds the keys and its header is ignored. Every other header is either an engine locale (`en`, `pt_BR`) or starts with `_`, which makes it a comment column that is not imported. Keys are case-sensitive and by convention uppercase. Source: Godot docs, *Localization using spreadsheets*.

**Plurals.** Since Godot 4.6 a CSV can carry plural forms: a `?plural` column holds the plural source, each locale's extra forms go on extra rows under the main row, and a `?pluralrule` row sets each locale's rule; plural strings are looked up with `tr_n()`. The draft keeps the plural in `_plural_en`; turning it into Godot's `?plural` layout is a step of `PROMPT-APPLY-TERMINOLOGY.md`, verified there with an import test. ⚠ RECONCILIAR: the exact multi-row layout against the Godot 4.7 docs' example.

**Where it lives:** this doc, until `PROMPT-APPLY-TERMINOLOGY.md` runs. That pass generates `data/loc/terms.csv` from §5 with the mapping above; from then on the CSV is the source and §5 is replaced by a pointer to it. One source at a time.

---

## §4 — Collisions and open choices

§4.1–4.15 confirmed by Diogo on 2026-10-09; §4.16–4.31 (batch 6, from the recon `95`) on 2026-10-16. Rows in §5 that these decisions settle carry **[DECIDED §4.N]** in their Note.

**4.1 Tier vs building level.** **[DECIDED]** *Tier* for rungs (T1–T5) on any ladder; *Level* for building size. The two only collide again in PT-BR ("nível"); that is for the PT-BR style guide.

**4.2 IG "Laborers" vs profession "Laborer".** **[DECIDED]** IG displayed as *Workers*; ID `IG_Laborers` stays as locked in `42`. Not "Labor Movement" (collides with law category Labor) and not "Trade Unions" (anachronistic in 1800).

**4.3 IG satisfaction vs needs satisfaction.** **[DECIDED]** IG: *Approval*. Pops: *Needs Met* (`needs_met`). *Alignment* stays separate: clout-weighted approval of the governing bloc. Retire "satisfaction" in player text.

**4.4 "Votação" (vote) vs `42` §7**, which says no voting is simulated. **[DECIDED]** *Enactment*; retire "vote". The probability is *Chance of Success* (not "Chance to Pass", which implies a vote). Outcomes are *Enacted* and *Failed*.

**4.5 Concept tier vs doctrine.** **[DECIDED]** Two concepts, not one. The unit's design is an ordinal ladder, one factor of Effectiveness; its player name is *Design Tier* (`design_tier`, batch 4). Doctrine is lateral: the Phase 2 design defined four permanent doctrinal axes (Maneuver & Shock, Firepower & Attrition, Discipline & Planning, Mobilization & Mass) that change each era, and `60` uses doctrine with commander Logistics as a source of command points. The Protocolo B session also kept them as separate lenses (concept tree vs doctrine). ⚠ RECONCILIAR: confirm in `60` that the four axes are still the doctrine model.

**4.6 Stratum vs social class.** **[DECIDED]** *Stratum* only (plural *Strata*). Code already uses `EPopStratum`. Retire "social class" and "class", including in `41`.

**4.7 Shortage vs deficit.** **[DECIDED]** *Shortage* and *Surplus* for goods; *Deficit* for money only. A positive money result is a positive *Balance*, never "surplus".

**4.8 "Bourgeoisie auto-investment" vs IG Petty Bourgeoisie.** **[DECIDED]** *Private Investment*, on screen and in docs. Code identifiers do not change.

**4.9 "Military".** **[DECIDED]** IG displayed as *Armed Forces*. Law category, panel and map mode share *Military*: the three name the same concept (the military domain) seen from three places, so rule 2 holds.

**4.10 Efficiency vs effectiveness.** **[DECIDED]** *Efficiency* for buildings, *Effectiveness* for units. Never swapped.

**4.11 Three-letter labels.** **[DECIDED]** No short labels. The rail shows icons only, with the full name on hover. Professions are always written in full. English names are short enough (Clerk, Clergy, Market, Military) that no abbreviation is needed. Retired: NAÇ, LEI, ECO, SOC, EXÉ, CLE. Rule 5 exceptions stay: T1–T5, IG, F-key labels.

**4.12 "Resource".** **[DECIDED]** *Good*, *Deposit*, *Strategic Good*; "resource" is not a player term.

**4.13 Demonyms.** **[DECIDED]** One regular *-ian* pattern for all three: *Albionian* / *Albionians*, *Galian* / *Galians*, *Nordenian* / *Nordenians*. "Nordic" is rejected because it names a real-world region.

**4.14 Era names.** **[DECIDED]** Format: *Era* + Roman numeral + one English noun (`Era II · Steam`; "Vapor" becomes *Steam*). Rule: an era name must not be the name of a good or an existing term, so "Steel", "Oil" and "Iron" are out (they collide with the goods and with Steel Tier). **[DECIDED batch 5]** The names: Era I · Flintlock, Era II · Steam, Era III · Telegraph, Era IV · Wireless, Era V · Mechanization, each the technology that marked its period. *Waits for recon:* each name is checked against the production method ladders in `33`; a clash renames that era. Structure already confirmed in an earlier session: five eras, Era 1 1800–1840, Era 2 1840–1870, Era 3 1870–1890, Era 4 1890–1914, Era 5 1914–1950. Earlier working titles (PT): Era 1 "Fogo em Linha", Era 2 "Aço e Vapor", Era 4 "Pólvora Sem Fumaça e o Grande Canhão", Era 5 "Guerra Total e Mecanização". ⚠ RECONCILIAR: no locked doc holds this structure yet.

**4.15 First-build languages.** **[DECIDED]** EN only. All text comes from keys from day one, and the missing-key test in `80` §12 runs from the start, so adding PT-BR later is filling one CSV column.

### Batch 6 — what the recon found (`95` §3, C1–C18)

**4.16 Steel ladder.** **[DECIDED]** The ladder of the goods doc (`34-goods-and-chains`, June 2026; it was written as `41-goods-and-chains` and renumbered because `41` is population): T1 Bloomery (Wood), T2 Puddling (blast furnace and puddling, Coal), T3 Bessemer Process (~1856), T4 Open Hearth (~1870), T5 Electric Arc (~1910, needs Electricity). Crucible is not a Tier: the goods doc keeps it as an optional branch. Electricity was deferred, so no nation runs T5 in the first build. The Unreal repo's three-method ladder and the mockups' ladder (Crucible at T3) are superseded.

**4.17 Market model.** **[DECIDED]** One **National Market** per nation, where its prices form: Economy v1's national stockpile, ported as coded. One **World Market**, the price reference for Imports and Exports, limited by Trade Capacity: **new design**, because Economy v1 has no trade at all, only unused scaffolding (`94` §9.5). The regional markets of the Unreal `20-economy.md` are out of the first build.

**4.18 Tier (C1).** **[DECIDED]** Tier only names rungs of a ladder: production methods, laws, unit designs. A good's processing depth is its category (Raw, Intermediate, Final). The Unreal nation tech tier (Underdeveloped / Developing / Developed) is superseded. The needs groups have names (§5). Unit experience gets its own word when a doc designs it.

**4.19 Level (C2).** **[DECIDED]** Level only names building size. Law rungs are Tiers (`42`). The tax setting is *Tax Rate*.

**4.20 Morale (C5).** **[DECIDED]** Morale belongs to units only. The national index the Unreal code called Civilian Morale is *Home Front*.

**4.21 Supply (C3).** **[DECIDED]** Supply is the market's. Military supply gets another word when a doc designs it.

**4.22 Modifier (C6).** **[DECIDED]** Modifier is a named effect only. The Unreal building policies (Push for Output and others) are not in the current design; if they return, they are *Building Policies*.

**4.23 Throughput (C7).** **[DECIDED]** Dev term only. The player sees Efficiency (buildings) and Trade Capacity (trade).

**4.24 Balance (C8).** **[DECIDED]** §4.7 stands: Balance is income minus expenses. The port names the money the state holds *Treasury*, never Balance (the Unreal `FTreasury::Balance`).

**4.25 Sphere (C10).** **[DECIDED]** Sphere names the technology spheres only. Diplomacy picks another word when it is designed.

**4.26 Campaign (C11).** **[DECIDED]** Campaign is one playthrough only.

**4.27 Capital (C12).** **[DECIDED]** Bare *Capital* is the capital province. Political Capital is never shortened, and its cap is *Political Capital Cap*.

**4.28 Government (C17).** **[DECIDED]** A building's owner is *State-Owned* or *Private*. Government types belong to the superseded Unreal design; the current one has the Governance law category.

**4.29 Currency (C18).** **[DECIDED]** *Crown* / *Crowns*, shown as a number with the Crown icon. Gold, Ouro, "g" and "£" are retired.

**4.30 Pop loyalty (C9).** **[OPEN]** Economy v1 moves a per-pop Loyalty each month, shown on screen as "Satisfação" or "Lealdade". Decide in the S1-06 recon whether it survives the port as its own value or folds into Needs Met. Until then it has no row.

**4.31 Closed without a new name.** C4 Stratum: the port separates Stratum and Profession (§5 Population). C13 Reserve: no conflict in English. C14 Labor: only the law category remains. C15 Doctrine: the Unreal AI theme becomes the dev term "AI Strategy". C16 Stance: dev only. Diário: Journal (batch 5). Also retired from the Unreal design: Reform (→ Enactment), Faction (→ Interest Group), Country Card (→ panel Nation), War Room (→ panel Military).

---

## §5 — Inventory

222 terms, grouped by domain. "Kind": `player` is shown on screen, `dev` stays in docs and code, `name` is a proper name. "Seen today" shows the most common words the mockups use for the concept right now, which is what gets retired. An "EN" cell written **Name** / Plural carries the plural of rule 8.

Batch status:

| Batch | Domains | State |
|---|---|---|
| 1 — Economy | Economy, Trade, Resources and deposits | **[DECIDED]** 2026-10-13, except rows marked *waits* |
| 2 — Population | Population | **[DECIDED]** 2026-10-13, except rows marked *waits* |
| 3 — Politics | Politics | **[DECIDED]** 2026-10-14, except law names and the Military ladder size |
| 4 — Military | Military, Technology | **[DECIDED]** 2026-10-15, except rows marked *waits* |
| 5 — Interface | Core and progression, Time and campaign, Meta, Interface, World and names | **[DECIDED]** 2026-10-15, except rows marked *waits* |
| 6 — Recon | Goods (new), the rows that waited for the recon, money, the collisions of `95` | **[DECIDED]** 2026-10-16, except three rows marked *waits* |

### Core and progression

| ID | EN | Kind | Meaning | Seen today in mockups | Docs | Note |
|---|---|---|---|---|---|---|
| `tier` | **Tier** / Tiers | player | Ordinal rung (T1–T5) on any ladder: production methods, laws, unit designs. | t2 ×23, tier ×21, t1 ×18, nível ×15 | 42 90 80 81 | **[DECIDED §4.1]** Building size uses Level. |
| `steel_tier` | **Steel Tier** | player | Tier of the best Steel production method running in a province or nation. | aço ×64, tier de aço ×14 | 80 81 | **[DECIDED]** The pillar. One name in map mode, top bar and tooltips. |
| `interlock` | **Interlock** | dev | Formula tying military effectiveness to industry: Design Tier × Equipment Tier. | interlock ×3 | 42 80 90 | **[DECIDED]** Player sees the two factors, not the word. |
| `modifier` | **Modifier** / Modifiers | player | A named effect from a law, tech, building or event; the only channel of effects. | modifiers ×6 | 42 80 90 | **[DECIDED]** |
| `odds` | **Chance** | player | Probability shown to the player, always as %. | chance ×7, odds ×2 | 80 | **[DECIDED]** Retire 'odds' in player text. |

### Economy

| ID | EN | Kind | Meaning | Seen today in mockups | Docs | Note |
|---|---|---|---|---|---|---|
| `production_method` | **Production Method** / Production Methods | player | How a building turns inputs into outputs; carries a Tier. | método ×11, métodos de produção ×2, pm ×1 | 42 80 90 91 | **[DECIDED]** Abbreviation PM is dev-only. |
| `building_level` | **Level** / Levels | player | Size of a building; each level adds jobs and output. | nível ×15, níveis ×1 | — | **[DECIDED §4.1]** Never 'tier'. |
| `good` | **Good** / Goods | player | Any tradeable item with a market price. | goods ×3, good ×3, bem ×1 | 33 90 93 | **[DECIDED §4.12]** Retire 'bem', 'mercadoria', 'resource', 'recurso'. |
| `good_raw` | **Raw** | player | Good category: extracted from deposits or land. | bruto ×1 | 90 93 | **[DECIDED]** |
| `good_intermediate` | **Intermediate** | player | Good category: made from raw goods, consumed by industry. | intermediário ×2 | 90 93 | **[DECIDED]** |
| `good_final` | **Final** | player | Good category: consumed by pops or the state. | final ×2 | 90 93 | **[DECIDED]** |
| `supply` | **Supply** | player | Amount of a good offered on a market per month. | — | 33 | **[DECIDED]** New in batch 1. |
| `demand` | **Demand** | player | Amount of a good wanted on a market per month. | — | 33 | **[DECIDED]** New in batch 1. |
| `market` | **Market** / Markets | player | Where a good's price forms from supply and demand. Two layers: National Market and World Market. | mercado ×33 | 33 80 90 95 | **[DECIDED batch 6]** §4.17. Retire 'Market Region', 'Local Market', 'Trade Hub'. |
| `national_market` | **National Market** | player | A nation's internal market: one per nation, where its prices form. | mercado nacional ×6 | 80 95 | **[DECIDED batch 6]** §4.17. The Economy v1 national stockpile. Retire 'stockpile' in player text. |
| `world_market` | **World Market** | player | Prices outside the nation; the reference for Imports and Exports. | mundo ×6, internacional ×5, mundial ×2 | 95 | **[DECIDED batch 6]** §4.17. One for the whole world in the first build. Retire 'Global Market'. |
| `shortage` | **Shortage** / Shortages | player | Demand above supply for a good. | falta ×12, déficit ×6, faltas ×6, em falta ×4 | 80 | **[DECIDED §4.7]** Goods only. Retire 'shortfall', 'Shortfall'. |
| `surplus` | **Surplus** | player | Supply above demand for a good. | superávit ×1 | — | **[DECIDED §4.7]** Goods only; never for money. |
| `price` | **Price** / Prices | player | Market price of a good. | preço ×7 | 33 | **[DECIDED]** |
| `input` | **Input** / Inputs | player | Good a production method consumes. | insumo ×2 | 80 | **[DECIDED]** |
| `output` | **Output** / Outputs | player | Good a production method produces. | — | 80 | **[DECIDED]** |
| `efficiency` | **Efficiency** | player | Multiplier on a building's output (staffing, inputs, modifiers). | eficiência ×7 | — | **[DECIDED §4.10]** Buildings only. Retire 'throughput', 'Throughput' in player text (§4.23). |
| `building` | **Building** / Buildings | player | A productive or civil structure in a province. | edifício ×8, edifícios ×7 | 90 93 | **[DECIDED]** |
| `building_slot` | **Building Slot** / Building Slots | player | Room for one building level in a province. | vagas ×2 | — | **[DECIDED]** |
| `construction` | **Construction** | player | A building or level being built; costs money, goods and months. | construir ×6, construção ×4, obras ×4, obra ×4 | 80 | **[DECIDED]** |
| `employment` | **Employment** | player | Share of a building's jobs filled. | emprego ×6, empregos ×3 | — | **[DECIDED]** |
| `profit` | **Profit** | player | Building income minus costs per month. | lucro ×2 | — | **[DECIDED]** |
| `private_investment` | **Private Investment** | player | Buildings funded automatically by the wealthy strata. | burguesia ×4 | 90 91 93 | **[DECIDED §4.8]** Also the doc term; retire 'Bourgeoisie auto-investment', 'auto-investment'. |
| `treasury` | **Treasury** | player | The state's money. | tesouro ×15 | 80 | **[DECIDED]** |
| `income` | **Income** | player | Money in per month. | receitas ×1 | 80 | **[DECIDED]** Money only; political capital uses Monthly Gain. |
| `expenses` | **Expenses** | player | Money out per month. | despesas ×2 | 80 | **[DECIDED]** |
| `balance` | **Balance** | player | Income minus expenses. | saldo ×1 | — | **[DECIDED §4.7]** A positive balance is not 'surplus'. |
| `deficit` | **Deficit** | player | Negative balance (money only). | — | — | **[DECIDED §4.7]** Money only. |
| `currency` | **Crown** / Crowns | player | The money unit of every nation. | — | 95 | **[DECIDED batch 6]** §4.29. Shown as a number with the Crown icon, no letter symbol (rule 5). Retire 'Gold', 'Ouro', 'g', '£'. |
| `taxes` | **Taxes** | player | Money collected from Pops each month, by Stratum. | — | 95 | **[DECIDED batch 6]** |
| `tax_rate` | **Tax Rate** | player | How hard a Stratum is taxed: Very Low, Low, Medium, High or Very High. | — | 95 | **[DECIDED batch 6]** §4.19. Code `ETaxLevel` keeps its ID. Retire 'Tax Level'. |
| `wages` | **Wages** | player | Money a building pays its workers each month. | — | 95 | **[DECIDED batch 6]** |
| `state_profits` | **State Profits** | player | Income from State-Owned buildings. | — | 95 | **[DECIDED batch 6]** |
| `maintenance` | **Maintenance** | player | Monthly upkeep of buildings, paid from the Treasury. | — | 95 | **[DECIDED batch 6]** |
| `army_upkeep` | **Army Upkeep** | player | Monthly cost of armies, paid from the Treasury. | — | 95 | **[DECIDED batch 6]** |
| `administration` | **Administration** | player | Monthly cost of running the state. | — | 95 | **[DECIDED batch 6]** Code `AdminCost`. |
| `debt` | **Debt** | player | Money the state owes. | — | 95 | **[DECIDED batch 6]** |
| `interest` | **Interest** | player | Monthly cost of Debt. | — | 95 | **[DECIDED batch 6]** |
| `loan` | **Loan** / Loans | player | Money the state borrows; it adds to Debt. | — | 95 | **[DECIDED batch 6]** Name only. Economy v1 takes loans automatically when the Treasury runs out; whether the port keeps that is an S1-06 question. |
| `bankruptcy` | **Bankruptcy** | player | A Treasury that cannot pay its Expenses. | — | 95 | **[DECIDED batch 6]** Name only; consequences undecided. |
| `ownership_state` | **State-Owned** | player | A building the state owns; its profit goes to the Treasury. | — | 95 | **[DECIDED batch 6]** §4.28. Retire 'Government' as an owner. |
| `ownership_private` | **Private** | player | A building Pops own; its profit goes to its owners. Private Investment builds them. | — | 95 | **[DECIDED batch 6]** §4.28. |
| `index_military_readiness` | **Military Readiness** | player | National index from 0.5 to 1.5, computed by the monthly economy tick. | — | 95 | **[DECIDED batch 6]** Economy v1 phase 9. |
| `index_industrial_capacity` | **Industrial Capacity** | player | National index from 0.5 to 1.5, computed by the monthly economy tick. | — | 95 | **[DECIDED batch 6]** Economy v1 phase 9. |
| `index_home_front` | **Home Front** | player | National index from 0.5 to 1.5 of civilian conditions, computed by the monthly economy tick. | — | 95 | **[DECIDED batch 6]** §4.20. Economy v1 phase 9. Retire 'Civilian Morale'. |
| `pm_bloomery` | **Bloomery** | player | Steel method, T1: iron ore smelted with Wood; needs no Tools. | — | 34 90 95 | **[DECIDED batch 6]** §4.16. |
| `pm_puddling` | **Puddling** | player | Steel method, T2: blast furnace and puddling on Coal. | pudlagem ×7 | 34 95 | **[DECIDED batch 6]** §4.16. Data ID `PM_BlastPuddling`. |
| `pm_bessemer` | **Bessemer Process** | player | Steel method, T3 (~1856): converter on Coal; needs Tools. | bessemer ×7 | 34 42 90 91 95 | **[DECIDED batch 6]** §4.16. T3, not T4. Retire 'Bessemer converter'. |
| `pm_open_hearth` | **Open Hearth** | player | Steel method, T4 (~1870): ore or scrap on Coal; needs Tools. | — | 34 95 | **[DECIDED batch 6]** §4.16. Retire 'Siemens-Martin'. |
| `pm_electric_arc` | **Electric Arc** | player | Steel method, T5 (~1910): ore or scrap on Electricity; needs Tools. | — | 34 95 | **[DECIDED batch 6]** §4.16. Electricity is deferred, so no nation runs T5 in the first build. Retire 'Crucible' (an optional branch, not a Tier). |

Batch 6 added the money rows Economy v1 already computes (`95` Part C, phases 5–9): taxes, wages, expenses, debt and the three national indices. Their numbers (rates, interest, ranges) are still `83` §3.5 debt.

### Trade

| ID | EN | Kind | Meaning | Seen today in mockups | Docs | Note |
|---|---|---|---|---|---|---|
| `tariff` | **Tariff** / Tariffs | player | Tax on imports of a good. | tarifa ×6, tarifas ×2 | — | **[DECIDED]** Name only; ranges per law ⚠ RECONCILIAR. |
| `tariff_income` | **Tariff Income** | player | Treasury income from tariffs. | alfândega ×2 | — | **[DECIDED]** Was `customs`; retire 'customs', 'alfândega'. |
| `import` | **Import** / Imports | player | Goods bought from the world market. | importa ×11 | — | **[DECIDED]** |
| `export` | **Export** / Exports | player | Goods sold to the world market. | exporta ×1 | — | **[DECIDED]** |
| `trade_capacity` | **Trade Capacity** | player | How much a nation can import and export per month, set by infrastructure. | capacidade de importação ×1 | 95 | **[DECIDED batch 6]** §4.17. Infrastructure types get names in the Sprint 02 geography batch. Retire 'bandwidth', 'Bandwidth'. |

### Resources and deposits

| ID | EN | Kind | Meaning | Seen today in mockups | Docs | Note |
|---|---|---|---|---|---|---|
| `deposit` | **Deposit** / Deposits | player | A source of a raw good in a province. | depósitos ×7, depósito ×3 | 43 80 90 | **[DECIDED §4.12]** Retire 'RGO', 'raw resource'. |
| `discovery` | **Discovery** | player | Monthly chance that an undiscovered deposit becomes discovered. | descoberto ×3, descoberta ×2 | 43 90 | **[DECIDED]** |
| `prospecting` | **Prospecting** | player | An ongoing search for a deposit in a province. | prospecção ×6, busca ×1 | 80 | **[DECIDED]** |
| `strategic_good` | **Strategic Good** / Strategic Goods | player | Good whose distribution gates nations geopolitically. | — | 33 90 | **[DECIDED §4.12]** Retire 'strategic resource'; a good, not a new type. |
| `discovered_undiscovered` | **Discovered / Undiscovered** | player | Whether a deposit has been discovered. | conhecido ×5, oculto ×4 | 80 | **[DECIDED]** Was `known_hidden`. 'Hidden' is dev-only (Designer Mode); retire 'known', 'conhecido', 'oculto' in player text. |

### Goods

The 31 goods of the goods doc (`34`, §4.16). "First build" marks the 12 the first build seeds (`93` S1-08). Data IDs are `G_` + the name in PascalCase (`G_IronOre`).

| ID | EN | Kind | Meaning | Seen today in mockups | Docs | Note |
|---|---|---|---|---|---|---|
| `good_grain` | **Grain** | player | Raw good: the food every Stratum eats. | — | 34 | **[DECIDED batch 6]** First build. |
| `good_iron_ore` | **Iron Ore** | player | Raw good: the ore Steel is made from. | — | 34 | **[DECIDED batch 6]** First build. |
| `good_coal` | **Coal** | player | Raw good: fuel for Steel from T2 and for industry. | — | 34 | **[DECIDED batch 6]** First build. |
| `good_wood` | **Wood** | player | Raw good: construction, Bloomery fuel, Furniture and Paper. | — | 34 | **[DECIDED batch 6]** First build. |
| `good_fiber` | **Fiber** | player | Raw good: cotton, wool and flax by region; made into Fabric. | — | 34 | **[DECIDED batch 6]** First build. Retire 'Cotton' as a good. |
| `good_fish` | **Fish** | player | Raw good: coastal food. | — | 34 | **[DECIDED batch 6]** |
| `good_colonial_goods` | **Colonial Goods** | player | Raw good: tea, coffee, sugar and tobacco from tropical plantations. | — | 34 | **[DECIDED batch 6]** |
| `good_copper` | **Copper** | player | Raw good: bronze for early Artillery; electrification later. | — | 34 | **[DECIDED batch 6]** |
| `good_sulfur` | **Sulfur** | player | Raw good: sulfur and saltpeter; made into Explosives. | — | 34 | **[DECIDED batch 6]** |
| `good_horses` | **Horses** | player | Raw good: cavalry and artillery draft. | — | 34 | **[DECIDED batch 6]** |
| `good_oil` | **Oil** | player | Raw good, late: fuel for navies, armor and aircraft. | — | 34 | **[DECIDED batch 6]** |
| `good_rubber` | **Rubber** | player | Raw good, late: tires, seals and insulation. | — | 34 | **[DECIDED batch 6]** |
| `good_bauxite` | **Bauxite** | player | Raw good, late: the ore Aluminum is made from. | — | 34 | **[DECIDED batch 6]** |
| `good_steel` | **Steel** | player | Intermediate good made by the Steel ladder; feeds Tools, Small Arms and Artillery. | — | 34 | **[DECIDED batch 6]** First build. Retire 'Pig Iron' (a production step, not a good). |
| `good_fabric` | **Fabric** | player | Intermediate good made from Fiber. | — | 34 | **[DECIDED batch 6]** First build. Retire 'Cloth'. |
| `good_tools` | **Tools** | player | Intermediate good; one good, its Tier lives in the methods of the building that makes it. Advanced methods need it. | — | 34 | **[DECIDED batch 6]** First build. |
| `good_explosives` | **Explosives** | player | Intermediate good made from Sulfur; feeds Ammunition and Artillery. | — | 34 | **[DECIDED batch 6]** |
| `good_glass` | **Glass** | player | Intermediate good made with Coal. | — | 34 | **[DECIDED batch 6]** |
| `good_paper` | **Paper** | player | Intermediate good made from Wood. | — | 34 | **[DECIDED batch 6]** |
| `good_aluminum` | **Aluminum** | player | Intermediate good, late, made from Bauxite. | — | 34 | **[DECIDED batch 6]** |
| `good_fertilizer` | **Fertilizer** | player | Intermediate good, late; raises Grain output. | — | 34 | **[DECIDED batch 6]** |
| `good_engines` | **Engines** | player | Intermediate good, late, made from Steel, Oil, Rubber and Copper. | — | 34 | **[DECIDED batch 6]** |
| `good_aircraft` | **Aircraft** | player | Intermediate good, late; dual-use, so civilians want it too. | — | 34 | **[DECIDED batch 6]** |
| `good_small_arms` | **Small Arms** | player | Final military good; its method Tier is the Equipment Tier of the units it equips. | — | 34 | **[DECIDED batch 6]** First build. |
| `good_artillery` | **Artillery** | player | Final military good; bare "Artillery" always means this good. | — | 34 | **[DECIDED batch 6]** First build. |
| `good_clothes` | **Clothes** | player | Final good made from Fabric; worn by Pops and as uniforms. | — | 34 | **[DECIDED batch 6]** First build. Retire 'Garments'. |
| `good_furniture` | **Furniture** | player | Final good made from Wood. | — | 34 | **[DECIDED batch 6]** First build. |
| `good_ammunition` | **Ammunition** | player | Final military good made from Explosives. | — | 34 | **[DECIDED batch 6]** |
| `good_liquor` | **Liquor** | player | Final good made from Grain. | — | 34 | **[DECIDED batch 6]** |
| `good_groceries` | **Groceries** | player | Final good: processed food from Grain, Fish and Colonial Goods. | — | 34 | **[DECIDED batch 6]** Retire 'Bread'. |
| `good_luxuries` | **Luxuries** | player | Final good made from Fabric, Glass and Colonial Goods. | — | 34 | **[DECIDED batch 6]** |

### Population

| ID | EN | Kind | Meaning | Seen today in mockups | Docs | Note |
|---|---|---|---|---|---|---|
| `pop` | **Pop** / Pops | player | A group of people sharing profession, stratum, culture and province. | pop ×4, pops ×1 | 41 90 | **[DECIDED]** |
| `stratum` | **Stratum** / Strata | player | Lower, Middle or Upper; sets political weight and consumption. | estrato ×6, estratos ×1 | 41 42 90 | **[DECIDED §4.6]** Retire 'social class', 'class'. |
| `stratum_lower` | **Lower Stratum** | player | The bottom of the three strata. | lower ×9, estrato baixo ×1 | 41 | **[DECIDED]** |
| `stratum_middle` | **Middle Stratum** | player | The middle of the three strata. | middle ×2, estrato médio ×1 | 41 | **[DECIDED]** |
| `stratum_upper` | **Upper Stratum** | player | The top of the three strata. | upper ×5, estrato alto ×1 | 41 | **[DECIDED]** |
| `profession` | **Profession** / Professions | player | What a pop does in production. | profissão ×2, profissões ×1 | 41 80 90 | **[DECIDED §4.11]** Always written in full. |
| `prof_peasant` | **Peasant** / Peasants | player | Profession, Lower Stratum: farming and rural work. | peasant ×4 | 41 80 95 | **[DECIDED batch 6]** Retire 'Farmer'. |
| `prof_laborer` | **Laborer** / Laborers | player | Profession, Lower Stratum: unskilled work in buildings and extraction. | laborer ×7, laborers ×1 | 80 | **[DECIDED §4.2]** IG_Laborers displays as Workers. |
| `prof_machinist` | **Machinist** / Machinists | player | Profession, Lower Stratum: skilled factory work. | machinists ×3, machinist ×2 | 80 95 | **[DECIDED batch 6]** Retire 'Factory Worker'. |
| `prof_artisan` | **Artisan** / Artisans | player | Profession, Middle Stratum: workshop production by hand. | artisan ×5, artisans ×3 | 80 95 | **[DECIDED batch 6]** |
| `prof_clerk` | **Clerk** / Clerks | player | Profession, Middle Stratum: office and administrative work. | clerk ×2 | 80 | **[DECIDED §4.11]** No short label; 'CLE' retired. Retire 'Bureaucrat'. |
| `prof_capitalist` | **Capitalist** / Capitalists | player | Profession, Upper Stratum: owns Private buildings and funds Private Investment. | capitalist ×3 | 80 95 | **[DECIDED batch 6]** Code `EPopStratum::Bourgeoisie` maps here. Not retired as a word, because the IG Petty Bourgeoisie keeps it. |
| `prof_aristocrat` | **Aristocrat** / Aristocrats | player | Profession, Upper Stratum: landed nobility. | aristocrat ×1 | 80 95 | **[DECIDED batch 6]** Retire 'Aristocracy'. |
| `prof_soldier` | **Soldier** / Soldiers | player | Profession, Lower Stratum: serves in the army; Manpower drains it first. | soldier ×1 | 41 80 95 | **[DECIDED batch 6]** |
| `prof_officer` | **Officer** / Officers | player | Profession, Middle Stratum: leads units. | officer ×2 | 41 80 95 | **[DECIDED batch 6]** |
| `needs` | **Needs** | player | Goods a Stratum consumes, in three groups: Life Needs, Everyday Needs, Luxury Needs. | necessidades ×2 | 41 80 81 95 | **[DECIDED]** Name. **[DECIDED batch 6]** The three groups below. |
| `needs_life` | **Life Needs** | player | First group of needs: food and basic goods. | — | 95 | **[DECIDED batch 6]** Names from the Unreal `20-economy.md`; contents follow `41`. |
| `needs_everyday` | **Everyday Needs** | player | Second group of needs: clothes, furniture and common comforts. | — | 95 | **[DECIDED batch 6]** |
| `needs_luxury` | **Luxury Needs** | player | Third group of needs: luxuries. | — | 95 | **[DECIDED batch 6]** |
| `needs_met` | **Needs Met** | player | Share of a pop's needs currently satisfied. | satisfação ×4 | 80 81 | **[DECIDED §4.3]** Retire 'satisfaction' for pops. |
| `literacy` | **Literacy** | player | Share of a pop that can read; gates some professions. | alfabetização ×4 | 41 95 | **[DECIDED batch 6]** Name. Mechanic open in `41`. |
| `promotion_demotion` | **Promotion / Demotion** | player | Pops moving up or down between professions and strata over time. | — | 41 | **[DECIDED]** Name. Was `mobility`; retire 'mobility' (reads as moving between provinces, which is Migration). Mechanic open in `41`. |
| `unemployed` | **Unemployed** | player | Pops without a job. | sem emprego ×1 | — | **[DECIDED]** |
| `migration` | **Migration** | player | Pops moving between provinces. | migração ×4 | 90 91 | **[DECIDED]** Name only. Mechanic undecided (doc 41 v1.2). |
| `culture` | **Culture** / Cultures | player | Per-pop cultural identity. | cultura ×3 | 41 90 | **[DECIDED]** |
| `religion` | **Religion** / Religions | player | The faith of a province (territorial, EU4 model). | religião ×4 | 41 42 90 | **[DECIDED]** Shares the word with law category Religion: the same domain seen from two places (as §4.9). |
| `state_religion` | **State Religion** | player | The nation's official faith. | — | 41 | **[DECIDED]** New in batch 2. |

Strata of the professions **[DECIDED batch 6]**: Lower — Peasant, Laborer, Machinist, Soldier. Middle — Artisan, Clerk, Officer. Upper — Capitalist, Aristocrat. There is no Clergy profession: religion is territorial (`41`). Engineer, used by one Unreal setup doc, is not a profession.

### Politics

| ID | EN | Kind | Meaning | Seen today in mockups | Docs | Note |
|---|---|---|---|---|---|---|
| `interest_group` | **Interest Group** / Interest Groups | player | Political faction derived from strata by affinity. | grupo ×5, grupos de interesse ×2 | 42 80 90 | **[DECIDED]** IG allowed as a short label after first use. Retire 'faction', 'Faction'. |
| `ig_landowners` | **Landowners** | player | IG_Landowners. | latifundiários ×11, latif. ×1 | 42 | **[DECIDED]** |
| `ig_industrialists` | **Industrialists** | player | IG_Industrialists. | industrialistas ×6 | 42 | **[DECIDED]** |
| `ig_petty_bourgeoisie` | **Petty Bourgeoisie** | player | IG_PettyBourgeoisie. | peq. burguesia ×4 | 42 | **[DECIDED]** |
| `ig_laborers` | **Workers** | player | IG_Laborers. Display differs from the profession on purpose. | trabalhadores ×9 | 42 | **[DECIDED §4.2]** |
| `ig_military` | **Armed Forces** | player | IG_Military. | forças armadas ×7, f. armadas ×1 | 42 | **[DECIDED §4.9]** |
| `ig_clergy` | **Clergy** | player | IG_Clergy. | clero ×4 | 42 | **[DECIDED]** |
| `clout` | **Clout** | player | An IG's share of political power; fractions only. | clout ×6, influência ×1 | 42 80 90 | **[DECIDED]** Retire 'influência'. |
| `governing_bloc` | **Governing Bloc** | player | IGs that together pass 50% clout, largest first. | bloco ×10, bloco governante ×4, quem manda ×1 | 42 80 90 | **[DECIDED]** |
| `outside_bloc` | **Outside the Bloc** | player | IGs not in the governing bloc; they affect enactment cost only (`42` §4.3). | — | 42 | **[DECIDED]** New in batch 3. Never 'opposition', which is clout against an enactment. |
| `ig_approval` | **Approval** | player | How close current laws are to an IG's preferred tiers. | sat. ×9, satisfação ×4 | 42 80 90 | **[DECIDED §4.3]** Retire 'satisfaction' for IGs. |
| `alignment` | **Alignment** | player | Clout-weighted approval of the governing bloc; multiplies Monthly Gain. | alinhamento ×2 | 42 90 | **[DECIDED §4.3]** Stays apart from Approval. |
| `preferred_tier` | **Preferred Tier** | player | The tier an IG wants in a law category. | quer ×7 | 42 80 90 | **[DECIDED]** Dev term 'stance' (`FLawStance`) stays in code. |
| `law` | **Law** / Laws | player | One rung on a law category ladder; exactly one active per category. | lei ×16, leis ×4 | 42 52 | **[DECIDED]** *Waits:* names of each law (`52`, missing from the Project and the repo). |
| `law_category` | **Law Category** / Law Categories | player | Economy, Labor, Military, Governance, Religion. | categoria ×2 | 42 52 | **[DECIDED]** |
| `lawcat_economy` | **Economy** | player | LAWCAT_Economy. | economia ×7 | 42 52 | **[DECIDED]** |
| `lawcat_labor` | **Labor** | player | LAWCAT_Labor. | trabalho ×12 | 42 52 | **[DECIDED]** US spelling (rule 9); retire 'Labour'. |
| `lawcat_military` | **Military** | player | LAWCAT_Military. | militar ×20 | 42 52 | **[DECIDED §4.9]** Shares the word with panel, map mode and Sphere. *Waits:* ladder size, for Sprint 05. M1 = 0 (`95`): there is no existing mechanic to absorb, so the ladder is new design; candidate Peacetime / Partial / Full Mobilization (Unreal `41-military.md`). |
| `lawcat_governance` | **Governance** | player | LAWCAT_Governance. | governança ×2 | 42 52 | **[DECIDED]** |
| `lawcat_religion` | **Religion** | player | LAWCAT_Religion. | religião ×4 | 42 52 | **[DECIDED batch 2]** Shares the word with province Religion. |
| `enactment` | **Enactment** / Enactments | player | Changing a law: pay up front, wait N months, one roll at the end. | votação ×13, em votação ×3 | 42 80 90 | **[DECIDED §4.4]** Retire 'vote', 'votação'. Retire 'reform', 'Reform' (Unreal design, §4.31). |
| `enactment_enacted` | **Enacted** | player | Enactment outcome: the law changed. | — | 42 | **[DECIDED §4.4]** |
| `enactment_failed` | **Failed** | player | Enactment outcome: the law did not change; capital is spent. | — | 42 | **[DECIDED §4.4]** |
| `political_capital` | **Political Capital** | player | Single political resource; only use in v1 is enactment. | capital ×20, capital político ×4 | 42 80 90 | **[DECIDED]** |
| `capital_gain` | **Monthly Gain** | player | Political capital generated per month: Governance base × Alignment (`42` §5.2). | — | 42 | **[DECIDED]** New in batch 3. Never 'Income' (money) or 'Capital Gain' (a real finance term). |
| `capital_cap` | **Political Capital Cap** | player | Maximum political capital (24× base gain); excess is lost. | teto ×2 | 42 80 | **[DECIDED]** Was 'Cap'. **[DECIDED batch 6]** Was Capital Cap (§4.27). Top bar shows "120 / 240"; the name is in the tooltip. |
| `enactment_chance` | **Chance of Success** | player | P(success) if the enactment resolved today. | chance hoje ×1, chance se resolvesse ×1, chance de passar ×1 | 42 80 | **[DECIDED §4.4]** Retire 'Chance to Pass'. |
| `support_opposition` | **Support / Opposition** | player | Clout moving toward / away from IG preferred tiers. | apoio ×2, oposição ×1 | 42 80 | **[DECIDED]** Only for an enactment; see `outside_bloc`. |
| `tech_gate` | **Requires** | player | Technology a law or method needs before it appears. | exige ×2, gate ×1 | 42 | **[DECIDED]** Dev term 'tech gate'; player sees 'Requires …'. |

### Military

| ID | EN | Kind | Meaning | Seen today in mockups | Docs | Note |
|---|---|---|---|---|---|---|
| `unit` | **Unit** / Units | player | One military formation inside an army. | — | 60 80 | **[DECIDED]** New in batch 4. |
| `army` | **Army** / Armies | player | A movable force with units and a commander. | exército ×16, exércitos ×5 | 80 | **[DECIDED]** |
| `battle` | **Battle** / Battles | player | A fight between armies in one province. | — | 60 80 | **[DECIDED]** New in batch 4. |
| `war` | **War** / Wars | player | State of armed conflict between nations. | — | 80 91 | **[DECIDED]** New in batch 4. |
| `peace` | **Peace** | player | State of no armed conflict between nations. | — | 91 | **[DECIDED]** New in batch 4. |
| `manpower` | **Manpower** | player | Men available to recruit; drains military pops first. | manpower ×7, reserva ×7 | 60 80 90 | **[DECIDED]** Retire 'reserva'. |
| `conscription` | **Conscription** | player | Drafting civilian pops once military pops run out. | conscrição ×3, convocação ×1 | 60 80 | **[DECIDED]** Retire 'convocação'. |
| `recruitment` | **Recruitment** | player | Adding units to an army; costs manpower, goods, money. | recrutar ×4, recrutamento ×2, reforçar ×1 | — | **[DECIDED]** 'reforçar' (refilling an existing unit) is a different concept; no doc defines it, so no row yet. |
| `design_tier` | **Design Tier** | player | Tier of a unit's design (Line Infantry → Skirmishers → Shock Troops); one factor of Effectiveness. | conceito ×5 | 42 80 90 | **[DECIDED §4.5]** Was `unit_concept`; retire 'concept', 'conceito', 'Concept Tier'. |
| `equipment_tier` | **Equipment Tier** | player | Tier of the production method of the military good that equips a unit (Small Arms, Artillery); the other factor of Effectiveness. | — | 42 90 | **[DECIDED]** New in batch 4. Replaces 'Method Tier' in player text. |
| `effectiveness` | **Effectiveness** | player | Design Tier × Equipment Tier. | eficácia ×2 | 42 80 90 | **[DECIDED §4.10]** Units only. |
| `doctrine` | **Doctrine** / Doctrines | player | Lateral doctrinal axis; with commander Logistics, a source of command points (doc 60). | doutrina ×6 | 60 90 | **[DECIDED §4.5]** Name; not a tier ladder. Model open in `61` §4, which records the candidate models. |
| `commander` | **Commander** / Commanders | player | Leads an army; Logistics feeds command points. | comandante ×8, comandantes ×3, cel. ×3, gen. ×1 | 60 90 | **[DECIDED]** Retire 'cel.', 'gen.'. Ranks get rows only when a doc defines them. |
| `morale` | **Morale** | player | A unit's cohesion in battle, 0–100; below 25 the unit routs. | mor ×3, moral ×1 | 10 60 95 | **[DECIDED batch 6]** §4.20. Units only. |
| `command_points` | **Command Points** | player | Tactical battle resource (phase 2). | — | 60 90 | **[DECIDED]** |
| `auto_resolve` | **Auto-resolve** | player | Battle resolved by the same engine without the card UI. | auto-resolve ×2, resolução automática ×2 | 60 80 90 | **[DECIDED]** Retire 'resolução automática'. |
| `battle_report` | **Battle Report** / Battle Reports | player | Why a battle went the way it did. | relatório ×4 | 80 | **[DECIDED]** |
| `occupation` | **Occupation** | player | Enemy control of a province during war. | ocupad ×3 | 80 | **[DECIDED]** |
| `line_infantry` | **Line Infantry** | player | Unit type. | infantaria de linha ×3 | — | *Waits:* the unit roster of the first build (`60`). |

Unit types are always qualified ("Field Artillery", "Siege Artillery"). Bare "Artillery" is the good `G_Artillery`. **[DECIDED batch 4]**

### Technology (phase 2)

| ID | EN | Kind | Meaning | Seen today in mockups | Docs | Note |
|---|---|---|---|---|---|---|
| `technology` | **Technology** / Technologies | player | Emerges through spark, diffusion, adoption. | tecnologia ×1 | 50 90 | **[DECIDED]** Phase 2. |
| `tech_sphere` | **Sphere** / Spheres | player | One of seven civilizational divisions (ETechSphere). | esfera ×1 | 50 90 | **[DECIDED]** Name. **[DECIDED batch 6]** The seven below, from the code (`ETechSphere`). Technology only (§4.25). |
| `sphere_political_social` | **Political & Social** | player | Tech Sphere: government, law and society. | — | 50 95 | **[DECIDED batch 6]** |
| `sphere_production_energy` | **Production & Energy** | player | Tech Sphere: industry, methods and power. | — | 50 95 | **[DECIDED batch 6]** |
| `sphere_economy` | **Economy** | player | Tech Sphere: trade, finance and markets. | — | 50 95 | **[DECIDED batch 6]** Shared name (§3). |
| `sphere_military` | **Military** | player | Tech Sphere: armies, weapons and war. | — | 50 95 | **[DECIDED batch 6]** Shared name (§3). |
| `sphere_communication` | **Communication** | player | Tech Sphere: transport, post and signals. | — | 50 95 | **[DECIDED batch 6]** |
| `sphere_culture_worldview` | **Culture & Worldview** | player | Tech Sphere: ideas, belief and education. | — | 50 95 | **[DECIDED batch 6]** |
| `sphere_natural_science` | **Natural Science** | player | Tech Sphere: physics, chemistry and biology. | — | 50 95 | **[DECIDED batch 6]** |
| `spark_diffusion_adoption` | **Spark / Diffusion / Adoption** | player | The three phases of tech emergence. | — | 50 90 | **[DECIDED]** Phase 2. |

### Time and campaign

| ID | EN | Kind | Meaning | Seen today in mockups | Docs | Note |
|---|---|---|---|---|---|---|
| `era` | **Era** / Eras | player | Named period of the campaign (e.g. Era II · Steam). | era ×13 | 81 | **[DECIDED §4.14]** Format, rule and the five names. **[DECIDED batch 6]** Checked against the method names of the goods doc (`34`): no clash. "Steam Engine" is an Engines method, a different name. |
| `tick` | **Tick** | dev | One simulation step (monthly in v1). | tick ×3 | 42 80 93 | **[DECIDED]** Player sees months, not ticks. |
| `game_speed` | **Speed** | player | Game speed 1–5. | velocidade ×7 | 80 | **[DECIDED]** |
| `pause` | **Pause** | player | Stops the simulation; commands still work and apply on the next tick. | pausa ×22, pausado ×7 | 80 | **[DECIDED]** |
| `soft_end` | **Continue Playing** | player | Choice offered at 1950 to keep going. | fim suave ×2 | 80 90 | **[DECIDED]** Dev term 'soft end'. |
| `campaign` | **Campaign** / Campaigns | player | One playthrough. | partida ×15, campanha ×2 | 90 | **[DECIDED]** Retire 'partida'; the templates' 'Nova partida' becomes 'New Campaign'. One playthrough only (§4.26). |

### Meta

| ID | EN | Kind | Meaning | Seen today in mockups | Docs | Note |
|---|---|---|---|---|---|---|
| `autosave` | **Autosave** | player | Save made automatically by the game. | autosave ×7 | 81 | **[DECIDED]** |
| `game_title` | **Le Grand Strategos** | name | Game title; French on purpose. | lgs ×23, le grand strategos ×3 | 90 | **[DECIDED]** Not translated. |

### Interface

| ID | EN | Kind | Meaning | Seen today in mockups | Docs | Note |
|---|---|---|---|---|---|---|
| `outliner` | **Outliner** | player | Right column listing ongoing processes. | outliner ×10 | 80 81 | **[DECIDED]** Paradox players know the word. |
| `journal` | **Journal** | player | History of past events (tab beside the Outliner). | diário ×27 | 81 | **[DECIDED]** Retire 'Diary'. |
| `gazette` | **Gazette** | player | Newspaper-style overlay for major events; pauses the game. | gazeta ×11 | 80 81 | **[DECIDED]** |
| `alert` | **Alert** / Alerts | player | Actionable notice in the alert strip. | alerta ×3, alertas ×1 | 80 81 | **[DECIDED]** |
| `map_mode` | **Map Mode** / Map Modes | player | What the map colors show. | modo ×14, modos de mapa ×1 | 80 81 | **[DECIDED]** |
| `mapmode_political` | **Political** | player | Map mode. | político ×7, pol ×3 | 80 | **[DECIDED §4.11]** Rail shows icon only; 'pol' retired. |
| `mapmode_interest_groups` | **Interest Groups** | player | Map mode: the IG with the largest local base in each province. | base política ×2 | 80 | **[DECIDED]** Was `mapmode_political_base`; retire 'Political Base', 'base política'. Rail shows icon only (§4.11). |
| `mapmode_steel_tier` | **Steel Tier** | player | Map mode: Steel Tier per province. | — | 80 83 | **[DECIDED]** New in batch 5. Same name as the term it shows. |
| `mapmode_market` | **Market** | player | Map mode: market prices and shortages. | — | 80 83 | **[DECIDED]** New in batch 5. **[DECIDED batch 6]** Shows the National Market (§4.17). |
| `mapmode_deposits` | **Deposits** | player | Map mode: discovered deposits and prospecting. | — | 80 83 | **[DECIDED]** New in batch 5. |
| `mapmode_population` | **Population** | player | Map mode: pops per province. | — | 80 83 | **[DECIDED]** New in batch 5. |
| `mapmode_military` | **Military** | player | Map mode: armies, occupation and fronts. | — | 80 83 | **[DECIDED §4.9]** New in batch 5. |
| `panel_nation` | **Nation** | player | Panel F1. | nação ×22, naç ×1 | 80 | **[DECIDED]** 'NAÇ' retired (§4.11). Shares the word with `nation`. Retire 'Country Card'. |
| `panel_politics` | **Politics** | player | Panel F2. | política ×16, lei ×16 | 80 | **[DECIDED]** 'LEI' retired (§4.11). |
| `panel_economy` | **Economy** | player | Panel F3. Tabs: National Market, World Market, Treasury. | eco ×2 | 80 | **[DECIDED]** 'ECO' retired (§4.11). Shares the word with law category Economy (same domain, as §4.9). Each market tab carries the name of the layer it shows ('International' → 'World Market'). **[DECIDED batch 6]** Tab names confirmed by §4.17. |
| `panel_society` | **Society** | player | Panel F5. | sociedade ×5, soc ×2 | — | **[DECIDED]** 'SOC' retired (§4.11). |
| `panel_military` | **Military** | player | Panel F4. | exé ×1 | 80 | **[DECIDED §4.9, §4.11]** 'EXÉ' retired. Retire 'War Room'. |
| `tooltip_lock` | **Lock** | player | Keep a nested tooltip open. | fixa ×3, fixado ×3, fixar ×2, trava ×1 | 80 81 | **[DECIDED]** |
| `designer_mode` | **Designer Mode** | dev | Dev-only overlay; never in release builds. | modo designer ×5 | 80 81 | **[DECIDED]** |
| `ui_rail` | **Rail** | dev | Left icon column: map modes and panels. | rail ×10 | 81 | **[DECIDED]** Never shown to the player. Icons only, full name on hover (§4.11). |
| `ui_system_panel` | **System Panel** | dev | 560 px left panel opened from the rail. | — | 81 | **[DECIDED]** |
| `ui_selection_dock` | **Selection Dock** | dev | 760×560 bottom-right panel for what the player clicked. | dock ×12 | 81 | **[DECIDED]** |
| `ui_top_bar` | **Top Bar** | dev | Bar across the top: date, speed, pause, Treasury, Political Capital. | — | 80 81 93 | **[DECIDED batch 6]** |
| `ui_alert_strip` | **Alert Strip** | dev | Row of Alerts under the Top Bar; only what needs action. | — | 80 81 93 | **[DECIDED batch 6]** |
| `ui_provenance_tooltip` | **Provenance Tooltip** | dev | Tooltip that breaks a number into its sources. | — | 80 81 93 | **[DECIDED batch 6]** |
| `ui_template` | **Template** / Templates | dev | Reusable screen layout and behavior shared by all games. | molde ×5, moldes ×3 | 81 | **[DECIDED]** Retire 'molde'. |
| `ui_skin` | **Skin** / Skins | dev | Per-game colors, fonts and name. | skin ×12 | 81 | **[DECIDED]** |

### World and names

| ID | EN | Kind | Meaning | Seen today in mockups | Docs | Note |
|---|---|---|---|---|---|---|
| `nation` | **Nation** / Nations | player | A country: Albion, Galia, Norden. | nação ×22 | 90 | **[DECIDED]** New in batch 5. Shares the word with panel F1. |
| `province` | **Province** / Provinces | player | The smallest map region; holds pops, buildings and deposits. | — | 80 90 | **[DECIDED]** New in batch 5. |
| `capital` | **Capital** / Capitals | player | A nation's capital province. | — | 95 | **[DECIDED batch 6]** §4.27. Never short for Political Capital. |
| `nation_albion` | **Albion** | name | Industrial power. | albion ×22 | 90 | **[DECIDED §4.13]** Demonym Albionian / Albionians. |
| `nation_galia` | **Galia** | name | Centralized gunpowder state. | galia ×32 | 90 | **[DECIDED §4.13]** Demonym Galian / Galians; 'galianos' retired. |
| `nation_norden` | **Norden** | name | Martial frontier, rich iron, primitive metallurgy. | norden ×59 | 90 | **[DECIDED §4.13]** Demonym Nordenian / Nordenians. |

---

## §6 — How the review runs

**[PROPOSED]**

1. **Collisions first.** ✅ Done 2026-10-09: all of §4 decided.
2. **Batches by domain.** One domain per sitting (economy, population, politics, military, interface). Claude proposes, Diogo confirms; confirmed rows get marked **[DECIDED]**. Batches 1–2 ✅ 2026-10-13, batch 3 ✅ 2026-10-14, batches 4–5 ✅ 2026-10-15, batch 6 ✅ 2026-10-16. This doc becomes LOCKED v1.0 when its three *waits* rows close.
3. **Fill the gaps.** ✅ `PROMPT-RECON-RESTART.md` reported on 2026-10-15 (`95`, Parts B and C, over the Unreal repo). Batch 6 settled every row it answered; the three left wait for `52`, Sprint 05 and the unit roster.
4. **Apply.** `PROMPT-APPLY-TERMINOLOGY.md`, after the recon and Diogo's approval: termbase into the repo, the termbase test, the retired-variant check, `CLAUDE.md`. Docs are renamed as each is next edited; one pass on the mockup canvases (`83`) in claude.ai.
5. **Guard.** The retired-variant check (§3 rule 7) runs in the repo from step 4 on, over the docs that are already in English.
6. **Geography batch (Sprint 02).** Terrain, climate and infrastructure names, needed by the FMG import.
7. **Translate.** Only after the EN names are stable: fill `pt_BR`, write the PT-BR style guide (date format, decimal comma, tone).

---

## §7 — What this inventory covered

- **Read in full:** `42`, `80`, `81`, `90`, `91`, `92`, `93`.
- **Scanned automatically:** all 25 boards of the "LG — Mockups de Interface 2" canvas and all 19 boards of the "Moldes de telas comuns" canvas. The "Seen today" column counts matches in their visible text. Counts are approximate: a short word like "aço" also matches unrelated uses.
- **From earlier sessions (chat archive, not docs):** the five-era structure and the four doctrinal axes used in §4.5 and §4.14.
- **Recon (`95`, 2026-10-15):** the Unreal repo's 28 docs and 197 code files. Batch 6 comes from it.
- **Goods doc (chat archive, June 2026):** the 31 goods and the Steel ladder (§4.16); the doc itself is missing from the Project and the repo.
- **Not covered:** the Godot repo (not on GitHub) and the missing system docs (`33`, `34`, `41`, `43`, `50`, `51`, `52`, `60`, `70`).

---

## §8 — Out of scope

- Translating anything into PT-BR now.
- Languages other than EN and PT-BR.
- Concepts of the superseded Unreal design that `95` found (elections, parties, ideologies, factions, the 11 law categories, diplomacy, battle cards, production modifiers, Trade Hub): no rows until a current doc defines them. Listed in `00` §6.
- Renaming locked code IDs. IDs from LOCKED docs (`IG_*`, `LAWCAT_*`) stay; only their display names are chosen here.

---

## Appendix — Decisions taken

- English is the source language for terms, docs, data and game text; conversation stays in Portuguese; PT-BR comes later as a translation. *(Confirmed by Diogo.)*
- All fifteen §4 resolutions, including the refinements: Needs Met, Chance of Success / Enacted / Failed, doctrine separate from the concept ladder, Surplus for goods only, no short labels on screen, *-ian* demonyms, era name format and rule, EN-only first build. *(Confirmed by Diogo, 2026-10-09.)*
- Batch 1 (Economy, Trade, Resources and deposits): every row not marked *waits*; `customs` → Tariff Income (`tariff_income`); `known_hidden` → Discovered / Undiscovered (`discovered_undiscovered`); new rows Supply and Demand; Market panel tabs carry the market layer names ('International' → 'World'). *(Confirmed by Diogo, 2026-10-13.)*
- Batch 2 (Population): every row not marked *waits*; `mobility` → Promotion / Demotion (`promotion_demotion`); new row State Religion; province Religion and law category Religion share the word; rule 8, countable terms carry a plural in the termbase. *(Confirmed by Diogo, 2026-10-13.)*
- Batch 3 (Politics): every row except law names and the Military ladder size; rule 9, US spelling everywhere; Cap → Capital Cap; new rows Monthly Gain (`capital_gain`) and Outside the Bloc (`outside_bloc`); 'opposition' reserved for clout against an enactment. *(Confirmed by Diogo, 2026-10-14.)*
- Batch 4 (Military, Technology): every row not marked *waits*; `unit_concept` → Design Tier (`design_tier`); new rows Equipment Tier, Unit, Battle, War, Peace; Effectiveness = Design Tier × Equipment Tier; unit types always qualified, bare "Artillery" is the good; rank abbreviations retired. *(Confirmed by Diogo, 2026-10-15.)*
- Batch 5 (Core, Time and campaign, Meta, Interface, World): every row not marked *waits*; panel F3 Market → Economy with tabs National Market / World Market / Treasury; map mode Political Base → Interest Groups (`mapmode_interest_groups`); new rows for the map modes Steel Tier, Market, Deposits, Population, Military and for Nation and Province; era names Flintlock, Steam, Telegraph, Wireless, Mechanization; 'Nova partida' → 'New Campaign'. *(Confirmed by Diogo, 2026-10-15.)*
- Rule 1 amended: IDs are permanent once `data/loc/terms.csv` exists; before that they may be renamed here. *(My proposal, applied to the renames above.)*
- Termbase columns (§3.1) laid out for Godot's CSV import: `keys` first, locales, then `_` comment columns; plurals kept in `_plural_en` until the apply pass converts them. *(My proposal.)*
- Definitions added to decided rows that had none (the three strata, Laborer, Clerk) so every decided row can pass the termbase test. *(My proposal.)*
- Doc number `82`, because `94` is reserved by `93`. *(My proposal.)*
- Batch 6, from the recon (`95`): the Steel ladder of the goods doc, with T5 Electric Arc inert until Electricity exists (§4.16); the market model, National Market plus one World Market (§4.17); a Goods section with the 31 goods, 12 marked for the first build (Cotton → Fiber, Pig Iron dropped, Artillery added); currency Crown / Crowns; the money rows of Economy v1; ownership State-Owned / Private; the indices Military Readiness, Industrial Capacity and Home Front; professions with their strata; the three needs groups; the seven Sphere names, with Economy and Military added to the shared names; Capital (province) and Political Capital Cap; Top Bar, Alert Strip, Provenance Tooltip; collisions C1–C18 resolved as §4.18–4.31, with pop Loyalty left [OPEN] for S1-06; goods doc renumbered `41-goods-and-chains` → `34-goods-and-chains`. *(Confirmed by Diogo, 2026-10-16.)*
- 2026-10-16: §4.17 corrected after the Economy v1 recon (`94`): the World Market is new design, not ported code. *(Fact from the recon.)*
- Marker *waits for recon* renamed *waits*, and termbase statuses `decided_waits` / `waits`, since the recon has reported. *(My proposal.)*
