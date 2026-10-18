# 00 — Start Here

**Status:** LIVING — the first doc anyone reads. Update it the day any line below stops being true.
**Date:** 2026-10-16
**Related:** `01` (how to work), `91` (route), `93` (sprint map), `92` (ADR-0001), `82` (names), `83` (mockups), `95` (recon), `90` (history)

---

## §1 — The game in five lines

*Le Grand Strategos* (LGS) is a real-time-with-pause grand strategy game set in a fictional industrializing world, 1800–1950, with a soft end: the player may keep playing past 1950.
Three nations: Albion (industrial power), Galia (centralized gunpowder state), Norden (martial frontier, rich iron, primitive metallurgy).
Pillar: nations make the same good with production methods of different Tier (Bloomery vs Bessemer Process for Steel), and a unit's Effectiveness = Design Tier × Equipment Tier, so industry decides wars.
Architecture model: Victoria 3. Content and the political capital idea: EU4.
Solo project, about 5 hours a week.

---

## §2 — Where things stand (2026-10-16)

- **Engine:** Godot 4.7, C#. Decided 2026-09-06 (`91` §0).
- **Unreal code:** the old Unreal 5.5 / C++ project (`StrategosCore`, `StrategosUI`) is reference only. Economy v1 is ported from it; `StrategosUI` is not ported. It is on GitHub: `Monkius-Maximus/LeGrandStrategos` (public; latest branch `claude/event-schema-debug-kqxid8`).
- **Godot repo:** exists on Diogo's machine, **not on GitHub**. The Economy v1 port has stalled. **Nothing runs yet. There is no build to play.**
- **Route:** Route D, "Fusion" (`91`). First build = 3 nations, 12 goods, the Steel ladder, politics v1, auto-resolved war.
- **Sprints** (`93`): Sprint 01 stalled on 2026-10-11. The sprint map (`93` §0) runs from a restart, Sprint 01-R (Oct 19 → Nov 15), to the first testable build at the end of Sprint 06 (about Apr 4, 2027). All dates **[PROPOSED]**.
- **Recon** (`95`, 2026-10-15): Parts B and C ran on the GitHub Unreal repo; Part A (Sprint 01 state) waits for the Godot repo to be pushed. `95` is a dated snapshot: its open questions are answered in `82` §4.16–4.31 and tracked in `93` §5, not edited into the report.
- **Economy v1 recon** (`94`, 2026-10-16, Sprint 01-R item R-2): the tick is portable; it has no randomness; the modifier registry the system docs rely on was never built; money is not conserved. Seven port decisions wait in `94` §10.
- **Names:** `82` batches 1–6 decided, 222 terms. Three rows wait: law names (`52` is missing), the Military law ladder size (Sprint 05), the unit roster.
- **Design is far ahead of code:** about ten system docs, most written for the Unreal version, and most of them missing (§4).

---

## §3 — How the plan changed

Old docs and old chats describe earlier versions of the plan. Read them with this table at hand.

| Topic | Earlier | Now | Recorded in |
|---|---|---|---|
| Period | 1820–1880 | 1800–1950 | `90` §1.1 |
| Engine | Unreal 5.5, C++ | Godot 4.7, C# (`Strategos.Sim` + `Strategos.Game`) | `91` §0, `92` |
| Presentation | 2D tile map in a 3D engine, with camera tilt and 3D decorations considered | 2D map (pixel grid in the mockups) | `70`, `83` |
| Battle | Protocolo B | Card battle (`60`); auto-resolve in the first build | `90`, `91` |
| Technology | Research engine with five parallel trees | Tech emergence, seven spheres (`50`) | `50` |
| Doctrine | Four axes × five eras (Phase 2), then a light version (Protocolo B), named doctrines in the Unreal docs | Feeds command points (`60`); model open | `61` |
| Military formula | `eficácia = tier_conceito × PM.Tier` | Effectiveness = Design Tier × Equipment Tier | `82` batch 4 |
| Steel ladder | 3 methods (Unreal docs); Crucible at T3 (mockups) | Bloomery, Puddling, Bessemer Process, Open Hearth, Electric Arc | `82` §4.16 |
| Markets | Regional markets (Unreal `20-economy.md`) | One National Market per nation and one World Market | `82` §4.17 |
| Politics | Factions, parties, elections, 11 law categories (Unreal `31-politics.md`) | Interest Groups, Enactment, 5 law categories | `42` |
| LGP | Separate spin-off | Absorption into LGS proposed | `91` §4 |
| Doc language | Portuguese | English, US spelling | `82` §1, §3 rule 9 |
| Doc numbers | `40-resources-and-production`, `42-depositos-e-descoberta`, `41-goods-and-chains` | `33`, `43`, `34` | `91` §0, `82` §4.16 |

