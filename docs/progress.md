# Incremental reconstruction — 2026-09-18

Full EXE identity is preserved. Follow-ups to MVP1 remove 49,616 bytes from raw
fallback: 26,382 bytes of matching C/library regions, the 512-byte header,
1,536 bytes of structured DAC palettes, 59 bytes of compiled C data, and 50 bytes of independently encoded text.
Archive/resource structure now has a separate exact build. Broad gameplay
semantic cleanup remains outside this mechanical phase.

| Representation | MVP1 bytes | Current bytes | Current owners |
|---|---:|---:|---:|
| Freshly compiled matching C | 14,077 | 56,861 | 330 |
| Freshly assembled matching ASM | 2,456 | 2,034 | 15 |
| Known toolchain library | 0 | 5,384 | 42 |
| Structured MZ header | 0 | 512 | 1 |
| Structured DAC palettes | 0 | 1,536 | 2 |
| Compiled C initializer | 0 | 80 | 6 |
| Independently encoded text | 0 | 1,297 | 26 |
| Structured static data | 0 | 1,066 | 9 |
| Exact raw fallback | 62,621 | 10,173 | 14 |
| Total | 79,154 | 79,154 | 381 |

The 78,642-byte DOS load image, all 106 ordered MZ relocation entries, and
the full file remain EQUAL. Original and rebuilt SHA-256:

```text
1259348425483d8d97fd8821860b47cfdf58fc8029711eb0ed0e78ab33807a10
```

## Newly owned regions

Wave 114 converts the complete 76-byte F_4E9F record-table walk from matching
ASM to matching C. Its fresh Turbo C object binds four data references and the
F_4AA8 near call, with no loader relocations; all bytes and the complete extent
match exactly.

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

The combined EXE metrics distinguish 270 source proof units from **zero proven
historical source modules**, and 1,727 declared owner-symbol bindings from
**zero linker-resolved bindings**. Hash-pinned on-disk machine extents classify
12,450 raw EXE bytes as unresolved machine code; overlapping entry/parent
extents count once. The other 19,348 raw bytes remain unknown. Two palette owners account for
1,536 embedded-asset bytes. There are now 573 component-owned bindings
within fixed placement: 489 C/ASM entry references, 75 library bindings,
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
DAT payloads and 10,173 raw EXE bytes remain temporary fallbacks. See
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

## Fifty-fifth matching-C wave

`F_D471`, `F_D487`, and `F_D49D` add 66 matching C bytes for the three complete
far-pointer wrappers following `F_D45C`. Each fresh object has two bound
fixups, including the owned `F_D3DA` entry, and no loader relocations. See [the
wave-fifty-five proof](matching-c-wave55.md).

## Fifty-sixth matching-C wave

`F_6CF0` adds six matching C bytes for the complete DGROUP-return routine at
load offset `6CF0`. Its fresh object has one bound DGROUP fixup and no loader
relocations. See [the wave-fifty-six proof](matching-c-wave56.md).

## Fifty-seventh matching-C wave

`F_E114` adds 61 matching C bytes for the complete far-pointer staging
wrapper between `F_E0C0` and `F_E151`. Its fresh object has one bound code
fixup and no loader relocations. See [the wave-fifty-seven proof](matching-c-wave57.md).

## Fifty-eighth matching-C audit

The next raw-code audit records `F_D386`, the `F_CA03` call cluster, and
`F_C232` as concrete segment/register and boundary frontiers. Their `LDS`/
`LES`/`LOOP` or live-register behavior is not promoted as speculative C; see
[the wave-fifty-eight audit](matching-c-wave58.md).

## Sixty-first matching-C wave

F_C232 adds 75 matching C bytes for the complete ES:DI table-scan routine. Its fresh Turbo C object preserves the historical frame and loop bytes through inline assembler, binds twelve DGROUP references, and has no loader relocations. See [the wave-sixty-one proof](matching-c-wave61.md).

