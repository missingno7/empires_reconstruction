# Fourteenth local C wave

F_CE9E (158 bytes) and F_CF3C (333 bytes) now rebuild from C with the pinned
Turbo C toolchain. Their complete emitted code, declared OMF fixups and MZ
relocation obligations match the original extents. Existing profile declarations
corroborate all external storage references. The call from F_CF3C to F_CE9E
resolves through its newly owned public rather than a numeric code address.

The arithmetic sequence `n<<=2;n+=i;n+=3;` preserves the original operations
in DI. The equivalent expression `n=n*4+i+3;` emits an AX temporary and four
extra bytes. A fresh compiler negative control rejects that rewrite; another
rejects changing F_CE9E's loop bound from four to three. Neither function
requires a raw byte fragment or a new initialized-data contribution.

Coverage is 198 matching C functions / 21,082 bytes. Raw EXE coverage is
49,201 bytes across 66 regions, including 28,028 classified unresolved machine
bytes. All three game files remain byte-identical. These are fixed-placement
component proofs, not recovered historical modules or real linking. The
matching-C frontier remains open.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave14.json
python -m unittest discover -s tests
python tools/reconstruct_game.py
```

Evidence: matching-wave14-evidence.json.
Recipe: recipes/c/matching-wave14.json.
