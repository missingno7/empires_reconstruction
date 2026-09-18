# Twenty-fifth local C wave: record access and pointer conversion

F_8480 (585 bytes) and F_7BFC (405 bytes) now reproduce their complete function
extents from freshly compiled C. Promotion compares every emitted byte and
checks all OMF fixups and MZ relocation obligations. Fifteen new references
resolve through owned component publics; remaining bindings have exact declared
bases. No new initialized-data or BSS ownership is claimed.

F_8480 preserves the observed record fields at offsets 0, 2, 7 and 11,
including two far pointers and an unsigned selector byte. Only accessed fields
are interpreted: the intervening byte is retained without an invented meaning.
The original passes a stored near offset as a far pointer using DS. The explicit
`(char *)(char near *)gc104` conversion emits the original segment push; passing
only the near pointer produces a 584-byte function and the wrong argument size.
A fresh negative control rejects that change. Loops, fall-through switch cases,
selector tables and all branch encodings match in full.

F_7BFC copies a 20-byte record from a far pointer into a stack local. Its source
retains twelve uninterpreted record bytes, the observed pointer/word fields,
reverse loop, post-increments and post-decrement. Fresh compilation calls the
pinned SCOPY public with the original length, including its segment relocation.
A negative control shortens the record and must fail full comparison. This is
an access/layout reconstruction, not proof of historical type names or modules.

Coverage is 217 C routines / 25,989 bytes. There are 61 raw EXE owners covering
44,028 bytes: 23,146 classified machine bytes and 20,882 unknown bytes. The EXE
and both DATs rebuild exactly. The historical held snapshot remains 19 candidates,
61 external symbols and 108 fixup sites; it is not an exhaustive list of local
candidates. The matching-C goal remains active.

F_D61C was also inspected: it reuses argument stack slots, changes BP mid-function,
and explicitly preserves segment registers. That is concrete evidence to pursue
other C candidates first, not proof that every possible C reconstruction has
been excluded. F_A768 remains a C-shaped candidate with a DS:13D9 record dependency.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave25.json
python tools/inventory_linkage.py
python tools/reconstruct_game.py
python -m unittest discover -s tests
```

Evidence: matching-wave25-evidence.json. Recipe: recipes/c/matching-wave25.json.
