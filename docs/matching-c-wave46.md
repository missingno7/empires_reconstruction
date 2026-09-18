# Forty-sixth matching-C wave

`F_B967` is now canonical C for its complete 56-byte render-loop wrapper. It
updates the far video pointer, dispatches the owned and raw helper entries, and
loops on the independently corroborated DS:B6CF state byte. F_B7F9 writes that
state byte while F_B967 reads it, closing the prior storage-evidence gap.

A fresh Turbo C object matches the complete extent and all 12 fixups. The raw
owner is split at the exact function boundary, with no opaque bytes inside the
claimed C contribution.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave46.json
python -m unittest tests.test_matching_c_wave46 -v
python tools/reconstruct_game.py
```

Matching C coverage is now 237 regions / 35,611 bytes and raw EXE coverage is
32,461 bytes. Evidence: `docs/matching-wave46-evidence.json`.
