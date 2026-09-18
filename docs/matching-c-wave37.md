# Thirty-seventh local C wave: turn loop

F_3A75 adds 2,722 matching C bytes. The complete turn loop now rebuilds with
the original `-B` compiler flag. Its 81 external fixups cover the board state,
cursor and sprite tables plus the recovered neighboring C entry points; each
DGROUP binding accounts for the compiler-emitted object addend before matching
the linked address.

Coverage is now 230 matching C routines / 34,063 bytes. The full EXE and both
DAT archives remain byte-identical.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave37.json
python -m unittest tests.test_matching_c_wave37 -v
python tools/reconstruct_game.py
```
