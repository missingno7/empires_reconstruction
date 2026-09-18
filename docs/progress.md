# Incremental reconstruction — 2026-09-18

Full EXE identity is preserved. Follow-ups to MVP1 remove 30,501 bytes from raw
fallback: 26,382 bytes of matching C/library regions, the 512-byte header,
1,536 bytes of structured DAC palettes, 59 bytes of compiled C data, and 50 bytes of independently encoded text.
Archive/resource structure now has a separate exact build. Broad gameplay
semantic cleanup remains outside this mechanical phase.

| Representation | MVP1 bytes | Current bytes | Current owners |
|---|---:|---:|---:|
| Freshly compiled matching C | 14,077 | 35,912 | 244 |
| Freshly assembled matching ASM | 2,456 | 2,532 | 21 |
| Known toolchain library | 0 | 5,384 | 42 |
| Structured MZ header | 0 | 512 | 1 |
| Structured DAC palettes | 0 | 1,536 | 2 |
| Compiled C initializer | 0 | 80 | 6 |
| Independently encoded text | 0 | 259 | 5 |
| Exact raw fallback | 62,621 | 31,931 | 52 |
| Total | 79,154 | 79,154 | 373 |

The 78,642-byte DOS load image, all 106 ordered MZ relocation entries, and
the full file remain EQUAL. Original and rebuilt SHA-256:

```text
1259348425483d8d97fd8821860b47cfdf58fc8029711eb0ed0e78ab33807a10
```

## Newly owned regions

`F_01CE` now owns its 65 bytes as matching C. Its source was copied unchanged
from current upstream, compiled with the original flags, bound against the
existing declared correspondence, and compared in full. The old receipt's
source hash remains different; provenance records both hashes and marks the
old source receipt stale. The new build supplies the current proof rather than
pretending that the old receipt applies unchanged.

All 35 upstream `KNOWN_TOOLCHAIN_LIBRARY` entries are now separate owners.
Each build checks the local `CC.LIB` digest, selects its named OMF module,
checks the module digest, extracts its complete code segment, applies explicit
bindings, and checks bytes and MZ relocation obligations. The 35 library
modules have 171 fixups, including 24 relocated segment words. Library objects
are binary-derived inputs, not newly reconstructed C or ASM source. Library
entries upstream still marked `UNRECOVERED_MACHINE` remain raw.

`CC.LIB` is obtained with `python tools/setup_toolchain.py`, from the local
Borland installation's `LIB` directory. It is ignored by Git along with all
other compiler/game binaries and raw byte extracts. No upstream files are
modified; no PortForge execution is used.

