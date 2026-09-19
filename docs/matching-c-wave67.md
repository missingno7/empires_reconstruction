# Sixty-seventh matching-C wave

`F_C8D4` is now canonical C for its complete 14-byte live-AL port-write
wrapper. The exact Turbo C object binds the configured DGROUP port word and
has no loader relocations.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave67.json
python -m unittest tests.test_matching_c_wave67 -v
python tools/reconstruct_game.py
```

The machine-readable [fresh proof](matching-wave67-evidence.json),
[promotion recipe](../recipes/c/matching-wave67.json), and
[regression test](../tests/test_matching_c_wave67.py) retain the complete
extent and equality result.

Matching-C coverage is now 256 regions / 36,363 bytes. Raw executable fallback
is 43 regions / 31,480 bytes. The full EXE and both DAT archives remain
byte-identical.
