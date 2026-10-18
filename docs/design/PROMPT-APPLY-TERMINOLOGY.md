# PROMPT — Apply the terminology to the repo

**For:** Claude Code, with the `le-grand-strategos` repo mounted.
**Type:** two turns. **Turn 1 — plan only**: read, write the plan, stop. **Turn 2 — implement**: only after Diogo approves the plan in a new message.
**Date written:** 2026-10-15 · **Updated:** 2026-10-16 (`82` batch 6: 222 terms, statuses renamed)
**Source of truth:** `82-terminology-and-localization.md` (attach it, or copy it to `docs/` first). §3 has the rules and the shared names, §3.1 the termbase columns, §5 the 222 terms.

---

## Preconditions — check them first, stop if one fails

1. `docs/95-recon-restart.md` exists (the report of `PROMPT-RECON-RESTART.md`). If not, stop: run the recon first.
2. `82` is available, either attached to this session or at `docs/82-terminology-and-localization.md`.
3. The solution builds (`dotnet build`). If it does not, stop and report; this pass does not fix the build.

Report each check with its evidence at the top of the plan.

---

## What this pass does

| Step | Change | Done when |
|---|---|---|
| 1 | Put `82` in `docs/` if it is not there | File present, unchanged from the attached copy |
| 2 | Generate `data/loc/terms.csv` from `82` §5 | Counts match (see below); every key unique |
| 3 | Convert plurals to Godot's `?plural` layout | Godot import test passes |
| 4 | Termbase test | Test exists, passes, and fails on a broken row |
| 5 | Retired-variant check | Check exists, passes on the scoped files, fails on a planted retired word |
| 6 | Update `CLAUDE.md` | Language and termbase rules match `82` |
| 7 | Report code identifiers that use retired words | A list in the plan; nothing renamed without approval |

## What this pass does not do

- It does not rewrite docs. `82` §1: each doc is converted to English when it is next edited.
- It does not rename locked code IDs (`IG_*`, `LAWCAT_*`, `82` §8), and it renames nothing else without approval in step 7.
- It does not touch the mockup canvases. Those live in claude.ai (`83`).
- It does not decide any term. Rows with status `waits` or `decided_waits` go into the CSV as they are, marked by `_status`.
- It does not create data IDs (`G_*`, `PM_*`). The Goods rows name the goods; the data seed is `93` S1-08.

---

## Step 2 — Generate `data/loc/terms.csv`

