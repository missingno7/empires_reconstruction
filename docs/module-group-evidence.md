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

## C_C5D1_C706 and C_C77A_C898

`F_C755` now has a separately proved symbolic TASM representation, so it is
no longer appropriate to retain an `asm db` C capsule solely to keep a
C-only ten-owner experiment intact. The former compatible C range is therefore
represented by two contiguous C candidates: `C_C5D1_C706` (four functions,
388 `_TEXT` bytes, no fixups) and `C_C77A_C898` (five functions, 346 bytes,
13 fixups). `F_C755` remains its own ordinary TASM object between them.

The canonical link uses both fresh C contributions and the symbolic TASM
object at their established offsets, then remains byte-identical. These are
compatible linker inputs, not claims about historical source-file identity or
mixed C/TASM translation-unit ownership.

### Individual-owner promotion constraint

`F_C77A`, `F_C7CB`, `F_C834`, `F_C877`, and `F_C898` are the five source
pieces consumed by `C_C77A_C898`.  The module probe deliberately requires each
canonical manifest owner to name the exact recipe source and compiler flags.
Consequently, replacing just one of those canonical C proof pieces with a
symbolic TASM owner invalidates the shared-C proof before later link stages.

This is evidence about the current compatible-module topology, not a linker
failure.  A future promotion of one of these functions must either retain a
separate C proof source for this candidate or replace `C_C77A_C898` as a whole
with an evidenced mixed/assembly module that preserves its 346-byte text,
public order, thirteen fixups, and downstream byte-identical TLINK result.

## C_D61C_D79C

`recipes/modules/C_D61C_D79C.json` combines F_D61C and F_D79C into one fresh
507-byte `_TEXT` contribution with no fixups. The canonical exact-link chain
uses it in place of both proof objects and retains byte-identical output. This
is structural module evidence only: both source owners are still classified as
mechanical inline-assembly capsules and need symbolic recovery separately.
