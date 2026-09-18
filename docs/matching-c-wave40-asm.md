# ASM linkage closure wave

F_4E9F now has a canonical hand-written TASM source at `asm/F_4E9F.ASM`.
The 76-byte extent walks the established 20-byte record table at DS:B3AE,
filters against DS:BFBA, looks up the two byte tables at DS:BF66 and DS:9BFC,
and calls the already owned F_4AA8 entry.

The recipe declares all five external bindings, including the two symbols that
the historical provider-held draft had left undecided. A fresh TASM object was
bound over the complete extent and matched every byte and fixup. Promotion
split the former raw owner without changing any neighboring placement.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave40-asm.json
python tools/reconstruct_game.py
python tools/inventory_linkage.py
```

The resulting build remains byte-identical (`AEPROG.EXE`, `AE000.DAT` and
`AE001.DAT`), and the linkage inventory reports zero held candidates and zero
unresolved symbols. Fresh-source coverage is now 231 C regions (34,575 bytes)
and 21 ASM regions (2,532 bytes).

Evidence: `docs/matching-wave40-asm-evidence.json`.