---

## §4 — Where everything lives

| Place | What | Who can open it |
|---|---|---|
| **Godot repo** | `Strategos.Sim`, `Strategos.Game`, tests | Diogo's machine only, until pushed |
| **GitHub repo** `Monkius-Maximus/LeGrandStrategos` | The old Unreal project and its docs (`docs/architecture/*`: older versions of the system docs) | Public |
| **System docs** `33` resources, `34` goods and chains, `41` population, `43` deposits, `50`/`51` technology, `52` laws, `60` battle, `70` architecture | Not in the GitHub repo nor in the Project (`95`). Location **[OPEN]**. The goods doc survives in the chat archive (June 2026), which is where `82` took the goods and the Steel ladder from | — |
| **claude.ai Project** "Jogo de Grande Estratégia" | `42` (politics), `81`–`83` at the top level; Claude-written docs under `claude/`: `00`, `01`, `21`, `61`, `80`, `90`–`95` and the Claude Code prompts | Diogo and Project members |
| **Canvases** (listed in `83`) | UI mockups | Diogo only (private) |

New docs Claude writes to the Project land under `claude/`; the tool cannot create top-level docs. Moving one to the top level is a manual step in claude.ai.

**[PROPOSED]** The Godot repo's `docs/` folder is the single canonical home. The Project mirrors it.

---

## §5 — Reading order

1. This doc, then `01-orchestrator.md` (how a conversation works on LGS).
2. `91` — the chosen route and why (§0, Route D, §4).
3. `93` — the sprint map and the current sprint.
4. `92` — ADR-0001, the boundary between the simulation and Godot.
5. `82` — names. Use them in everything you write.
6. System docs: `33` resources, `34` goods and chains, `41` population, `42` politics, `52` laws, `43` deposits, `60` battle, `61` doctrine, `50`/`51` technology, `21` eras, `70` architecture. Most use Unreal vocabulary (`U*Asset`, `FName`, `FRandomStream`); the contracts port, the signatures do not (`90` §3.3).
7. `80`, `81`, `83` — interface.
8. `95` and `94` — what the old repo actually contains: the general recon (its questions answered in `82` §4.16–4.31) and the Economy v1 recon that the port follows.
9. `90` — history and glossary. Its engine section and code conventions describe the Unreal version.

---

## §6 — Superseded: do not build from these

- The Unreal `StrategosUI` module and its enum reconciliation debt (`91` §0).
- The unmerged Diplomacy branches. Diplomacy is redesigned from a doc first (`91` §0).
- Protocolo B (replaced by `60`).
- The five-tree research engine (replaced by `50`).
- Any doc that says 1820–1880 — including the GitHub repo's `CLAUDE.md` and `docs/architecture/*`.
- `40-resources-and-production.md` (now `33`), `42-depositos-e-descoberta.md` (now `43`), `41-goods-and-chains.md` (now `34`).
- The Phase 2 doctrine docs: recorded in `61`; reference only until the doctrine model is decided.
- `docs/architecture/20-economy.md` in the Unreal repo as a description of Economy v1: it describes a weekly, regional model that was never built; the code is the reference (`95`).
- The Unreal design concepts `95` found with no place in the current design: elections, parties, parliament, ideologies, factions and coups; the 11 law categories; the diplomacy vocabulary (until a diplomacy doc exists); battle cards and the Archer and Pike regiment types; building production modifiers; Trade Hub; the eras Late Mercantile … Early Modern; the 1836 sandbox scenario. `82` gives them no rows (§8 there).

---

## §7 — One status legend

Every doc moves to these markers when it is next edited.

| Marker | Meaning | Older markers it replaces |
|---|---|---|
| **[DECIDED]** | Confirmed by Diogo | [TRAVADO], *(Confirmado por Diogo)* |
| **LOCKED vX** | The whole doc is confirmed; a change needs a new version | LOCKED v1.0 |
| **[PROPOSED]** | A proposal waiting for confirmation | [PROPOSTA], [INFERIDO] |
| **[OPEN]** | Only Diogo can decide | [ABERTO] |
| **[REF]** | Decided elsewhere; points there | [REF] |
| **⚠ RECONCILIAR** | Not yet checked against the code or another doc | — |

