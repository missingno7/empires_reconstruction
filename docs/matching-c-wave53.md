# Fifty-third matching-C wave

`F_0215` is now canonical C for the complete 29-byte indexed-store routine
at load offset `0215`. Its fresh Turbo C object reproduces the stack frame,
two indexed DGROUP stores, and return sequence without retaining opaque bytes.

The OMF contribution has two bound DGROUP fixups and no loader relocations.
The [promotion recipe](../recipes/c/matching-wave53.json),
[fresh proof](matching-wave53-evidence.json), and
[regression test](../tests/test_matching_c_wave53.py) retain the complete
extent and binding checks.

| Owner | Code bytes | Fixups | Load relocations |
|---|---:|---:|---:|
| F_0215 | 29 | 2 | 0 |

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave53.json
python -m unittest tests.test_matching_c_wave53 -v
python tools/reconstruct_game.py
```

Coverage is now 243 matching-C regions / 35,891 bytes and 52 raw regions /
31,952 bytes. The full EXE and both DAT archives remain byte-identical.
