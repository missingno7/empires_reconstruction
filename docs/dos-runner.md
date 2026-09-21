# DOS tool runner

Turbo C 2.0, TASM 1.0, Turbo Link 2.0, `CC.LIB` and `C0C.OBJ` remain the
hash-pinned historical build inputs; `compile_sources` verifies their SHA256
before every session. The host used to execute those DOS programs is separate
and is configured in `layout/toolchain.json` under `runner`.

## Runner selection

`tools/dos_runner.py` is the single runner interface. `resolve_runner` picks,
in order: an explicit `--runner`/`--msdos-player`/`--dosbox` argument, the
`EMPIRES_DOS_RUNNER`, `MSDOS_PLAYER` and `DOSBOX` environment variables, then
the lock's `runner.default` (`msdos-player`) with `runner.msdos_player_default`,
`runner.msdos_player_overrides` and `runner.fallback` (`dosbox`).

Every unit runs exactly the historical command on every host: `tcc -c -mc -1-
-f- -N- [-B] Rnnnn.C` (Turbo C performs its own TASM handoff for `-B` units
and for units whose inline `asm` restarts the compile), `tasm /mx Rnnnn.ASM`
for assembly units, and one `tlink @LINK.RSP`. The player invokes each program
as a native process with `-e -v5.00`, a reduced DOS environment holding only
the tool directory on `PATH` and the working directory as `TEMP`/`TMP`; the
DOSBox batch path sets `PATH=C:\TC\BIN` for the same files. Neither host has a
`TURBOC.CFG`; headers are staged next to the sources.

* **MS-DOS Player, nmlgc ReC98 P0281 fork** (`C:/tools/nmlgcdos/msdos.exe`,
  ia32 core, based on upstream 2024-06-24) runs Turbo C and TASM. It is the
  normal runner for grinding, module probes, matching and acceptance.
* **MS-DOS Player, upstream `i86_x64` build** runs TLINK only
  (`runner.msdos_player_overrides`). The fork drops the upstream `-f` file
  limit and TLINK 2.0 fails with DOS error 6 (invalid handle) once a link
  names about twenty objects; the upstream build links the full 276-object
  set with an identical EXE in 30 of 30 trials, idle and under CPU load.
* **DOSBox Staging** is the reference and debug backend and is never started
  on the normal path. `python tools/build_exe.py --runner dosbox verify` runs
  the same build through it; `python tools/probe_runner_parity.py` builds
  through both hosts and compares every staged DOS input, every compiler and
  assembler object, the link map, the relocation table and the final EXE.

```powershell
python tools/build_exe.py verify                       # MS-DOS Player
python tools/build_exe.py --runner dosbox verify       # DOSBox reference
python tools/probe_runner_parity.py                    # parity gate, both hosts
```

## Why the upstream player builds are not used for Turbo C and TASM

With only `TCC.EXE`, `TASM.EXE` and one source in a clean directory, the
upstream 2026-09-13 builds mis-execute the historical commands in a
timing-dependent way: `tcc -B` on a unit whose assembly is 41 KB fails or
hangs on every CPU variant (`i86`: hang on first run; `i386`: error and hang;
`ia32`: error), and plain `tasm /mx` on the 64 KB RUNTIME_BLOCK source under
the `i486_x64` build reports garbled symbols ("Undefined symbol: NF0H") in 9
of 30 runs and hangs in a later loop, while the `i86_x64` build fails only
under host CPU contention. The same files assemble identically in isolation
and on retry, the on-disk intermediate assembly is byte-identical to `tcc -S`
output, LF-only copies assemble fine, a 50 ms pause before TASM changes
nothing, and no concurrency or shared-directory collision is involved (one
process at a time, unique stems). The fork shows none of this: 100 of 100
tiny and 40 of 40 large `tcc -B` runs, 80 of 80 RUNTIME_BLOCK assemblies
under load, and 10 of 10 full-plan sessions.

## Canonical DOS text inputs

Every text file handed to the historical tools is written by our
infrastructure through `reconstruct.dos_text`: latin-1 bytes with explicit
CRLF line endings and no DOS EOF byte. `compile_sources` stages all C, header
and assembly sources through it (the DOSBox batch path did the same conversion
before), and `build_production` writes the TLINK response file through it.
This is staging hygiene, not a workaround: no host converts text for us.
`tests/test_dos_text_inputs.py` pins the writer, the staging path and the
determinism of the historical `-B` handoff on the default runner.

## Object parity

Objects from the two hosts are linker-equivalent, not byte-identical: Turbo C
and TASM record the source timestamp in an OMF comment (pinned for the BSS
sources, volatile elsewhere), TASM emits PUBDEF records in an order that
depends on its memory layout, and the player reports canonical upper-case
file names so THEADR reads `R0002.C` where DOSBox reads `r0002.c`. The
parity probe erases only the timestamp field and otherwise compares segments,
publics, externals and fixups. The linked EXEs are byte-identical.
