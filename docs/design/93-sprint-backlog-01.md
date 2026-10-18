# 93 — Sprint Map and Sprint 01 Backlog

**Status:** LIVING · [LATEST]. Sprint 01 **stalled** (closed 2026-10-11 without its goal); Sprint 01-R **[PROPOSED]**.
**Date:** 2026-10-18 (first version 2026-09-13, in Portuguese)
**Route:** **D — Fusion** (`91` §2): LGS with LGP's scope discipline.
**Capacity:** about 5 h a week, so a 4-week sprint is about 20 h.
**Related:** `00-start-here.md`, `80-interface-e-ux.md` §13, `82-terminology-and-localization.md`, `91-direcionamentos.md`, `92-adr-0001-fronteira-sim-godot.md`, `95-recon-restart.md`

Converted to English on 2026-10-15 (`82` §1). Terms follow `82`.

---

## §0 — Sprint map

One line per sprint, from now to the first testable build. Sprint 01's dates are history; every later date is **[PROPOSED]**, derived from 20 h per 4 weeks. The UI column is the slice of `80` §13.

| Sprint | Dates | Goal (one sentence) | UI that enters | State |
|---|---|---|---|---|
| **01** | Sep 13 → Oct 11, 2026 | Godot repo builds; `Strategos.Sim` passes the boundary test, loads 12 goods and runs the 9-phase economy tick headless and deterministic | None | **Stalled.** Nothing runs (`00` §2). Exact item status unknown (`95` Part A blocked) |
| **01-R** | Oct 19 → Nov 15 | Same goal as 01, restarted from the recon list | None | **[PROPOSED]** §2 |
| **02** | Nov 16 → Dec 13 | The world has data: FMG import CLI, deterministic deposit seeding with authored overrides, tick on the imported world | None | Sketched (§6) |
| **03** | Dec 14 → Jan 10, 2027 | First pixels: rendered map, map mode renderer with Political, Steel Tier and Market; Selection Dock for a Province; Provenance Tooltip; Top Bar with date, speed, pause. **All text from keys from day one** (`82` §4.15) | Map, 3 map modes, Selection Dock, Top Bar | Not started |
| **04** | Jan 11 → Feb 7 | Economy is playable on screen | Economy panel (National Market, World Market, Treasury), Outliner with Construction, Alert Strip, Deposits map mode | Not started |
| **05 — Politics v1** | Feb 8 → Mar 7 | Interest Groups, Clout, Governing Bloc, Enactment with Political Capital; 3 nations with authored overrides | Politics panel, Political Capital in the Top Bar, Enactment in the Outliner, Interest Groups map mode | Not started |
| **06 — War v1** | Mar 8 → Apr 4 | War and peace with auto-resolve using Effectiveness = Design Tier × Equipment Tier. **First testable build** | Military panel, Battle Report, Military map mode, Gazette | Not started |
| **Phase 2** | after the first playtest | Card battle (`60`), Technology (`50`) | Battle screen, Technology panel | Out of scope |

Reading the map:

- `91` estimated 4–5 months to a testable build. From the restart this map gives about 5.5 months (Oct 19 → Apr 4), because Sprint 01 used a month without code.
- Each sprint ends with something checkable; the "done when" of each is written when the sprint before it closes.
- If a sprint slips, the later dates move with it. No sprint absorbs the next one's scope.

---

## §1 — Where Sprint 01 stopped

From `95-recon-restart.md` (2026-10-15). Part A of that recon could not run: the Godot repo is not on GitHub and the session was not linked to Diogo's computer. What the evidence says:

| Item | State | Evidence |
|---|---|---|
| S1-01 Doc hygiene | Partial, outside the repo | `00` §3 records 1820–1880 → 1800–1950 and `42-depositos` → `43`. The Unreal repo still says 1820–1880 |
| S1-02 Repo and solution | Exists, per Diogo | `00` §2; not verified |
| S1-03 … S1-05, S1-07 … S1-09 | Unknown | Needs `95` Part A |
| S1-06 Economy v1 recon | **Done 2026-10-16** | `94-recon-economy-v1.md`, on the Unreal repo `Monkius-Maximus/LeGrandStrategos` |