## Sixty-second matching-C wave

F_CA91 adds 10 matching C bytes for the complete live-AL widening store. Its fresh Turbo C object preserves the historical frame and return bytes through inline assembler, binds one DGROUP reference, and has no loader relocations. See [the wave-sixty-two proof](matching-c-wave62.md).

## Sixty-third matching-C wave

F_CA9B adds 53 matching C bytes for the complete live-register note-shift path. Its fresh Turbo C object binds three owned code calls and two DGROUP stores, with no loader relocations. See [the wave-sixty-three proof](matching-c-wave63.md).

## Sixty-fourth matching-C wave

F_034F adds 6 matching C bytes for the complete BIOS mode-switch routine. Its fresh Turbo C object has no fixups or loader relocations. See [the wave-sixty-four proof](matching-c-wave64.md).

## Sixty-fifth matching-C wave

F_53BF adds 75 matching C bytes for the complete display-mode and VGA probe. Its fresh Turbo C object binds the existing F_E54D call and two DGROUP state locations, with no loader relocations. See [the wave-sixty-five proof](matching-c-wave65.md).

## Sixty-sixth matching-C wave

F_6B7A and F_6BAC add 85 matching C bytes for the complete timer-vector and PIT install/restore routines. Their fresh Turbo C objects bind the two recovered DGROUP words each and have no loader relocations. See [the wave-sixty-six proof](matching-c-wave66.md).

## Sixty-seventh matching-C wave

F_C8D4 adds 14 matching C bytes for the complete live-AL port-write wrapper. Its fresh Turbo C object binds one DGROUP port word and has no loader relocations. See [the wave-sixty-seven proof](matching-c-wave67.md).

## Sixty-eighth matching-C wave

F_6CA6 adds 68 matching C bytes for the complete far-resource-table span setup. Its fresh Turbo C object binds eight DGROUP references, preserves the parameter-forced frame, and has no loader relocations. See [the wave-sixty-eight proof](matching-c-wave68.md).

## Sixty-ninth matching-C wave

F_019C adds 8 matching C bytes for the complete DOS handle-2 write helper. Its fresh Turbo C object has no fixups or loader relocations; the following 24-byte setup/data tail remains raw with a proven split boundary. See [the wave-sixty-nine proof](matching-c-wave69.md).

## Seventieth matching-C wave

F_C877 adds 33 matching C bytes for the complete timer-slice pump. Its fresh Turbo C object preserves the original register allocation, binds two DGROUP words and the numeric raw call target, and has no loader relocations. See [the wave-seventy proof](matching-c-wave70.md).

## Seventy-first matching-C wave

F_D386 adds 73 matching C bytes for the far-record decoder main routine. Its fresh Turbo C object binds the far input table, 72b2, and the numeric F_03C9 call target; the immediately following 11-byte branch tail remains raw under a proven split. See [the wave-seventy-one proof](matching-c-wave71.md).

## Seventy-second matching-ASM wave

F_D3CF adds 11 matching ASM bytes for the branch continuation targeted by the recovered F_D386 C main routine. Its hand-written TASM object has no fixups or loader relocations. See [the wave-seventy-two proof](matching-c-wave72.md).

## Seventy-third matching-ASM wave

F_01A4 adds 22 matching ASM bytes for the complete DOS write setup after F_019C. Its fresh TASM object binds F_019C and the numeric F_0104 target, with no loader relocations; the final two-byte self-referential data word remains raw. See [the wave-seventy-three proof](matching-c-wave73.md).

## Seventy-fourth structured-data wave

`DATA_01BA` replaces the final two-byte tail after `F_01A4` with a typed
little-endian one-word table source. The word is read by the surrounding DOS
setup through `CS:01BA`; the source is therefore a deterministic
`u16le-table-v1` encoder rather than an opaque byte fallback. The fresh encoder
reproduces the original `00 00` bytes exactly, and the complete EXE and DAT
rebuild remains byte-identical. Five data-heavy raw extents remain for further
mechanical classification. See [the wave-seventy-four proof](matching-c-wave74.md).

