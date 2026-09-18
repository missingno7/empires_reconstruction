# Thirty-fifth local C wave: beam walk

F_5AC3 adds 981 matching C bytes. The ray traversal, collision checks and
dirty-rectangle rebuild now compile from the recovered C source. Its complete
DGROUP references are bound to the exact board, object, sprite and beam state
offsets; the compiler-generated `SCOPY@` helper resolves through the pinned
library public.

Coverage is now 227 matching C routines / 30,892 bytes. The full EXE and both
DAT archives remain byte-identical.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave35.json
python -m unittest tests.test_matching_c_wave35 -v
python tools/reconstruct_game.py
```
