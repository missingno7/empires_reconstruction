# Current phase: mechanical and structural reconstruction

Direction supplied by the project owner on 2026-09-18. Read [vision.md](vision.md)
for the North Star. MVP1 established exact EXE reconstruction; this phase
extends that invariant across the complete game and progressively replaces
opaque regions with rebuildable structure.

**First reconstruct what exists. Later explain why it exists.**

Current priority: **first make it reconstructible, then understandable, finally
beautiful**. Prefer the highest practical lossless source form. The same
component encoders/toolchain should accept edited PNG/JSON/level/C/ASM inputs
to produce modified DOS files; unchanged inputs must still match originals.
Semantic reasoning is allowed to unblock structure, not as a substitute for
mechanical progress on another available frontier.

The [architectural end-state](build-reconstruction.md), clarified on 2026-09-18,
requires layout to emerge from reconstructed components and build rules.
Placement matching is an intermediate proof. Original files ultimately belong
only to verification; unknown byte/resource fallbacks must disappear. Keep
placement, component, structural-build and whole-build progress distinct.

Keep the fixed-layout EXE builder as an independent comparison scaffold.
Never weaken matching, discard quirks, normalize padding or duplicate data,
infer linkage from comparison operands at build time, or turn this into a
source port. PortForge can supply missing observations but is not a runtime
dependency. Preserve the two upstream projects as read-only evidence sources:

- `D:/Games/DOS/dos_recosystem/empires_forged`: correspondence, code/toolchain,
  OMF, bindings, runtime observations and library proofs.
- `D:/Games/DOS/ancient-empires-reverse-engineered`: archive, compression,
  graphics, palette, level, room and actor-format research.

## Productive frontiers

1. **Matching C/ASM:** consume unrecovered extents with historical compiler
   output, full extent/fixup comparisons and structured differences. Automate
   small code-generation mutations where useful.
2. **Linkage closure:** aggregate unresolved symbols across held candidates;
   prefer independently evidenced bindings that unlock several regions.
3. **Original modules:** test shared translation units when ordering, publics,
   common data bases and flags provide evidence. Separate proof units are not
   automatically recovered historical modules.
4. **Link topology:** recover module/library order, data/BSS placement,
   startup, alignments, relocations and linker options incrementally. Keep
   fixed placement working throughout.
5. **EXE data classification:** distinguish machine code, runtime code,
   static data, strings, tables, assets and padding where evidence is strong.
   Use structural names; retain unknown when evidence is insufficient.
6. **DAT archives:** partition AE000 and AE001 into exact table/resource/
   trailing-byte ownership, then prove byte-identical repacking.
7. **Resource matching:** distinguish raw, decoded, rebuildable and matching
   representations. Successful decoding alone does not establish matching.
8. **Compression:** reconstruct RLE and pair-span/LZW-like encoder policies,
   including selection, dictionary behavior, widths, termination and padding.
   Require encoded bytes equal the original stream, not merely a decode loop.
9. **Assets:** promote graphics/font/palette representations only after their
   encoder reproduces the original bytes; preserve unknown components.
10. **Levels and bytecode:** recover structural record/offset relationships
    and round-trip payloads before requiring exact recompression.

Work across these frontiers according to available evidence. A hard function
is not a reason to switch into broad semantic renaming. Semantic reasoning is
allowed when needed to establish a structural fact; speculative gameplay
names, polished APIs and general cleanup are not this phase's deliverables.

## Evidence, metrics and blockers

Every material finding should become code, a manifest, a matching test, a
build rule or persistent evidence. Track byte coverage as well as counts.

For the EXE, report C, ASM, source-derived, library/runtime, classified static
data, embedded-asset, unknown and unresolved-code bytes; distinguish proof
units from original modules; count declared and linker-resolved bindings.
For each DAT, report accounted bytes, resource partition/type counts, decoded,
rebuildable and exact-matching resource/byte counts, exact compressed bytes,
and remaining raw resources. Keep representation depth distinct from byte
identity achieved by raw fallback.

[blockers.json](blockers.json) is the persistent ledger: target, current
state, what matches, first unresolved fact, available evidence, what would
unblock it and expected leverage. New evidence may resolve an entry; neither
one blocked target nor an unattempted frontier implies global saturation.
Only consider a dedicated semantic pass when most productive mechanical
routes depend on missing semantic understanding.