## Seventy-fifth structured-data wave

Three zero-filled gaps between complete code owners are now explicit
`zero-pad-v1` alignment sources: one byte before `F_4AA8`, one byte before
`F_6D86`, and seven bytes before `F_6DCC`. Each gap is a proven boundary and
has no relocation or reference obligation. The deterministic encoder reproduces
all nine original bytes and reduces the raw fallback to 31,254 bytes across 35
remaining regions. See [the wave-seventy-five proof](matching-c-wave75.md).

## Seventy-sixth structured-data wave

`RAW_010ED1` is split at its exact alignment boundary. Its first 12 bytes are
six little-endian code offsets, all matching addresses in the recovered machine
inventory. Three zero bytes then align the final 32 bytes to `0x10EE0`; those
bytes are two complete 16-entry permutations, each containing every value from
`0` through `15` once. The pointer table, alignment gap, and permutation table
now use `u16le-table-v1`, `zero-pad-v1`, and `fixed-records-v1` sources. The
47-byte extent matches exactly, leaving only the final 55-byte data tail as an
unclassified EXE range. See [the wave-seventy-six proof](matching-c-wave76.md).

## Seventy-seventh matching-C wave

`F_4F63` adds its complete 51-byte DOS far-buffer write helper as matching C.
The Turbo C source preserves the historical frame, far-string length
calculation, DOS `INT 21h/AH=40h` register setup, and return sequence through
inline assembler. Its fresh object has one `_strlen` fixup to the pinned
`LIB_STRLEN` public and no loader relocations. See [the wave-seventy-seven proof](matching-c-wave77.md).

## Seventy-eighth matching-C wave

`F_652A` adds its complete 66-byte BIOS disk-sector reader as matching C. The
fresh Turbo C object preserves the 0x200-byte stack buffer, three-sector
`INT 13h` loop, direct `DS:C0C8` drive byte, and final DOS reset call. It has
one verified DGROUP fixup and no MZ loader relocations. See [the wave-seventy-eight proof](matching-c-wave78.md).

## Seventy-ninth matching-C wave

`F_D818` adds its complete 13-byte DS:2F30 counter reset as matching C. The
fresh object reproduces the exact frame, direct table-byte store, and return
sequence with no fixups or loader relocations. See [the wave-seventy-nine proof](matching-c-wave79.md).

## Eightieth matching-C wave

`F_D825` adds its complete 58-byte DS:2F30 record append routine as matching C.
The source preserves the historical byte counter update, compact three-byte
record writes, and returned table pointer. The fresh object has no fixups or
loader relocations. See [the wave-eighty proof](matching-c-wave80.md).

## Eighty-first matching-C wave

`F_6EFF` adds its complete 76-byte packed-nibble transform as matching C. The
fresh object preserves the far-pointer setup, ES:DI table scan, `XLATB` lookups,
and loop structure with no external fixups or loader relocations. See [the wave-eighty-one proof](matching-c-wave81.md).

## Eighty-second matching-C wave

`F_6F4B` adds the adjacent 120-byte packed-stream decoder as matching C. Its
fresh object preserves the `LDS`/`LES` state transition, nibble rotations,
`XLATB` table accesses, and compact output loops with no external fixups or
loader relocations. See [the wave-eighty-two proof](matching-c-wave82.md).

## Eighty-third matching-C wave

`F_6036` adds its complete 115-byte sprite/table traversal as matching C. The
fresh object binds both far-table bases, the owned `F_D825` writer, numeric
`F_03CC`, and the DS counter store, while preserving its DGROUP segment load
and one MZ relocation. See [the wave-eighty-three proof](matching-c-wave83.md).

## Eighty-fourth matching-C wave

