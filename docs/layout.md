# Ownership and address rules

`layout/manifest.json` is authoritative. Its ordered `regions` partition the
entire on-disk EXE exactly once. The builder rejects overlapping ranges, gaps,
duplicate owner IDs, invalid sizes, unexpected compiled or raw lengths,
unresolved fixups, relocation differences and mismatched bytes. All owners
carry expected SHA-256 digests. `matching_status: EQUAL` records the imported
status; only the current build's `build/report.json` establishes a fresh result.

`kind` selects representation (`MATCHING_C`, `MATCHING_ASM`,
`KNOWN_TOOLCHAIN_LIBRARY`, `MZ_HEADER`, `RAW`, or `EXACT_DATA`); `classification` is
independent descriptive metadata. Future
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

An `EXACT_DATA` owner with encoder `omf-segment-v1` explicitly claims an entire
initialized `_DATA` contribution from a matching C owner's fresh OBJ. The
source paths must agree, the emitted segment must cover exactly the owner,
and its placement must be the C owner's component-based module-segment
binding. Relocated data segments are currently rejected. `C_DATA_75F3` is
the first such owner: its 15-byte initializer is generated with the function,
not extracted from the original EXE. Bootstrap extraction skips this source.

Library owners select the **complete** named module's `_TEXT` contribution,
including private helpers. `build.library`, `build.library_module` and
`build.module_sha256` fix the binary input; `layout/toolchain.json` pins the
whole library. The library itself is local and Git-ignored. These owners are
not counted as matching C/ASM and do not claim newly recovered source.
Data/BSS bases and library-public offsets taken from established upstream
receipts are frozen explicitly in `build.module_segments` and `build.bindings`.
They are never inferred from the original comparison bytes during a build.

In particular, `F_56C6.C` once contained `static char q139d[2] = {7, 0}` as a
matching stand-in; it now references the shared `g139d` dialog descriptor. Its `_DATA` base is explicitly fixed at DGROUP offset
`0x139D`, from the upstream proof's `module_data_base_votes/_DATA` evidence
and the source's declaration. This MVP imports that established correspondence
as fixed metadata; the builder never repeats the upstream voting process
against original bytes. The two actual data bytes retain their raw owner.
The build makes no claim about historical translation-unit ownership or
whether these standalone objects could be linked as a conventional program.

The 512-byte MZ header is the `MZ_HEADER` owner, encoded from
`layout/mz-header.json`. It includes 106 relocation entries at file offset
`0x22`. The declared load image is 78,642 bytes. This EXE has no
bytes beyond its declared MZ size, but the parser/importer retain trailing
file bytes as a separate owner when present. Entry state is relative
`CS:IP = 0000:0000`, `SS:SP = 1C50:00E6`; minimum allocation is `0x92C`
paragraphs, maximum `0xFFFF`. Runtime allocation and generated memory are not
extra bytes in the executable file.

## Structured MZ source

`tools/mz.py` owns both parsing and encoding. `header_document` decodes a
fixture only during the explicit one-time recovery operation;
`encode_header` takes the JSON document and total file length, never executable
bytes. Normal builds encode this source and compare it to the original header
before invoking the compiler. The source-derived header then supplies the MZ
layout and relocation obligations used to bind code regions.

All 14 fixed words are explicit unsigned 16-bit integers:

| Fields | Meaning |
|---|---|
| `e_magic` | `0x5A4D`, the two signature bytes |
| `e_cblp`, `e_cp` | Last-page byte count and 512-byte page count; zero last-page count means a full page |
| `e_crlc` | Number of relocation entries; must equal the explicit list length |
| `e_cparhdr` | Header size in 16-byte paragraphs |
| `e_minalloc`, `e_maxalloc` | Additional allocation bounds in paragraphs |
| `e_ss`, `e_sp`, `e_ip`, `e_cs` | Initial segment-relative execution state |
| `e_csum` | Original checksum word, preserved rather than recalculated |
| `e_lfarlc` | File offset of the relocation table |
| `e_ovno` | Original overlay-number word |

The relocation list preserves its exact order and each original segment:offset
pair. Entries are neither sorted, deduplicated nor normalized to equivalent
linear addresses. Those operations could preserve some loader behavior but
would change the executable bytes. Relocation targets must address complete
words within the declared load image.

`before_relocations_hex` preserves six uninterpreted bytes at `[0x1C,0x22)`:
`01 00 FB 20 72 6A`. `after_relocations_hex` preserves the 54 zero padding
bytes at `[0x1CA,0x200)`. They are explicit source bytes, with lengths checked
against `e_lfarlc`, `e_crlc` and `e_cparhdr`; the encoder supplies no implicit
padding or corrected count/size fields. Unknown fields and malformed values
are rejected instead of silently ignored.

`python tools/recover_mz_header.py` performed the initial RAW-to-header
promotion after proving an exact round trip. It is now a no-op so it cannot
overwrite edits to the canonical JSON source. Generated header bytes are
written to `build/regions/MZ_HEADER.bin`; the old ignored raw header file is
not consumed and is not required on a fresh checkout.

## Independent terminated text

`ascii-nul-v1` sources contain exactly `format` and `text`. The encoder emits
ASCII text followed by one zero byte, rejecting embedded terminators and
non-ASCII characters. Control characters remain explicit JSON escapes. Each
complete string has an EXACT_DATA owner and C references derive its DGROUP
offset from that owner. Canonical text documents live under src/data and are
never overwritten by raw extraction. See [the first text ownership proof](matching-c-wave10.md).

`ascii-v1` is the corresponding exact adapter for an unterminated ASCII span.
It emits the source text byte-for-byte, preserving CR separators and any
non-NUL control bytes. It is used only when the complete owner extent is
relocation-free and the boundary is independently established.
