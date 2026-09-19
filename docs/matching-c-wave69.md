# Sixty-ninth matching-C wave

`F_019C` is now canonical C for its complete 8-byte DOS handle-2 write
helper. The following 24-byte setup/data tail remains explicitly raw until
its own source boundary is recovered. The fresh Turbo C object has no fixups
or loader relocations, and the raw remainder is preserved byte-for-byte.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave69.json
python -m unittest tests.test_matching_c_wave69 -v
python tools/reconstruct_game.py
```

The machine-readable [fresh proof](matching-wave69-evidence.json),
[promotion recipe](../recipes/c/matching-wave69.json), and
[regression test](../tests/test_matching_c_wave69.py) retain the complete
subextent and equality result.

Matching-C coverage is now 258 regions / 36,439 bytes. Raw executable fallback
is 42 regions / 31,404 bytes. The full EXE and both DAT archives remain
byte-identical.
