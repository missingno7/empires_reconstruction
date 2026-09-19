# Sixty-eighth matching-C wave

`F_6CA6` is now canonical C for its complete 68-byte far-resource-table span
setup. A parameter-forced Turbo C frame preserves the original calling shape;
the recovered `unsigned char far *` table declaration and symbolic inline
assembly now let Turbo C emit the SI save/restore and all eight DGROUP fixups
directly. No loader relocations are present.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave68.json
python -m unittest tests.test_matching_c_wave68 -v
python tools/reconstruct_game.py
```

The machine-readable [fresh proof](matching-wave68-evidence.json),
[promotion recipe](../recipes/c/matching-wave68.json), and
[regression test](../tests/test_matching_c_wave68.py) retain the complete
extent and equality result.

Matching-C coverage is now 257 regions / 36,431 bytes. Raw executable fallback
is 42 regions / 31,412 bytes. The full EXE and both DAT archives remain
byte-identical.
