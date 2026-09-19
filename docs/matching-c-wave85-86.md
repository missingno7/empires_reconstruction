# Matching C waves 85–86

Waves 85 and 86 convert two complete raw executable extents into freshly
compiled matching C owners. The sources remain mechanical inline assembly with
explicit symbol bindings; no semantic cleanup is required for the byte match.

| Owner | File extent | Bytes | Fixups | MZ load relocations |
|---|---:|---:|---:|---:|
| `F_9EC3` | 41155–41280 | 125 | 4 | 40700, 40712 |
| `F_643A` | 26170–26410 | 240 | 26 | none |

`F_9EC3` walks two indexed far-pointer tables and copies a clipped row through
both tables. Its two `mov bx,DGROUP` instructions reproduce the two relocated
segment words. `F_643A` updates a record, performs the original seek/read/write/
close calls, and invokes the existing reconstructed helpers. Its 26 OMF fixups
are all resolved through the declared DGROUP and code bindings.

Both promotions passed fresh compile, bind, relocation, and full-byte checks.
`python tools/reconstruct_game.py` still produces an exact EXE and exact DAT
archives. The remaining raw executable ownership is now 30,172 bytes.
