# Translation-unit structure: evidence and rules

The exact binary is the oracle for *which functions were compiled together*, not
only for what each function compiled to. This note records the compiler facts
established on 2026-09-21 with the pinned Turbo C 2.0 / TASM 1.0 toolchain, the
tools that encode them, and the runs proven to be single translation units.

## Facts (each verified by a probe against the original bytes)

1. **`-B` is a symptom, not a choice.** Turbo C 2.0 restarts a unit through the
   assembler ("Restarting compile using assembly") when it meets an inline `asm`
   statement; the whole .C file is then emitted as .ASM and assembled by TASM,
   which relaxes some jumps differently from the native object writer. A module
   that is byte-exact only with `-B` and contains no `asm` therefore shared its
   historical .C file with a function that had one. `tools/audit_tu_flags.py`
   reports every such unexplained flag (target: 0).
2. **`-B` intolerance is a TU anchor.** `tools/probe_module.py ID --as-c ID=-B`
   over every plain-C module found these NOT byte-exact under `-B`: HITTEST
   (C_5A3B_6021), HUD (C_6FC3_747B), MENULOOP (F_7964), DIALOG (C_7D91_880A),
   PUZZLE (C_8A37_969D), ROUNDEND (C_9A0E_9D79), ANIMFRAM (F_9DCC), SLOTMENU
   (C_A33F_AD0E), LEVEL (C_AF45_C15E), PLAYERSL (C_CDDD_D344), HINTDLG (F_D4B3),
   OPLVOICE (C_DB60_DDC7), OPLREG (C_E095_E54D), MUSIC (M_DDD9_DF98, also by
   relocation topology). None of them can have shared a .C file with inline asm.
   Every other C module tolerates `-B` (byte-neutral), so its TU membership is
   decided by neighbours, not by its own bytes.
3. **`-k` has no C-level explanation.** OPTIONS (M_CB5C_CD23) and PLAYERSL grow
   under `-k`; a tiny module that needs `-k` next to hand-written assembler is a
   hand-written frame, not a compiler option (see the sound cluster below).
4. **Prologue fingerprints.** Turbo C saves `push si` then `push di` (after
   `sub sp,n`) and only when the function (or its inline asm) names them; the
   epilogue is `pop di / pop si / [mov sp,bp] / pop bp / ret`. A framed routine
   that uses SI/DI without saving them, saves AX/CX/DX/BX inside a frame, or
   pushes `di` before `si` was written by hand. Probe: SNDSCL4 + M_C5A8_C5C6 as
   one C unit emits `push si` around each asm body and is 6 bytes longer.
5. **Word-alignment pads mark assembler module boundaries.** Turbo C emits
   `_TEXT segment byte public`; the four one-byte pads in the image (before
   F_4AA8, M_6D86_6DCC, M_D61C_D79C, M_D818_D825) are TLINK aligning a
   `segment word public` TASM module, so those four are genuine separate
   assembler modules. Odd-address assembler routines were byte-packed behind
   another routine of the same unit.
6. **Relocation order is a pure-C TU test.** A native Turbo C object emits
   FIXUPPs descending; two neighbouring pure-C modules that each carry an EXE
   relocation cannot be one unit unless the later module's relocation precedes
   the earlier one's in the EXE table (SLOTS/SLOTCOPY: separate units).

## Proven single translation units (`python tools/probe_tu.py FIRST LAST`)

| Run | Modules | Bytes | Flags | Why it is one unit |
|---|---|---|---|---|
| F_3A75..C_49E3_4A93 | TURNLOOP, BRDTERR, MENUBKDP, LVLDRV, BLITPAT, BOOTSEED, CAMPADV, GAME | 4146 | '' | TURNLOOP and LVLDRV need `-B`; BOOTSEED's `asm sti` supplies it. CAMPADV/GAME are contiguous and `-B`-neutral (ambiguous extension). |
| F_4F63..F_50C1 | DOSWRT2, CMDLINE, BIOSEQP | 367 | '' | CMDLINE needs `-B`; BIOSEQP's `asm int 11h` supplies it. |
| F_520A..C_5321_56C6 | VIDMODE, INTRO | 2097 | -B | VIDMODE needs `-B`; no asm inside the run, so its asm neighbour must be M_50D2_53BF (under test as inline asm / C). |

Runs rejected by the oracle: F_520A..C_5A3B_6021 with `-B` (HITTEST shrinks by
one byte: anchor), F_CB48+OPTIONS with `-k` (OPTIONS grows 4 bytes per
function: SNDSTOP is not a `-k` C unit with OPTIONS).

## Tools

- `tools/probe_tu.py FIRST LAST [--flags ..] [--source MODULE=cand.C] [--asm UNIT.ASM]`
  compiles a contiguous run as one C unit (assembler modules inside it need a
  C candidate) or assembles one TASM candidate for the whole run.
- `tools/audit_tu_flags.py [--strict]` lists `-B`/`-k` flags no source explains.
- `tools/tu_recipe.py FIRST LAST NEW_ID --evidence ..` writes the grouped-module
  recipe for a proven run; `tools/merge_module.py` then makes it one file.
