# Exact C recovery

The user authorized renewed matching-C work for compiler-like production ASM.
F_AB66 is now an ordinary Turbo C translation unit in src/F_AB66.C, replacing
asm/F_AB66.ASM in production. The ASM is retained as reference. No inline ASM,
DB directives, object corrections, or padding were added to obtain the match.

All 385 linked bytes match the historical routine. Its natural stack frame has
four bytes of locals and SI/DI register variables. Declaring the locals in the
compiler's required order reproduces their BP offsets. The typed far-pointer
prototype of fa28d is essential: an unprototyped call generated four extra bytes
for pointer construction. With that prototype, the whole routine matches.
The record layout exposes descriptor words at offsets 13 and 15 in a 27-byte
record; casts preserve the original far access behavior. This is a reconstruction
that the historical compiler emits exactly, not proof of recovered original text.

Full acceptance builds and tests verify the complete EXE, all 106 relocations
in exact order, zero object transformations, and fixture-free construction.
The module count stays 293; production TASM modules decrease from 53 to 52.
The CHEAP runtime queue remains unconsumed.

## Next candidates and exclusions

* F_652A: likely C with BIOS inline assembly. Its 512-byte local and SI retry
  loop are compiler-like; INT instructions and register setup are not pure C.
* F_1ECD, F_1F91, F_4E9F and F_6EFF: saved DS, string operations, LOOP counters
  or explicit register preservation across calls are evidence against ordinary
  Turbo C output. Do not convert based only on the presence of a BP frame.

The refreshed asm-origin-review.json inventories current production ASM and
records F_AB66 separately under exact_c_recoveries.

## F_880A and former F_8C04: one exact C function

src/F_880A.C now replaces both production ASM owners. All 557 bytes match,
including the sparse key switch, dense direction switch, and cleanup/return.
The former 506-byte F_880A and 51-byte F_8C04 extents and hashes remain in
manifest provenance. Their ASM files remain reference sources.

The cleanup consumes BP locals and saved SI/DI from the selection loop; it has
no independent prologue. No active source or binding references its former
synthetic public _f_8c04. That public was removed instead of manufacturing an
extra callable C function. The manifest now owns the entire function together.

The packed input and view structures explain the far-pointer accesses and the
30-byte stack frame. Unsigned view.direction explains zero extension of chained
assignments; signed input.count explains CBW. Ordinary while/if/switch statements
reproduce both jump tables, including the unreachable compiler return jump.
The compiler object reaches TLINK unchanged, with no byte directives or padding.

Full EXE acceptance remains byte-identical with 106 relocations in exact order.
Production now has 292 source modules, 441 linker inputs, and 50 TASM modules.
These two recoveries total 942 bytes of ordinary C. The 72 runtime CHEAP cards
remain available; this work consumed none of them. Next candidate: F_7964 with
its shared-frame F_7DD3 continuation.

## F_7964 and former F_7DD3: exact menu loop

src/F_7964.C replaces both ASM owners with one ordinary C function. All 664
bytes match, including the sparse key table, indirect near callbacks through a
far table, nested result switch, redraw loop, and cleanup. Manifest provenance
preserves the old boundaries and hashes; both ASM files remain reference only.

The former cleanup public had only one active symbolic caller: the parent early
return targeting cleanup+0x23, which is the shared epilogue. Ordinary return now
produces that jump naturally. No synthetic entry or object correction is used.
The 20-byte record layout explains the stride and callback/geometry fields.
Writing gc0fe->count <= ++selected preserves the observed evaluation order;
the reversed equivalent comparison compiled two bytes shorter with Turbo C.

Full acceptance remains exact: 106 relocations, no unresolved symbols, no object
transformations or EXE fallback. Current production has 291 unchanged source
objects, 440 linker inputs, and 48 TASM modules. Ordinary C recoveries now total
1,606 bytes across three functions; no runtime grinder cards were consumed.
