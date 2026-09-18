# Thirty-eighth local C wave: sprite load and cell animation

F_21A9 and F_233E add 512 matching C bytes. F_21A9 binds the two boot sprite
buffers at DS:9CF2 and DS:A6B6 before publishing their far pointers. F_233E
binds the DS:79BF cell table and rebuilds both animation loops; its helper calls
and indexed DGROUP references are resolved from complete fixup targets.

Coverage is now 232 matching C routines / 34,575 bytes. The full EXE and both
DAT archives remain byte-identical.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave38.json
python -m unittest tests.test_matching_c_wave38 -v
python tools/reconstruct_game.py
```
