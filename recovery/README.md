# Recovery: inactive reference and oracle sources

Nothing under `recovery/` is compiled by the production build. The production
build consumes exactly the sources listed under `source`/`sources` in
`layout/production-plan.json` (127 modules); everything else that used to live
in `src/` and `asm/` has been moved here, under the same file names, so that
history and provenance stay intact without cluttering the production-facing
tree.

`layout/manifest.json` still names these files: a region built from a
different, active source can carry a `provenance.prior_source`,
`provenance.matching_c_source`, or similar field that points into `recovery/`
for provenance -- i.e. "this earlier session's evidence lives here," not
"this file is compiled."

## What's here

`recovery/src/` (70 files) and `recovery/asm/` (56 files) hold four kinds of
retired material from earlier reconstruction waves:

1. **C references that are `asm db` capsules of an ASM module.** These are
   whole-function-body `asm { db ... }` wrappers written to document or probe
   a hand/TASM-assembled routine byte-for-byte; they were never a mechanical
   replacement, because a capsule reproduces bytes without expressing the
   compiler's own code generation. For example `recovery/src/F_4B0C.C` and
   `recovery/src/DOS_STUB.C` are recorded as `INSTRUCTION_CAPSULE`/
   `LEGITIMATE_DATA` capsules in `docs/current/asm-capsule-audit.json`; the
   active production source for `F_4B0C` is the symbolic TASM module
   `asm/F_4B0C.ASM`, as recorded in `docs/current/asm-provenance.md`.

2. **Superseded single-function ASM later merged into grouped
   `asm/M_*.ASM` modules.** Early waves kept one `.ASM` file per function;
   once adjacent functions were confirmed to belong to one linked object,
   they were regrouped into a single `asm/M_<start>_<end>.ASM` module and the
   old per-function files became redundant. For example
   `recovery/asm/F_C5A8.ASM`, `recovery/asm/F_C5B3.ASM`, and
   `recovery/asm/F_C5C6.ASM` were merged into the active
   `asm/M_C5A8_C5C6.ASM`; `recovery/asm/F_C1A0.ASM`, `recovery/asm/F_C1F7.ASM`,
   and `recovery/asm/F_C232.ASM` were merged into `asm/M_C1A0_C232.ASM`.

3. **C modules retired by later promotions.** Some production modules were
   originally reconstructed as several independent per-function C or ASM
   files and were later replaced, whole, by one ordinary Turbo C translation
   unit that reproduces the same bytes without any inline-asm or capsule
   trick. `recovery/src/F_DDD9.C`, `recovery/src/F_DE7E.C`,
   `recovery/src/F_DEFA.C`, and `recovery/src/F_DF98.C` are the four
   predecessor references for what is now `src/MUSIC.C`; `recovery/src/F_6B1A.C`
   and `recovery/src/F_6B4A.C` are the predecessor references for what is now
   `src/KEYBIOS.C`. `recovery/asm/MUSIC.ASM` and `recovery/asm/M_6B1A_6B4A.ASM`
   are the ASM routes those C units replaced (`F_6BCF`, now expressed in
   `src/TIMERIRQ.C`, was promoted directly from ASM with no separate
   single-function C predecessor to retain).

4. **The frozen inline-ASM oracle, `recovery/src/RUNTIME_BLOCK.C`,** for
   `asm/RUNTIME_BLOCK.ASM`. `RUNTIME_BLOCK` is Turbo C/Borland vendor runtime
   and startup dispatch code, not application logic, and it is assembled
   directly from `asm/RUNTIME_BLOCK.ASM` in production. `recovery/src/RUNTIME_BLOCK.C`
   is kept only as the historical migration oracle used for symbol naming --
   it was never a compiler-verified replacement and is not attempted as one.

## Why these stay

Each of these files is real evidence produced during reconstruction: capsule
audits, byte-for-byte probes, and superseded-but-once-canonical sources that
downstream reports (`docs/current/asm-provenance.md`,
`docs/current/asm-capsule-audit.json`, `docs/current/exact-c-recovery.md`,
and others) cite directly by path. Moving them out of `src/`/`asm/` keeps the
production-facing tree limited to what actually builds, while `git mv`
preserves their history and this directory keeps them reachable at the same
relative file names the existing reports and recipes already name.
