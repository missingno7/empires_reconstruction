# Forty-first matching-C wave

`F_200F` is now a canonical C reconstruction. Its complete 266-byte body
contains two signed-index loops (four and seven iterations) followed by the
fixed thirteen-call initializer tail. The source uses typed two-dimensional
far arrays so Turbo C emits the original `push ds` pointer construction; the
loop index remains a signed `int` to preserve the historical `JL` branches.

All sixteen external storage/code bindings are declared. A fresh pinned
Turbo C object matches the complete extent and all 30 OMF fixups, with no
initialized-data or relocation obligations hidden outside the claimed C
owner. The former raw region is split at the exact extent boundary.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave41.json
python tools/reconstruct_game.py
```

The full three-file rebuild remains byte-identical. Matching C coverage is
now 232 regions / 34,841 bytes and raw EXE coverage is 33,231 bytes.

Evidence: `docs/matching-wave41-evidence.json`.