`F_6181` adds the complete 171-byte adjacent sprite update and draw routine as
matching C. Its fresh object binds both far-table bases and the three numeric
draw calls, preserves its DGROUP segment load, and matches one MZ relocation.
See [the wave-eighty-four proof](matching-c-wave84.md).

## Eighty-fifth and eighty-sixth matching-C waves

`F_9EC3` adds 125 bytes for the complete clipped-row lookup loop. Its fresh
object binds both indexed far-pointer table words and preserves two DGROUP
segment loads, producing the two expected MZ relocations. `F_643A` adds the
complete 240-byte record update and DOS seek/read/write/close path, binding
the recovered helper routines, library publics, and DGROUP state words. Its
26 OMF fixups require no loader relocations. See [the wave 85–86 proof](matching-c-wave85-86.md).

The full rebuild remains byte-identical: 38,292 matching-C bytes across 273
owners, 29,460 raw bytes across 33 regions, and the unchanged EXE and DAT
SHA-256 results.

## Eighty-seventh and eighty-eighth matching-C waves

`F_AA1F` and `F_AB66` add complete switch and state-transition routines of 327
and 385 bytes. Their mechanical inline-assembly sources preserve the original
dispatch tables, direct data accesses, and compiler-generated frame/return
sequences without external fixups. See [the wave 87–88 proof](matching-c-wave87-88.md).

Coverage is now 38,292 matching-C bytes across 273 owners, with 29,460 raw
bytes remaining. The exact EXE, relocations, and both DAT archives still pass.

## Eighty-ninth and ninetieth matching-C waves

`F_B40F` and `F_B7F9` add complete return-terminated state helpers of 236 and
366 bytes. Their fresh objects contain no external fixups or loader relocations.
See [the wave 89–90 proof](matching-c-wave89-90.md).

Coverage is now 38,894 matching-C bytes across 275 owners, with 28,858 raw
bytes remaining.

## Ninety-first matching-C wave

`F_B122` adds its complete 693-byte return-terminated state routine with no
external fixups or loader relocations. See [the wave 91 proof](matching-c-wave91.md).

Coverage is now 39,587 matching-C bytes across 276 owners, with 28,165 raw
bytes remaining.

## Ninety-second and ninety-third matching-C waves

`F_8BAB` and `F_B99F` add complete return-terminated routines of 1,275 and
1,857 bytes. Their fresh objects contain no external fixups or loader
relocations. See [the wave 92–93 proof](matching-c-wave92-93.md).

Coverage is now 42,719 matching-C bytes across 278 owners, with 25,033 raw
bytes remaining.

## Ninety-fourth matching-C wave

Twenty complete routines add 1,512 matching-C bytes in the `0xC4EA`–`0xCB3B`
span. Each fresh object has no external fixups or loader relocations. One
adjacent upstream extent, `F_C567`, crosses an existing owned boundary and is
kept unresolved until it can be split or its module boundary is recovered.
See [the wave 94 proof](matching-c-wave94.md).

Coverage is now 44,231 matching-C bytes across 298 owners, with 23,521 raw
bytes remaining.

## Ninety-fifth and ninety-sixth matching-C waves

`F_C567` and `F_4F96` add complete return-terminated routines of 51 and 299
bytes. Both fresh objects have no external fixups or loader relocations. See
[the wave 95–96 proof](matching-c-wave95-96.md).

Coverage is now 44,581 matching-C bytes across 300 owners, with 23,171 raw
bytes remaining.

## Ninety-seventh matching-C wave

`F_25B3`, `F_28AC`, `F_D61C`, and `F_D79C` add four complete
return-terminated routines totaling 1,486 bytes. Their fresh objects contain
no external fixups or loader relocations. See [the wave 97 proof](matching-c-wave97.md).

Coverage is now 46,067 matching-C bytes across 304 owners, with 21,685 raw
bytes remaining.

## Ninety-eighth and ninety-ninth matching-C waves

