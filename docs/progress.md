# Incremental reconstruction — 2026-09-18

Full EXE identity is preserved. Follow-ups to MVP1 remove 4,844 bytes from raw
fallback: 4,332 bytes of matching C/library regions and the 512-byte header.
Embedded assets and gameplay semantics remain outside this mechanical work.

| Representation | MVP1 bytes | Current bytes | Current owners |
|---|---:|---:|---:|
| Freshly compiled matching C | 14,077 | 14,142 | 126 |
| Freshly assembled matching ASM | 2,456 | 2,456 | 20 |
| Known toolchain library | 0 | 4,267 | 35 |
| Structured MZ header | 0 | 512 | 1 |
| Exact raw fallback | 62,621 | 57,777 | 66 |
| Total | 79,154 | 79,154 | 248 |

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

Eighteen tests pass, including fresh C/ASM mutations, library/module identity
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

The next useful step is to assess remaining upstream refused regions one at
a time when a missing binding has independent evidence. Runtime-generated
memory must remain distinct from original on-disk bytes; full native linking,
semantic recovery and asset decoding are still separate work.
