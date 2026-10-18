# 83 — UI Artifact Index

**Status:** LIVING — update when a canvas is added or superseded
**Date:** 2026-10-16
**Related:** `80-interface-e-ux.md`, `81-guia-de-design-de-ui.md`, `82-terminology-and-localization.md`

The UI mockups live as published canvases, not as project docs. This index is how another chat finds them. They are private to Diogo; open them from his account.

**One editor at a time.** On 2026-10-15 two sessions converted the same canvas in parallel and one had to merge over the other. Before editing a canvas, check this doc's date and the canvas version; when in doubt, ask Diogo which session owns it.

---

## §1 — Canvases

| Canvas | Link | State |
|---|---|---|
| **LGS — Interface Mockups 2** (was "LG — Mockups de Interface 2") | https://claude.ai/artifact/UGF6sZjZ27HHfc5NooNpti | **Current.** The reference for in-game screens. English, `82` names, `81` type scale since 2026-10-15. Batch 6 names not applied yet (§3.1). |
| **Moldes de telas comuns** (common screen templates) | https://claude.ai/artifact/RSZjgTP9BPYLT79tSXNkGS | **Current.** Shared templates for all of Diogo's games, plus the LGS and SDG skins. Still Portuguese (§3.6). |
| LGS — Mockups de Interface (v1) | https://claude.ai/artifact/EUwHXQXyFckSRyUjP8LPGu | **Superseded** by v2. Wrong nation colors, 1600×900. Not translated. |

---

## §2 — Boards in "LGS — Interface Mockups 2"

1920×1080 unless noted. Shared parts are imported, so changing one changes every screen. File names are English since 2026-10-15; the old Portuguese files were removed from the canvas.

**Shared parts:**

| File | What it is | Props |
|---|---|---|
| TopBar | Nation, Treasury, Political Capital, Manpower, Steel Tier, Shortages, era and date, speed and pause | `date` (US long date), `era` (`Era II · Steam`), `cap`, `capmax`, `paused` = `no` \| `yes` |
| Rail | Icons only, full name on hover. Seven map modes, then five panels (Nation F1, Politics F2, Economy F3, Military F4, Society F5) | `mode` = `political` \| `steel_tier` \| `market` \| `deposits` \| `population` \| `interest_groups` \| `military`; `panel` = `""` \| `nation` \| `politics` \| `economy` \| `military` \| `society` |
| Outliner | Outliner and Journal tabs | `enactment` = `active` \| `none`; `war` = `no` \| `yes`; `tab` = `outliner` \| `journal` |
| Map | 8 px pixel-grid map with the seven map modes | `mode` (as Rail), `sel` (province id), `inset` (0 or 560) |

Every shared part throws on an unknown prop value.

| File | Board title | What it shows |
|---|---|---|
| Main | 1 · Main screen | Default game screen, Steel Tier map mode |
| ComparisonA, ComparisonB (1920×1480) | 0A, 0B · Comparison | What was kept, merged, deferred and dropped from the two older prototypes (39 elements) |
| MergeRules | 0C · Merge rules | The 7 merge rules (one style source, system panel vs selection dock, etc.) |
| Province | 2 · Selected province | Province selection dock and building detail |
| Politics | 3 · Politics | Governing Bloc, Interest Groups, law matrix, Enactment preview |
| NationalMarket | 4 · Economy — National Market tab | Economy panel (F3), tabs National Market · World Market · Treasury |
| Military | 5 · Military at war | Unit cards, Effectiveness = Design Tier × Equipment Tier, Battle Report |
| Nation | 6 · Nation | Nation overview + Journal |
| Gazette | 7 · Major event | Major-event Gazette, game paused |
| DecisionEvent (960×720) | 8 · Decision event | Reserved format for a decision event (strike at Jarnvik); no system yet |
| Commander (960×720) | 9 · Commander | Commander card, phase 2 |
| Components (1920×1240) | 10 · Component catalog v2 | Component catalog |
| Society | 11 · Society | Strata, Professions, Needs; Population tab of the province dock |
| Deposits | 12 · Deposits map mode | Prospecting in a province |
| Construction | 13 · Construction | Buildings tab and the build picker |
| Enactment | 14 · Enactment in progress | Month 4 of 6 and the two result cards (Enacted, Failed) |
| Army | 15 · Selected army | Move order with ETA, Recruitment tab |
| WorldMarket | 16 · Economy — World Market tab | World prices, Tariffs, Imports and Exports, Trade Capacity |
| Treasury | 17 · Economy — Treasury tab | Income, Expenses, Balance, 12-month projection |
| NestedTooltip (1200×720) | 18 · Nested tooltip | Three layers and Lock |

