# Incremental reconstruction — 2026-09-18

Full EXE identity is preserved. Follow-ups to MVP1 remove 6,380 bytes from raw
fallback: 4,332 bytes of matching C/library regions, the 512-byte header,
and 1,536 bytes of structured DAC palettes.
Archive/resource structure now has a separate exact build. Broad gameplay
semantic cleanup remains outside this mechanical phase.

| Representation | MVP1 bytes | Current bytes | Current owners |
|---|---:|---:|---:|
| Freshly compiled matching C | 14,077 | 14,142 | 126 |
| Freshly assembled matching ASM | 2,456 | 2,456 | 20 |
| Known toolchain library | 0 | 4,267 | 35 |
| Structured MZ header | 0 | 512 | 1 |
| Structured DAC palettes | 0 | 1,536 | 2 |
| Exact raw fallback | 62,621 | 56,241 | 67 |
| Total | 79,154 | 79,154 | 251 |

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
| Structured payloads round-tripped | 67 | 86 |
| Canonical matching resources | 26 | 1 |
| Canonical matching resource bytes (including headers) | 3,773 | 26,138 |
| Exact recompressed payload bytes | 3,721 | 0 |
| Structured canonical sources | 26 | 1 |
| Remaining raw resources | 63 | 130 |

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

The combined EXE metrics distinguish 146 source proof units from **zero proven
historical source modules**, and 787 declared owner-symbol bindings from
**zero linker-resolved bindings**. Hash-pinned on-disk machine extents classify
34,965 raw EXE bytes as unresolved machine code; overlapping entry/parent
extents count once. The other 21,276 raw bytes remain unknown. Two palette owners account for
1,536 embedded-asset bytes. There are now 191 component-owned bindings
within fixed placement: two palette references, 151 C/ASM entry references,
and 38 library public references. See [embedded-palettes.md](embedded-palettes.md).

[linkage-blockers.json](linkage-blockers.json) snapshots 27 held upstream
candidates (10,488 extent bytes): 24 linkage refusals and three byte differences,
with 69 undecided symbols at 118 fixup sites. The shared `_b437a`, `_b4380`,
and `_b4386` symbols each affect two candidates covering 4,484 bytes; these
candidate sets overlap, so the leverage counts must not be added. This is
historical evidence, not a fresh match grant. `tools/inventory_linkage.py`
refreshes the ranking without modifying upstream.

Forty-eight tests pass, including independent hand-authored codec streams,
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
DAT payloads and 56,241 raw EXE bytes remain temporary fallbacks. See
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

151 code bindings across 75 callers now identify selected entry publics of
92 owned C/ASM components. Migration requires both the established address
and the selected public name to agree, followed by a fresh complete EXE match.
Near and far fixups resolve through owners. The remaining numeric bindings,
fixed placement and zero historical-linker coverage remain explicit.
See [code-bindings.md](code-bindings.md).

## Owned library references

38 additional references across 23 callers now resolve through publics read
from 17 pinned library modules. Eleven target nonzero module-relative offsets.
Public offsets are read from OMF at build time, while module placement remains
fixed. Full EXE bytes and relocations remain EQUAL. See
[library-bindings.md](library-bindings.md).