[VIGENTE] (practice in use, never confirmed) becomes [DECIDED] once Diogo confirms it, otherwise [PROPOSED].

---

## §8 — Names

- **[PROPOSED]** The game is *Le Grand Strategos* (French on purpose); the only short form is **LGS**. Retire "LGE", "LG" and "Le Grand Estrategos".
- Every in-game term: `82`.

---

## §9 — The family of games

Three projects share a method and some parts. All three are paused (October 2026). None share code.

| Project | What it is | Shares with LGS | State |
|---|---|---|---|
| **LGS** | This game | — | Paused; design ahead of code |
| **LGP** — *Le Grand Polemarkos* | 4X real-time-with-pause spin-off, 2–4 h sessions, Godot / C# | The three nations, the Tier and interlock pattern, the design principles | Paused. Absorption into LGS proposed in `91` §4: **[OPEN]** |
| **SDG** | Football career game: the week, club, player, transfers, medical department, simulated match | The common screen templates ("Moldes de telas comuns", with an SDG skin), the design principles | Paused. Full name and docs: **[OPEN]** |

Shared by all three: the design principles (`90` §1.4), the working method (`90` §3.2), the common screen templates (`83`).

---

## §10 — Open decisions that block a collaborator

1. Canonical doc home (§4). **[PROPOSED]**
2. LGS as the only short name (§8). **[PROPOSED]**
3. LGP: absorb, pause, or keep separate. **[OPEN]** since `93`.
4. Confirm the `91` §0 discards (`StrategosUI`, Diplomacy branches). **[OPEN]** since `93`.
5. Restart plan after Sprint 01: Sprint 01-R in `93` §2. **[PROPOSED]**; its first step is pushing the Godot repo.
6. Population migration. **[OPEN]** (`41` v1.2)
7. The doctrine model (`61` §4) and what triggers an era (`21` §3). **[OPEN]**
8. Where the missing system docs live (§4). **[OPEN]**
9. The seven port decisions of `94` §10 (Loyalty, the automatic loan, money, scarce goods, method order, baskets and indices, building policies). **[OPEN]**

---

## §11 — How work happens

- **Propose, then confirm.** Nothing is [DECIDED] until Diogo says so.
- **Recon before implementation.** Claude Code audits the repo and writes a report; code changes only after approval.
- **Reports are snapshots.** A recon report keeps its date and content; decisions taken on it go to the doc that owns them.
- **Design in claude.ai, code in Claude Code** with the repo mounted. A claude.ai session can also read any GitHub repo of Diogo's and run read-only recons.
- **Fail fast, no fallbacks, one source of truth** (`90` §1.4).

---

## Appendix — Decisions taken

- Facts from Diogo: the repo exists; the Economy v1 port stalled; nothing runs; the other two games are LGP and SDG. *(Stated by Diogo, 2026-10-10.)*
- SDG described from its canvas "SDG — Mockups de interface 2". *(My reading.)*
- §4 canonical home, §7 legend, §8 short name. *(My proposals.)*
- 2026-10-15: §1 formula and §3 rows updated to the batch 4 names (Design Tier, Equipment Tier). *(Follows `82` batch 4, confirmed by Diogo.)*
- 2026-10-15: eras and doctrine moved out of the chat archive into `21` and `61`; stale `claude/` copies of `81`, `82`, `83` deleted. *(Approved by Diogo.)*
- 2026-10-15: §2, §4, §6 and §10 updated from the recon (`95`) and the new sprint map (`93`). *(Facts from the recon; the sprint dates are my proposal.)*
- 2026-10-16: `82` batch 6 reflected in §2, §3, §6 and §10; goods doc numbered `34`; "Route D" replaces "Roteiro D". *(Batch 6 confirmed by Diogo.)*
- 2026-10-16: `01-orchestrator.md` and `HANDOFF-2026-10-16.md` added. *(Requested by Diogo.)*
- 2026-10-16: `95` kept as a dated snapshot; its answers live in `82` and `93` (§11). *(My proposal; it replaces the plan to edit `95`.)*
