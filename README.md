# Empires reconstruction

Rebuild `AEPROG.EXE` as fixed, independently owned file ranges. **The complete
79,154-byte EXE matches the original byte-for-byte**, including its MZ header
and relocation table. The build compiles 126 C regions, assembles 20 ASM
regions, and extracts 35 pinned Borland library modules. The MZ header is
encoded from explicit metadata; 66 raw regions cover everything else. Library
and header bytes are reported separately from compiled source.

From this directory, with Python 3.10+:

```powershell
python tools/reconstruct.py
```

This produces `build/AEPROG.EXE` and `build/report.json`, prints coverage and
hashes, and exits nonzero on a failure. Every invocation builds fresh objects.
It needs DOSBox Staging (installed here at
`C:/Program Files/DOSBox Staging/dosbox.exe`) and the three pinned Borland
executables plus `CC.LIB`, installed locally in `toolchain/`. There are no Python
package dependencies and no PortForge dependency at build or game runtime.

Game files, raw byte extracts, compiler binaries, and build output are not
distributed in Git. On a fresh checkout, place your own original `AEPROG.EXE`
in `assets/`, then recreate the raw owners and copy the compiler tools from
the existing local upstream installation once:

```powershell
python tools/extract_raw.py
python tools/setup_toolchain.py
# Alternatively: python tools/setup_toolchain.py --from "X:/your/TC/BIN"
# If libraries are elsewhere, add --lib-from "X:/your/TC/LIB".
```

The setup and build both verify the hashes in `layout/toolchain.json`.
The entire toolchain directory is ignored by Git and is not redistributed. Override the
emulator location with `--dosbox PATH` or the `DOSBOX` environment variable;
override the Borland directory with `--toolchain PATH`.

The build follows [layout/manifest.json](layout/manifest.json), the sole
authority for ownership. Ranges use **file offsets, start inclusive and end
exclusive**. Each source owner includes its public-symbol selection, compiler
flags, address bindings, provenance and expected byte digest. `src/` and `asm/`
contain preserved upstream source; `layout/mz-header.json` contains all header
words, ordered relocations and preserved padding; `raw/` contains separate
exact byte files for the remaining raw owners. No source owner is filled from
the original EXE.

Turbo C/TASM emit OMF objects in a fresh `build/session-*` directory. The
builder selects each declared public extent, applies its declared address
bindings, verifies its MZ relocation obligations, and concatenates all owners.
Library owners use complete `_TEXT` contributions extracted from the pinned
`CC.LIB`, through the same binding and relocation checks. The original in
`assets/` is the comparison fixture. The header is encoded and verified before
compilation; altered fields or padding fail with the `MZ_HEADER` owner named.
Unimported code, data, and embedded assets retain raw ownership. `AE000.DAT` and
`AE001.DAT` remain untouched; reconstructing those files is outside MVP1.

```powershell
python -m unittest discover -s tests -v
```

Tests include freshly compiled C and ASM mutations, library identity/selection
errors, changed library code, wrong symbol bindings, raw corruption, length
errors, gaps, overlaps, malformed OMF, relocation errors, header fields/padding,
relocation ordering, stale-success invalidation, and failed promotions leaving
ownership unchanged. Compiler logs, bound regions, fixups,
tool identities and per-owner results are retained under `build/` for review.
Failed runs remove the previous published EXE/report. Session directories can
be removed when their evidence is no longer needed.

See [address and layout rules](docs/layout.md),
[upstream inventory](docs/upstream-inventory.md), and
[current progress and promotion workflow](docs/progress.md).
[MVP1 results](docs/mvp1-report.md) preserve the original milestone.
The longer-term direction is in
[docs/vision.md](docs/vision.md).
