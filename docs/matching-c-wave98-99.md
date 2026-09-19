# Matching C waves 98–99

The two non-returning state loops that previously lacked a compiler epilogue
are now recovered as matching C. Their sources emit the exact loop bytes and
publish a second OMF label at the verified extent boundary; the recipe binds
the first public with `span: 1`, so compiler output after the label is outside
the claimed component.

| Owner | File extent | Bytes | Fixups | Loader relocations |
|---|---:|---:|---:|---:|
| `F_7964` | 31588–32211 | 623 | 0 | none |
| `F_880A` | 35338–35844 | 506 | 0 | none |

This resolves the exact terminal-boundary uncertainty without adding an
invented `RET`. Full EXE and DAT equality still pass; raw executable ownership
is now 20,556 bytes.
