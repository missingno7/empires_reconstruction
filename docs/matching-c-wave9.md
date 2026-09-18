# Ninth local C wave

F_B60F (190 bytes) and F_B6CD (165 bytes) now compile to their complete original
code, adding 355 matching C bytes.

F_B60F traverses 32-byte records after the count at DS:B3AE. The local pointer
is derived explicitly from that declared base plus one; no independent binding
for DS:B3AF is needed. Field offsets, record stride and the count-byte read are
all checked by the complete compiled extent. Two assignment expressions must
retain their assignment results while evaluating the following indexed loads.
Splitting one into two statements removes four bytes and fails the comparison.

F_B6CD preserves chained stores, ordered calls, pointer comparison and its loop
threshold. Its call to F_B60F resolves through the newly owned component public.
Other available C/ASM targets likewise use component-derived entry bindings.
Regression controls reject an altered threshold and the split assignment.
No inline ASM, emitted byte blobs or partial code spans are used.

## Remaining probes

F_B772 (135 bytes) matches after expressing record fields relative to the same
count-byte base. It remains held on undeclared DS:C588 and DS:C5A2 globals.
F_A33F (203 bytes) also matches, but references unowned strings at DS:12D9 and
DS:1660. These strings occupy separate regions; they need explicit independent
data ownership rather than a fabricated contiguous compiler contribution.
Both drafts remain ignored local research in build/c-wave9.

Current coverage: 189 matching C functions / 19,302 bytes. Raw coverage is
51,022 bytes, including 29,805 classified unresolved machine bytes. All three
game files still match exactly. Matching-C recovery remains active; this batch
does not establish original module boundaries or real linking.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave9.json
python -m unittest discover -s tests
python tools/reconstruct_game.py
```

Recipe: `recipes/c/matching-wave9.json`. Fresh metadata-only results:
`matching-wave9-evidence.json`.
