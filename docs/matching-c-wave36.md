# Thirty-sixth local C wave: selection and setup routine

F_A85E adds 449 matching C bytes. The complete routine now rebuilds with its
pooled strings represented as external DS data at 1389h, 1375h and 12D0h,
matching the original linked data pool. Its remaining state and helper
references are bound from complete fixups, so the source emits no fabricated
local `_DATA` segment.

Coverage is now 229 matching C routines / 31,341 bytes. The full EXE and both
DAT archives remain byte-identical.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave36.json
python -m unittest tests.test_matching_c_wave36 -v
python tools/reconstruct_game.py
```
