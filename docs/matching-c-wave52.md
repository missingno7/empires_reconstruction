# Fifty-second matching-C wave

`F_CE00` is now canonical C for the complete 42-byte routine between
`F_CDDD` and `F_CE2A`. Its fresh Turbo C object reproduces the far-pointer
probe, register-held comparison, conditional `F_A13F` call, pinned
`_longjmp`, and zero-return epilogue.

The OMF contribution has five bound fixups and no loader relocations. Calls to
`F_86C9` and `F_A13F` resolve through their owned entry publics, while
`_longjmp` resolves through the pinned `LIB_SETJMP` module.

| Owner | Code bytes | Fixups | Load relocations |
|---|---:|---:|---:|
| F_CE00 | 42 | 5 | 0 |

The [promotion recipe](../recipes/c/matching-wave52.json),
[fresh proof](matching-wave52-evidence.json), and
[regression test](../tests/test_matching_c_wave52.py) retain the complete
extent and binding checks.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave52.json
python -m unittest tests.test_matching_c_wave52 -v
python tools/reconstruct_game.py
```

Coverage is now 242 matching-C regions / 35,862 bytes and 52 raw regions /
31,981 bytes. The full EXE and both DAT archives remain byte-identical.
