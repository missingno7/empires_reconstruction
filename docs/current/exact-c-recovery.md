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

## F_B40F: compact descriptor expansion

All 236 bytes now compile from ordinary C in src/F_B40F.C. The two input
resources contain word offsets; the routine adds each byte offset plus two to
the resource base, stores far pointers, then fills remaining slots from slot 0.
The pinned library memmove public replaces the former fixed relative calls.
Two existing four-byte BSS reserves now expose gc592/gc596 publics. Their old
reserve labels incorrectly said gc590/gc594; their offsets and sizes did not
change. Full acceptance verifies the resulting symbolic references and exact
relocation order. No object corrections or inline assembly were introduced.

Current production retains 291 source objects and 440 linker inputs, with 47
TASM modules. Four exact C recoveries total 1,842 bytes. Runtime cards remain
untouched. F_AA1F is another promising compiler-style state loop for follow-up;
F_9EC3 uses saved DS, XLAT and LOOP and is not a pure-C priority.

## F_AA1F: exact state-driven record transition

src/F_AA1F.C now reproduces all 327 bytes with ordinary C, including the sparse
key switch, signed remainder wraparound, 200-byte stack buffer, far-pointer
assignment and return paths. Separating increment/decrement from the remainder
assignment avoids unsequenced C modifications while preserving identical output.
The final switch case falls through to the common cleanup without a break;
adding that break emits an extra two-byte jump with the pinned compiler.

Full EXE acceptance is byte-identical with all 106 relocations in exact order,
zero unresolved symbols, and unchanged compiler objects. Production has 46 TASM
modules; five exact C recoveries total 2,169 bytes. No runtime card was consumed.
The ASM reference remains available. This proves an exact C reconstruction,
not the original source text or authorship.

## F_B7F9: display and workspace initialization

src/F_B7F9.C reproduces all 366 bytes as ordinary C. Three bounded loops update
the display and fill workspace ranges. Existing gb3af/gb52f arrays express the
board fields as indexed accesses, avoiding new interior BSS aliases. Direct
calls now use established symbol bindings instead of fixed relative operands.
No inline ASM, byte directives, storage movement or object corrections are used.

Production has 45 TASM modules; six recent exact C recoveries total 2,535 bytes.
The runtime grinder queue is unchanged. Inspection of F_6036, F_60A9, F_6181,
F_C914, F_CA9B and F_CAF1 found string operations, special register preservation,
stack-argument rewriting or live-register inputs; these are not pure-C priorities.

## F_4F96: compiler-to-assembler path recovered

src/F_4F96.C reproduces all 299 bytes as ordinary C with Turbo C -B and the
pinned TASM. Direct object generation produced 298 bytes, reversed CMP operand
encoding in both sparse-switch searches, and different jump relaxation. The
-B path naturally reproduces the historical short jumps plus NOPs. No inline
assembly, explicit padding, generated-source edits or object corrections are
needed. Existing runtime _argc/_argv publics provide the argument vector.

The parser accepts case-insensitive graphics switches E/C/T/M/V selecting
values 1/2/3/4/5, I selecting sound state 0, and SI/SA/ST selecting sound states
0/2/1. These selector values alone do not prove a specific sound device.
Production now has 44 TASM-source modules. This C module still invokes TASM
through the normal compiler handoff. Seven exact C recoveries total 2,834 bytes.

F_28AC remains ASM: a bounded C candidate reproduces its loops and addressing,
but two byte-to-coordinate conversions lack the original XOR DX,DX and use
MOV AH,0 instead of XOR AH,AH. Candidate extent is 214 versus 218 bytes. Tested
integer-width/cast variations and -B did not resolve this; no padding or invented
instructions were added. Further compiler-expression evidence is needed.
