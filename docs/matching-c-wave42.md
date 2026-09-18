# Forty-second matching-C wave

`F_2119` now owns its complete 144-byte far-buffer copy loop as matching C.
The routine loads record `0x26` through the owned `F_656C` public, then performs
eight iterations of three `memmove` calls with the original `0x82/0x62/0x92`
sizes and `0x84/0x64/0x94` source strides. The destination begins at DS:2380
and the source is the established DS:C5C6 far pointer plus two bytes.

The source deliberately calls the pinned `CC.LIB` `_memmove` public at module
offset 84; this is the target represented by the original call displacement,
distinct from the module's `_movmem` entry at offset zero. A fresh Turbo C
object matches all 144 bytes and six complete fixups.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave42.json
python tools/reconstruct_game.py
```

The complete EXE and both DAT archives remain byte-identical. Matching C
coverage is now 233 regions / 34,985 bytes and raw EXE coverage is 33,087 bytes.

Evidence: `docs/matching-wave42-evidence.json`.
