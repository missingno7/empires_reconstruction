# Twenty-fourth local C wave: traversal and pointer offsets

F_969D (431 bytes) and F_90A6 (435 bytes) reproduce their complete function
extents from freshly compiled C. Full OMF binding and byte comparison cover
all emitted code, fixups and relocation obligations. All referenced bases
have exact declarations or previously established bindings; nine code bindings
resolve through existing component publics. No new data ownership is claimed.

F_969D uses two-byte records arranged in six-column rows. Its four switch cases
traverse those records in different orders, retaining signed-byte comparisons,
two register counters, stack locals, bounds and the compiler-generated table.
The existing DS:C316 declaration corroborates its base. The test changes one
case's comparison value and requires full comparison to fail.

In F_90A6, `((int *)gc5c6+2)[k]` reproduces the original four-byte memory
displacement after scaling the index. The equivalent `((int *)gc5c6)[k+2]`
emits two increments before scaling and produces one extra byte. A fresh
negative control rejects that rewrite. Other loops preserve a by-value
structure argument, far-pointer offset tables and copy-call sequences.

The profile includes both broad region records and exact base declarations.
These proofs rely on the exact bases and observed accesses, not an assertion
that every profile region length is a recovered allocation. In particular,
the 240-byte destination stride does not establish a complete object extent
or historical BSS layout. Code remains fixed-placement matching C.

Refreshing the historical linkage snapshot removes the already owned F_A13F.
It now retains 19 candidates / 9,227 extent bytes, 61 symbols and 108 fixup
sites. Local pending probes remain documented separately.

Coverage is 215 C routines / 24,999 bytes. Raw EXE coverage is 45,018 bytes,
including 24,136 classified machine bytes and 20,882 unknown bytes. The full
EXE and both DAT archives remain byte-identical; the goal remains active.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave24.json
python tools/inventory_linkage.py
python -m unittest discover -s tests
python tools/reconstruct_game.py
```

Evidence: matching-wave24-evidence.json and linkage-blockers.json.
Recipe: recipes/c/matching-wave24.json.
