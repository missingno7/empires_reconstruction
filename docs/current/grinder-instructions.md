# Grinder instructions

Use only `docs/current/` for current status. The working runtime source is
`asm/RUNTIME_BLOCK.ASM`; `src/RUNTIME_BLOCK.C` is the frozen migration oracle.
All card ranges are **runtime-relative, start inclusive, end exclusive**.

1. Run `python tools/reconstruction_factory.py refresh`, then
   `python tools/reconstruction_factory.py next`.
2. Choose the highest-priority **CHEAP** task. Read its JSON card and the
   applicable rules in `docs/current/tasm-reconstruction-rules.md`.
3. Edit only the card's source-line interval. Add local labels within that
   interval as needed. Do not edit recipes, tools, other sources, or the oracle.
   Keep unresolved bytes explicit. The disassembly is a guide; TASM decides encoding.
4. Run the card's `verification_command`. FAST assembles only the runtime module
   and checks all 6,571 bytes, publics, ordered fixups/frames/addends, edit scope,
   and a strict decrease in unresolved bytes. Unchanged exact source fails.
5. Apply a reported known rule and retry. After two unsuccessful targeted fixes,
   or immediately for an unfamiliar architectural issue, run:
   `python tools/check_candidate.py TASK_ID --block "exact reason"`.
   This archives the failed source, records its diagnostic, restores only an
   isolated candidate edit, refreshes the queue and marks BLOCKED_SUPERVISOR.
   Continue with the next CHEAP task. Do not spend hours investigating.
6. When FAST passes, run the card's `acceptance_command` (`--promote`). It reruns
   FAST, recompiles every final C/ASM/BSS input without caches, emits DATA,
   runs exactly one TLINK, and proves all 106 relocations in order and the full
   EXE hash/fixture equality. A failed build never leaves a current success receipt.
7. Inspect the diff. Commit the source edit and generated `docs/current/` reports
   only after ACCEPTED. Do not stage unrelated user changes. A promotion does
   not commit automatically.
8. The accepted queue is already refreshed. Pick the next CHEAP task and repeat.

Do not refresh away an unaccepted source change: the command rejects it. Do not
reuse a previous PASS. Reports carry complete construction-input fingerprints.
Supervisor and ambiguous code/data tasks never block CHEAP task selection.
`MEDIUM` tasks require a model capable of reading their CFG; they are not selected
by `next`.

Setup: `python -m pip install --no-user --target build/python-deps -r requirements-factory.txt`.
The compiler, assembler, library and linker are local hash-pinned Borland inputs;
see the repository README for installation. No decoder/network is needed in the
canonical EXE construction path.

## Unattended batch handover

Before the first edit, run `python tools/audit_grinder_queue.py`. Work serially:
only one worker may change the runtime source and promote cards. Use `next`
after every promotion or block; saved card line numbers become stale.

The queue is finite. If `next` returns null, record the final metrics and stop;
do not automatically begin MEDIUM, supervisor, archive-overlay or re-C work.
There is no guarantee that a particular number of cards will occupy a full night.

Blocked attempts are recorded by byte range as well as task ID. Subsequent
regrouping must not make the same failed bytes eligible again. The blocker
archives only the isolated candidate edit, restores its accepted baseline and
continues with other CHEAP work. Never manually clear failure memory. If scope
restoration fails or unrelated files changed, stop for supervision.

Two targeted fixes remain the limit. A failed acceptance is never success;
archive/restore the bounded attempt using `--block` and continue only if that
succeeds. If the final attempt failed acceptance and removed the build receipt,
run `python tools/build_exe.py verify` on restored sources, refresh, and rerun
the audit before reporting a verified handover.

The checkout has pre-existing modified and untracked files. Do not stage all
files or create a commit that absorbs another task's edits. Promotion does not
require a commit. Leave accepted edits and generated cards reviewable when a
clean task-only commit cannot be made safely.

Platform evidence is in `runtime-platform.json`; representation/origin triage is
in `asm-origin-review.json`. ASM is not proof of historical ASM authorship.
These reports do not authorize speculative C conversions or additional archive
work in the overnight grinder.
