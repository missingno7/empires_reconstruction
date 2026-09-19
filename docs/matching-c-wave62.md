# Sixty-second matching-C wave

`F_CA91` is now canonical C for its complete 10-byte executable extent. The
routine widens the caller's live AL into the DGROUP word at DS:1E92; the source
keeps the exact historical frame and return bytes through Turbo C inline
assembler. Its fresh object has one DGROUP fixup and no loader relocations.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave62.json
python -m unittest tests.test_matching_c_wave62 -v
python tools/reconstruct_game.py
```

The machine-readable [fresh proof](matching-wave62-evidence.json),
[promotion recipe](../recipes/c/matching-wave62.json), and
[regression test](../tests/test_matching_c_wave62.py) retain the complete
extent and equality result.

Matching-C coverage is now 251 regions / 36,130 bytes. Raw executable fallback
is 49 regions / 31,713 bytes. The full EXE and both DAT archives remain
byte-identical.
