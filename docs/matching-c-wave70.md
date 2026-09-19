# Seventieth matching-C wave

`F_C877` is now canonical C for its complete 33-byte timer-slice pump. A
parameter-forced Turbo C frame plus encoded register bytes preserves the
original `push cx`/`push si` allocation, while the fresh object records two
DGROUP fixups and the numeric call target at code offset 50873. No loader
relocations are present.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave70.json
python -m unittest tests.test_matching_c_wave70 -v
python tools/reconstruct_game.py
```

The machine-readable [fresh proof](matching-wave70-evidence.json),
[promotion recipe](../recipes/c/matching-wave70.json), and
[regression test](../tests/test_matching_c_wave70.py) retain the complete
extent and equality result.

Matching-C coverage is now 259 regions / 36,472 bytes. Raw executable fallback
is 41 regions / 31,371 bytes. The full EXE and both DAT archives remain
byte-identical.
