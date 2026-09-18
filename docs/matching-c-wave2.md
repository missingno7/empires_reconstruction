# Second local matching C wave

Thirteen functions now compile to 920 previously raw EXE bytes. The complete
EXE, ordered relocation table, load image and both DAT files remain identical.
No inline ASM, byte patching or excluded instruction bytes are used.

| Owner | Bytes | Reconstructed source pattern |
|---|---:|---|
| F_568C | 58 | Ordered calls with constant arguments |
| F_7417 | 44 | Far-pointer offset table with 22-byte header |
| F_7443 | 56 | Far-pointer offset table with 32-byte header and register index |
| F_7676 | 31 | Six-argument call |
| F_9402 | 62 | Boolean register value and far-pointer offset table |
| F_984C | 37 | Three ordered calls |
| F_9871 | 30 | Call with two global arguments |
| F_9962 | 64 | Calls with a global-coordinate adjustment |
| F_99A2 | 64 | Related call sequence with a different adjustment |
| F_99E2 | 44 | Two fields from a four-byte record array |
| F_A004 | 50 | Bounded far-pointer copy loop and returned pointer |
| F_ACE7 | 39 | Flag comparison in a 27-byte record array |
| F_AF45 | 341 | Corrected held C draft: increment result used in comparison |

The held F_AF45 draft compiled to 340 bytes. Separately incrementing the global
and comparing it let Turbo C emit `cmp word [g13ef],3`. The original instead
increments memory, loads AX and compares AX. Writing `if (++g13ef[0] >= 3)`
produces that exact sequence and restores the original branch displacements.
The explicit final return preserves the jump to the epilogue. This closes an
actual source/code-generation difference, rather than accepting equivalent
behavior or patching an operand.

Global bindings name exact storage-object bases in the pinned upstream model.
Code targets use owned components where available and recorded function entries
otherwise, including existing runtime dispatch entries at 039F/03BA/03C0/03C3.
Their unresolved implementation does not get credited as reconstructed code.

The sources and all declarations are captured in `recipes/c/matching-wave2.json`.
Promotion freshly compiles every candidate and checks its full extent and OMF
fixups before changing canonical ownership:

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave2.json
python tools/reconstruct_game.py
```

`matching-wave2-evidence.json` records source hashes and match results. Promotion
keeps each recipe's receipt separate so the first wave's evidence is preserved.
Regression tests freshly compile all thirteen functions, then reject the old
split increment/comparison and an incorrect table-header size.

Current matching C coverage is 149 functions / 15,353 bytes. The two local waves
together added 23 functions / 1,211 bytes. Raw EXE coverage is 55,030 bytes:
33,754 classified unresolved machine bytes and 21,276 unclassified bytes.
Historical modules, data placement and real linking remain unfinished.
