# Thirty-fourth local C wave: board redraw

F_2AE2 adds 1,762 matching C bytes. The complete redraw routine now binds its
full set of direct DGROUP references, including the four animation buffers
already corroborated by F_338A and the board, cursor and sprite tables. The
historical `-B` compiler flag is preserved from the recovered source; the
fresh object matches the entire extent and all 111 fixups are explicitly
resolved.

Coverage is now 227 matching C routines / 29,911 bytes. The full EXE and both
DAT archives remain byte-identical.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave34.json
python -m unittest tests.test_matching_c_wave34 -v
python tools/reconstruct_game.py
```
