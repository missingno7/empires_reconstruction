# Fifty-fifth matching-C wave

`F_D471`, `F_D487`, and `F_D49D` are now canonical C for the three complete
far-pointer wrappers immediately after `F_D45C`. Each fresh Turbo C object
reproduces its table pointer, argument constants, owned `F_D3DA` call, stack
cleanup, and return path.

Each 22-byte OMF contribution has two bound fixups and no loader relocations.
The [promotion recipe](../recipes/c/matching-wave55.json),
[fresh proof](matching-wave55-evidence.json), and
[regression test](../tests/test_matching_c_wave55.py) retain all complete
extents and binding checks.

| Owners | Code bytes | Fixups | Load relocations |
|---|---:|---:|---:|
| F_D471, F_D487, F_D49D | 66 | 6 | 0 |

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave55.json
python -m unittest tests.test_matching_c_wave55 -v
python tools/reconstruct_game.py
```

Coverage is now 247 matching-C regions / 35,978 bytes and 51 raw regions /
31,865 bytes. The full EXE and both DAT archives remain byte-identical.
