# Empires reconstruction — MVP1

Rebuild `AEPROG.EXE` as fixed, independently owned file ranges. **The complete
79,154-byte EXE matches the original byte-for-byte**, including its MZ header
and relocation table. The build compiles 125 C regions and assembles 20 ASM
regions; 64 raw regions cover everything else.

From this directory, with Python 3.10+:

```powershell
python tools/reconstruct.py
```

This produces `build/AEPROG.EXE` and `build/report.json`, prints coverage and
hashes, and exits nonzero on a failure. Every invocation builds fresh objects.
It needs DOSBox Staging (installed here at
`C:/Program Files/DOSBox Staging/dosbox.exe`) and the three pinned Borland
binaries already installed locally in `toolchain/`. There are no Python
package dependencies and no PortForge dependency at build or game runtime.

Game files, raw byte extracts, compiler binaries, and build output are not
distributed in Git. On a fresh checkout, place your own original `AEPROG.EXE`
in `assets/`, then recreate the raw owners and copy the compiler tools from
the existing local upstream installation once:

```powershell
python tools/extract_raw.py
python tools/setup_toolchain.py
# Alternatively: python tools/setup_toolchain.py --from "X:/your/TC/BIN"
```

The setup and build both verify the hashes in `layout/toolchain.json`.
Compiler binaries are ignored by Git and are not redistributed. Override the
emulator location with `--dosbox PATH` or the `DOSBOX` environment variable;
override the Borland directory with `--toolchain PATH`.

The build follows [layout/manifest.json](layout/manifest.json), the sole
authority for ownership. Ranges use **file offsets, start inclusive and end
exclusive**. Each source owner includes its public-symbol selection, compiler
flags, address bindings, provenance and expected byte digest. `src/` and `asm/`
contain preserved upstream source; `raw/` contains separate exact byte files
for every remaining owner. No source owner is filled from the original EXE.

Turbo C/TASM emit OMF objects in a fresh `build/session-*` directory. The
builder selects each declared public extent, applies its declared address
bindings, verifies its MZ relocation obligations, and concatenates all owners.
The original in `assets/` is the comparison fixture. The MZ header, unimported
code, data, and embedded assets retain raw ownership. `AE000.DAT` and
`AE001.DAT` remain untouched; reconstructing those files is outside MVP1.

```powershell
python -m unittest discover -s tests -v
```

Tests include freshly compiled C and ASM mutations, a wrong symbol binding,
raw corruption, length errors, gaps, overlaps, malformed OMF, relocation
errors, and stale-success invalidation. Compiler logs, bound regions, fixups,
tool identities and per-owner results are retained under `build/` for review.
Failed runs remove the previous published EXE/report. Session directories can
be removed when their evidence is no longer needed.

See [address and layout rules](docs/layout.md),
[upstream inventory](docs/upstream-inventory.md), and
[MVP1 results](docs/mvp1-report.md). The longer-term direction is in
[docs/vision.md](docs/vision.md).
