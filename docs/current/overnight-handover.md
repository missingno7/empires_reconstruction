# Overnight grinder handover

Verified frontier: 72 CHEAP cards / 3,866 bytes; no MEDIUM cards; one
SUPERVISOR card / one byte (19A2). The three former MEDIUM cards were accepted.
All 73 cards pass the queue audit. Fresh EXE acceptance and 49 targeted tests pass.
Six unresolved control-flow domains remain separately visible in blockers.json;
symbolic source does not imply all caller assumptions have been proved.

Start with `docs/current/grinder-instructions.md`, then run:

```powershell
python tools/audit_grinder_queue.py
python tools/reconstruction_factory.py next
```

Consume CHEAP cards serially. Use the exact card FAST/promotion commands, allow
at most two targeted fixes, and use `--block` to archive/restore a failed bounded
attempt before taking another card. Blocked byte ranges survive card regrouping;
accepted source restoration preserves exact file bytes, including line endings.
Run the audit again when stopping. Do not schedule or launch additional workers
against this checkout concurrently.

Whole repeated bodies now form the unrolled work units. Both 15-byte and
21-byte instruction patterns have pinned TASM tests against every one of the
80 repetitions. See the UNROLLED_TRANSFER rule and each card's guidance.

The queue is finite. Stop when no CHEAP task remains; do not silently start
MEDIUM, re-C, or archive work to fill elapsed time. No overnight run has been
started by preparing this handover.

## Supervisor findings to preserve

* Graphics selectors 2/5 load replacement code from AE000:003/:002 before the
  first runtime entry. Built-in selectors 1/3/4 therefore use a different code
  image. Do not treat a mode-5 dispatch through the built-in table as the normal
  VGA execution path.
* The two archive drivers total 5,918 decoded bytes. Recursive traversal covers
  1,884/1,886 bytes and 3,497/4,032 bytes respectively, with no decode overlap.
  All remaining bytes are now typed tables (528 bytes) or skipped NOP padding
  (nine bytes across both images). The CGA indirect target set is complete.
* The greedy encoder does not reproduce their compressed streams. The existing
  explicit pair-span instruction format does: both complete streams were
  independently re-encoded exactly. Plans are regenerated under ignored
  `build/overlay-compression-plans/` by `python tools/report_runtime_platform.py`.
  The next supervisor step is canonical decoded-ASM source ownership plus an
  exact archive promotion/checker path. These are not yet consumable cards.
* ASM representation is not proof of original assembly authorship. The
  `asm-origin-review.json` inventory covers all 44 remaining production TASM modules;
  C-like state machines and shared-frame continuations are marked for later
  review. No speculative ASM-to-C conversion belongs in the current grinder.
* Audio evidence includes PIT/speaker output, an OPL timer/register path, and
  an alternate packed-nibble output path. OPL3-specific use is not established.
  F_53BF calls the OPL probe; historical wording calling it a VGA probe should
  not be reused as evidence.

Detailed local evidence is in `runtime-platform.json`, `runtime-cfg.json`, and
`runtime-boundary-recovery.md`. Archive drivers are a separate held frontier;
they are not counted as recovered production bytes or overnight queue capacity.

The follow-up findings, including the translated unused table and suspicious
cross-routine dispatch, are in `runtime-supervisor-findings.md`.

## Alternate graphics boundary follow-up

`python tools/report_runtime_platform.py` now regenerates both overlay CFG
reports using byte-pinned evidence in `tools/overlay_boundaries.py`.
`python -m unittest discover -s tests -p test_overlay_boundaries.py` checks
complete disjoint byte ownership, skipped-jump padding, all width values and
coordinate carry combinations, dispatch destinations, and stale-proof rejection.

The CGA table at 02D2–02E2 contains eight absolute CS words selecting six
copy/alignment paths. The AND/RCL/SHL sequence bounds its index without caller
assumptions. Following it recovers 315 previously unvisited instruction bytes.
The tables at 0980–0A80 and 0A80–0B80 implement color-pair translation and
nonzero-nibble transparency masks; their formulas match all 256 entries each.
Coordinates here are relative to the replacement image loaded at CS:039C.

Next supervisor task: integrate decoded ASM ownership and explicit compression
plans into the archive source build. Establish a checker that verifies assembled
payload bytes, re-encoded resource bytes, and the full archive before promotion;
then issue bounded cards from overlay CFG work_units. These units are held
planning evidence, not approved grinder cards. The existing 72-card EXE queue
is unchanged. No increase in canonical recovered archive bytes is claimed.

User-authorized exact C recovery has since replaced F_AB66 (385 bytes) with
ordinary C. See docs/history/exact-c-recovery.md; this did not consume any runtime card.

F_880A and its former F_8C04 continuation are also now one exact ordinary C
function (557 bytes). Production has 292 unchanged source objects and 441 linker
inputs; the full EXE and relocation order still match. F_7964/F_7DD3 is next.

F_7964/F_7DD3 is now also one exact C function (664 bytes). Current production
has 291 unchanged source objects, 440 linker inputs, and 48 TASM modules.

F_B40F now compiles as ordinary C (236 exact bytes); 47 production TASM modules
remain. Two resource-pointer BSS aliases were recovered without moving storage.

F_AA1F is now ordinary C too (327 exact bytes). Production has 46 TASM modules;
the 72-card runtime CHEAP queue remains untouched by these C recoveries.

F_B7F9 is now ordinary C (366 exact bytes), using existing workspace arrays.
Production has 45 TASM modules; runtime grinder cards remain unconsumed.

F_4F96 now compiles as exact ordinary C through Turbo C -B/TASM (299 bytes).
There are 44 TASM-source modules; F_28AC remains held on a conversion mismatch.


## Compiler-path review and run entry point

Use `docs/current/grinder-run-prompt.md` for the prepared unattended run.
`grinder-readiness.json` inventories all remaining ASM sources and keeps held C
work separate from executable runtime cards. Check it with
`python tools/prepare_grinder_handover.py --check` after the queue audit.
Regenerate it after accepted runtime work with `python tools/prepare_grinder_handover.py`.
No grinder has been started. The arithmetic -B probes match bytes but do not
resolve the shared module's relocation ordering; no new C CHEAP cards were issued.
