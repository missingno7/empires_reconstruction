# Forty-third matching-C wave

`F_2986` is now canonical C for its complete 167-byte far-pointer renderer
helper. The source preserves unsigned byte loads, the saved DS:00BC word,
the bounded scan from `p[4] + 5`, and the two raw `F_03CC` call shapes. Typed
far arrays at DS:B1CC and DS:893C reproduce the original segment/offset
arguments without embedding opaque machine bytes.

A fresh Turbo C object matches the full extent and all seven fixups. The raw
owner is split at the exact function boundary, while its numeric bindings to
the still-raw `F_03CC` entry remain explicit and independently checkable.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave43.json
python tools/reconstruct_game.py
```

The complete EXE and both DAT archives remain byte-identical. Matching C
coverage is now 234 regions / 35,152 bytes and raw EXE coverage is 32,920 bytes.

Evidence: `docs/matching-wave43-evidence.json`.
