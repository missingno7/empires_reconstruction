# Fiftieth matching-C wave

`F_CE2A` is now canonical C for a recovered complete 62-byte extent. The
upstream inventory stopped at the final call, but the following 12 bytes are a
single cleanup/return tail before the next owned function. The recovered source
matches the full body after binding seven code calls, the pinned `_longjmp`
library public and four DGROUP references.

The fresh OMF contribution has 11 fixups and no loader relocations. Its source
uses the reversed chained assignment that Turbo C needs to emit the original
`xor ax,ax` plus the two stores in their original order. No raw bytes are
included in the claimed C owner.

| Owner | Code bytes | Fixups | Load relocations |
|---|---:|---:|---:|
| F_CE2A | 62 | 11 | 0 |

The [promotion recipe](../recipes/c/matching-wave50.json),
[fresh proof](matching-wave50-evidence.json), and
[regression test](../tests/test_matching_c_wave50.py) retain the complete
extent and binding checks.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave50.json
python -m unittest tests.test_matching_c_wave50 -v
python tools/reconstruct_game.py
```

Coverage is now 240 matching-C regions / 35,785 bytes and 52 raw regions /
32,058 bytes. The full EXE and both DAT archives remain byte-identical.
