# Fifteenth local C wave: complete boundaries

| Function | Complete C bytes |
|---|---:|
| F_D3DA | 130 |
| F_C15E | 66 |
| F_C0E0 | 126 |

All three compile exactly with the pinned Turbo C toolchain. Full emitted
code and all 39 fixups match the original contributions; there are no MZ
relocation obligations or initialized-data contributions in this batch.
External references agree with exact declarations in the pinned correspondence
profile. Twelve references resolve through component publics, including
F_C15E's call to the newly owned F_C0E0. Addresses still use fixed placement.

## F_D3DA boundary correction

The upstream 105-byte extent covers CS:D3DA..D443, ending immediately after
the call at D440 to F_880A. Disassembly continues with caller argument cleanup,
saving the result in DI, passing the local far pointer to F_F3F7, restoring the
result to AX, and the complete epilogue at D456..D45C. The final instruction
is RET at D45B. The independent next entry F_D45C begins at D45C and calls
back to D3DA at D468. All 130 bytes fit within the previously raw owner.

The recipe records both upstream and corrected lengths and pins the digest
of the complete contribution. The test freshly compiles the full function,
checks its complete bytes and epilogue, and rejects binding the object to the
old 105-byte span. The recovered extra 25 bytes reduce the previously unknown
raw category, rather than the upstream-classified machine category.

Additional fresh compiler controls change the pointer displacement in F_D3DA,
the 1000-byte indexing stride in F_C0E0, and a call argument in F_C15E. Each
must fail complete comparison. There are no hidden raw fragments in these C
owners and no inference of original source filenames or module boundaries.

## Remaining observations

Complete disassemblies of F_C2EA, F_C359 and F_C3DB show interdependent AL/AH,
SI and ES:DI inputs. F_C7CB preserves AX/CX/ES and uses LOOP with explicit ES
accesses. F_C232 likewise uses shared register state and LOOP. F_D79C changes
BP to access/reuse a manually assembled argument stack and uses string/segment
setup. These are not promoted as C: ordinary Turbo C calling conventions have
not been shown to reproduce them. This is a concrete calling-convention
obstacle, not proof that no C formulation or original assembly wrapper exists.
F_3A75 also has a truncated upstream extent and needs a larger boundary audit.
The matching-C frontier remains open.

Current coverage: 201 C functions / 21,404 bytes; raw EXE 48,879 bytes,
including 27,731 classified machine bytes and 21,148 unknown bytes.
The complete EXE and both DAT archives remain byte-identical.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave15.json
python -m unittest discover -s tests
python tools/reconstruct_game.py
```

Evidence: matching-wave15-evidence.json.
Recipe: recipes/c/matching-wave15.json.
