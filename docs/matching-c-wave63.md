# Sixty-third matching-C wave

`F_CA9B` was recovered as canonical C for its complete 53-byte executable extent. The
source preserves the live AH/AL, BX and ES:DI inputs, binds calls to the owned
`F_CADB`, `F_CAE6` and `F_CAD0` entries, and binds the two DGROUP stores. The
fresh Turbo C object has five fixups and no loader relocations.

The canonical source is now readable symbolic TASM. The five source-level
references remain the same, while the recovered 17FCh literal table base is
explicitly retained pending identification of its data owner.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave63.json
python -m unittest tests.test_matching_c_wave63 -v
python tools/reconstruct_game.py
```

The machine-readable [fresh proof](matching-wave63-evidence.json),
[promotion recipe](../recipes/c/matching-wave63.json), and
[regression test](../tests/test_matching_c_wave63.py) retain the complete
extent and equality result.

Matching-C coverage is now 252 regions / 36,183 bytes. Raw executable fallback
is 48 regions / 31,660 bytes. The full EXE and both DAT archives remain
byte-identical.
