# Sixty-sixth matching-C wave

`F_6B7A` and `F_6BAC` are now canonical C for the adjacent timer-vector
routines: 50 bytes to install the timer and program the PIT, followed by 35
bytes to restore the vector and reset the PIT. Each fresh Turbo C object binds
the two recovered DGROUP words and has no loader relocations.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave66.json
python -m unittest tests.test_matching_c_wave66 -v
python tools/reconstruct_game.py
```

The machine-readable [fresh proof](matching-wave66-evidence.json),
[promotion recipe](../recipes/c/matching-wave66.json), and
[regression test](../tests/test_matching_c_wave66.py) retain both complete
extents and equality results.

Matching-C coverage is now 255 regions / 36,349 bytes. Raw executable fallback
is 44 regions / 31,494 bytes. The full EXE and both DAT archives remain
byte-identical.
