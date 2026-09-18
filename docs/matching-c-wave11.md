# Eleventh local C wave and recovered function boundary

| Owner | Code bytes | Reconstructed form |
|---|---:|---|
| F_4943 | 160 | Nested loops, packed-record updates and complete epilogue |
| F_A43C | 233 | Conditional toggle, timed loop and ordered calls |
| F_D26C | 216 | Setup calls, indexed selection and return |

These complete routines add 609 matching C bytes. TEXT_22D2 adds 14 bytes of
independently encoded terminated ASCII text used by F_D26C. All references
are corroborated or derived from component ownership. Twenty-three additional
external references now resolve through owned component entries/data.

The upstream F_4943 extent had 157 bytes and stopped at CS:49E0. Its loop's
exit targets CS:49E0, where the next three instructions restore SI, restore BP
and return. Existing owner F_49E3 begins immediately afterward. The corrected
160-byte C contribution emits the complete prologue, loops and epilogue.
Recipe provenance records both lengths and the boundary evidence. A negative
check rejects the old truncated extent rather than accepting a partial match.
The raw machine classification decreases by 606 bytes, while total raw code
falls by 609: the recovered epilogue was outside the pinned machine extent.

F_A43C retains assignment expressions for its toggle and color update and
preincrement in its threshold comparison. A split-increment mutant fails the
complete comparison. Another control alters F_D26C's selection bound. No
inline assembly or encoded machine bytes are used.

Coverage now totals 193 matching C functions / 20,114 bytes. Static source
components total 100 bytes (59 compiler-generated and 41 independent text).
Raw coverage is 50,169 bytes, including 28,996 classified unresolved machine
bytes. All three game files remain exact; the overall goal remains active.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave11.json
python -m unittest discover -s tests
python tools/reconstruct_game.py
```

Recipe: recipes/c/matching-wave11.json. Fresh metadata-only proof:
matching-wave11-evidence.json.
