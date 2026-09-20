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

## M_6D86_6DCC

`layout/structural-source-modules.json` now records one compatible symbolic
TASM contribution for the contiguous `F_6D86`, `PAD_006FC5`, and `F_6DCC`
interval. The apparent seven-byte pad is not padding: the LZ decoder addresses
it through `CS` as `bitlen`, `inleft`, `bitcnt`, and `bitbuf`. Its original
initialized value is seven zero bytes; `bitlen` is set to 9 by the decoder
before its first use.

`asm/M_6D86_6DCC.ASM` emits the complete 377-byte contribution with `_f6d86`
at offset 0 and `_f6dcc` at offset 70. TASM's ordinary self-segment fixups
bind the state references at link time. The canonical structural link uses the
single untouched object in place of two function proof objects plus a synthetic
padding object, while preserving every downstream address and the complete
byte-identical executable. This is strong compatible-module evidence from code
adjacency and direct CS-state references; it does not claim the historical
source filename.

## M_6B1A_6B4A

The adjacent `F_6B1A` and `F_6B4A` BIOS INT 16h helpers are now one compatible
76-byte symbolic TASM source module. The first routine blocks and dispatches
F1–F10 through the recovered helper calls; the second tests for an available
key without consuming it. `asm/M_6B1A_6B4A.ASM` emits their entry publics at
offsets 0 and 48 and retains both canonical call fixups. The structural link
uses this untouched ordinary object, reducing another pair of fragment-level
proof objects without asserting an original filename or source boundary.

## M_C1A0_C232

The contiguous 221-byte sound-control run now builds as one symbolic TASM
module. `_fc1a0` is the master tick, directly calls `_fc1f7`, and `_fc1f7`
directly calls `_fc232`; the three entries also share the compact sound-state
cluster at DGROUP offsets `0x1764..0x17F4`. The module emits their publics at
offsets 0, 87, and 146 and preserves all 36 code/data fixups after ordinary
linking. This is compatible source-module evidence from a direct call chain,
contiguous layout, and common storage. The normal structural build consumes
one untouched object in place of three proof fragments and remains
byte-identical.

## M_C9A4_CA91

`M_C9A4_CA91` replaces six contiguous proof fragments with a 247-byte
symbolic TASM module. It contains the table-command decoder, its four-way
dispatcher, and every dispatcher target. The entries share the DGROUP command
control block at `0x1E84..0x1E94`, and the dispatcher calls the targets within
the same contribution. Fresh TASM emits the historical public offsets 0, 95,
145, 173, 223, and 237, with the five original data fixups. The untouched
ordinary object binds to the exact full extent and preserves the
byte-identical structural link. This is compatible-module evidence based on
contiguity, direct control flow, and shared state; it does not assert an
original filename.

## M_C27D_C567

`M_C27D_C567` is a 797-byte compatible symbolic TASM module covering the
contiguous `F_C27D`, `F_C2EA`, `F_C359`, `F_C3DB`, `F_C440`, `F_C501`, `F_C549`, and
`F_C567` run. `F_C27D` directly drives the dispatcher and renderer in the same contribution. Its lead entry directly dispatches to the second, third, and
fifth entries. `F_C3DB` reads the `1788h/178Ah` scale state that `F_C567`
writes, and `F_C549` updates paired command-control flags. The single object
emits the historical entry offsets 0, 111, 241, 342, 535, 607, and 637, has no
OMF fixups, and its full bound extent matches. The normal structural build now
uses it unchanged in place of seven proof objects and remains byte-identical.
This is strong compatible-module evidence from direct local control flow,
adjacency, and shared state, without asserting the original source filename.

## M_CB5C_CD23

`M_CB5C_CD23` combines the five contiguous row-editing and menu-control
entries into one 641-byte symbolic TASM object. The three leading routines
operate on fields of the selected record; the final two render and accept
ten-row menus from the same table/state area. Its eight public entry/end
labels retain their original offsets, the contribution has no OMF fixups, and
the complete structural link remains byte-identical. This is compatible module
evidence from shared selected-row state, record-table access, and adjacency.

## M_DAD7_DB35

`M_DAD7_DB35` combines three contiguous state-table helpers into a 137-byte
symbolic TASM contribution. Each obtains a signed selector from the high byte
of a two-byte `DS:2FD2h` table entry; the latter two retain their ordinary
external calls to the reconstructed handler entries. The complete structural
link is byte-identical, including the two natural TASM fixups. This is a
compatible source-module result based on shared table access and adjacency.

## M_D818_D825

`M_D818_D825` groups the adjacent 13-byte reset and 58-byte append operations
for the initialized `DS:2F30h` packed record table. One symbolic TASM object
preserves both entry offsets and the exact 71-byte no-fixup contribution; the
full linked executable remains byte-identical. This is compatible module
evidence from direct shared table ownership and adjacency.

## M_C5A8_C5C6

`M_C5A8_C5C6` replaces three adjacent setters with one 41-byte symbolic TASM
contribution. The entries store or select fields in the same SI-relative
`17A4h..17DCh` command-control block, retain public offsets 0, 11, and 30,
and have no OMF fixups. The full structural link remains byte-identical.

## M_D386_D3CF

`M_D386_D3CF` is an 84-byte symbolic TASM recovery of the far-record decoder
and its direct negative-selector continuation. The latter begins immediately
after the decoder and is the target of its signed branch, then returns to the
decoder's table lookup. The module retains entry offsets 0 and 73 plus the
canonical far-data and handler-call fixups. Its complete fresh link is
byte-identical, replacing the prior 73-byte `asm db` decoder capsule with
readable control flow.

## M_988F_98CB

`M_988F_98CB` combines two contiguous fixed-rectangle display helpers into one 121-byte symbolic TASM contribution. They share the recovered `C34E/C350` coordinates and paired display call pattern; the combined object retains the original entry/end offsets and full byte-identical link.
