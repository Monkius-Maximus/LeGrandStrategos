# 22 — Era Progression: How Strategy Games Do It, and What LGS Keeps

**Status:** ANALYSIS · [LATEST] — research for decision 10; the direction is decided, the mechanics are proposals.
**Date:** 2026-10-18
**Decision this serves:** each nation enters an era when it meets that era's criteria, not by calendar. *(Confirmed by Diogo, 2026-10-17; recorded in `21`.)*
**Related:** `21-campaign-eras.md` (the five eras), `61-doctrine-axes.md` (doctrine nodes per era), `50` (tech emergence, missing), `82` §4.14 (era names), `80` (Gazette, Top Bar)
**Numbering:** `22` sits next to `21`, which this doc feeds.

---

## §1 — The decision and the question

Diogo's direction: an era begins for a nation when that nation meets the era's criteria. Prosperity, events and the player's actions can speed a nation up or leave it "stopped in time". This opens alternative histories: what if a nation never industrializes?

The question here is how far that direction moves away from what players of the genre expect, and what to borrow so the system feels familiar where it can.

---

## §2 — How the genre does it

| Game | Unit that has an era | What starts it | Being behind | Catch-up |
|---|---|---|---|---|
| **Europa Universalis IV** — Ages | The whole world | Events, not dates: e.g. the Age of Reformation starts 10 years after Protestantism appears; Absolutism 10 years after Global Trade ([EU4 Wiki — Ages](https://eu4.paradoxwikis.com/Ages)) | Nothing; every nation is in the same Age | — |
| **Europa Universalis IV** — Institutions | Each province, then each nation | An institution spawns in provinces meeting conditions after a year, then spreads to neighbors; a nation embraces it once present in 10 % of its development ([EU4 Wiki — Institutions](https://eu4.paradoxwikis.com/Institutions)) | Tech costs +15 %, +30 %, then +50 % for each institution not embraced | Spread from neighbors; paying to embrace |
| **Europa Universalis V** — Ages | The whole world | One age per century, at listed start years (Renaissance 1342 … Revolutions 1737) ([EU5 Wiki — Age](https://eu5.paradoxwikis.com/Age)) | Nothing for the age; institutions decide | Institution spread, driven by literacy, control and trade |
| **Europa Universalis V** — Institutions | Each location, then each country | Each age's institutions get a monthly spawn chance that grows; spread scales with literacy; a country embraces at 20 % presence (EU5 Wiki — Age) | Locked advance branches | Spread from market centers, capitals, neighbors and imports |
| **Crusader Kings III** — Eras | Each culture | 8 innovations of the current era, plus the era's earliest year (Early Medieval 900, High 1050, Late 1200); then progress based on average Development ([CK3 Wiki — Innovation](https://ck3.paradoxwikis.com/Innovation)) | Fewer traditions slots and unlocks | Exposure to neighbors who have the innovation; Cultural Spread +40 %; Fascination |
| **Victoria 3** — Tech eras | No nation era; eras only group technologies (I–V) ([Vic3 Wiki — Technology](https://vic3.paradoxwikis.com/Technology)) | — | Earlier-era techs left unresearched add to cost | Technology spread, driven mainly by literacy |
| **Hearts of Iron IV** | No eras; techs carry a year | — | Researching ahead of a tech's year costs 200 % more time per year early ([EIP guide](https://eip.gg/hoi4/guides/research/)) | Simply waiting for the year |
| **Stellaris** | The whole galaxy, as phases | Game years set at setup; the end-game crisis begins rolling at the end-game year, 2400 by default, and the player can move it ([Gamer Empire](https://gamerempire.net/stellaris-when-does-the-crisis-start/)) | — | — |
| **Civilization VI** | World era and each player's era | World era moves with the median progress of all civilizations; a player's era moves with its own techs and civics ([Civ Wiki — Era (Civ6)](https://civilization.fandom.com/wiki/Era_(Civ6))) | Techs from eras before the world era cost 20 % less; later ones 20 % more | The 20 % discount |
| **Civilization VII** — Ages | The whole world | An Age Progress meter fills each turn and faster with Legacy Path milestones; all players transition at once, after an end-of-age Crisis ([Civ VII guide](https://civilization.2k.com/civ-vii/game-guide/gameplay/ages-explanation)) | Missing milestones gives a Dark Age legacy | Legacies carried into the next age |
| **Humankind** | Each player | Earn 7 of the era's 21 Era Stars ([PC Gamer](https://www.pcgamer.com/humankind-fame-era-stars/)) | Others score Fame first | "Competitive Spirit" stars while others are ahead |
| **Millennia** (Paradox Arc) — Ages | The whole world, chosen by the leader | The first nation to research enough techs and an Age locks it into the timeline for all; Variant Ages are alternate-history branches, Crisis Ages hit everyone ([Millennia Dev Diary — Ages](https://www.paradoxinteractive.com/games/millennia/news/ages-part-one)) | You live in the Age others chose | Racing to claim the next Age |

---

## §3 — What players already know

Four patterns cover the genre:

1. **Global age with a trigger.** EU4, EU5, Stellaris, Civ VII, Millennia. The age is a mood for the whole world: new rules, new objectives. Players read it as the scenery changing.
2. **Spreading institutions.** EU4 and EU5. Progress is a thing that appears somewhere and spreads along borders and trade; being behind raises costs. Paradox players know this best, and it is the clearest model of "a nation stuck in time".
3. **Per-group eras with a threshold.** CK3 (per culture) and Humankind (per player). Each group crosses a threshold on its own, with a minimum year in CK3. Players accept that two neighbors live in different eras.
4. **No eras, only tech timing.** Victoria 3 and HOI4. Time is shown by the tech tree; the player compares tech, not eras.

LGS's direction, an era per nation by criteria, is pattern 3. CK3 is the closest Paradox reference, so Paradox players will recognize it. What LGS adds is the possibility of a nation never crossing, which no listed game allows outright. CK3 comes nearest: tribal cultures cannot advance past the Tribal Era (CK3 Wiki — Innovation).

---

## §4 — Keep, change, borrow

| Familiar element | Keep or change | Why |
|---|---|---|
| Five named eras with a mood (`21`) | **Keep** | Every game above names its periods; the five names are decided (`82` §4.14). |
| One era for the whole world | **Change** | Diogo's decision: per nation. |
| A threshold to cross (CK3's 8 innovations, Humankind's 7 stars) | **Borrow** | Players understand "meet N of M criteria". It allows several paths into an era. |
| A minimum year (CK3) | **Borrow, as a floor only** | Stops a runaway nation from reaching Mechanization in 1830, without forcing anyone forward. |
| Spread from neighbors (EU4/EU5 institutions, CK3 exposure, Vic3 spread) | **Borrow** | It is how a stuck nation gets unstuck without the player, and how ideas cross borders. It also makes geography matter. |
| A cost for being behind (EU4 tech penalty, Civ VI ±20 %) | **Borrow, carefully** | A small cost keeps the gap meaningful; a large one makes it permanent. |
| Catch-up help (Humankind stars, Civ VI discount) | **Borrow** | Being behind should feel like a challenge, not a lost game. |
| Alternate-history branches (Millennia's Variant Ages) | **Later** | A good fit for "what if" stories, but a large content cost. Phase 2 or later. |
| Forced advance by date (Civ VII, EU5) | **Reject** | Contradicts the decision. |

---

## §5 — Proposal for LGS

All items **[PROPOSED]**.

### 5.1 The rule

A nation enters Era N+1 when, after the era's earliest year, it meets **k of the m criteria** of that era. Criteria come from systems that already exist (principle 6: derive, do not invent).

```
EligibleYear(N+1) ≤ CurrentYear
Met(nation)       = number of Era N+1 criteria the nation satisfies
EntersEra         = Met(nation) ≥ k
```

### 5.2 Criteria come from existing systems

| Kind | Example for Era II · Steam | Source system |
|---|---|---|
| Industry | Steel Tier ≥ 2 in any province | Economy (`34`, `82` §4.16) |
| Infrastructure | Trade Capacity ≥ X, or a Railway building | World Market (`82` §4.17); geography batch (Sprint 02) |
| Prosperity | Average Needs Met of the Lower Stratum ≥ Y for 12 months | Population (decision 1) |
| Knowledge | Literacy ≥ Z, or N technologies of the era | Population (`41`), Technology (`50`, phase 2) |
| Politics | A law at a given Tier, e.g. Economy ≥ T2 | Politics (`42`) |

Diogo's example, a nation without prosperity that does not invest and stays behind, falls out of the Prosperity and Industry criteria without a special rule.

### 5.3 Spread and catch-up

- **Exposure.** Each neighbor or trade partner already in Era N+1 counts as progress toward one criterion, as CK3 counts neighboring counties that know an innovation.
- **No permanent lock.** A nation behind the world's leading era by two or more eras gets a discount on the criteria (lower thresholds), like Civ VI's 20 % discount on old-era techs.

### 5.4 What the player sees

- **Top Bar.** "Era II · Steam" is the player's own era. A small marker shows the leading era in the world when it differs.
- **Nation panel (F1).** An Era block lists the next era's criteria as a checklist with progress, `k of m`, and the earliest year. Each line has a Provenance Tooltip.
- **Gazette.** Any nation entering a new era is a major event. The first nation into an era gets a world headline, e.g. "Albion enters the Age of Steam".
- **Map mode.** A Political-style mode colored by each nation's era. Alternatively, an era band on the Steel Tier mode.
- **Journal.** Records each nation's era entries.

### 5.5 What changes elsewhere

- **`61` doctrine.** Nodes per era work per nation without change: a nation receives its axis nodes when it enters the era.
- **`93` first build.** It planned Tiers advancing by dated events (`93` §4). Per-nation eras need the criteria check instead.
  - The first build can use the Industry, Prosperity and Politics criteria, which all exist by Sprint 05.
  - Knowledge criteria wait for Technology (`50`, phase 2).
  - This adds one system to the first build. It is small: a monthly check over values the Sim already computes.

---

## §6 — Questions for Diogo

1. **Rule:** `k of m` criteria with an earliest-year floor. Accept, or prefer a single score (like Civ VII's progress meter) per nation?
2. **Floor years:** the era start years of `21` (1840, 1870, 1890, 1914) as the earliest possible entry?
3. **Spread:** neighbors and trade partners in a later era count toward one criterion. Accept?
4. **Catch-up:** a discount for nations two eras behind. Accept, or let them fall behind freely?
5. **Can a nation go back an era** (collapse, war)? No game above does it. Proposal: no.
6. **Variant eras** (alternate-history branches like Millennia's): note for later, or design now?

---

## Appendix — Decisions taken

- Eras begin per nation by criteria, not by calendar. *(Confirmed by Diogo, 2026-10-17.)*
- The comparison table is research; game mechanics are cited from wikis and official guides as of 2026-10-18 and may change with patches. *(Research.)*
- Everything in §5. *(My proposals.)*
