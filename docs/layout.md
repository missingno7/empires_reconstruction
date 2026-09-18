# Ownership and address rules

`layout/manifest.json` is authoritative. Its ordered `regions` partition the
entire on-disk EXE exactly once. The builder rejects overlapping ranges, gaps,
duplicate owner IDs, invalid sizes, unexpected compiled or raw lengths,
unresolved fixups, relocation differences and mismatched bytes. All owners
carry expected SHA-256 digests. `matching_status: EQUAL` records the imported
status; only the current build's `build/report.json` establishes a fresh result.

`kind` selects representation (`MATCHING_C`, `MATCHING_ASM`, `RAW`, or
`EXACT_DATA`); `classification` is independent descriptive metadata. Future
font/table/asset encoders can add a representation while retaining the same
range and verification model. No semantic classification is inferred for raw
load-image regions in MVP1.

## Coordinates

All conversion between file offsets and load-module offsets lives in
`tools/mz.py`. For this input:

| Coordinate | Meaning / conversion |
|---|---|
| File offset | Byte position in `AEPROG.EXE`; canonical ownership coordinate |
| MZ load-module offset | `file_offset - 0x200`, only inside the load image |
| Segment:offset relative to load base | `16 * segment + offset` |
| Runtime physical address | `16 * runtime_load_segment + load_offset`; the load segment is a DOS loader choice |
| `_TEXT` offset | Load-module offset here because `_TEXT` begins at load offset 0 |
| `DGROUP` offset | Offset from load-module byte `0xFA30` |
| OBJ offset | Offset within one module's segment; it has no final placement by itself |
| Relocation target | A **word location** in the load image, computed from an MZ table entry; distinct from the value stored there |

The upstream profile uses physical `_TEXT = 0x10100` and `DGROUP = 0x1FB30`
in its observation environment. Their difference is `0xFA30`. Imported data
bindings subtract the declared physical frame exactly once; code bindings
become `_TEXT` offsets. The reconstruction does not assume that DOS always
loads the finished game at physical `0x10100`.

Example: `F_56C6` owns file `[0x58C6, 0x5C3B)`, corresponding to load-module
`[0x56C6, 0x5A3B)`. This is 885 bytes. Its source calls resolve against
original `_TEXT` offsets. A near-call operand is
`target + displacement + OBJ_addend - (load_location + 2)`, modulo 65536.
Data offset fixups use declared DGROUP offsets plus displacement and addend.
Internal `_TEXT` references use `owner_load_start - public_OBJ_offset` as
the module's code base.

Far code pointers are emitted as offset and load-relative segment words; the
segment word must have a corresponding MZ relocation. DGROUP segment fixups
produce `0xFA3`, also requiring an MZ relocation. The DOS loader later adds
the actual load segment. The builder preserves the original relocation-table
order and entries exactly and separately checks source-derived relocation
locations. Unsupported fixup forms fail instead of being ignored.

## Extents, stand-ins and raw ownership

The next public symbol (or the declared public `span`, or segment end)
determines the extracted size. The builder never clips compiler output to the
expected length. A module may contain support bytes outside that selected
extent: those bytes do not acquire ownership merely because they appear in an
OBJ. They remain owned by the manifest's other regions.

In particular, `F_56C6.C` contains `static char q139d[2] = {7, 0}` as a
matching stand-in. Its `_DATA` base is explicitly fixed at DGROUP offset
`0x139D`, from the upstream proof's `module_data_base_votes/_DATA` evidence
and the source's declaration. This MVP imports that established correspondence
as fixed metadata; the builder never repeats the upstream voting process
against original bytes. The two actual data bytes retain their raw owner.
The build makes no claim about historical translation-unit ownership or
whether these standalone objects could be linked as a conventional program.

The 512-byte MZ header is one raw owner, including its 106 relocation entries
at file offset `0x22`. The declared load image is 78,642 bytes. This EXE has no
bytes beyond its declared MZ size, but the parser/importer retain trailing
file bytes as a separate owner when present. Entry state is relative
`CS:IP = 0000:0000`, `SS:SP = 1C50:00E6`; minimum allocation is `0x92C`
paragraphs, maximum `0xFFFF`. Runtime allocation and generated memory are not
extra bytes in the executable file.
