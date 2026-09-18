# Fifty-first matching-C wave

`F_CDDD` is now canonical C for a recovered complete 35-byte extent. The
upstream inventory ended at the `_longjmp` call; the following eight bytes are
the complete zero-return cleanup before the next routine. The source compiles
to the original far-pointer call, conditional `_longjmp`, and return path.

The fresh OMF contribution has four fixups and no loader relocations. The
`F_86C9` reference is promoted to its owned entry public, while `_longjmp`
resolves through the pinned `LIB_SETJMP` module. No opaque bytes are included
inside the C owner.

| Owner | Code bytes | Fixups | Load relocations |
|---|---:|---:|---:|
| F_CDDD | 35 | 4 | 0 |

The [promotion recipe](../recipes/c/matching-wave51.json),
[fresh proof](matching-wave51-evidence.json), and
[regression test](../tests/test_matching_c_wave51.py) retain the complete
extent and binding checks.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave51.json
python -m unittest tests.test_matching_c_wave51 -v
python tools/reconstruct_game.py
```

Coverage is now 241 matching-C regions / 35,820 bytes and 52 raw regions /
32,023 bytes. The full EXE and both DAT archives remain byte-identical.
