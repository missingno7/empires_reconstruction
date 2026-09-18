# Eighth local C wave

Three complete routines compile to 424 exact bytes with the pinned Turbo C.

| Owner | Bytes | Reconstructed form |
|---|---:|---|
| F_2A70 | 114 | Walk length-prefixed blocks and return a 16-bit pointer conversion |
| F_32FA | 144 | Packed byte fields, signed division and a byte-update loop |
| F_9466 | 166 | Arithmetic branches, small switch and reversed call arguments |

F_2A70 explicitly converts the final pointer expression to int, preserving the
original return instructions. It does not claim a recovered far-pointer return
contract. F_32FA needs only the loop counter declared register; making both
locals register exchanges SI and DI. Turbo C still keeps the second local in
a register without that qualifier. The complete byte match preserves this
compiler detail. Mutation checks reject the double-register declaration and
a changed final pointer displacement.

All contributions are full C bodies without inline assembly, byte injection
or excluded bytes. The recipe `recipes/c/matching-wave8.json` records exact
function extents, declared references and upstream identity. Fresh comparison
results are in `matching-wave8-evidence.json`.

## Held evidence from this pass

F_2119 (144 bytes) and F_2986 (167 bytes) also match fresh code probes, but are
not promoted. Their DS:2380 and DS:B1CC/893C data bases lack exact corroborating
objects in the pinned profile. Those are linkage-evidence tasks, not failed
C reconstructions. Ignored drafts remain under build/c-wave8.

The recorded F_4943 extent ends in a loop with an exit outside its extent and
no epilogue included; a complete-function reconstruction needs boundary work.
F_6181 and F_6036 use direct segment and string operations with manually
managed stack/register sequences. No blanket claim about C impossibility is
made for those routines.

Coverage now totals 187 matching C functions / 18,947 bytes, with 59 bytes of
compiler-owned initialized data. Raw coverage is 51,377 bytes, including
30,160 classified unresolved machine bytes. The three-file build remains
byte-identical. The broader goal remains active.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave8.json
python -m unittest discover -s tests
python tools/reconstruct_game.py
```
