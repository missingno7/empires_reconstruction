# Empires reconstruction

A byte-identical reconstruction of AEPROG.EXE from Turbo C 2.0 / TASM 1.0
sources: `src/` holds the game's C translation units, `asm/` the hand-written
assembler modules (sound driver, sprite/tile blitters, decoders, draw queue) and
the runtime block, `include/` the shared interfaces.  71 modules link in one
TLINK invocation to the original SHA with all 106 relocations in order.
Which functions were compiled together is itself proven against the binary --
see [translation-unit structure](docs/current/tu-structure.md) -- and the
remaining work is tracked in [the closure frontier](docs/current/closure-frontier.md)
and [ASM provenance](docs/current/asm-provenance.md).

The runtime-block recovery workflow is still available: start with
[the grinder instructions](docs/current/grinder-instructions.md),
[generated status](docs/current/status.json), and
[the ranked queue](docs/current/grinder-queue.json).

## Portable Windows build (SDL3)

Branch `portable-sdl3` carries a behavior-preserving native port under
`portable/`: C17 game logic (every historical translation unit and all ten
assembler modules reimplemented as typed C), an 8-bpp software renderer
that reproduces the VGA overlay's primitives, the 236.7 Hz fixed-step
timer, a BIOS-shaped keyboard service, the SOUND.ASM state machine driving
Nuked OPL3 and a PC-speaker synthesizer, and SDL3 for window/input/audio.
It needs only CMake, MSVC, git and the original `AE000.DAT`/`AE001.DAT`:

```powershell
cmake -S . -B build-portable -G "Visual Studio 18 2026" -A x64
cmake --build build-portable --config Release
build-portable\portable\platform\sdl3\Release\empires.exe --assets assets
ctest --test-dir build-portable -C Release
```

Settings (music/sound volume, fullscreen, frame interpolation, data
directories) persist in `empires.json` next to the executable, written
with the defaults on first run.  See
[docs/portable/architecture.md](docs/portable/architecture.md) (the
contract), [docs/portable/build.md](docs/portable/build.md) (build, run,
configuration, tests) and `docs/portable/*.md` for the inventories and audits.  The
historical build above stays the oracle: its decoders, graphics primitives
and sound driver are certified against the real 8086 code (MS-DOS Player /
Unicorn fixtures under `portable/tests/fixtures`).

```powershell
# Install the analysis-only decoder into the local ignored build directory.
python -m pip install --no-user --target build/python-deps -r requirements-factory.txt

# Refresh authoritative reports and select the highest-priority CHEAP task.
python tools/reconstruction_factory.py refresh
python tools/reconstruction_factory.py next

# Use the exact commands in the selected candidate card.
python tools/check_candidate.py RUNTIME_BLOCK:START-END
python tools/check_candidate.py RUNTIME_BLOCK:START-END --promote
```

The working runtime source is [asm/RUNTIME_BLOCK.ASM](asm/RUNTIME_BLOCK.ASM),
assembled directly by TASM 1.0. Its former inline-ASM C source is kept as a
frozen oracle under `recovery/src/`. FAST measures emitted source ranges, checks all runtime bytes, publics
and ordered fixups, rejects edits outside the card, and requires fewer unresolved
bytes. Recursive CFG analysis leaves ambiguous code/data and indirect edges for
supervision. [Tested TASM rules](docs/current/tasm-reconstruction-rules.md) and
structured failure hints prevent repeated compiler investigations.

The production build consumes [one explicit module plan](layout/production-plan.json)
whose grouped modules come from `recipes/modules/*.json` listed in `tools/production_plan.py`:
final C/ASM modules, source DATA, partitioned BSS, then **one TLINK invocation**.
It checks the full original SHA and all 106 relocations in exact order.

```powershell
# Fresh, uncached ACCEPTANCE (also compares the locally supplied fixture).
python tools/build_exe.py verify

# Fixture-independent construction; exact SHA and relocation checks still apply.
python tools/build_exe.py --no-verify

# Research only: cache unchanged compiler/assembler objects.
python tools/build_exe.py --research

# Infrastructure and actual pinned-tool regression tests.
python -m unittest discover -s tests -p test_factory.py
python -m unittest discover -s tests -p test_tasm_reconstruction_rules.py
python -m unittest discover -s tests -p test_build_exe.py
```

Grouped modules live in named files (`tools/merge_module.py`); standalone files are
renamed with `tools/rename_source.py`. `python tools/probe_tu.py FIRST LAST` compiles a
contiguous run of modules as one translation unit (or assembles one TASM candidate
with `--asm`), `tools/tu_recipe.py` turns a proven run into a group recipe, and
`python tools/audit_tu_flags.py --strict` keeps every -B/-k flag explained by inline
asm in the same unit. `python tools/probe_module.py OWNER [--source OWNER=alt.C]` compiles single
modules and compares their code and native DATA bytes with the original; use it
before any declaration or header change. Shared interfaces live in `include/`; `python tools/rename_symbol.py old=new`
renames symbols everywhere and records the original address name in
`docs/current/symbol-names.json`.
Acceptance never consumes cached source objects or old probe receipts. Failed
builds invalidate published success. Promotion refreshes the queue; review and
commit the bounded edit after ACCEPTED. A blocker command archives and restores
an isolated failed attempt, records its diagnostic, and moves it to supervisor
review. See [supervisor instructions](docs/current/supervisor-instructions.md)
and [the ABI/type census](docs/current/interface-conflicts.json).

Supply your own original game files in `assets/` and the pinned historical tools:

```powershell
python tools/setup_toolchain.py --from "C:/TC/BIN" --lib-from "C:/TC/LIB" --linker-from "C:/TC/BIN/TLINK.EXE"
python tools/extract_raw.py
python tools/reconstruct_archives.py prepare
```

`layout/toolchain.json` pins Turbo C 2.0, TASM 1.0, TLINK 2.0, CC.LIB and C0C.OBJ.
The DOS runner is MS-DOS Player (the nmlgc ReC98 fork for Turbo C and TASM,
the upstream i86 build for TLINK); DOSBox Staging is the reference backend for
parity checks. See [runner setup](docs/dos-runner.md). Game binaries and toolchain files are not
redistributed. The Python decoder is only needed for the recovery factory, not
for canonical EXE construction. Archive reconstruction remains available with
`python tools/reconstruct_game.py` and `python tools/pack_archives.py verify`.

Only `docs/current/` is authoritative current status. Older checkpoints and wave
reports are retained as [historical evidence](docs/history/README.md), with stable
paths for provenance and existing tests. The [workflow audit](docs/current/workflow-audit.md)
records the migration priorities.

`src/` and `asm/` hold only the sources the production plan actually compiles.
Everything else that once lived there — capsule-only C references with `asm db`
bytes, per-function ASM later merged into grouped `asm/M_*.ASM` modules, C
modules retired by later promotions, and the frozen inline-ASM oracle for
`asm/RUNTIME_BLOCK.ASM` — has moved to [`recovery/`](recovery/README.md) under
the same file names. Nothing under `recovery/` is compiled; `layout/manifest.json`
`source` fields for inactive regions point there for provenance.
