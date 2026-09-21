# Historical-source closure frontier

Authoritative list of what still stands between the exact tree and "complete game
logic in readable C + only irreducible historical ASM".  Maintained by the
supervisor; every row names the next experiment.  Baseline is always the exact
build (SHA 1259348425483d8d97fd8821860b47cfdf58fc8029711eb0ed0e78ab33807a10,
106 relocations, `python -m unittest discover -s tests -p test_build_exe.py`).

Census (`docs/current/source-quality.json`, per member section):
C with inline asm 2441 bytes / 17 members; symbolic ASM 5758 bytes / 61
members; runtime block 6571; C 43740 bytes / 260 members.  Unexplained compiler
flags (`tools/audit_tu_flags.py`): 0.

## 1. PURE_C opportunities (C with inline asm that may still shrink)

| Member | Unit | Bytes | asm lines | Why it remains | Last probe | Next experiment | Leverage |
|---|---|---|---|---|---|---|---|
| F_338A board_run_unit_script | BOARD.C | 870 | 5 | `mov al,es:[bx+3]; neg ax; ...; mov es:[bx+3],al`: 28 standalone spellings + 8 in-context probes show TC 2.0 folds every byte-lvalue negation to `neg al`, and every route to a word-width `neg ax` (int temp, static, register) materialises `mov ah,0` plus a spill/reload; the missing extension relies on AH still being 0 from the compiler's own `and ax,0Fh` three statements earlier, which only a human knew | 2026-09-21 (build/probes/neg) | none: irreducible hand asm inside a C function, with concrete compiler evidence | closed |
| F_50D2 / F_53BF video/sound-hardware probes | STARTUP.C | 312 | 35 | recovered as C with pseudo-registers this session; fragments left: `mov display_mode,N` stores (NOP-padded forward EXTRN), ES:SI ROM probes, signed `cmp bl/jl`, `xchg`, `loop`, flag tests after INT | 2026-09-21 EXACT | `cmp word ptr display_mode,N` -> `*(int *)&display_mode == N`; `_BL` sign test via `(signed char)` temp; keep the store NOPs (they are the TCC-generated-ASM signature) | medium |
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
| F_4AA8, F_4B0C, F_4E9F, F_4EEB | 1211 | word-aligned pad before F_4AA8; bytecode interpreter with LODS/jump table; BP repurposed | one module hypothesis `probe_tu F_4AA8 F_4EEB --asm` (byte-neutral; alignment says 4AA8 starts a word-aligned TASM module and 4E9F/4EEB at odd addresses are packed behind it); F_4B0C's CALL_REL macro -> symbolic externs |
| F_6036, F_60A9, F_6181 | 502 | LOOP/LODS/XLAT bodies, mid-function `mov bp,sp` | same one-module hypothesis (60A9/6181 odd-aligned behind 6036) |
| M_6D86_6DCC, F_6EFF, F_6F4B | 573 | word-aligned pad + internal pad (two TASM modules), blitters | merge F_6EFF/F_6F4B behind F_6DCC (odd addresses) |
| F_1ECD, F_1F91, F_9EC3 | 325 | odd addresses between C units, TC-order prologues, XLAT/LODS bodies | undecidable between asm-body-in-C and byte-aligned TASM: keep, documented in tu-structure.md |
| M_D386_D3CF, M_D61C_D79C, M_D818_D825 | 662 | `push di; push si` hand order, word-aligned pads, stack-argument patching | M_D61C_D79C's six `call $+...` renderer targets -> symbolic externs (public index) |
| RUNTIME_BLOCK | 6571 | EGA driver / library runtime | none in this phase |

## 3. TU structure

Proven this session (all EXACT, `tools/probe_tu.py`): BOARD.C, GAME.C, STARTUP.C,
KEYBOARD.C, TIMER.C, FONT.C.  Ambiguous (byte-neutral) extensions left separate:
INTRO.C behind STARTUP.C; KEYIRQ.C (cannot join KEYBOARD.C: `push cs` needs the
non-interrupt view of the handler); RECTTAB+OPLINIT+VOXSLOAD+VOXCHAN; the 13
helpers F_D3DA..F_D60C (EXACT as one unit; group after naming); ANIMSTEP..SLOTROW
(EXACT as one unit).  Anchors that end units: see tu-structure.md.

## 4. Interfaces and data

- `docs/current/interface-audit.md` alternate views stand (a74a2 2-D vs flat is
  now also a TU boundary proof between BOARD.C and GAME.C).
- `display_mode` (char) vs former `bbfcd`/`mode` byte views unified in STARTUP.C;
  VIDEO.C still carries the `mode` unsigned-char view (documented).
- SOUND.ASM: remaining absolute DS displacements listed in asm/SOUND.ASM header.

## 5. Naming / readability

89 publics were behaviour-named on 2026-09-21 from their own banners, callers and
data (evidence table: the naming research in the commit message; originals in
docs/current/symbol-names.json).  15 address names remain, all with only a
mechanical description: f250c, f2986, f32fa (BOARD.C script opcode handlers),
f568c (INTRO), f7417 (HUD), f9402/f9440 (PUZZLE messages), f9962/f99a2 (SCORE
frame draws), face7/fb09a/fb4fb/fb772 (SLOTMENU/LEVEL), f_c5a8/f_c5c6 (sound
state setters for DS:17A4/17DC).  Next: name the sound state block fields
(DS:1760..17F4, 1E84..1E94) from SOUND.ASM's per-proc comments, which unlocks
the last two and the remaining absolute displacements; the globals table in the
research (g9ade, gc360, g8bfe, g736/g738/g73a, g237c, gb68..gb70, gb3ae, gbfc8,
gbfc4, g9bfc/gbf66, gc5cc) is the next batch.

## 6. Source layout

Done: recovery/src, recovery/asm hold the 126+19 retired references.  Left:
`asm/F_4EEB.ASM` belongs in asm/ (rename_source); recipes/modules C-candidate
recipes for proven-ASM modules moved next to their sources under recovery/.

## 7. Blockers / tooling

- BOARDSCR `neg ax` (row 1): closed as irreducible (see row).
- `tools/interface_census.py` still globs src/*.C only (recovery/ excluded on purpose).
