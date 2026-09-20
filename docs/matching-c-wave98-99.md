# Matching C waves 98–99

The two non-returning state loops that previously lacked a compiler epilogue
are recovered with exact terminal boundaries. `F_7964` remains matching C;
`F_880A` is now intentional symbolic TASM, with its calls and branch tables
expressed as normal assembler relationships. Both publish a boundary label so
the component ends without inventing a `RET`.

| Owner | File extent | Bytes | Fixups | Loader relocations |
|---|---:|---:|---:|---:|
| `F_7964` | 31588–32211 | 623 | 0 | none |
| `F_880A` | 35338–35844 | 506 | 0 | none |

This resolves the exact terminal-boundary uncertainty without adding an
invented `RET`. The current structural EXE build remains byte-identical; the
historical raw-ownership count in this wave receipt is retained only as
chronology.
