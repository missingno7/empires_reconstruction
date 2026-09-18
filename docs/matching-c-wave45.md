# Forty-fifth matching-C wave

`F_B593` is now canonical C for its complete 124-byte far-pointer cleanup
routine. It frees the nine pointer globals used by the adjacent F_B122 loader
and clears the DS:0720 state word. F_B122 independently reads and populates the
same pointer-storage addresses, closing the previous storage-corroboration gap.

A fresh Turbo C object matches the full extent and all 28 fixups. The raw owner
is split at the exact function boundary; the source contains no copied machine
bytes.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave45.json
python -m unittest tests.test_matching_c_wave45 -v
python tools/reconstruct_game.py
```

Matching C coverage is now 236 regions / 35,555 bytes and raw EXE coverage is
32,517 bytes. Evidence: `docs/matching-wave45-evidence.json`.
