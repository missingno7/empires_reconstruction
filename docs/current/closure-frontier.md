# Historical-source closure frontier

Authoritative list of what still stands between the exact tree and "complete game
logic in readable C + only irreducible historical ASM".  Maintained by the
supervisor; every row names the next experiment.  Baseline is always the exact
build (SHA 1259348425483d8d97fd8821860b47cfdf58fc8029711eb0ed0e78ab33807a10,
106 relocations, `python -m unittest discover -s tests -p test_build_exe.py`).

Census (`docs/current/source-quality.json`, per member section, 2026-09-21
end of wave 4): 71 modules (60 C units, 10 assembler modules + the runtime
block); C with inline asm 1924 bytes / 16 members; symbolic ASM 5758 bytes / 61
members; runtime block 6571; C 44257 bytes / 261 members.  Unexplained compiler
flags (`tools/audit_tu_flags.py`): 0.  Reconstruction artifacts left: none
(no capsules, no `call $+`, no label-less branches, no invented flags).

## 1. C with inline asm (all closed with compiler evidence)

| Member | Unit | Bytes | asm lines | Why it remains | Last probe | Next experiment | Leverage |
|---|---|---|---|---|---|---|---|
| F_338A board_run_unit_script | BOARD.C | 870 | 5 | `mov al,es:[bx+3]; neg ax; ...; mov es:[bx+3],al`: 28 standalone spellings + 8 in-context probes show TC 2.0 folds every byte-lvalue negation to `neg al`, and every route to a word-width `neg ax` (int temp, static, register) materialises `mov ah,0` plus a spill/reload; the missing extension relies on AH still being 0 from the compiler's own `and ax,0Fh` three statements earlier, which only a human knew | 2026-09-21 (docs/history/probes/neg-ax-forms.C) | none: irreducible hand asm inside a C function, with concrete compiler evidence | closed |
| F_50D2 / F_53BF video/sound-hardware probes | STARTUP.C | 312 | 33 | recovered as C with pseudo-registers; fragments left: `mov display_mode,N` stores (NOP-padded forward EXTRN), ES:SI ROM probes, signed `cmp bl/jl`, `xchg`, `loop`, flag tests after INT, and the `cmp/jne` ladder at l_done (`if (x != N) goto L` inverts to `je/jmp`, +2 bytes each; probed) | 2026-09-21 EXACT | none known; the sound probe's `== 3` test is now C | closed |
| F_6B1A / F_6B4A BIOS keyboard read/poll | KEYBOARD.C | 76 | 35 | branch on ZF straight after `int 16h`; pseudo-register `_FLAGS` reads compile to pushf/pop (probed, grows) | 2026-09-21 | none known; record as irreducible (INT flag-return protocol) | low |
| F_6B7A / F_6BAC timer install/restore | TIMER.C | 85 | 45 | explicit `push ax/dx/ds/es` around DOS calls and `push cs / pop ds`: no C expression saves caller registers | 2026-09-21 | none; irreducible (register choreography) | low |
| F_6BCF timer_irq_handler | TIMER.C | 87 | 2 | bare `pushf`/`popf` around the body; `__emit__` would only hide it | -- | irreducible | -- |
| F_6CA6 sprite_sheet_select, F_6CF6 text_line_width | FONT.C | 138 | 51 | `les si,[bx+table]` far walk with ES:SI, `lodsb` loop with `es:[bx+di]` width lookup; TC never emits LODS and would reload ES per access | 2026-09-21 rejected | none; irreducible hand bodies inside the C unit | low |
| F_1F17 shadow_bitmap_hit_test | SHBMPHIT.C | 122 | 51 | `lds si`, `loop`, shift ladder; hand body with a TC frame | -- | irreducible | low |
| F_D85F / F_D89A record table delete / hit | RECTTAB.C | 145 | 61 | `lodsb`, `rep movsb`, `loop`, BP repurposed (`mov bp,ax`) | -- | irreducible | low |
| F_01BC video_load_palette | VIDEO.C | 18 | 1 | `les dx,pal` (3 bytes) vs `_ES=`/`_DX=` (8 bytes) | 2026-09-21 | irreducible | -- |
| F_490D boot / F_50C1 equipment probe | GAME.C / STARTUP.C | 71 | 3 | `asm sti` / `asm int 11h` are the units' only inline asm; converting them to `__sti__()`/`__int__()` breaks the -B-shaped neighbours (TURNLOOP, LVLDRV, CMDLINE) -- the asm is historically real | 2026-09-21 (INIT probe) | keep; documented in docs/current/tu-structure.md | -- |

## 2. Genuine ASM (hand-written; evidence in docs/current/asm-provenance.json)

