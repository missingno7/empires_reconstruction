# Empires reconstruction

Rebuild `AEPROG.EXE`, `AE000.DAT` and `AE001.DAT` as independently owned file
ranges and resources. **All 690,588 bytes across the three files match the
originals exactly**, including executable relocations and archive offsets.
The build compiles 345 C regions and no ASM regions, and extracts 42 pinned
Borland library modules (41 code contributions
and one 257-byte data contribution). The MZ header is
encoded from explicit metadata. Two embedded DAC palettes rebuild from structured
RGB tables (1,536 bytes); sixteen raw regions cover the remaining 5,093 EXE bytes.
See [palette ownership and proof](docs/embedded-palettes.md).
488 code references also resolve through [owned entry publics](docs/code-bindings.md).
Another 75 resolve through [publics read from pinned library modules](docs/library-bindings.md).
The latest [data-structure wave](docs/raw-data-wave140.md) removes 22 bytes
from opaque EXE fallback. The latest [matching-C wave](docs/matching-c-wave130.md)
closed the executable ASM frontier. The latest [runtime ownership wave](docs/runtime-library-wave1.md)
identifies six pinned CC.LIB modules.
The [matching-C frontier audit](docs/matching-c-frontier-audit.md) verifies
that all pinned proven code extents are already owned and no unresolved machine
extent intersects the remaining raw data.
Both archives have explicit resource ownership. Twenty-six compressed resources
re-encode exactly; 25 use PNG plus JSON and one uses a structured image bank. The uncompressed
first level also rebuilds from structured source, for 27 matching resources.
The other 193 retain raw payloads. Library, header, structured and fallback
coverage are reported separately.

This is a matching bootstrap, not yet reconstruction of the complete original
build. The [architectural target](docs/build-reconstruction.md) requires layout
to emerge from independent components and recovered build rules. DAT packing
now derives offsets from component order and emitted sizes; the EXE still uses
fixed placement, and both paths retain explicitly counted opaque fallbacks.
A separate [compression-source experiment](docs/compression-sources.md) rebuilds
both DATs from decoded payloads and explicit compression instructions. It
preserves unresolved parse choices without claiming the historical encoder policy.

From this directory, with Python 3.10+:

```powershell
python tools/reconstruct_game.py
```

This produces all three files, `build/report.json`, `build/archives-report.json`
and `build/game-report.json`, prints coverage and hashes, and exits nonzero on
a failure. Every invocation builds fresh objects. For only the EXE, use
`python tools/reconstruct.py`; for only DATs, use
`python tools/reconstruct_archives.py`.
The whole-game command also checks independently packed DATs against both
the fixed-layout archive output and the original fixtures.
It needs DOSBox Staging (installed here at
`C:/Program Files/DOSBox Staging/dosbox.exe`) and the three pinned Borland
executables plus `CC.LIB`, installed locally in `toolchain/`. There are no Python
package dependencies and no PortForge dependency at build or game runtime.

Game files, raw byte extracts, compiler binaries, and build output are not
distributed in Git. On a fresh checkout, place your own original `AEPROG.EXE`,
`AE000.DAT` and `AE001.DAT` in `assets/`, then recreate the local sources and copy the compiler tools from
the existing local upstream installation once:

```powershell
python tools/extract_raw.py
python tools/reconstruct_archives.py prepare
python tools/setup_toolchain.py
# Alternatively: python tools/setup_toolchain.py --from "X:/your/TC/BIN"
# If libraries are elsewhere, add --lib-from "X:/your/TC/LIB".
```

The setup and build both verify the hashes in `layout/toolchain.json`.
The entire toolchain directory is ignored by Git and is not redistributed.
Override the emulator location with `--dosbox PATH` or the `DOSBOX` environment
variable; override the Borland directory with `--toolchain PATH`.

Once local component sources are prepared, the independent DAT packer can run
without original files or fixed-layout manifests:

```powershell
python tools/pack_archives.py
# Verification is separate and needs the original fixtures:
python tools/pack_archives.py verify --fixed-output build
```

Its component recipes live in `recipes/archives/`; generated offsets, lengths
and receipts live in `build/packed/`. These recipes contain no historical
placement or expected-byte fields. Original files are used only by verification
in this path. The opaque local payload fallbacks are still temporary sources.

The EXE build follows [layout/manifest.json](layout/manifest.json), the sole
authority for executable ownership. DAT manifests live in `layout/archives/`.
Ranges use **file offsets, start inclusive and end
exclusive**. Each source owner includes its public-symbol selection, compiler
flags, address bindings, provenance and expected byte digest. `src/` and `asm/`
contain preserved upstream source; `layout/mz-header.json` contains all header
words, ordered relocations and preserved padding; `raw/` contains separate
exact byte files for the remaining raw owners. No source owner is filled from
the original EXE.

Turbo C/TASM emit OMF objects in a fresh `build/session-*` directory. The
builder selects each declared public extent, applies its declared address
bindings, verifies its MZ relocation obligations, and concatenates all owners.
Library owners use complete `_TEXT` or `_DATA` contributions extracted from the pinned
`CC.LIB`, through the same binding and relocation checks. The original in
`assets/` is the comparison fixture. The header is encoded and verified before
compilation; altered fields or padding fail with the `MZ_HEADER` owner named.
Unimported code, data, and embedded assets retain raw ownership. Archive tables
and type/flag headers are encoded from metadata. Matching payloads are encoded
from local decoded or structured sources; all other payloads retain raw bytes.
All 220 resources decode; 153 payloads pass strict structural round trips,
including standalone/nested bitmaps, banks, fonts and levels. Payload encoding and exact recompression are separate
metrics. See [archive formats and proof boundaries](docs/archive-formats.md).
The [editable image and nested-format pipeline](docs/resource-sources.md)
accepts PNG edits through the same DOS encoders and archive packer. A separate
[shared-compilation experiment](docs/module-group-evidence.md) proves four
adjacent C functions can emit matching relative layout in one OBJ; it does not
yet establish historical module ownership or real linking.

```powershell
python -m unittest discover -s tests -v
```

Tests include freshly compiled C and ASM mutations, library identity/selection
errors, changed library code, wrong symbol bindings, raw corruption, length
errors, gaps, overlaps, malformed OMF, relocation errors, header fields/padding,
relocation ordering, stale-success invalidation, and failed promotions leaving
ownership unchanged. Archive tests also cover malformed tables, empty slots,
trailing bytes, codec truncation, exact historical RLE streams, structured
pixel/level data, clean bootstrap and corrupted-source failures.
Compiler logs, bound regions, fixups,
tool identities and per-owner results are retained under `build/` for review.
Failed runs remove the previous published EXE/report. Session directories can
be removed when their evidence is no longer needed.

See [address and layout rules](docs/layout.md),
[upstream inventory](docs/upstream-inventory.md), and
[current progress and promotion workflow](docs/progress.md).
[MVP1 results](docs/mvp1-report.md) preserve the original milestone.
The active [mechanical reconstruction phase](docs/matching-phase.md) follows
[docs/vision.md](docs/vision.md); [the blocker ledger](docs/blockers.json)
separates concrete blockers from still-open productive frontiers.
