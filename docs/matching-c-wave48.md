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

`F_7202` is already canonical C from wave six. The current raw audit instead
tested `F_D818`, `F_C877` and `F_D825`; their fresh probe objects are recorded in
the [wave-forty-nine audit](matching-c-wave49.md). `F_6CA6` remains the earlier
segment-manipulation blocker. None of these probes changes ownership merely by
being C-shaped.
