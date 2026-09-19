# Shared compilation experiment

`recipes/modules/C_6C26_6C87.json` combines the existing source pieces for
F_6C26, F_6C57, F_6C6F and F_6C87 in one translation unit. Selection evidence:
contiguous original extents, identical compiler settings, compatible repeated
extern declarations, and the same `_gb76`/`_gc0d0` storage bindings. Sources
were not renamed or rewritten to manufacture the result.

```powershell
python tools/probe_module_group.py
```

One fresh Turbo C OBJ emits public offsets **0, 49, 73, 97** and a complete
128-byte `_TEXT` contribution. Every original function extent and all **16
fixups** match through the existing independent binding/relocation scaffold.
The recipe supplies source order and compiler flags, not public offsets.
The compiler sees neither original bytes nor desired addresses.

This proves **compatible shared compilation and relative code layout**. The
canonical exact structural-link experiment now replaces these four proof
objects with this one fresh object before its later shared-module and DATA
stages; TLINK preserves every downstream code address and the final EXE is
byte-identical. It does not prove these were the original translation-unit
boundaries or recover ownership of their external data. Historical modules
proven and linker-resolved bindings therefore remain zero.

The tool saves source/OBJ/compiler digests, publics and per-owner fixup results
under `build/module-group-*` and `build/module-group-report.json`. The compact
snapshot in [module-group-evidence.json](module-group-evidence.json) is historical
experiment evidence; rerun the command for a fresh comparison.
