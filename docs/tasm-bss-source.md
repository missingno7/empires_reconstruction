# Turbo Assembler BSS source

The exact structural link no longer supplies game BSS through `DGSCF.OBJ` or a
custom OMF writer. `tools/bss_asm.py` generates TASM source contributions from
the canonical 37,250-byte reserve and its 247 canonical public anchors. The
pinned TASM 1.0
assembles a word-aligned public `_BSS` segment in `DGROUP`; Turbo Link 2.0 then
derives the unchanged BSS base, runtime BSS placement, stack base and MZ fields.

`recipes/data/bss-contributions.json` is the deterministic contribution plan.
Its first contribution is `F01CEBSS.OBJ`, a 34-byte symbolic owner containing
`GAME_BSS`, `_cur_idx`, `_g3902`, and `_g3904`. `ROWPTRS.OBJ` owns the 1,952-byte
row-pointer table initialized by `F_0281`; `RSTATEB.OBJ` owns the 16-byte shared
command/render-state island at bytes 1,986–2,001; and `C470BSS.OBJ` owns the
ten recovered 27-byte records. `SLOTBSS.OBJ`, `ANIMBSS.OBJ`, `GC360BSS.OBJ`,
`FLAGSBS.OBJ`, `OCTAVES.OBJ`, `NOTEIDX.OBJ`, and `VOICEPTR.OBJ` add other
bounded state, including the 44-byte eleven-entry far-pointer table initialized
by `F_DEFA`. Seven `GAMEBSS` fragments hold the remaining 34,398-byte anchored
reserve. The plan must cover the full logical reserve, and
its rebased labels must equal the canonical map before the link begins. None of the objects contributes
load-image payload. The
staged source timestamp is fixed because TASM records it in an OMF comment; the
full historical-linker output remains byte-identical.

This removes the synthetic DGROUP/BSS object from the exact path. It does not
prove the historical translation-unit boundary of either object. The canonical
anchor map and ordered contributions are checked against linker-binding
evidence on every structural link, but 34,398 bytes remain aggregate storage.
Those are the next ownership constraints rather than reasons to retain a
synthetic object.
