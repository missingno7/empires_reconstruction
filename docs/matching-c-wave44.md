# Forty-fourth matching-C wave

`F_4517` is now canonical C for its complete 279-byte far-buffer loader. The
source preserves the five 0x319-byte and six 0xBB-byte transfers, the staged
0xAD2-byte block, the F_68AA record load, and the far-pointer table expansion.
The source uses the independently bound DS:79BF, DS:74A2, DS:8C12, DS:99D6 and
DS:72B2 storage objects; no original code bytes are embedded.

A fresh Turbo C object matches the full extent and all 24 fixups. The raw owner
is split at the exact function boundary and the complete EXE remains equal.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave44.json
python -m unittest tests.test_matching_c_wave44 -v
python tools/reconstruct_game.py
```

Matching C coverage is now 235 regions / 35,431 bytes and raw EXE coverage is
32,641 bytes. Evidence: `docs/matching-wave44-evidence.json`.
