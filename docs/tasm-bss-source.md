# Turbo Assembler BSS source

The exact structural link no longer supplies game BSS through `DGSCF.OBJ` or a
custom OMF writer. `tools/bss_asm.py` generates TASM source contributions from
the canonical 37,250-byte reserve and its 247 canonical public anchors. The
pinned TASM 1.0
assembles a word-aligned public `_BSS` segment in `DGROUP`; Turbo Link 2.0 then
derives the unchanged BSS base, runtime BSS placement, stack base and MZ fields.

`recipes/data/bss-contributions.json` is the deterministic contribution plan.
Its first contribution is `F01CEBSS.OBJ`, a 34-byte symbolic owner containing
`GAME_BSS`, `_cur_idx`, `_g3902`, and `_g3904`. `GAMEBSS.OBJ` is the rebased
37,216-byte anchored remainder with the other 243 labels. The plan must cover
the full logical reserve, and its rebased labels must equal the canonical map
before the link begins. Neither object contributes load-image payload. The
staged source timestamp is fixed because TASM records it in an OMF comment; the
full historical-linker output remains byte-identical.

This removes the synthetic DGROUP/BSS object from the exact path. It does not
prove the historical translation-unit boundary of either object. The canonical
anchor map and ordered contributions are checked against linker-binding
evidence on every structural link, but 37,216 bytes remain aggregate storage.
Those are the next ownership constraints rather than reasons to retain a
synthetic object.