| Module | Bytes | Evidence class | Next useful experiment |
|---|---|---|---|
| M_C1A0_CB48 asm/SOUND.ASM | 2492 | frames without SI/DI saves, AX..BX saves in frames, REPT macro, fixup-free internal calls; 7 former C wrappers were artifacts | behaviour names for the 41 register-ABI routines; replace the remaining absolute DS displacements once the state block is named |
| asm/SPRITES.ASM (M_4AA8_4EEB) | 1211 | word-aligned pad before F_4AA8; odd-address members packed behind it; bytecode interpreter with LODS/jump table; BP repurposed; all calls symbolic | none |
| asm/SPRDRAW.ASM (M_6036_6181) | 502 | LOOP/LODS/XLAT bodies, mid-function `mov bp,sp`; 60A9/6181 odd-aligned behind 6036 | none |
| asm/DECODE.ASM (M_6D86_6F4B) | 573 | word-aligned pad + internal pad, RLE/LZ/4bpp decoders; 6EFF/6F4B odd-aligned behind 6DCC | none |
| asm/RECTQ.ASM, asm/BOARDCOL.ASM, asm/ANIMROW.ASM (F_1ECD, F_1F91, F_9EC3) | 325 | odd addresses between C units, TC-order prologues, XLAT/LODS bodies | undecidable between asm-body-in-C and byte-aligned TASM: keep, documented in tu-structure.md |
| asm/ICONANIM.ASM, asm/DRAWQ.ASM, asm/DRAWQBUF.ASM (M_D386_D3CF, M_D61C_D79C, M_D818_D825) | 662 | `push di; push si` hand order, word-aligned pads, stack-argument patching; all calls symbolic | none |
| RUNTIME_BLOCK | 6571 | EGA driver / library runtime; all 80 relative branches labelled | none in this phase |

## 3. TU structure

Proven this session (all EXACT, `tools/probe_tu.py`): BOARD.C, GAME.C, STARTUP.C,
KEYBOARD.C, TIMER.C, FONT.C.  Ambiguous (byte-neutral) extensions left separate:
INTRO.C behind STARTUP.C; KEYIRQ.C (cannot join KEYBOARD.C: `push cs` needs the
non-interrupt view of the handler); RECTTAB+OPLINIT+VOXSLOAD+VOXCHAN;
ANIMSTEP..SLOTROW (EXACT as one unit); F_1D47..F_1EC0.  The helpers
F_D3DA..F_D60C became HELPMENU.C, HINTDLG.C, PLRLDPUB.C, SNDREQ.C, RESCACHE.C.
Anchors that end units: see tu-structure.md.

## 4. Interfaces and data

- `docs/current/interface-audit.md` alternate views stand (a74a2 2-D vs flat is
  now also a TU boundary proof between BOARD.C and GAME.C).
- `display_mode` (char) vs former `bbfcd`/`mode` byte views unified in STARTUP.C;
  VIDEO.C still carries the `mode` unsigned-char view (documented).
- include/SOUND.H declares the sound state every consumer types identically;
  docs/current/sound-state.md maps every word with its mechanism.  Words that
  only asm/SOUND.ASM touches (DS:17C4..17DC pause-renderer cursors, 17E4, 1766,
  1814, 182C/1832 tables) stay ASM-internal equates with a `no data public`
  comment: they sit inside string-table DATA components and no C unit reads
  them, so no data public is added for them (supervisor decision, freeze pass).
- Open interface findings: 0 (docs/current/interface-audit.md, round
  2026-09-21 closure); remaining syntactic differences are documented
  CODEGEN_ALTERNATE_VIEWs.

## 5. Naming / readability

97 publics and 29 globals were behaviour-named on 2026-09-21 from their own
banners, callers and data (originals and retired aliases in
docs/current/symbol-names.json; research in docs/history/).  12 address names
remain, each with only a mechanical description and a recorded missing fact
(docs/history/naming-research-2026-09-21.md and the frontier research of the
freeze pass): f250c, f2986, f32fa (BOARD.C script opcode handlers), f7417
(HUD), f9402/f9440 (PUZZLE cached-text draws), f9962/f99a2 (SCORE frame draws,
no callers in C), fb09a, fb772 (LEVEL), f_c5a8/f_c5c6 (sound setters of
LOW-confidence state words).  They are not renamed on purpose: a speculative
name would be worse than the address.

## 6. Source layout

Done: recovery/src, recovery/asm hold the 126+19 retired references.  Left:
`asm/F_4EEB.ASM` belongs in asm/ (rename_source); recipes/modules C-candidate
recipes for proven-ASM modules moved next to their sources under recovery/.

## 7. Blockers / tooling

- None open.  `tools/probe_module.py` cannot bind two modules on its own
  (MUSIC's CC.LIB long-arithmetic externals, secondary runtime publics);
  acceptance covers them.

## 8. Freeze

Historical-source closure is complete: fresh acceptance on HEAD, exact SHA,
106 ordered relocations, tests green, 0 raw runtime bytes, 0 unexplained TU
flags, 0 topology mismatches, 0 capsules, 0 UNKNOWN provenance, 0 open
interface findings, every remaining ASM member evidenced, every inline-asm
fragment with compiler evidence, docs/layout in agreement, and
docs/current/portability-boundaries.md written.  The tree is tagged as the
exact historical oracle for the portable port; historical reconstruction work
stops here.
