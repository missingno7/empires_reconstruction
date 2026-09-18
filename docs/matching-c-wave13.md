# Thirteenth local C wave: verified storage observations

F_B772 now reproduces its complete 135-byte code contribution. It was held
because the pinned upstream profile had no exact storage declarations for
DS:C588 and DS:C5A2. Local evidence now records independent accesses:

| Word address | Write in F_B772 | Independent read in F_B99F |
|---|---|---|
| DS:C588 | CS:B781, store AX | CS:BB7B, word comparison |
| DS:C5A2 | CS:B784, immediate word store | CS:BA05, load BX |

The instruction boundaries were inspected in complete function disassemblies.
Metadata pins both function extents and their hashes. tools/storage_evidence.py
verifies the original identity, complete function identities, instruction forms,
direct addresses, 16-bit access width, separate read/write functions and absence
of relocation overlap. Both addresses are beyond the on-disk load image.
Promotion and every full EXE build check that the C bindings agree with this
verified evidence. Changed addresses, observation sites, extents, identities,
missing access evidence and contradictory binding values are rejected.

This is evidence for accessed word locations under fixed placement. It does
not establish original variable names, complete object extents, BSS allocation
order, original modules, or linker-derived addresses. No bytes are added to
static-data ownership and no BSS bytes are copied from a binary fallback.
The source's record accesses retain the established count-byte base plus
explicit displacements. Fresh complete C comparison and a changed loop-bound
control verify the recovered function.

Coverage is now 196 matching C functions / 20,591 bytes. Raw EXE coverage is
49,692 bytes, including 28,519 classified unresolved machine bytes. All three
game files remain exact. More held candidates may be unlocked by similarly
bounded evidence; the goal remains active.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave13.json
python -m unittest discover -s tests
python tools/reconstruct_game.py
```

Evidence: storage-binding-evidence.json and matching-wave13-evidence.json.
Recipe: recipes/c/matching-wave13.json.
