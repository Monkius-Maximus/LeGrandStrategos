# 01 — Orchestrator

**Status:** LIVING — how any conversation in this Project works on LGS. Update it when the method changes.
**Date:** 2026-10-16
**Related:** `00-start-here.md` (state), `82` (names), `93` (sprints), `90` §1.4 (principles), `HANDOFF-2026-10-16.md` (last full snapshot)

The short version for the Project instructions field is at the end (§9).

---

## §1 — First moves in every conversation

1. **Read `00-start-here.md`.** It holds the dated state. Note its **Date**.
2. **Look for newer work.** Several sessions run in parallel. Search recent chats (`recent_chats`, `conversation_search`) for anything after `00`'s date that touches the docs you will use. If a newer session changed a doc, read the doc again from the Project before relying on it.
3. **Route the request** with §3 and read only the docs it needs.
4. **Check the open decisions** in `00` §10 and `93` §5. If the request depends on one, say so and ask, or propose; do not quietly decide it.
5. **Say in one sentence what you will do**, then start.

---

## §2 — Which source wins

1. Diogo's latest message.
2. The numbered docs in the Project; between two docs, the newer **Date**. A conflict between docs is reported, never silently resolved.
3. Reports (`94`, `95`, future recons): evidence about the code as of their date.
4. The code itself, when a session can read it, over any doc that describes it.
5. The chat archive (old conversations).
6. Claude's memory of the user. It is a pointer, not a source; it lags the docs.

Old summaries and memory still describe LGS as an Unreal 5.5 project. It is a Godot 4.7 / C# project (`00` §2).

---

## §3 — Routing

| Request | Read first | Produce | Update after |
|---|---|---|---|
| Any name, label or word on screen | `82` | A proposed row, or the existing name | `82` row and appendix after confirmation |
| Sprint, plan, "what next" | `00`, `93` | An answer or a plan change | `93`, `00` §2 |
| A system's design (economy, politics, war…) | `00` §4 and §6, the system doc, `82`, the matching recon | A proposal, then a doc | The system doc, `82` for new names, `00` if the state changes |
| A system marked [OPEN] or missing | `00` §10 | Questions or options, not a full design | — |
| UI or mockups | `80`, `81`, `83`, `82` | Canvas edits | `83` (and check `83`'s one-editor rule first) |
| Code, repo, port | `93`, `92`, `94`/`95`, the repo `CLAUDE.md` | A Claude Code prompt (recon first) or a read-only recon from claude.ai | `93`, a report doc |
| "Where are we?" / onboarding someone | `00`, latest handoff | A short answer | — |
| The other games (LGP, SDG) | `00` §9 | Answer within what `00` records; LGP's fate is [OPEN] | — |

---

## §4 — Propose, then confirm

- Put decisions as a **numbered list**, each with **your recommendation first** and one line of why. Mark the ones that are Diogo's choice.
- Diogo answers in short form: **"ok"** accepts every recommendation; otherwise he adjusts by number. Treat "ALL OK, KEEP PRODUCING AS PLANNED" the same way.
- After confirmation: mark the items **[DECIDED]**, and add an appendix line to the owning doc: what was decided, *(Confirmed by Diogo, YYYY-MM-DD.)*. Your own proposals that he did not confirm stay **[PROPOSED]** and are labeled *(My proposal.)*.
- If, while applying, you find that a confirmed item cannot work as written (as with Steel T5 and the fail-fast loader), apply the nearest correct version and **say so in your reply**; never hide the change.
- One question at a time in conversation; batches of decisions go as one numbered list.

---

## §5 — Writing rules

- **Language:** docs, data and game text in English, US spelling; the conversation with Diogo in Portuguese. A Portuguese doc is converted when it is next edited.
- **Names:** only `82` names. A new player-visible concept needs a proposed `82` row before it appears anywhere. Never a retired variant (each row's Note lists them).
- **Markers:** [DECIDED], LOCKED vX, [PROPOSED], [OPEN], [REF], ⚠ RECONCILIAR, *waits* (`00` §7).
- **No fabricated identifiers.** An id not verified against code or data gets ⚠ RECONCILIAR.
- **Every doc ends with "Appendix — Decisions taken".**
- **Reports are snapshots.** Do not edit `94`, `95` or later recons; record their consequences in the owning doc.
- **Doc numbers in use:** 00, 01, 21, 33, 34, 41, 42, 43, 50, 51, 52, 60, 61, 70, 80, 81, 82, 83, 90, 91, 92 (ADR-0001), 93, 94, 95, 96 (reserved). Check this list before numbering a new doc; add the new number here.
- **Style:** plain prose and tables. Mermaid only for linear flows, never for dense webs.

---

## §6 — Tool mechanics in this Project

- `project_read` before every rewrite; `project_write` replaces the whole doc.
- `project_write` with `local_path` works only for a file under the session's working directory; write the file there first.
- New docs written by Claude land under `claude/`. Moving one to the top level is a manual step in claude.ai.
- The Unreal repo is public: clone it read-only with `add_repo` + `git clone --depth 1`. The Godot repo is not reachable until Diogo pushes it.
- Canvases (`83`): read the current version before publishing; only one session edits a canvas at a time.
- To check names: regenerate the termbase from `82` §5 (the mapping is in `PROMPT-APPLY-TERMINOLOGY.md` step 2) and search the docs you changed for retired words.

---

## §7 — Before you finish

- If the state changed: update `00` §2 (and §10 if a decision opened or closed).
- If the plan changed: update `93`.
- If a name appeared or changed: `82`.
- Appendix lines added to every doc you changed.
- Reply in Portuguese: what came out, what changed in which docs, and the next decision Diogo has to make. No step-by-step recap.

---

## §8 — Do not

- Design a system marked [OPEN] or missing (diplomacy, religion, migration, technology tree, doctrine) without asking first.
- Reopen a [DECIDED] item without new evidence; when evidence appears, present it and ask.
- Add a fallback, a silent default or a second way of doing something.
- Trust a doc's claim about the code when the code can be read.
- Write player text that is not an `82` name.
- Edit a dated report.

---

## §9 — Text for the Project instructions field

Paste this into the Project's custom instructions:

```
Projeto: Le Grand Strategos (LGS), grand strategy em Godot 4.7 / C#.
Antes de qualquer resposta sobre o projeto:
1. Leia claude/00-start-here.md (estado atual, datado) e claude/01-orchestrator.md (como trabalhar).
2. Busque conversas recentes por trabalho mais novo que a data do 00; se houver, releia os docs afetados.
3. Siga o roteamento do 01 §3 e leia só os docs necessários.
Regras: docs, dados e texto do jogo em inglês (US); conversa em português. Use só os nomes do doc 82.
Propor-então-confirmar: listas numeradas com recomendação primeiro; "ok" aceita todas.
Recon antes de implementar; fail fast; sem fallbacks; uma fonte de verdade; sem IDs inventados (⚠ RECONCILIAR).
Docs numerados vencem memória e conversas antigas. Relatórios (94, 95) são snapshots: não editar.
Ao terminar: atualizar 00, 93 e 82 quando o estado, o plano ou um nome mudar, e dizer qual a próxima decisão do Diogo.
```

---

## Appendix — Decisions taken

- This orchestrator and the instruction text. *(Requested by Diogo, 2026-10-16; content is my proposal.)*
- Source-of-truth order (§2) and doc number registry (§5). *(My proposal.)*
