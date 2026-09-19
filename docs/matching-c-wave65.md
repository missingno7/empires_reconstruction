# Sixty-fifth matching-C wave

`F_53BF` is now canonical C for its complete 75-byte display-mode selection
and VGA probe. The source binds the existing `F_E54D` probe call and the two
DGROUP state locations; fixed branch bytes preserve the original live-register
control flow. The fresh Turbo C object has six fixups and no loader relocations.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave65.json
python -m unittest tests.test_matching_c_wave65 -v
python tools/reconstruct_game.py
```

The machine-readable [fresh proof](matching-wave65-evidence.json),
[promotion recipe](../recipes/c/matching-wave65.json), and
[regression test](../tests/test_matching_c_wave65.py) retain the complete
extent and equality result.

Matching-C coverage is now 254 regions / 36,264 bytes. Raw executable fallback
is 46 regions / 31,579 bytes. The full EXE and both DAT archives remain
byte-identical.
