# Seventy-first matching-C wave

`F_D386` is now canonical C for its complete 73-byte far-record decoder main
routine. The routine's `js` target is the immediately following 11-byte tail,
which remains an explicitly raw component until a second proof recovers its
source form. The fresh Turbo C object binds the far input table, `a72b2`, and
the numeric `F_03C9` call target, with no loader relocations.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave71.json
python -m unittest tests.test_matching_c_wave71 -v
python tools/reconstruct_game.py
```

The machine-readable [fresh proof](matching-wave71-evidence.json),
[promotion recipe](../recipes/c/matching-wave71.json), and
[regression test](../tests/test_matching_c_wave71.py) retain the complete
main extent and the raw-tail boundary.

Matching-C coverage is now 260 regions / 36,545 bytes. Raw executable fallback
is 41 regions / 31,298 bytes. The full EXE and both DAT archives remain
byte-identical.