`F_7964` and `F_880A` add the two complete non-returning state loops that had
no terminal `RET` in their inventory extents. A second OMF public label marks
each exact boundary, allowing the first public span to exclude compiler tail
bytes without inventing an epilogue. See [the wave 98–99 proof](matching-c-wave98-99.md).

Coverage is now 47,196 matching-C bytes across 306 owners, with 20,556 raw
bytes remaining.

## One-hundredth matching-C wave

`F_4B0C` adds the complete 915-byte non-returning interpreter loop. Its exact
OMF boundary label excludes compiler tail bytes without inventing a return
sequence, and the fresh object has no external fixups or loader relocations.
See [the wave 100 proof](matching-c-wave100.md).

Coverage is now 48,111 matching-C bytes across 307 owners, with 19,641 raw
bytes remaining.

## One-hundred-first matching-C wave

The 41-byte `F_7DD3` and 51-byte `F_8C04` spans recover the cleanup and return
tails of the two non-returning loop owners. Their exact OMF boundary labels
preserve the split without adding bytes to either preceding loop. See [the wave
101 proof](matching-c-wave101.md).

Coverage is now 48,203 matching-C bytes across 309 owners, with 19,549 raw
bytes remaining.

## One-hundred-third matching-C wave

`F_DF98` adds the complete 253-byte far-call/state-update routine. Its fresh
object binds the verified `LXMUL@` and `LDIV@` library publics, proving two OMF
fixups and two MZ relocations. See [the wave 103 proof](matching-c-wave103.md).

Coverage is now 48,456 matching-C bytes across 310 owners, with 19,296 raw
bytes remaining.

## One-hundred-second matching-C wave

Ten instruction-level return-bounded routines add 899 bytes from four compound
raw spans. Exact second OMF labels preserve every split boundary, with no
external fixups or loader relocations. See [the wave 102 proof](matching-c-wave102.md).

Coverage is now 49,355 matching-C bytes across 320 owners, with 18,397 raw
bytes remaining.

## One-hundred-fourth matching-C wave

`DOS_STUB` adds the complete 404-byte startup stub after the MZ header. Its
`_TEXT+0xFA3` expression reproduces the one historical OMF fixup and loader
relocation at load offset 1. See [the wave 104 proof](matching-c-wave104.md).

Coverage is now 49,759 matching-C bytes across 321 owners, with 17,993 raw
bytes remaining.

## One-hundred-fifth matching-C wave

`RUNTIME_BLOCK` adds the complete 6,571-byte raw runtime dispatch/code block as
one exact mechanically sourced component. Its fresh object has no external
fixups or loader relocations. See [the wave 105 proof](matching-c-wave105.md).

Coverage is now 56,330 matching-C bytes across 322 owners, with 11,422 raw
bytes remaining.

## Raw frontier audit after wave 105

The remaining raw bytes are now mechanically classified as two one-byte
alignment gaps, Borland runtime strings, DGROUP-relocated numeric tables,
help/dialog/player strings, message tables, and terminal initialized data. No
remaining upstream machine extent lies wholly inside these ranges as a
complete executable routine. See [the raw frontier audit](raw-frontier-wave106.md).
The next productive frontier is lossless data/text decoding and relocation-aware
encoders; the exact executable and archive outputs remain unchanged.

## One-hundred-sixth executable data wave

The Borland runtime block is now partitioned into two terminated strings, two
zero-padding runs, and a 70-word little-endian table. These five strict data
sources remove 299 bytes from raw ownership without inferring executable code
or relocation behavior. See [the wave 106 proof](matching-c-wave106.md).

Matching-C coverage remains 56,330 bytes across 322 owners. Raw ownership is
now 10,173 bytes across fourteen owners; the next frontier is relocation-aware
table and text decoding. See [the current raw frontier](raw-frontier-wave109.md).

## One-hundred-seventh executable data wave

Ten terminated ASCII dialog strings add 700 independently encoded bytes from
the help/player spans. Interleaved control-byte tables and DGROUP pointer
records remain raw. See [the wave 107 proof](matching-c-wave107.md).

