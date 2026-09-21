# Empires reconstruction

The executable reconstruction now has a bounded, cheap-model production workflow.
Start with [the grinder instructions](docs/current/grinder-instructions.md),
[generated status](docs/current/status.json), and
[the ranked queue](docs/current/grinder-queue.json).

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
assembled directly by TASM 1.0. Its former inline-ASM C source remains a frozen
oracle. FAST measures emitted source ranges, checks all runtime bytes, publics
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
renamed with `tools/rename_source.py`. `python tools/probe_module.py OWNER [--source OWNER=alt.C]` compiles single
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
