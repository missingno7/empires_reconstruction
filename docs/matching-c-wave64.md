# Sixty-fourth matching-C wave

`F_034F` is now canonical C for its complete 6-byte BIOS mode-switch routine.
Turbo C emits the exact `mov ax,3` / `int 10h` / `ret` sequence with no fixups
or loader relocations.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave64.json
python -m unittest tests.test_matching_c_wave64 -v
python tools/reconstruct_game.py
```

The machine-readable [fresh proof](matching-wave64-evidence.json),
[promotion recipe](../recipes/c/matching-wave64.json), and
[regression test](../tests/test_matching_c_wave64.py) retain the complete
extent and equality result.

Matching-C coverage is now 253 regions / 36,189 bytes. Raw executable fallback
is 47 regions / 31,654 bytes. The full EXE and both DAT archives remain
byte-identical.
