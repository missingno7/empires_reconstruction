# Fifty-sixth matching-C wave

`F_6CF0` is now canonical C for the complete six-byte DGROUP-return routine
at load offset `6CF0`. Its fresh Turbo C object reproduces the global load,
short-jump epilogue, and return exactly.

The OMF contribution has one bound DGROUP fixup and no loader relocations. The
[promotion recipe](../recipes/c/matching-wave56.json),
[fresh proof](matching-wave56-evidence.json), and
[regression test](../tests/test_matching_c_wave56.py) retain the complete
extent and binding checks.

| Owner | Code bytes | Fixups | Load relocations |
|---|---:|---:|---:|
| F_6CF0 | 6 | 1 | 0 |

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave56.json
python -m unittest tests.test_matching_c_wave56 -v
python tools/reconstruct_game.py
```

Coverage is now 248 matching-C regions / 35,984 bytes and 51 raw regions /
31,859 bytes. The full EXE and both DAT archives remain byte-identical.
