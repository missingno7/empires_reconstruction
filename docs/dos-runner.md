# DOS tool runner

Turbo C 2.0, TASM 1.0, Turbo Link 2.0, `CC.LIB` and `C0C.OBJ` remain the
hash-pinned historical build inputs.  The host used to execute those DOS
programs is separate.

`tools/dos_runner.py` selects MS-DOS Player when `msdos.exe` is available
through `MSDOS_PLAYER`, the configured runner path or `PATH`.  It invokes each
historical executable directly with a short DOS 5.00 environment.  DOSBox
Staging remains the reference/fallback backend and is isolated behind the same
runner interface.

```powershell
$env:MSDOS_PLAYER = 'C:/tools/msdos.exe'
python tools/build_exe.py verify

# Reference backend
python tools/build_exe.py --runner dosbox --dosbox 'C:/Program Files/DOSBox Staging/dosbox.exe' verify
```

The runner parity probe executes representative Turbo C objects and the full
source-to-TLINK build through both hosts:

```powershell
python tools/probe_runner_parity.py --msdos-player $env:MSDOS_PLAYER --dosbox $env:DOSBOX
```

TASM records a volatile source timestamp in an OMF comment record.  MS-DOS
Player can also emit PUBDEF and `LEDATA` records in a different order for the
unusually large `RUNTIME_BLOCK` object.  The parity probe therefore compares
the linker-visible OMF meaning: SEGDEFs, bytes, publics, externals and fixups.
The pinned-library replacement retains the historical record topology where
that matters.  The final TLINK EXE and all 106 relocation pairs must still be
byte-identical.
