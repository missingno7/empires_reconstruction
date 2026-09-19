# Seventy-fifth structured-data wave

The three remaining short zero spans between recovered executable owners are
alignment padding, not unresolved instructions:

| Source | Load range | Length | Boundary evidence |
|---|---:|---:|---|
| `PAD_004CA7` | `0x4CA7..0x4CA8` | 1 | `F_4A93` ends at `0x4CA7`; `F_4AA8` starts at even offset `0x4CA8` |
| `PAD_006F85` | `0x6F85..0x6F86` | 1 | `F_6D3C` ends at `0x6F85`; `F_6D86` starts at even offset `0x6F86` |
| `PAD_006FC5` | `0x6FC5..0x6FCC` | 7 | `F_6D86` ends at `0x6FC5`; `F_6DCC` starts at four-byte boundary `0x6FCC` |

All nine bytes are zero, have no MZ relocation entries, and sit wholly between
complete owned code extents. Their canonical JSON sources use the deterministic
`zero-pad-v1` encoder, which validates the length and emits only zero bytes.
Fresh encoding matches every original span, and the complete EXE and DAT
rebuild remains byte-identical. This wave removes three opaque raw owners while
leaving only the two longer data-heavy raw spans for further classification.
