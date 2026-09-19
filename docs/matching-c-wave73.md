# Seventy-third matching-ASM wave

`F_01A4` is now canonical ASM for the complete 22-byte DOS write setup after
F_019C. Its TASM source binds the recovered F_019C helper and the numeric
F_0104 target; the final two-byte self-referential data word remains an
explicit raw component until a data encoder is justified. The object has two
code fixups and no loader relocations.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave73.json
python -m unittest tests.test_matching_c_wave73_asm -v
python tools/reconstruct_game.py
```

The machine-readable [fresh proof](matching-wave73-evidence.json),
[promotion recipe](../recipes/c/matching-wave73.json), and
[regression test](../tests/test_matching_c_wave73_asm.py) retain the complete
setup extent and equality result.

Matching-C coverage remains 260 regions / 36,545 bytes. Matching-ASM coverage
is now 23 regions / 2,565 bytes. Raw executable fallback is 40 regions /
31,265 bytes. The full EXE and both DAT archives remain byte-identical.
