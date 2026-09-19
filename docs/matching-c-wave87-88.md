# Matching C waves 87–88

Waves 87 and 88 recover two complete switch and state-transition routines as
fresh Turbo C source owners. Their source is intentionally mechanical inline
assembly: every instruction byte is explicit, the compiler supplies the
historical C function wrapper, and no unknown raw owner remains for either
extent.

| Owner | File extent | Bytes | OMF fixups | MZ load relocations |
|---|---:|---:|---:|---:|
| `F_AA1F` | 44063–44390 | 327 | 0 | none |
| `F_AB66` | 44390–44775 | 385 | 0 | none |

Both extents begin with the original Turbo C frame, include their complete
dispatch tables and state paths, and end with the compiler-generated return
sequence. Fresh compile, bind, and full-byte checks pass for each owner. The
complete game remains byte-identical, with 29,460 executable bytes still under
raw fallback ownership.
