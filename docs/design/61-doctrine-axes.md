# 61 — Doctrine Axes

**Status:** DRAFT — the doctrine model is OPEN; this doc records what exists so the decision can be made
**Date:** 2026-10-16
**Related:** `60-sistema-de-batalha.md` (command points), `21-campaign-eras.md`, `82` (§4.5, `doctrine`, `design_tier`), `91` (Route D), `95` (Part C, doctrine)
**Sources:** the June 2026 "Phase 2" and "Fase2-Eixos" design sessions, the Protocolo B session (June 2026), the Unreal repo's `docs/architecture/11-units.md` (via `95`), and `82` §4.5.
**Numbering:** `61` is free in the Unreal repo too (`95` lists its docs).

---

## §1 — Four versions of doctrine exist

| When | Version | What it says | Status today |
|---|---|---|---|
| June 2026, Phase 2 | **Four axes × five eras** | Four permanent doctrinal axes; each era opens one node per axis; the player invests military XP in axes. Diogo kept this over a simpler EU4-style alternative because it "sells the idea more". | Never reconciled with `60` |
| June 2026, Protocolo B | **Light doctrine** | Doctrine as a light layer of national identity: one doctrinal choice per unit class, not a full tree. | Protocolo B is superseded by `60` |
| `60`, battle system | **Doctrine feeds command points** | Command points in card battle = commander Logistics + doctrine. | Current battle doc, but missing from the Project and the repo (`95`) |
| Unreal repo, `11-units.md` | **Named doctrines** | Discrete doctrines chosen laterally, each with a scope (National, Commander or both), conditional modifiers, battle cards it grants or forbids, and doctrines it conflicts with. Unlocked by technology nodes; some gated by laws ("Universal Conscription unlocks mass doctrines"). Examples: Napoleonic Line, War of Movement, Trench Warfare, Fire and Movement, Élan, Mahanian, Jeune École. | Unreal design; never coded (`95` Part C) |

What is decided across all four (`82` §4.5): **doctrine is lateral, not a tier ladder**, and it is separate from Design Tier. Effectiveness = Design Tier × Equipment Tier; doctrine does not enter that formula.

The recon found no "concept tier" and no four-axis model anywhere in the Unreal repo (`95` Part C). The four axes exist only in the June sessions recorded here.

---

## §2 — The four axes (Phase 2 version)

| Axis | The question it answers | Origin |
|---|---|---|
| **Maneuver & Shock** | How do I break through and get around the enemy? | HOI4 Mobile Warfare archetype |
| **Firepower & Attrition** | How do I kill more with every shot? | HOI4 Superior Firepower |
| **Discipline & Planning** | How do I hold, plan and dig in? | HOI4 Grand Battleplan |
| **Mobilization & Mass** | How do I turn population into an army? | HOI4 Mass Assault |

The idea of an axis that **changes its content each era** comes from the HOI4 mod *The Great War Redux*: doctrine starts locked in trench attrition and moves toward maneuver as the war changes.

### 2.1 The nodes, axis by era

| Era | Maneuver & Shock | Firepower & Attrition | Discipline & Planning | Mobilization & Mass |
|---|---|---|---|---|
| I · Flintlock | Assault Column / Cavalry Charge | Coordinated Volley | Defensive Square | Levée en Masse |
| II · Steam | Mounted Reconnaissance | Precision Rifle Fire | Railway Logistics | Reservist Conscription |
| III · Telegraph | Skirmisher Infiltration | Fragmentation Barrage | Field Fortification | Professional Army |
| IV · Wireless | Infiltration Tactics (*Stoßtruppen*) | Quick-Firing Artillery Dominance | Trench Warfare | Nation in Arms |
| V · Mechanization | Mechanized Blitzkrieg | Combined Fire (land and air) | Elastic Defense in Depth | Industrial Attrition |

Node names are translated from the Portuguese originals and are **not** in the termbase yet. They join `82` only if this version is chosen (§4).

### 2.2 Rules the Phase 2 version set

- **One linear chain per axis.** A node requires the node of the previous era on the same axis; Era I nodes are roots.
- **Transformation by era.** Entering an era, every axis the nation has invested in gets its node for that era. The player chooses whether to deepen an axis or spread across axes.
- **Resource gate, no fallback.** A node lists the military goods it needs. If one is missing, the node is inert. It does not fall back to the previous era's node.
- **Schema apart from runtime rules.** The data shape of a node is separate from the rules that apply it.

### 2.3 Data shape (engine-agnostic)

The June session wrote a TypeScript schema. Its content, without engine types:

| Field | Meaning |
|---|---|
| `id` | e.g. `discipline_e4_trench_warfare` |
| `axis` | one of the four axes |
| `era` | 1–5 |
| `name`, `description` | display text, from the termbase |
| `requires` | the previous node on the same axis, or none for Era I |
| `resource_gate` | military goods required; missing one makes the node inert |
| `modifiers` | effects, emitted through the modifier registry like every other effect |
| `potentiates` | unit classes the node strengthens, with a modifier and a value |

⚠ RECONCILIAR — the June schema listed its own modifier keys (soft attack, breakthrough, entrenchment, supply use, and others). They were written before the modifier registry was reconciled and may collide with real modifier IDs. Under the rule "no fabricated identifiers" (`90` §1.4, principle 5), none of them is carried over here.

---

## §3 — What the first build needs

Route D's first build has auto-resolved war and puts card battle in phase 2 (`91`). Auto-resolve runs the same engine headless (`60`). So the first build needs doctrine only if the auto-resolver reads command points.

**[PROPOSED]** No doctrine in the first build. Command points use commander Logistics alone until the doctrine model is decided. This keeps one source of truth and does not ship a placeholder doctrine.

---

## §4 — Open decisions

1. **[OPEN] Which doctrine model is current:** the four axes × five eras, the light version (one choice per unit class), the named doctrines of the Unreal design, or something new written into `60`.
2. **[OPEN] How doctrine feeds command points** (`60`). `60` is missing; the Unreal battle code has command points (`CommandPoints`, `RefreshCommandPoints`) but no doctrine (`95`).
3. **[OPEN] How doctrine is earned** if axes are kept. The Phase 2 version used military XP; nothing defines where XP comes from. The named-doctrine version unlocks through technology.
4. **[OPEN] Naval and air doctrine.** The Phase 2 session summarized them by era but did not structure them as axes. The named-doctrine version has two naval schools (Mahanian, Jeune École).
5. **[PROPOSED] No doctrine in the first build** (§3).

---

## Appendix — Decisions taken

- Doctrine is lateral, separate from Design Tier, and outside the Effectiveness formula. *(Confirmed by Diogo, `82` §4.5 and batch 4.)*
- Phase 2 kept the full four-axis system over the EU4-style simplification. *(Diogo, June 2026; not reconfirmed since `60`.)*
- English translations of axis and node names. *(Mine; the axis names match `82` §4.5.)*
- Modifier keys from the June schema not carried over. *(Mine, under `90` §1.4 principle 5.)*
- No doctrine in the first build. *(My proposal.)*
- 2026-10-16: the named-doctrine model of the Unreal design added as a fourth version, from `95`. *(`82` batch 6, confirmed by Diogo.)*
