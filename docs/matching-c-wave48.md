# Forty-eighth matching-C wave

`F_9D8E` is now canonical C for its complete 62-byte entry. Its source also
emits a 20-byte initialized `_DATA` record containing the far pointer to the
nearby 209-byte text component. The fresh OMF data fixup is bound to
`TEXT_118C`, and its segment word produces the original MZ relocation at load
offset 68758.

This promotes the complete code, initialized data and text source together:
there is no opaque byte inside any claimed contribution. The record boundary is
the identified DS:125D object immediately preceding the existing DS:1271 table.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave48.json
python -m unittest tests.test_matching_c_wave48 -v
python tools/reconstruct_game.py
```

Matching C coverage is now 239 regions / 35,723 bytes. The wave also removes
229 raw bytes: 209 structured text bytes and 20 compiled data bytes. Evidence:
`docs/matching-wave48-evidence.json`.

## Additional probe audit

Two newly tested raw extents remain outside canonical ownership. `F_7202` has
a complete 104-byte C-shaped probe, but the pinned compiler emits a direct
`push [gc0f0]` while the original loads `CX` before evaluating the far-pointer
argument; making that value non-direct changes register allocation or extent
length. `F_6CA6` uses `LES`, `ES` extraction and several direct segment-state
stores; C probes spill the far pointer and exceed its complete 68-byte extent.
Neither probe is promoted, and neither receives opaque fallback bytes.
