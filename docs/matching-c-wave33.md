# Thirty-third local C wave: scripted event dispatcher

F_338A adds 870 matching C bytes. The routine dispatches the event stream,
selects one of four runtime animation buffers, updates the board table and
clears indexed move records. Its four far-buffer bindings are the exact
DS:9A5C, DS:99DA, DS:9B6E and DS:9AE0 addresses observed independently in the
existing board redraw routine F_2AE2 and in F_338A itself. The DS:B3AF indexed
record store is the byte immediately after the established DS:B3AE count base
used by F_B60F.

The historical compiler emits one word-negation sequence for the byte field;
the source keeps that instruction sequence as a small Turbo C inline-assembly
fragment while the dispatcher body remains C. The fresh OMF is checked over the
complete extent and binds all 24 fixups explicitly.

Coverage is now 225 matching C routines / 28,149 bytes. The full EXE and both
DAT archives remain byte-identical.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave33.json
python -m unittest tests.test_matching_c_wave33 -v
python tools/reconstruct_game.py
```
