# Twenty-second local C wave: record copies and field stores

F_ADCF reproduces its complete 374-byte function from C. Its 27-byte structure
retains the exact byte fields and unaligned word fields observed in the code.
The original first-empty-slot loop, record-shift loop, final slot copy, field
stores and final call all compile identically. The source uses structure
assignment; the compiler emits the SCOPY calls and their relocation sites.

The DS:C360 binding uses the independently checked base observations introduced
in wave 21. Other data references retain corroborated declarations, and code
references to SCOPY and F_A13F resolve through owned components. Full fresh
comparison covers the complete emitted bytes, OMF fixups and MZ relocation
obligations. A changed word-field initializer is rejected by the regression
check. No original instruction fragments are used within the C contribution.

This establishes the accessed record layout for this routine. It does not
claim complete BSS allocation, historical structure names, module grouping or
linker-derived layout. F_AB66 was inspected next; it still contains a reference
to an undeclared structured-data base at DS:139D. That routine remains raw and
was not counted as a C match.

Coverage is 212 C routines / 23,812 bytes. Raw EXE coverage is 46,205 bytes,
including 25,323 classified machine bytes and 20,882 unknown bytes. All three
game files remain byte-identical. The matching-C goal remains open.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave22.json
python -m unittest discover -s tests
python tools/reconstruct_game.py
```

Evidence: matching-wave22-evidence.json and storage-binding-evidence.json.
Recipe: recipes/c/matching-wave22.json.
