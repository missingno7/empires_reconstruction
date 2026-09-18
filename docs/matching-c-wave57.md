# Fifty-seventh matching-C wave

`F_E114` is now canonical C for the complete 61-byte far-pointer staging
wrapper between `F_E0C0` and `F_E151`. Its fresh Turbo C object reproduces the
28-byte stack frame, sign-extended word copies from the far source, owned
`F_E0C0` call, and return sequence.

The OMF contribution has one bound code fixup and no loader relocations. The
[promotion recipe](../recipes/c/matching-wave57.json),
[fresh proof](matching-wave57-evidence.json), and
[regression test](../tests/test_matching_c_wave57.py) retain the complete
extent and binding checks.

| Owner | Code bytes | Fixups | Load relocations |
|---|---:|---:|---:|
| F_E114 | 61 | 1 | 0 |

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave57.json
python -m unittest tests.test_matching_c_wave57 -v
python tools/reconstruct_game.py
```

Coverage is now 249 matching-C regions / 36,045 bytes and 51 raw regions /
31,798 bytes. The full EXE and both DAT archives remain byte-identical.
