# Forty-seventh matching-C wave

`F_7932` is now canonical C for its complete 50-byte pointer-dispatch wrapper.
It selects the caller-provided far pointer or the default DS pointer, records
the selected value, and dispatches the existing owned helpers. The pointer
storage and all three helper entries are bound from the complete fresh OMF
proof; the default pointer base is retained as an explicit numeric DGROUP
binding.

A fresh Turbo C object matches the complete extent and all 8 fixups. The raw
owner is split at the exact function boundary, with no opaque bytes inside the
claimed C contribution.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave47.json
python -m unittest tests.test_matching_c_wave47 -v
python tools/reconstruct_game.py
```

Matching C coverage is now 238 regions / 35,661 bytes and raw EXE coverage is
32,411 bytes. Evidence: `docs/matching-wave47-evidence.json`.