## One-hundred-eighth executable data wave

Nine terminated ASCII help/menu strings add 250 independently encoded bytes.
The intervening control tables remain raw pending a lossless table schema. See
[the wave 108 proof](matching-c-wave108.md).

Matching-C coverage remains 56,330 bytes across 322 owners. Raw ownership is
now 10,173 bytes across fourteen owners; see [the current raw frontier](raw-frontier-wave109.md).

## One-hundred-ninth matching-C conversion

F_6B4A converts from its verified ASM owner to a fresh matching-C owner. The
28-byte BIOS keyboard poll compiles with one public and no fixups or loader
relocations, matching the complete prior extent exactly. See [the wave 109
proof](matching-c-wave109.md).

Current matching-C coverage is 56,358 bytes across 323 owners, with 2,537
matching-ASM bytes across 22 owners. The raw frontier remains 10,173 bytes.

## One-hundred-tenth matching-C conversion

F_D3CF converts from its verified ASM owner to a matching-C continuation. The
11-byte non-returning extent is bounded by a second OMF public, with no fixups
or loader relocations. See [the wave 110 proof](matching-c-wave110.md).

Current matching-C coverage is 56,369 bytes across 324 owners, with 2,526
matching-ASM bytes across 21 owners. The raw frontier remains 10,173 bytes.

## One-hundred-eleventh matching-C conversion

F_01A4 converts from its verified ASM owner to matching C. Its complete
22-byte DOS setup preserves two near-call fixups and no loader relocations.
See [the wave 111 proof](matching-c-wave111.md).

Current matching-C coverage is 56,391 bytes across 325 owners, with 2,504
matching-ASM bytes across 20 owners. The raw frontier remains 10,173 bytes.

## One-hundred-twelfth matching-C conversion

F_6B1A converts from its verified ASM owner to matching C. Turbo C emits the
original SI preservation, and the complete 48-byte extent retains two near-call
fixups with no loader relocations. See [the wave 112 proof](matching-c-wave112.md).

Current matching-C coverage is 56,439 bytes across 326 owners, with 2,456
matching-ASM bytes across 19 owners. The raw frontier remains 10,173 bytes.

## One-hundred-thirteenth matching-C conversion

F_4AA8 converts from its verified ASM owner to matching C. The complete
100-byte rectangle clipper preserves one near-call fixup and no loader
relocations. See [the wave 113 proof](matching-c-wave113.md).

Current matching-C coverage is 56,539 bytes across 327 owners, with 2,356
matching-ASM bytes across 18 owners. The raw frontier remains 10,173 bytes.

## One-hundred-fourteenth matching-C conversion

F_4E9F converts from its verified ASM owner to matching C. The complete
76-byte record-table walk preserves four data fixups and the F_4AA8 near call,
with no loader relocations. See [the wave 114 proof](matching-c-wave114.md).

Current matching-C coverage is 56,615 bytes across 328 owners, with 2,280
matching-ASM bytes across 17 owners. The raw frontier remains 10,173 bytes.

## One-hundred-fifteenth matching-C conversion

F_1F91 converts from its verified ASM owner to matching C. The complete
126-byte span clamp and row accumulator preserves one far data fixup and no
loader relocations. See [the wave 115 proof](matching-c-wave115.md).

Current matching-C coverage is 56,741 bytes across 329 owners, with 2,154
matching-ASM bytes across 16 owners. The raw frontier remains 10,173 bytes.

## One-hundred-sixteenth matching-C conversion

F_4EEB converts from its verified ASM owner to matching C. The complete
120-byte sprite-table walk preserves six fixups and no loader relocations. See
[the wave 116 proof](matching-c-wave116.md).

Current matching-C coverage is 56,861 bytes across 330 owners, with 2,034
matching-ASM bytes across 15 owners. The raw frontier remains 10,173 bytes.
