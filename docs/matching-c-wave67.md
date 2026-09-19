# Sixty-seventh source-recovery wave

`F_C8D4` is canonical symbolic TASM for its complete 14-byte live-AL
port-write wrapper. The exact object binds the configured DGROUP port word and
has no loader relocations. Its initially recovered C inline-assembly capsule
was replaced with named instructions while preserving the complete extent.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave67.json
python -m unittest tests.test_matching_c_wave67 -v
python tools/reconstruct_game.py
```

The machine-readable [fresh proof](matching-wave67-evidence.json),
[promotion recipe](../recipes/c/matching-wave67.json), and
[regression test](../tests/test_matching_c_wave67.py) retain the complete
extent and equality result.

The historical figures above are retained as promotion provenance. The current
source-quality report is the canonical view of readability progress.