Write the generator as a small script under `tools/` (C# console or a script; say which in the plan) so it can be rerun whenever `82` changes. Mapping, from `82` §5:

- Read only the tables between the `## §5` and `## §6` headings. A row is a table line whose first cell starts with a backticked ID.
- `_domain` = the nearest `###` heading above the row.
- `_id` = the ID without backticks. `keys` = `TERM_` + `_id` in upper case.
- EN cell `**Name** / Plural` → `en` = Name, `_plural_en` = Plural. EN cell `**Name**` → `_plural_en` empty.
- `_kind` = the Kind cell. `_definition` = the Meaning cell; `—` becomes empty.
- `_retired` = every single-quoted word in the sentences of the Note cell that contain "retire" or "retired", joined with `;`, duplicates removed.
- `_status`: Note contains `[DECIDED` and the italic marker `*Waits` → `decided_waits`; only `[DECIDED` → `decided`; only `*Waits` → `waits`; neither → `proposed`.
- `pt_BR` empty. Column order: `keys,en,pt_BR,_id,_kind,_domain,_plural_en,_definition,_retired,_status`.

**Expected counts for `82` dated 2026-10-16:** 222 rows; `decided` 219, `decided_waits` 2 (`law`, `lawcat_military`), `waits` 1 (`line_infantry`), `proposed` 0. Shared names: exactly the six of `82` §3 (Military ×4, Economy ×3, Religion, Nation, Steel Tier, Market ×2 each). If the `82` you have is newer, recompute and list the differences in the plan instead of matching these numbers.

## Step 3 — Plurals

Godot 4.6+ reads plurals from a `?plural` column, with each locale's extra forms on extra rows below the main row and a `?pluralrule` row per locale; code looks them up with `tr_n()`. Before converting `_plural_en`:

1. Check the exact layout against the Godot docs page *Localization using spreadsheets* for the Godot version in the repo, and quote the example you follow in the plan.
2. Convert. Keep the `_plural_en` column only if Godot's layout leaves no other place for the English plural; say which in the plan.
3. Add an import test in `Strategos.Game` (not in `Strategos.Sim`; ADR-0001) that loads the CSV and checks `tr_n()` on `TERM_POP` returns "Pop" for 1 and "Pops" for 2.

## Step 4 — Termbase test

A test project with no Godot reference (`tests/Strategos.Terms.Tests`, or say why another place is better). It fails when:

- a key is duplicated, or does not equal `TERM_` + upper(`_id`);
- an `_id` is not snake_case;
- a row with status `decided` or `decided_waits` has an empty `en` or `_definition`;
- `_kind` is not `player`, `dev` or `name`;
- an `en` value appears on two or more rows, unless that name and exactly those IDs are listed under "Shared names" in `82` §3. Read the list from `82`; do not copy it into the test;
- a word in any row's `_retired` list appears, as a whole word, inside any row's `en` value.

Prove it: break one row on purpose, see the test fail, revert.

## Step 5 — Retired-variant check

Same test project. For every `decided` row, search for each word in its `_retired` list and fail with `file:line: 'word' is retired, use 'en' (term_id)`.

**Scope.** Some retired words are ordinary English ("class", "known", "concept", "vote", "odds", "g") and Portuguese docs will contain every Portuguese word until they are converted. So:

- **UI text** (scene files and data under `src/Strategos.Game` and `data/`, excluding `data/loc/terms.csv`): check every retired word.
- **Docs**: check only the files listed in `docs/.english-docs` (create it, one path per line; start with `00`, `82`, `83` if they are in `docs/`). A doc joins the list when it is converted. Dated reports (`95`, `96`) never join: they are snapshots (`00` §11).
- Match whole words, case-sensitive. Skip fenced code blocks and text in backticks.

In the plan, list every retired word with the scope you propose for it (UI only, or UI + English docs). Diogo approves that list before step 5 is built.

Also add the fixed retired list of `82` §3 rule 9 and `93` S1-01, checked in English docs: `Labour`, `Centre`, `Colour`, `1820–1880`, `42-depositos`, `40-resources`, `41-goods`.

## Step 6 — `CLAUDE.md`

Replace any rule that says docs or comments are written in Portuguese with:

- English is the source language for docs, comments, data and game text, in US spelling (`82` §1, §3 rule 9).
- Every player-visible string comes from a key in `data/loc/terms.csv` or another loc file; no literal strings in UI (`80` §12).
- Use the termbase names; never a retired variant. The check in step 5 enforces it.
- Conversation with Diogo stays in Portuguese.

Quote the old lines and the new lines in the plan.

## Step 7 — Code identifiers

From `docs/95-recon-restart.md` Part B.5, list the code identifiers (types, fields, enum values, files) that carry a retired word or a Portuguese word. For each: file, identifier, the term it maps to, and whether it is locked. Rename nothing in this step. Diogo picks which ones become a follow-up task.

---

## The plan (turn 1)

Write `docs/96-plan-apply-terminology.md`:

1. Preconditions with evidence.
2. Steps 1–7, each with the files to create or change, the commands to run, and how its "done when" will be shown.
3. The retired-word scope table (step 5).
4. The code identifier list (step 7).
5. Questions for Diogo.

Then stop.

## Implementation (turn 2, after approval)

- One Conventional Commit per step (`docs: add terminology doc 82`, `feat(loc): generate termbase from doc 82`, `test(loc): termbase and retired-variant checks`, `docs: update CLAUDE.md language rules`).
- After each step, run `dotnet build` and `dotnet test` and paste the summary.
- If a step's "done when" fails, stop and report. No workaround, no skipped test.