---

## §2 — Sprint 01-R: the restart

**[PROPOSED]** Order matters: the first two items replace guesses with evidence.

| # | Item | Est. | Where | Done when |
|---|---|---|---|---|
| R-0 | Push the Godot repo to GitHub (private is fine) | 0.5 h | Diogo's machine | `add_repo` from a claude.ai session can clone it |
| R-1 | Run `95` Part A on the Godot repo | 1 h | claude.ai or Claude Code | `95` Part A filled; the restart list below is replaced by its answer to A.10, with hours |
| R-2 | ✅ S1-06 Economy v1 recon | 2 h | claude.ai, on the GitHub Unreal repo | Done 2026-10-16: `94-recon-economy-v1.md` answers the eight questions; its §10 holds seven decisions for Diogo |
| R-3 | ✅ Decide the `95` open questions that block the port: market model (Q4), stratum vs profession (Q5), Steel ladder (Q8) | 0.5 h | Conversation | Done 2026-10-16: `82` §4.16 (Steel ladder), §4.17 (market model), §5 Population (Stratum and Profession separate) |
| R-4 … | The remaining S1 items, as Part A sizes them (expected: S1-03, S1-04, S1-05, S1-07, S1-08, S1-09) | ≤ 16 h | Claude Code | Sprint 01 goal (§0) true |

`PROMPT-APPLY-TERMINOLOGY.md` is **not** in 01-R. Its precondition is a passing build. `82` batch 6 ran on 2026-10-16, so the termbase side is ready; the pass belongs at the start of Sprint 03, before the first UI string.

---

## §3 — Sprint 01 backlog

Nine items, ordered by dependency. Unchanged in scope from the first version; carried into 01-R as R-4.

**S1-01 — Doc hygiene** · 1 h · depends on nothing
- `42-depositos-e-descoberta.md` → `43-depositos-e-descoberta.md`, and fix every cross-reference.
- `40-resources-and-production.md` → `33-resources.md` in every reference.
- `41-goods-and-chains.md` → `34-goods-and-chains.md` in every reference (`82` §4.16: `41` is population).
- Every 1820–1880 → 1800–1950.
- **Done when:** `grep -r "1820"`, `grep -rn "42-depositos"` and `grep -rn "41-goods"` return nothing; no doc number is used twice.

**S1-02 — Repo and two-project solution** · 2 h · depends on nothing
```
le-grand-strategos/
  Strategos.sln
  src/
    Strategos.Sim/          # .NET 8 class library, no Godot reference
    Strategos.Game/         # Godot 4.7.x project, references Strategos.Sim
  tests/
    Strategos.Sim.Tests/    # xUnit
  docs/
  tools/                    # later: FMG import CLI
  CLAUDE.md
```
- Godot 4.7.2 stable; do not chase 4.8-dev (`91` §0).
- **Done when:** `dotnet build Strategos.sln` passes; the Godot project opens without errors; first commit `chore: scaffold godot + sim solution`.

**S1-03 — ADR-0001 boundary test** · 1 h · depends on S1-02
- A test inspects the assemblies `Strategos.Sim` references, transitively, and fails if any name starts with `Godot`.
- **Done when:** the test passes today and fails when a `GodotSharp` reference is added on purpose (then reverted).

**S1-04 — Repo `CLAUDE.md`** · 1 h · depends on S1-02
- The 7 engineering principles of `90` §1.4; the recon → approval → implementation cycle; the ADR-0001 invariant in one line; the `⚠ RECONCILIAR` marker.
- Language rule, updated by `82` §1: **English for docs, comments, data and game text, US spelling; conversation with Diogo in Portuguese.** (The first version said PT-BR docs; the old Unreal `CLAUDE.md:18` still does.)
- Conventions: Conventional Commits, `SimId` as key, `DeterministicRng` as the only randomness.
- **Done when:** asked for a change in `Strategos.Sim` that needs randomness, Claude Code proposes `DeterministicRng` unprompted.

