# Matching C waves 95–96

Two exact raw code spans are now independent matching-C sources:

| Owner | File extent | Bytes |
|---|---:|---:|
| `F_C567` | 51047–51098 | 51 |
| `F_4F96` | 20886–21185 | 299 |

Both are complete return-terminated routines with zero external fixups and no
loader relocations at this checkpoint. `F_C567` is now symbolic TASM and has
three direct OMF references to the recovered sound-data state words `_g1788`
and `_g178a`. Its boundary remains correctly aligned immediately before the
owned `F_C59A` routine. Full EXE and DAT equality remains intact; 23,171
executable bytes remain raw at this historical checkpoint.
