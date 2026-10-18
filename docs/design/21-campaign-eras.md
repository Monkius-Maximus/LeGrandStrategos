# 21 — Campaign Eras

**Status:** DRAFT — structure and names decided; what triggers an era is open
**Date:** 2026-10-16
**Related:** `82` (§4.14, era names), `61-doctrine-axes.md` (doctrine nodes per era), `80` (era transition screen), `91` (Route D)
**Sources:** the June 2026 "Phase 2" design session (chat archive, not a doc until now) and `82` batch 5.
**Numbering:** the Unreal repo has its own `21-progress.md` (technology and eras over 1820–1925); it is superseded (`00` §6, `95` Q3), so `21` here does not collide with a current doc.

---

## §1 — The five eras

| Era | Name | Years | What changes in warfare and industry |
|---|---|---|---|
| I | **Flintlock** | 1800–1840 | Line fire and volleys; smoothbore muskets; bronze artillery; cavalry charges. |
| II | **Steam** | 1840–1870 | Rifled guns; railways move armies; steam ships and ironclads; the telegraph appears. |
| III | **Telegraph** | 1870–1890 | Repeating rifles; field fortification; fragmentation artillery; mature telegraph networks. |
| IV | **Wireless** | 1890–1914 | Smokeless powder; machine guns; quick-firing artillery; radio; dreadnought navies. |
| V | **Mechanization** | 1914–1950 | Total war; tanks and motorized logistics; aircraft; combined arms. |

**[DECIDED]** Five eras and these boundaries. Diogo confirmed the five-era structure in the June 2026 session, which took the summary table of the original material as the source of truth.

**[DECIDED]** The names, from `82` batch 5. Format: *Era* + Roman numeral + one English noun, e.g. "Era II · Steam". A name never repeats a good or another term. **[DECIDED batch 6]** Checked against the method names of the goods doc (`34`): no clash. "Steam Engine" is a method of Engines, a different name.

**Superseded:** the Unreal repo's five eras (Late Mercantile 1820–1840, Steam Age, High Industrial, Late Imperial, Early Modern to 1925; `95` Q3).

⚠ RECONCILIAR — the June session flagged a date mismatch inside the original material: its summary table and its narrative sections disagree on where Eras III–V start. The table above follows the summary table. Check it against the source file if it is still in the repo.

---

## §2 — What eras drive

| System | Use of the era | Where |
|---|---|---|
| Doctrine | Each doctrine axis has one node per era; entering a new era gives each invested axis its node for that era. | `61` |
| Unit classes | The Phase 2 design lists unit classes per era (land, sea, air, support). | Chat archive; to be reconciled with `60` |
| Interface | An era transition has its own Gazette screen; the Top Bar shows the current era. | `80`, `83` |
| Names | The name format and the five names. | `82` §4.14 |

Technology (`50`) does not use eras. It advances through spark, diffusion and adoption. In the first build of Route D, tiers advance by dated events (`93` §4).

---

## §3 — Open decisions

1. **[OPEN] What triggers an era.** Two options:
   - **Calendar.** Every nation enters the era on its start date. Simple, and it matches the dated events of the first build.
   - **Per nation.** A nation enters the era when it reaches a threshold, for example a Steel Tier. This expresses the asymmetry pillar, but it adds a rule and a screen.

   **[PROPOSED]** Calendar in the first build. Per nation stays a possible phase 2 change, decided after playtests show whether the asymmetry needs it.
2. **[OPEN] Is the era global or per nation?** This follows from decision 1: calendar means global.
3. **[OPEN] What else happens at a transition** besides doctrine nodes and the Gazette: new unit classes, new laws or methods becoming available? Nothing else is decided.

---

## §4 — Out of scope

- Era-locked content lists (which unit, law or method appears in which era). They belong to the system docs that own that content.
- The soft end at 1950 (`82` `soft_end`, `90` §1.1): the campaign continues past Era V; no Era VI is planned.

---

## Appendix — Decisions taken

- Five eras with the boundaries in §1. *(Confirmed by Diogo, June 2026 session.)*
- Era names Flintlock, Steam, Telegraph, Wireless, Mechanization. *(Confirmed by Diogo, `82` batch 5, 2026-10-15.)*
- Names checked against the goods doc's methods; no clash. *(`82` batch 6, confirmed by Diogo, 2026-10-16.)*
- The descriptions in the "What changes" column. *(My summary of the June session's era tables.)*
- Calendar trigger for the first build. *(My proposal.)*