**S1-05 — ADR-0002: data format on disk** · 1 h
- JSON (readable, diffable, hand-editable) vs MessagePack (fast, opaque). Recommendation: JSON; revisit only if loading passes 1 s.
- **Done when:** a one-page ADR in `docs/`, status ACCEPTED, with a reversal section like ADR-0001.

**S1-06 — Economy v1 recon** · 2 h · depends on nothing now (`95`)
- Audit only. Answer: (1) the 9 tick phases in order, with what each reads and writes; (2) the price formula, constants and where they live; (3) the Private Investment algorithm (`RunBourgeoisieAutoInvestment`), triggers and thresholds; (4) which fields of `UGoodAsset`, `UProductionMethodAsset`, `UBuildingTypeAsset` are data and which are Unreal-only; (5) the `UActiveModifierRegistry` lifecycle; (6) where randomness is, and whether it is deterministic; (7) what the per-pop Loyalty reads and feeds, so `82` §4.30 can decide whether it survives the port or folds into Needs Met; (8) the automatic loan in `SettleTreasury`: when it fires and what depends on it, since an automatic loan is a fallback (`90` §1.4) and the port must keep or drop it on purpose.
- Start from `95` Part A: tick in `EconomySubsystem.cpp:217-227`, price in `:138-156`, Private Investment in `:1163-1263`. Trust the code over `20-economy.md`, which describes a model never built.
- **Done when:** `docs/94-recon-economy-v1.md` answers 1–8 and lists what is Unreal-only and dies in the port.

**S1-07 — `Strategos.Sim` primitives** · 3 h · depends on S1-03, S1-06
- `SimId` (readonly struct, value equality, stable hash), `DeterministicRng` (explicit, serializable state; replaces `FRandomStream`), `Tier` (its own type, so a method Tier cannot be added to a law Tier).
- **Done when:** each has tests; one test serializes the RNG mid-sequence, restores it and gets the same next 10 values.

**S1-08 — Data model, loader, 12-good seed** · 5 h · depends on S1-05, S1-07
- POCOs `GoodDefinition`, `ProductionMethodDefinition`, `BuildingTypeDefinition`, from the S1-06 contracts, with no Godot type.
- Loader with fail-fast validation: an unknown `SimId` aborts with file, line and id.
- Seed, from `82` (Goods, §4.16): the 12 first-build goods — Grain, Iron Ore, Coal, Wood, Fiber, Steel, Fabric, Tools, Small Arms, Artillery, Clothes, Furniture — and the Steel ladder T1–T4: Bloomery, Puddling, Bessemer Process, Open Hearth. T5 Electric Arc needs Electricity, which is not seeded; under the fail-fast loader it enters the data together with Electricity, not before.
- **Done when:** a test loads `data/` and finds 12 goods and Steel Tiers T1–T4; a broken reference makes the loader throw.

**S1-09 — 9-phase economy tick, headless** · 5 h · depends on S1-08
- Port the tick per S1-06: phase order, dynamic prices, Private Investment, consumption by Stratum. One National Market per nation (`82` §4.17). Synthetic world in `data/`, no FMG.
- **Done when:** (1) a console runner does 120 ticks (10 years) without exceptions and prints each good's price series; (2) two runs with the same seed are byte-identical; (3) removing the Coal Supply makes the Steel price rise.

| Item | Est. |
|---|---|
| S1-01 … S1-09 | 21 h |

---

## §4 — Out of scope for Sprint 01 / 01-R

- FMG import (Sprint 02).
- Any rendering, UI or Godot scene beyond the empty project that builds.
- Politics, Interest Groups, laws, Political Capital (Sprint 05).
- Card battle (Phase 2). Technology: in the first build Tiers advance by dated events — a named, temporary fallback. **[LATEST]** Eras now start per nation by criteria (`21`, decided 2026-10-17), which needs a monthly criteria check instead of dated events; `22` §5.5 proposes which criteria the first build can use.
- Religion, diplomacy, migration.
- The other 19 goods, Electricity and the other production method ladders.
- `PROMPT-APPLY-TERMINOLOGY.md` (Sprint 03, §2).

---

## §5 — Open decisions

