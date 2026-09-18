# Twenty-first local C wave: corroborated indexed storage base

| Function | Complete matching C bytes |
|---|---:|
| F_A09D | 162 |
| F_AD25 | 170 |
| F_A13F | 31 |

The pinned Turbo C compiler reproduces all 363 bytes, including structure-copy
calls, loop bounds, pointer arithmetic, fixups and MZ relocation obligations.
F_AD25 calls the newly owned F_A13F through its component public. Initialized
storage remains unchanged and no bytes are credited as reconstructed BSS.

## DS:C360 evidence

The profile lacks an exact declaration for this base. Two complete function
extents now provide independently checked observations:

- At CS:A0AD, F_A09D pushes DS:C360 and record 62, calls the already reconstructed
  F_68AA, and cleans up six argument bytes. F_68AA's existing source copies
  the staged resource to the supplied buffer.
- At CS:AD2A, F_AD25 scales SI by 27, adds C360, copies DS to ES, and compares
  the byte at ES:BX with zero.

The verifier pins original and complete-function hashes, checks instruction
boundaries and full instruction forms, decodes the base, stride, record ID and
relative call target, checks relocation overlap, and requires separate,
nonoverlapping function extents for the two observations. The call target must
also correspond to its matching C owner in the manifest. Promotion and every
full EXE build validate this evidence before using the numeric binding.

This establishes a referenced DS base with a 27-byte indexing stride. It does
not establish a complete array extent, allocation order, historical name,
module BSS ownership or linker-derived address. The declaration deliberately
has no claimed object size. It is stronger than accepting a probe merely
because its numeric operands match, while fixed placement remains scaffolding.

Tests reject changed base, stride, record, target, site, function extent,
missing observations and changed segment-selection instructions even after
re-pinning their hashes. They also reject a contradictory C binding and moved
callee owner. Fresh C controls change the record size and resource IDs and
must fail full comparison.

## Open candidate

F_A658 also emits its complete 272-byte routine exactly in a provisional probe,
but its DS:13C5 structured-data reference lacks an exact declaration. That probe
remains unpromoted; it is not counted below. Other DS:C360-dependent routines
can now be tested using the established evidence.

Coverage is 211 C routines / 23,438 bytes. Raw EXE coverage is 46,579 bytes,
including 25,697 classified machine bytes and 20,882 unknown bytes. The EXE
and both DATs remain byte-identical. The matching-C goal remains active.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave21.json
python -m unittest discover -s tests
python tools/reconstruct_game.py
```

Evidence: storage-binding-evidence.json and matching-wave21-evidence.json.
Recipe: recipes/c/matching-wave21.json.
