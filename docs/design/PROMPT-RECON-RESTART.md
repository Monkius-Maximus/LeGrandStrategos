# PROMPT — Recon: Sprint 01 state and terminology

**For:** Claude Code, with the `le-grand-strategos` repo mounted.
**Type:** RECON ONLY. Audit and write one report. No code change, no doc edit, no rename, no commit except the report file.
**Output:** `docs/95-recon-restart.md` (`94` is reserved for the Economy v1 recon of S1-06; do not use it).
**Date written:** 2026-10-11

---

## Context you need

- The project is *Le Grand Strategos* (LGS): grand strategy, 1800–1950, Godot 4.7 / C#.
- Sprint 01 (`93-sprint-backlog-01.md`, items S1-01 to S1-09) ran 2026-09-13 → 2026-10-11 and stalled. Nothing runs yet.
- ADR-0001 (`92`): `Strategos.Sim` references no Godot assembly, enforced by a test.
- Terminology: `82-terminology-and-localization.md` is the termbase. English is the source language of the project. §4 of `82` is DECIDED; §5 is mostly PROPOSED.
- If `82`, `92` or `93` are not in `docs/`, say so at the top of the report and work from this prompt.

## Rules

1. Read and run; do not change. `dotnet build` and `dotnet test` are allowed. Editing, formatting, renaming, deleting, installing packages or fixing anything is not.
2. Every claim cites evidence: a file path and line, a command and its output, or "not found" with the search you ran.
3. When something is ambiguous, report the ambiguity. Do not pick an answer.
4. If something this prompt needs cannot be reached (a doc, the old Unreal repo), write it as a blocker and continue with the rest.
5. Write the report in English.

---

## Part A — Where Sprint 01 stopped

For each item S1-01 … S1-09 in `93` §2, report:

| Item | Status (done / partial / not started) | Evidence | Its DoD, checked one by one | What blocks it |
|---|---|---|---|---|

Specifically:

1. Run `dotnet build` on the solution and `dotnet test`. Paste the summary lines and every error.
2. Does the solution match the layout in `93` S1-02 (`src/Strategos.Sim`, `src/Strategos.Game`, `tests/Strategos.Sim.Tests`, `docs/`, `tools/`, `CLAUDE.md`)? List differences.
3. Does the ADR-0001 boundary test exist, and does it pass?
4. Does `CLAUDE.md` exist? Quote any line that conflicts with `82` §1. (S1-04 told it docs and comments are PT-BR; `82` now says English.)
5. Is there an ADR-0002 (data format)? What does it decide?
6. Does `docs/94-recon-economy-v1.md` exist? If yes, summarize its answers to the six questions of S1-06. If no, say whether the old Unreal repo is reachable from this session.
7. Do `SimId`, `DeterministicRng` and `Tier` exist, with tests?
8. Is there any data under `data/`? How many goods, and how many Steel production methods?
9. How much of the 9-phase tick exists? List each phase found and what it reads and writes.
10. **The restart question:** what is the smallest list of tasks that makes the S1-09 DoD pass (120 ticks headless, byte-identical determinism, "no coal → steel price rises")? Order them and estimate each in hours. Mark which are already half done.

---

## Part B — Terminology inventory

Run the inventory of `82` §5 over everything `82` could not read: the system docs in `docs/` (`33`, `41`, `43`, `50`, `51`, `52`, `60`, `70` and any others) and the code (type, field, enum and file names; comments).

1. **Per `82` §5 row:** the words the docs and code use for that concept today. One table per `82` domain, same IDs.
2. **Concepts not in `82`:** any player-visible concept in these docs or the code that has no row. Give the doc, the word used and a one-line meaning.
3. **New collisions:** one word used for two concepts, or two words for one, that `82` §4 does not already cover.
4. **Retired words still present.** Search docs and code for these and give counts per file:
   `votação`, `vote`, `satisfação`, `satisfaction`, `classe social`, `social class`, `déficit` (used for goods), `recurso` / `resource` (as a player term), `auto-investment`, `auto-investimento`, `molde`, `galianos`, `Chance to Pass`, `NAÇ`, `LEI`, `ECO`, `SOC`, `EXÉ`, `CLE`, `1820`, `42-depositos`, `40-resources`.
5. **Code identifiers that carry a retired or Portuguese word.** List them. Do not rename them; `82` §8 says locked IDs stay.

---

## Part C — Rows marked ⚠ RECONCILIAR in `82`

Answer each from the docs or code, with evidence. "Not defined anywhere" is a valid answer.

| `82` row | Question |
|---|---|
| `market` | What are the three market layers called in `33`? |
| `world_market` | What does `33` call the layer outside the nation? |
| `pm_bloomery`, `pm_puddling`, `pm_bessemer` | The full Steel ladder in `33`: the name and Tier of each of the five methods. |
| `tariff` | Are tariff ranges per Economy law defined anywhere (`52`, `33`)? |
| `trade_capacity` | How does `33` gate imports by infrastructure? What is the quantity called? |
| `prof_*` | The full profession list in `41`, with each profession's stratum. |
| `needs` | Is the needs ladder defined in `41`? Its levels and names. |
| `literacy`, `mobility` | Does `41` define either? |
| `migration` | Does any doc decide population migration? |
| `doctrine`, `unit_concept` (`82` §4.5) | How does `60` model doctrine: a tier ladder, a lateral choice, or something else? Is "concept tier" in any doc or the code? Do the four doctrinal axes (Maneuver & Shock, Firepower & Attrition, Discipline & Planning, Mobilization & Mass) appear anywhere? |
| `morale` | Does `60` define morale? |
| `era` (`82` §4.14) | Does any doc define the eras (count, dates, names)? |

Also, if the old Unreal Core is reachable: **M1 from `52` §7.** How many states does the existing civil/military split expose? If it is not reachable, say so.

---

## Report format

`docs/95-recon-restart.md`, in this order:

1. **Summary** — five lines: what runs, what is the first blocker, the restart list's total hours, the number of retired-word hits, the number of ⚠ RECONCILIAR rows answered.
2. Blockers (what this recon could not reach).
3. Part A.
4. Part B.
5. Part C.
6. **Open questions for Diogo** — only the ones the evidence could not settle.

Stop after writing the report. Do not start implementation. Implementation needs Diogo's explicit approval in a new turn.
