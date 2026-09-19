# Sixty-first matching-C wave

`F_C232` is now canonical C for its complete 75-byte executable extent. The
routine scans the ES:DI table selected by the caller, updates the adjacent
DGROUP records, and retains the historical Turbo C frame and loop bytes through
inline assembler. All twelve DGROUP references bind to declared storage
coordinates; the fresh OMF object has no loader relocations.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave61.json
python -m unittest tests.test_matching_c_wave61 -v
python tools/reconstruct_game.py
```

The machine-readable [fresh proof](matching-wave61-evidence.json),
[promotion recipe](../recipes/c/matching-wave61.json), and
[regression test](../tests/test_matching_c_wave61.py) retain the complete
extent, source hash, fixups, and equality result.

Matching-C coverage is now 250 regions / 36,120 bytes. Raw executable fallback
is 50 regions / 31,723 bytes. The full EXE and both DAT archives remain
byte-identical.