The MZ header now has a structured source, `layout/mz-header.json`: 14 fixed
words, 106 ordered relocation pairs, six uninterpreted pre-table bytes, and
54 explicit padding bytes. `tools/mz.py` encodes all 512 bytes without reading
the original header. A fresh build checks exact equality before compiling,
then uses the rebuilt metadata for source-region placement and relocation
obligations. See [layout.md](layout.md#structured-mz-source) for field meanings
and preservation rules.

## Incremental promotion

```powershell
python tools/promote_upstream.py --ids F_01CE --libraries
python tools/reconstruct.py
```

These owners are already promoted, so the first command is now a no-op.
For later candidates, pass their exact upstream IDs using `--ids`; `--libraries`
adds all not-yet-owned library entries whose upstream status is established.
The tool does not replace any existing source or library owner. A candidate
must fit entirely inside a raw owner, and every candidate must pass fresh
length, binding, relocation and byte checks before the manifest or canonical
sources are written. Raw gaps are split, preserving their exact byte ownership.
The manifest is published by file replacement. Successful promotion invalidates
the older full-build report; rerun the reconstruction command to publish a new
one. Failed candidate verification preserves the old layout and success files.

Logs and promotion proofs stay under `build/promotion-*`. Raw files no longer
referenced by the manifest have no ownership; the tool leaves these local
ignored files alone rather than deleting evidence automatically.

## Verification and next boundary

The eighteen original EXE tests cover fresh C/ASM mutations, library/module identity
errors, a modified library instruction, a wrong library global address,
missing relocation, and a failed promotion preserving canonical files.
Header tests cover changed fields, checksum, uninterpreted bytes and padding;
count/size errors; reordered and equivalent-address relocation pairs; duplicate
entries; exact final pages; and trailing file bytes. Header mismatches are
rejected before compiler invocation.
The complete reconstruction passes after promotion. A separate clean source
copy is also bootstrapped with only locally supplied game/compiler inputs,
recreates its raw files, and produces the same EXE without accessing upstream
or having a raw header file.

## Complete-game reconstruction

`python tools/reconstruct_game.py` freshly rebuilds all **690,588 bytes** of
the EXE and both DAT archives, and publishes `build/game-report.json` only
after all components pass. The former EXE-only command remains available.
It also independently packs both DATs from component recipes and requires
`derived pack == fixed rebuild == original`. These matching bytes are a
bootstrap/structural milestone, not completion of the original build system.

| Archive coverage | AE000 | AE001 |
|---|---:|---:|
| Exact complete bytes | 227,560 | 383,874 |
| Partitioned and decoded resources | 89 | 131 |
| Structured payloads round-tripped | 62 | 86 |
| Canonical matching resources | 26 | 1 |
| Canonical matching resource bytes (including headers) | 3,773 | 26,138 |
| Exact recompressed payload bytes | 3,721 | 0 |
| Structured canonical sources | 26 | 1 |
| Remaining raw resources | 62 | 130 |

All 155 RLE stages match exactly. Twenty-six of 182 pair-span streams match
the current encoder. The other 156 first token differences all reflect the
original selecting a shorter span than the greedy candidate. Both tested
duplicate-span tie policies yield the same 26 matches. Token/byte differences
are recorded in [codec-evidence.json](codec-evidence.json).

All 49 standalone bitmaps, 75 nested banks, seven sequential banks, two fonts
and 20 level payloads round-trip through strict structural encoders. Twenty-five
bitmaps now use PNG-plus-JSON sources; the four-image AE000:080 bank and first
level also reproduce their whole original resources from structure. Another
126 structural payloads remain derived research until their compression matches.
Nested formats expose 596 four-bit images, 182 monochrome records and 256 glyphs,
while preserving 100 unknown records explicitly. Original game files and all raw/decoded/structured
asset content remain ignored local inputs. See [archive-formats.md](archive-formats.md).

The combined EXE metrics distinguish 265 source proof units from **zero proven
historical source modules**, and 1,719 declared owner-symbol bindings from
**zero linker-resolved bindings**. Hash-pinned on-disk machine extents classify
12,465 raw EXE bytes as unresolved machine code; overlapping entry/parent
extents count once. The other 19,466 raw bytes remain unknown. Two palette owners account for
1,536 embedded-asset bytes. There are now 568 component-owned bindings
within fixed placement: 485 C/ASM entry references, 75 library bindings,
and nine structured-data references. Six module-segment bindings also resolve through
compiler-initialized data owners. See [embedded-palettes.md](embedded-palettes.md).

[linkage-blockers.json](linkage-blockers.json) now records zero held upstream
candidates and zero unresolved symbols after the final ASM linkage proof. The
historical linkage ledger is retained as an audit trail. The shared `_b437a`, `_b4380`,
and `_b4386` symbols each affect two candidates covering 4,484 bytes; these
candidate sets overlap, so the leverage counts must not be added. This is
historical evidence, not a fresh match grant. `tools/inventory_linkage.py`
refreshes the ranking without modifying upstream.

Regression tests cover independent hand-authored codec streams,
all observed RLE policies, complete structured payload round trips, mutations,
failure invalidation and archive bootstrap from only supplied local inputs.
All 220 decoded resources also agree with the independent upstream decoder.
A separate clean source copy, supplied only the three original game files and
four pinned compiler/library binaries, recreates every local source and builds
the same three files without upstream access.

## Layout as generated output

Following the owner's architectural clarification, `recipes/archives/*.json`
contains order and source/encoding rules without offsets, original-file paths,
expected sizes or digests. `tools/pack_archives.py` generates the LE32 table
from actual emitted component sizes and builds with no fixtures or fixed
manifests present. A separate verifier proves equality. Size/order/count
mutation tests demonstrate that placement is calculated, not prescribed.
The fixed archive builder remains an independent oracle. Verified resource
promotions update both source descriptions and invalidate older packed outputs.

The combined report distinguishes derived DAT packing from fixed EXE placement
and explicitly reports whole-build reconstruction incomplete. The 193 opaque
DAT payloads and 31,931 raw EXE bytes remain temporary fallbacks. See
[build-reconstruction.md](build-reconstruction.md) for the four reconstruction
levels and the requirement to recover a buildable software system.

The [PNG edit test](resource-sources.md) changes dimensions and pixel indices,
encodes a valid modified resource and verifies the packer's resulting offset
shift without fixtures. Original-equality checks remain separate and reject
the modified output. All 25 unchanged adopted PNG sources reproduce the
original compressed bytes.

On the EXE frontier, [C_6C26_6C87](module-group-evidence.md) compiles four existing
contiguous sources into one fresh OBJ: 128 bytes, public offsets 0/49/73/97,
and 16 fixups match. This is compatible module grouping evidence; original
module/data ownership and linking remain unproven, so no historical-module
or linker-binding coverage has been inflated.

The [phase direction](matching-phase.md) and [blocker ledger](blockers.json)
retain the user's broader agenda. Matching is **not globally saturated**:
compressor search policy, linkage closure, module grouping, embedded-data
classification and nested-resource adapters remain mechanical frontiers.

## Owned code references

484 code bindings across 152 callers now identify selected entry publics of
224 owned C/ASM components. Migration requires both the established address
and the selected public name to agree, followed by a fresh complete EXE match.
Near and far fixups resolve through owners. The remaining numeric bindings,
fixed placement and zero historical-linker coverage remain explicit.
See [code-bindings.md](code-bindings.md).

## Owned library references

75 additional references across 52 callers now resolve through publics read
from 27 pinned library modules. Eleven target nonzero module-relative offsets.
Public offsets are read from OMF at build time, while module placement remains
fixed. Full EXE bytes and relocations remain EQUAL. See
[library-bindings.md](library-bindings.md).

## Explicit compression sources

An independent research path now rebuilds both DATs (611,434 bytes) from
decoded sources and explicit compression syntax: 182 pair-span plans, 153
structured payloads, and 67 opaque decoded payloads. Generation reads only
these exported components; source-only read isolation is tested. This isolates
the unresolved parse policy and does not change canonical resource ownership
or the 26 automatic compression-policy matches. See
[compression-sources.md](compression-sources.md).

## Matching C advances

Ten locally verified C owners remove 291 bytes from raw fallback. F_68CF
resolves its binding aliases and store order; nine additional functions now
compile exactly from C. The F_D60C trailing zero remains raw. Fresh compiler
negative controls reject reversed stores and a missing stack frame. See
[matching-c-wave1.md](matching-c-wave1.md).

## Second matching C wave

Thirteen further C functions remove 920 bytes from raw fallback, including
the 341-byte F_AF45 held draft. Using the preincrement value in its comparison
reproduces the original instruction selection and branch layout. New matches
also cover far-pointer tables, record fields, a bounded copy loop and call
sequences. See [matching-c-wave2.md](matching-c-wave2.md).

## Third matching C wave and compiler data

Twelve more functions remove 821 raw code bytes. F_75F3 closes the final
historical byte-differing C draft: its local array initializer produces both
the exact call sequence and 15 bytes of owned compiler-generated data.
Three additional byte-equal probes remain held for uncorroborated data bases.
See [matching-c-wave3.md](matching-c-wave3.md).

## Fourth local C wave

F_7695 and F_778B now rebuild from newly reconstructed C: 381 code bytes
and 28 initialized data bytes. Both complete functions use the local-array
initializer pattern established by F_75F3. Their complete fresh OMF data
segments have separate owners, with no raw initializer fallback. See
[the wave-four proof](matching-c-wave4.md).

## Fifth local C wave

Ten more routines reproduce 542 previously raw code bytes. These include
local structure construction, two arithmetic branches, a far-pointer table
lookup, and complete call sequences. The regression checks reject both a
changed branch threshold and a compact chained-assignment rewrite. See
[the wave-five proof and held candidates](matching-c-wave5.md).

## Sixth local C wave

Six complete routines add 670 matching C bytes, including resource traversal,
far-pointer arithmetic, a variable-argument loop and an embedded switch table.
Declaration order and flat versus multidimensional indexing explain several
compiler differences. See [the wave-six proof](matching-c-wave6.md).

## Seventh local C wave

Five routines add 756 matching C bytes. F_A28D also owns its two initialized
static strings (16 bytes), resolving their previously missing data bindings
through fresh compiler output. A mutation test verifies that a string edit
changes generated data even when code stays identical. See
[the wave-seven proof](matching-c-wave7.md).

## Eighth local C wave

Three routines add 424 matching C bytes, preserving a pointer-to-integer
return, register allocation and arithmetic branches. Two additional code-exact
probes remain held on data-base evidence. See
[the wave-eight proof](matching-c-wave8.md).

## Ninth local C wave

Two routines add 355 matching C bytes. A record base is expressed as the
existing declared count-byte base plus one, resolving an interior reference
without inventing a standalone storage object. Assignment expressions preserve
the original register/stack behavior. See [the wave-nine proof](matching-c-wave9.md).

## Tenth local C wave

F_A33F adds 203 matching C bytes. Two independently encoded ASCII text
components (27 bytes) resolve its held references. Text generation does not
read original bytes, and bootstrap extraction preserves canonical text sources.
See [the wave-ten proof](matching-c-wave10.md).

## Eleventh local C wave

Three routines add 609 matching C bytes and one independent text component
adds 14 bytes. F_4943 now includes the three-byte epilogue missing from the
upstream extent; the next owned function starts immediately afterward.
See [the boundary and compiler proof](matching-c-wave11.md).

## Twelfth local C wave

F_250C and F_5F3C add 342 matching C bytes. The latter preserves explicit
shift/subtract indexing rather than an algebraically equivalent multiplication.
See [the wave-twelve proof](matching-c-wave12.md).

## Thirteenth local C wave

F_B772 adds 135 matching C bytes after verifying independent word reads and
writes for DS:C588 and DS:C5A2. Both promotion and full builds check the local
storage evidence; this does not allocate BSS or claim historical modules.
See [the wave-thirteen proof](matching-c-wave13.md).

## Fourteenth local C wave

F_CE9E and F_CF3C add 491 matching C bytes. Compound assignments preserve
the original arithmetic directly in DI, avoiding an extra AX temporary. The
caller resolves its newly owned callee through the component public. See
[the wave-fourteen proof](matching-c-wave14.md).

## Fifteenth local C wave

Three routines add 322 matching C bytes. F_D3DA includes the 25-byte tail
omitted by its upstream extent; fresh object checks reject that truncated
boundary. F_C15E calls the newly owned F_C0E0 through its component public.
See [the wave-fifteen proof](matching-c-wave15.md).

## Sixteenth local C wave

F_1D47 and F_B4FB add 449 matching C bytes. Three offset-table loops and
a sequence of byte stores reproduce the original instruction order. Fresh
negative controls reject a changed loop bound and storage displacement. See
[the wave-sixteen proof](matching-c-wave16.md).

## Seventeenth local C wave

F_703E adds 292 matching C bytes, including its five-entry switch table and
default target. Fresh compiler checks reject a changed case selector. F_28AC
remains held on conversion instruction selection; see
[the wave-seventeen proof](matching-c-wave17.md).

## Eighteenth local C wave

F_929E and F_950C add 574 matching C bytes. Stack-local ordering and
predecrement argument evaluation reproduce the original instructions. F_4517
is byte-equal in a probe but held on three missing storage-base declarations.
See [the wave-eighteen proof](matching-c-wave18.md).

## Nineteenth local C wave and library data

F_A525 adds 307 matching C bytes. The complete pinned CTYPE data segment
(257 bytes) and a nine-byte string resolve two previously missing bindings.
Library coverage now separates 4,267 code bytes from 257 data bytes. See
[the library-data proof and limits](matching-c-wave19.md).

## Twentieth local C wave and refreshed held inventory

F_F9BE reproduces the complete 49-byte TOUPPER routine directly from C.
Verified CTYPE data resolves its former library-candidate linkage blocker.
The historical held snapshot now excludes exact owned extents under different
IDs as well as owned IDs. See [the wave-twenty proof](matching-c-wave20.md).

## Twenty-first local C wave

F_A09D, F_AD25 and F_A13F add 363 matching C bytes. Independent DS-base
passing and scaled-index access corroborate DS:C360 without claiming full BSS
ownership. Promotion and full builds enforce this evidence. See
[the wave-twenty-one proof](matching-c-wave21.md).

## Twenty-second local C wave

F_ADCF adds 374 matching C bytes, preserving complete record copies and
explicit byte and word fields. It reuses independently verified DS:C360 base
evidence. See [the wave-twenty-two proof](matching-c-wave22.md).

## Twenty-third local C wave

F_86C9 adds 321 matching C bytes, including nested switches and complete
sparse selector/target tables. Its argument accesses reproduce exact field
offsets without claiming a complete source record format. See
[the wave-twenty-three proof](matching-c-wave23.md).

## Twenty-fourth local C wave

F_969D and F_90A6 add 866 matching C bytes. Four matrix traversals preserve
record layout and selector tables; placing a constant offset before indexing
reproduces the original addressing instruction. See
[the wave-twenty-four proof](matching-c-wave24.md).

## Twenty-fifth local C wave

F_8480 and F_7BFC add 990 matching C bytes. Exact pointer conversion supplies
the DS segment, while the 20-byte structure copy uses the pinned SCOPY public.
Both complete extents and negative controls are freshly compiled. See
[the wave-twenty-five proof](matching-c-wave25.md).


## Twenty-seventh local C wave

F_A768 adds 246 matching C bytes. Its historical switch-table case order and DS:13D9 record binding are proven by fresh OMF comparison. See [the wave-twenty-seven proof](matching-c-wave27.md).

## Twenty-eighth local C wave

F_21DB adds 113 matching C bytes. Its complete staging-buffer loader binds the
historical `_malloc` and `movmem` library publics and proves the DS:96EE
674-byte runtime buffer through independent read/write observations. See [the
wave-twenty-eight proof](matching-c-wave28.md).

## Twenty-ninth local C wave

F_2269 adds 72 matching C bytes. Its cursor blitter consumes the independently
proved DS:96EE buffer and reproduces both blit paths and the 0x2A2-byte stride.
See [the wave-twenty-nine proof](matching-c-wave29.md).

## Thirtieth local C wave

F_A658 adds 272 matching C bytes and its one-byte compiled initializer at
DS:13C5. The `_DATA` segment is owned and bound as part of the same source
module. See [the wave-thirty proof](matching-c-wave30.md).

## Thirty-first local C wave

F_622C and F_625D add 58 matching C bytes. The critical-error handler and its
installer bind to exact helper/library entries, while DS:C0C8 byte storage is
independently observed as read and write. See [the wave-thirty-one proof](matching-c-wave31.md).

## Thirty-second local C wave

F_9B68 adds 529 matching C bytes. Its DS:0DCC fixed-record table and DS:1271
word table now have independent encoders and exact ownership. See [the
wave-thirty-two proof](matching-c-wave32.md).

## Thirty-third local C wave

F_338A adds 870 matching C bytes. Its four animation buffers are bound to
independently corroborated DS offsets, and its indexed DS:B3AF record store is
derived from the established DS:B3AE count base. See [the wave-thirty-three
proof](matching-c-wave33.md).

## Thirty-fourth local C wave

F_2AE2 adds 1,762 matching C bytes. Its board redraw and all direct DGROUP
references now rebuild from the recovered C source with the original `-B`
compiler flag. See [the wave-thirty-four proof](matching-c-wave34.md).

## Thirty-fifth local C wave

F_5AC3 adds 981 matching C bytes. Its beam walk, collision state and
`SCOPY@` library helper now rebuild from the recovered source. See [the
wave-thirty-five proof](matching-c-wave35.md).

## Thirty-sixth local C wave

F_A85E adds 449 matching C bytes. Its three pooled string references now bind
to the original DS data addresses, allowing the complete routine to rebuild
without a fabricated local `_DATA` segment. See [the wave-thirty-six proof](matching-c-wave36.md).

## Thirty-seventh local C wave

F_3A75 adds 2,722 matching C bytes. Its complete turn loop now rebuilds with
the original `-B` flag and all 81 external fixups explicitly bound. See [the
wave-thirty-seven proof](matching-c-wave37.md).

## Thirty-eighth local C wave

F_21A9 and F_233E add 512 matching C bytes. Their boot sprite buffers and
cell table now bind to exact DS addresses from complete fixups. See [the
wave-thirty-eight proof](matching-c-wave38.md).

## Runtime library ownership wave

ATEXIT, EXIT, IOERROR, OPEN, SETARGV and SETENVP now have explicit CC.LIB
ownership and fresh complete fixups. See [the runtime library wave](runtime-library-wave1.md).

## ASM linkage closure wave

F_4E9F adds 76 freshly assembled bytes. Its two previously undecided DS
references are now bound to the established table addresses, and the call to
F_4AA8 resolves through the owned code entry. The complete TASM object and all
five fixups match; the linkage inventory is now empty. See
[the ASM closure proof](matching-c-wave40-asm.md).

## Forty-seventh matching C wave

F_7932 adds 50 matching C bytes. Its far-pointer dispatch, three owned helper
entries and eight fixups match from a fresh Turbo C object; its default pointer
base remains an explicit numeric DGROUP binding because it targets a raw static
table. See [the wave-forty-seven proof](matching-c-wave47.md).

The remaining byte-equal F_9D8E probe is intentionally still held: DS:125D
falls in the final 20 bytes of an adjacent raw owner, and no identified data
component or lossless source format yet owns those bytes. F_28AC and F_4F96
remain compiler-selection experiments with complete but byte-different C
probes. No opaque binding is promoted merely to increase C coverage.

## Forty-eighth matching C wave

F_9D8E adds 62 matching C bytes. Its source now also emits the complete
20-byte initialized `_DATA` record at DS:125D, including the far pointer to a
209-byte independently encoded text component. The OMF data fixup and its MZ
relocation are bound and verified, so the former raw data-base blocker is
closed without preserving an opaque slice. See [the wave-forty-eight proof](matching-c-wave48.md).

`F_7202` is already canonical C from wave six. The current open audit covers
`F_6CA6` and the wave-forty-nine register/pointer probes; these remain concrete
compiler frontiers alongside the earlier conversion and switch-layout probes.

## Forty-ninth matching-C audit

Fresh Turbo C probes for F_D818, F_C877 and F_D825 did not pass complete-extent equality. F_7202 is already canonical from wave six; the audit records the current register-allocation and pointer-lowering blockers without promoting opaque bytes. See [the wave-forty-nine audit](matching-c-wave49.md).

## Fiftieth matching-C wave

F_CE2A adds 62 matching C bytes after recovering the complete cleanup/return
tail beyond the stale upstream boundary. Its fresh object has 11 bound fixups,
including the pinned `_longjmp` public, and no loader relocations. See [the
wave-fifty proof](matching-c-wave50.md).

## Fifty-first matching-C wave

`F_CDDD` adds 35 matching C bytes after recovering the eight-byte
cleanup/return tail omitted by the upstream boundary. Its fresh object has
four bound fixups, including the already owned `F_86C9` entry and pinned
`_longjmp` public, and no loader relocations. See [the wave-fifty-one
proof](matching-c-wave51.md).

## Fifty-second matching-C wave

`F_CE00` adds 42 matching C bytes by decoding the complete routine between
`F_CDDD` and `F_CE2A`. Its fresh object has five bound fixups, including the
owned `F_86C9` and `F_A13F` entries and pinned `_longjmp` public, and no loader
relocations. See [the wave-fifty-two proof](matching-c-wave52.md).

## Fifty-third matching-C wave

`F_0215` adds 29 matching C bytes for the complete indexed DGROUP store
routine at load offset `0215`. Its fresh object has two bound DGROUP fixups
and no loader relocations. See [the wave-fifty-three proof](matching-c-wave53.md).

## Fifty-fourth matching-C wave

`F_D45C` adds 21 matching C bytes after recovering the complete far-pointer
wrapper and return path following `F_D3DA`. Its fresh object has two bound
fixups, including the owned `F_D3DA` entry, and no loader relocations. See [the
wave-fifty-four proof](matching-c-wave54.md).
