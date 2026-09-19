# Seventy-second matching-ASM wave

`F_D3CF` is now canonical ASM for the complete 11-byte branch continuation
targeted by the recovered F_D386 C main routine. The hand-written TASM source
has no fixups or loader relocations and removes the final opaque tail from this
component.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave72.json
python -m unittest tests.test_matching_c_wave72_asm -v
python tools/reconstruct_game.py
```

The machine-readable [fresh proof](matching-wave72-evidence.json),
[promotion recipe](../recipes/c/matching-wave72.json), and
[regression test](../tests/test_matching_c_wave72_asm.py) retain the complete
extent and equality result.

Matching-C coverage remains 260 regions / 36,545 bytes. Matching-ASM coverage
is now 22 regions / 2,543 bytes. Raw executable fallback is 40 regions /
31,287 bytes. The full EXE and both DAT archives remain byte-identical.
