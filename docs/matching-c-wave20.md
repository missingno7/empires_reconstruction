# Twentieth local C wave: TOUPPER reconstructed from C

F_F9BE now owns the complete 49-byte routine previously held as LIB_TOUPPER.
Fresh Turbo C output matches every byte, including EOF handling, unsigned-byte
conversion, character-table access, arithmetic and the complete epilogue.
The sole external binding resolves through the verified CTYPE data owner's
__ctype public plus one. No library code is copied into this C contribution.
A fresh negative control changes the classification mask and must fail full
comparison.

The generic upstream F_F9BE extent is 81 bytes. The more precise LIB_TOUPPER
entry covers the complete 49-byte library contribution ending at CS:F9EF,
immediately after RET at F9EE. The source reproduces this entire narrower
component, not a partial routine. Remaining bytes outside it are not claimed.
The recipe pins the more precise entry and its complete original extent hash.

## Historical blocker snapshot

The inventory formerly excluded only matching owner IDs. It now also excludes
a historical candidate when another current C, ASM or pinned library owner has
exactly the same start, end, expected digest and EQUAL status. Resolved aliases
are recorded with both IDs and the upstream entry hash. Near matches, raw/data
fallbacks, changed digests, shifted boundaries and nonmatching status cannot
close a blocker. This inventory is metadata reconciliation, not a replacement
for fresh compilation and complete build verification.

Refreshing it removes F_1D47, F_9440, F_A28D and LIB_TOUPPER. The first three
were already recovered in earlier batches. The snapshot now contains 20 held
candidates / 9,258 extent bytes, 62 distinct symbols and 109 fixup sites.
Local pending probes remain documented separately; this is not an exhaustive
claim about all remaining C possibilities.

Coverage is 208 C routines / 23,075 bytes. Raw EXE coverage is 46,942 bytes,
including 26,060 classified machine bytes and 20,882 unknown bytes. All three
game files remain exact. Fixed placement, unresolved storage and remaining
compiler differences keep the overall matching-C goal open.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave20.json
python tools/inventory_linkage.py
python -m unittest discover -s tests
python tools/reconstruct_game.py
```

Evidence: matching-wave20-evidence.json and linkage-blockers.json.
Recipe: recipes/c/matching-wave20.json.
