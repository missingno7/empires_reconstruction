# Fourth local C wave: two initialized-array functions

F_7695 and F_778B were previously unrecovered machine extents with no upstream
C drafts. Reconstructing their complete disassemblies as C produces exact
Turbo C 2.0 output, including their local array initialization via SCOPY@.

| Code owner | Code bytes | Data owner | Data bytes | Data address |
|---|---:|---|---:|---|
| F_7695 | 178 | C_DATA_7695 | 13 | DS:0B96 |
| F_778B | 203 | C_DATA_778B | 15 | DS:0BA3 |

The functions use ordinary C calls, register locals, far pointers under the
compact memory model, and string-literal array initializers. There is no inline
assembly, instruction patch, or excluded byte. Each emitted _DATA contribution
is owned in full and comes from the same fresh object as its code. Both far
SCOPY@ calls resolve through the pinned library public and preserve their MZ
relocations. External addresses are corroborated by exact upstream function
entries or storage-object bases; existing component ownership is used when
available.

The recipe is `recipes/c/matching-wave4.json`; fresh metadata-only proof is
`matching-wave4-evidence.json`. The initializer regression suite also compiles
and compares these two functions and both data contributions. Its negative
controls still prove that an initializer edit cannot disappear behind raw data.

Matching C coverage is now 163 functions and 16,555 bytes. Compiler-owned
initialized data totals 43 bytes across three components. Raw EXE coverage is
53,785 bytes: 32,552 recorded unresolved machine bytes and 21,233 unclassified
bytes. The three-file build remains byte-identical. These are independent
component proofs under fixed placement, not a claim of original module or
linker recovery.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave4.json
python -m unittest discover -s tests
python tools/reconstruct_game.py
```