| Decision | Blocks | Source |
|---|---|---|
| Where the Godot repo lives (push to GitHub) | 01-R R-0 | `95` Q1 |
| ~~Market model for the port~~ ✅ National Market + one World Market | — | `82` §4.17 |
| ~~Stratum and Profession as separate axes~~ ✅ Separate | — | `82` §5 Population |
| ~~The Steel ladder~~ ✅ Goods doc ladder, T5 waits for Electricity | — | `82` §4.16 |
| ~~Pop Loyalty~~ ✅ Folds into Needs Met [LATEST] | — | `82` §4.30 |
| ~~Automatic loan~~ ✅ The player handles a negative Treasury; an optional setting takes Loans automatically; with it off, an alert without pause [LATEST] | — | `82` `loan`, `35` §1 |
| Money model, scarce-goods sharing, method execution order | S1-08 / S1-09 | `94` §10 questions 3–5; analysis in `35` |
| What a negative Treasury does until the player acts | S1-09 | `35` §5 question 4 |
| Baskets and indices over the new goods | S1-09 | `94` §10 question 6; Diogo asked for alternatives |
| Building policies, privatize, nationalize | Not in S1-09 | `94` §10 question 7; review of current docs against the old design first |
| Era mechanics (rule, floor years, spread, catch-up) | Sprint 05–06 | `22` §6 |
| Missing design docs added to the Project or repo (`33`, `34`, `41`, `43`, `50`, `51`, `52`, `60`, `70`) | Law names in `82`; Sprints 02–06 | `95` Q2 |
| Military law ladder size (M1 = 0) | Sprint 05 | `95` Q7 |
| ~~LGP~~ ✅ Kept separate [LATEST] | — | `00` §9 |
| Confirm the `91` §0 discards (`StrategosUI`, Diplomacy branches) | Nothing now | `00` §10 |
| Population migration | Not in the first build | `41` v1.2 |

---

## §6 — Sprint 02 sketch (not committed)

1. FMG import as a .NET CLI under `tools/`: Azgaar JSON → provinces and adjacencies in the ADR-0002 format. Runs in CI.
2. Deterministic deposit seeding on the imported world, with authored overrides for Albion, Galia and Norden (`43`).
3. Tick on the real world: swap S1-09's synthetic data for the imported world.
4. Geography names batch in `82` (terrain, climate, infrastructure), before the import writes any of them (`82` §6 step 6).
5. Doc hygiene, if cut from 01-R.

---

## Appendix — Decisions

- Route D — Fusion. *(Confirmed by Diogo, 2026-09-13.)*
- Sprint 01 scope: foundation + Economy v1 port; ~5 h a week; 4-week sprints, ~20 h. *(Confirmed by Diogo.)*
- FMG import moved to Sprint 02; ADR-0002 recommends JSON; 12-good cut. *(My proposals, 2026-09-13.)*
- 2026-10-15: Sprint 01 recorded as stalled; sprint map §0 with dates; Sprint 01-R plan §2; S1-04 language rule updated to `82` §1; `PROMPT-APPLY-TERMINOLOGY.md` placed at the start of Sprint 03. *(My proposals.)*
- 2026-10-15: doc converted to English. *(Follows `82` §1, confirmed by Diogo.)*
- 2026-10-16: R-3 done through `82` batch 6 — market model, Stratum and Profession separate, Steel ladder; the 12 goods changed (Cotton → Fiber, Pig Iron → Artillery). *(Confirmed by Diogo.)*
- 2026-10-16: S1-06 done (`94`). *(Recon report.)*
- 2026-10-16: S1-08 seeds Steel T1–T4 only, because a fail-fast loader cannot accept T5 without Electricity; S1-06 gains questions 7 (Loyalty) and 8 (automatic loan); S1-01 adds the `41-goods` → `34` rename; UI names follow `82` (Selection Dock, Top Bar, Alert Strip, Provenance Tooltip). *(My proposals, following batch 6.)*
- 2026-10-18: §4 and §5 updated with the decisions of 2026-10-17 (Loyalty, negative Treasury, LGP, eras per nation); open port questions point to `35`. *(Decisions confirmed by Diogo, 2026-10-17.)*
