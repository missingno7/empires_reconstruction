# Reconstruct the build, not only its output

Project-owner clarification, 2026-09-18. The final build must emerge from
independent reconstructed components. Original game files are verification
fixtures, not normal sources for unknown ranges or resources. Keep the
fixed-layout bootstrap intact as an independent comparison path.

## Four levels of proof

| Level | What is established | What is not yet implied |
|---|---|---|
| Placement | A component matches at an explicitly declared original location. | Its position follows from recovered linking rules. |
| Component | Source independently compiles/encodes to the complete matching component. | The original whole-build topology is recovered. |
| Structural build | Ordering, sizes, compression, modules, segments and packing/linking relationships generate layout. | Unknown component fallbacks have disappeared. |
| Emergent whole build | Reconstructed components and recovered rules produce all original files without unknown fallbacks or forced placement. | Exact historical source filenames or attractive semantic names are required. |

A function's address must eventually follow from correct source modules,
generated code/data sizes, object order, alignment, runtime/library selection,
segments and linker behavior. A resource's offset must follow from resource
order, complete encoding/compression, preceding emitted lengths and packing
rules. Original offsets may be checked after generation; they must not drive
generation in the final path.

Binary component sources can be legitimate when their format and role are
understood and binary is intentionally canonical. Moving an opaque original
slice into a separate file, or calling it an asset, does not establish this.
Asset contents still remain local and excluded from Git under the owner's
distribution constraint.

## First structural build step: independent DAT packing

```powershell
python tools/pack_archives.py
python tools/pack_archives.py verify --fixed-output build
```

The first command reads only `recipes/archives/*.json` and their local source
components. It does not open `assets/`, fixed manifests, original offset tables,
expected digests or expected output lengths. Its recipes strictly reject
placement/oracle fields. Resource order is the list order; empty slots remain
explicit. It encodes every component, computes `4 * (resource_count + 1)`,
accumulates emitted block lengths, generates all LE32 offsets including the
terminal sentinel, and appends any separate trailing component. There is no
resource alignment or padding inserted by this rule.

The second command performs verification after generation. It checks the
receipt, component recipe consistency with the fixed scaffold, original
fixture identity, and complete byte equality. `--fixed-output` additionally
compares the fixed builder's output. The verified result is:

```text
AE000: derived pack == fixed rebuild == original (227,560 bytes)
AE001: derived pack == fixed rebuild == original (383,874 bytes)
```

`python tools/reconstruct_game.py` now runs all of these paths and records
three-way DAT equality in the combined report. `build/packed/packing-report.json`
records generated offsets, actual sizes, source/output hashes and fallback
counts. Construction status is `BUILT_UNVERIFIED`; equality is a separate
`build/packed/verification.json` verdict. Individual packing failures remove
previous packed outputs and verification. Missing components fail; generation
never extracts replacements from originals.

Recipes were migrated once from established component identities and resource
order. That migration discarded every historical offset, length and digest.
`archive_recipe.py` synchronizes source choices after verified promotions; the
packer never invokes migration. The fixed layout remains a separate verification
description, not the packer's input. Tests change a component's length and
resource order/count and prove that subsequent offsets and table size change
naturally. Real packing also passes with no originals or fixed manifests
present and with file-read guards restricting generation to component inputs.

## Honest boundary of this result

The DAT **packing rule** is recovered for these archives. The DAT **component
reconstruction** remains incomplete: 193 resources are opaque encoded fallbacks,
and 27 resources use structured sources (25 PNG/JSON images, one image bank,
one level). Mixed banks and level records retain explicitly opaque fields.
No unknown payload became understood merely
because its offset is now computed. This step does not recover the historical
compressor's remaining match-selection behavior.

The EXE remains at fixed placement with declared bindings (including 572 owner-derived code/data references and six owned module-data bindings), 31,371 raw
bytes, and no proven reconstruction of its original source modules or historical
linker topology. Its 270 source proof units do not count as recovered original
modules. One [compatible shared-compilation experiment](module-group-evidence.md)
now proves four functions' relative placement in a single OBJ, without yet
establishing historical module identity. The combined report explicitly sets
`whole_build_reconstruction_complete` to false.

Next structural work should recover module/data ownership and symbol resolution
alongside component recovery and encoder policy. Keep both EXE paths until a
linked output independently agrees with the fixed scaffold and originals. The
primary build can stop depending on the scaffold only when this relationship
is established without unknown fallback or externally forced placement.


Buffer-storage evidence can now bind an independently observed runtime buffer outside the on-disk load image while keeping its uninitialized bytes out of the EXE source. The first such declaration is DS:96EE, shared by the F_21DB loader and F_2269 blitter.
