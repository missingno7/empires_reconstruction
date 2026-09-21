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

7. **Private DATA must be contiguous.** One unit emits one `_DATA` segment in
   source order, so the initialized-data components of members of one TU are
   adjacent in DGROUP. `probe_tu` refuses a run whose members' DATA components
   are not contiguous (HUD..ANIMFRAM: PROMPTS' C_DATA_778B and DLGELOST's
   TEXT_118C are apart, so those two units are separate).

## Proven single translation units (`python tools/probe_tu.py FIRST LAST`)

All promoted on 2026-09-21 (recipes/modules/*.json carry the evidence text):

| Unit | File | Modules merged | Bytes | Why it is one unit |
|---|---|---|---|---|
| C_200F_3986 | src/BOARD.C | BOARDDRW, BRDPAINT, BOARDSCR | 6758 | BRDPAINT needs `-B`; BOARDSCR's inline asm supplies it; the a74a2 view conflict (2-D here, flat in BRDTERR) ends the unit before GAME.C. BOARDDRW is a `-B`-neutral extension. |
| C_3A75_4A93 | src/GAME.C | TURNLOOP, BRDTERR, MENUBKDP, LVLDRV, BLITPAT, BOOTSEED, CAMPADV, GAME | 4146 | TURNLOOP and LVLDRV need `-B`; BOOTSEED's `asm sti` supplies it (converting it to `__sti__()` breaks them). CAMPADV/GAME are `-B`-neutral extensions up to the word-aligned pad. |
| C_4F63_520A | src/STARTUP.C | DOSWRT2, CMDLINE, BIOSEQP, video/sound probes (ex M_50D2_53BF, now C), VIDMODE | 958 | CMDLINE and VIDMODE need `-B`; BIOSEQP's `asm int 11h` and the probe fragments supply it; the NOP-padded byte stores in F_50D2 are TASM sizing a forward EXTRN in a TCC-generated unit. INTRO (exact in the unit too) left separate. |
| C_6990_6B74 | src/KEYBOARD.C | KEYCHAIN, KEYIRQH, KEYBIOS, KEYBUFDR, KEYCHACT | 490 | KEYIRQH needs `-B`; KEYBIOS supplies it. KEYIRQ cannot join: its setvect cast needs the non-interrupt view (`push cs`) of the handler symbol. |
| C_6B7A_6C87 | src/TIMER.C | TIMERINS, TIMERRST, TIMERIRQ, TIMER | 300 | asm-bearing throughout; one far interrupt-pointer view of int8_saved_vector and the symbolic `offset timer_irq_handler` only exist inside one unit. |
| C_6CA6_6D3C | src/FONT.C | SPRSHSEL, SPRSHIDX, DLGLNHT, TXTLNWID, TXTWRAP | 223 | contiguous asm-bearing font helpers between TIMER.C and the word-aligned M_6D86_6DCC. |

Runs rejected by the oracle: F_520A..C_5A3B_6021 with `-B` (HITTEST shrinks by
one byte: anchor); F_CB48+OPTIONS with `-k` (OPTIONS grows 4 bytes per function);
F_2AE2..C_49E3_4A93 (no single a74a2 declaration keeps BRDPAINT and BRDTERR
exact); C_695E_697D+F_699E (no single keyboard_irq_handler declaration keeps
KEYIRQ's `push cs` and the interrupt definition); HUD..ANIMFRAM (DATA not
contiguous).  Byte-neutral runs left ungrouped: F_1D47..F_1EC0, F_D3DA..F_D60C,
F_9F40..F_A28D, STARTUP+INTRO.

## Tools

- `tools/probe_tu.py FIRST LAST [--flags ..] [--source MODULE=cand.C] [--asm UNIT.ASM]`
  compiles a contiguous run as one C unit (assembler modules inside it need a
  C candidate) or assembles one TASM candidate for the whole run.
- `tools/audit_tu_flags.py [--strict]` lists `-B`/`-k` flags no source explains.
- `tools/tu_recipe.py FIRST LAST NEW_ID --evidence ..` writes the grouped-module
  recipe for a proven run; `tools/merge_module.py` then makes it one file.