---

## §3 — Known debts

1. **Batch 6 names the boards do not show yet** (`82`, 2026-10-16):
   - **Steel ladder.** The boards show Bloomery, Puddling, Crucible, Bessemer Process, Siemens-Martin. `82` §4.16 is Bloomery T1, Puddling T2, Bessemer Process T3, Open Hearth T4, Electric Arc T5; Crucible is not a Tier. Touches the Steel detail, Province (the Crucible and its Machinists), Military and Army (Steel requirements of Equipment Tiers), Main and the Steel Tier map mode legend.
   - **Needs groups:** Basic / Comfort / Luxury → Life Needs / Everyday Needs / Luxury Needs (Society).
   - **Money:** Interest on Debt → Interest; Treasury figures need the Crown icon (`82` `currency`); Treasury lines follow the money rows (Taxes, Tariff Income, State Profits; Maintenance, Army Upkeep, Administration, Interest).
   - **Goods:** any good outside the 31 of `82` Goods goes (e.g. a bare "Iron").
   - **Still waiting:** law names (Closed Mercantilism, Serfdom, Regulated Hours, Limited Conscription, Broad Conscription, Absolute Monarchy, State Church) until `52` is found.
2. **Words used on the boards that have no `82` row yet:** Batch (recruitment lot), Jobs, Layer (tooltip depth), Garrison, Unavailable (method state), Orders, Overview / Information (dock tabs), Portrait. Add rows or replace them when the termbase grows.
3. **Marked ⚠ RECONCILIAR on the boards:** tariff ranges per Economy law, tax rates by Stratum, interest. The names exist since batch 6; the numbers are not defined in any doc yet.
4. **Data inconsistencies between boards** (not fixed, need a decision or a data pass):
   - TopBar values are fixed (Treasury 1,240, Manpower 18.4k), while Nation (1851) and Military/Army (1848) show other numbers. Making them props would fix it.
   - Military: Norden's Artillery is at Equipment Tier T1 and "depends on T2 Steel", but Norden already has T2 Steel; Army says T2 Artillery "requires T3 Steel". The rule linking Steel Tier to Equipment Tier is not defined.
   - Province: the Crucible requires 1.8k Machinists, but the Steelworks has only 1,400 jobs. (The Crucible itself leaves with debt 1.)
   - Society: Kolfjord's professions add up to 45.8k of 48.2k people, and Jobs 2,950 is low next to 19.8k Laborers.
   - Nation calls Norden a "Confederation" while its government is Absolute Monarchy.
5. **Not designed on purpose** (no locked doc, or phase 2): technology, doctrines, card battle, religion, diplomacy, migration.
6. **Moldes de telas comuns** is still Portuguese. It serves LGS and SDG, and SDG's language is not decided. The LGS skin needs at least "Nova partida" → "New Campaign" (`82` `campaign`).

Done on 2026-10-15:
- Screen text in English with the `82` names; numbers and dates in US format.
- File names in English (Votacao → Enactment, Mercado → NationalMarket, …), shared-part props in English, every shared part fails fast on an unknown value.
- `81` type scale on every board: body 16, secondary 14, labels 13, nothing below 13 px; control borders `#717b8b`.
- Rail icons only with names on hover; map mode Interest Groups.
- Economy panel tabs on all three market boards.
- Effectiveness = Design Tier × Equipment Tier on Military and Army; unit card stats in full (Attack, Defense, Mobility, Morale); unit types qualified (Line Infantry, Field Artillery); commanders without rank.
- Map deposit labels written in full (Iron Ore, Coal, Wood).
- Fixed: Machinist count (1.8k); Coal, Iron Ore, Wood, Grain and Tools prices on NationalMarket now match WorldMarket; Steel imports 40 t; Political Capital after a failed enactment (15 of 72, next attempt ~15 months); 1st Army's battle at Vindal in the Outliner; "tick" removed from player text.
