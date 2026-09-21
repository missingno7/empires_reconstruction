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

src/MENULOOP.C replaces both ASM owners with one ordinary C function. All 664
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

src/CMDLINE.C reproduces all 299 bytes as ordinary C with Turbo C -B and the
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

At that checkpoint F_28AC remained ASM: a bounded C candidate reproduces its loops and addressing,
but two byte-to-coordinate conversions lack the original XOR DX,DX and use
MOV AH,0 instead of XOR AH,AH. Candidate extent is 214 versus 218 bytes. Tested
integer-width/cast variations and -B did not resolve this; no padding or invented
instructions were added. Further compiler-expression evidence is needed.

## F_B122 and F_28AC: two further exact C recoveries

F_B122 now compiles directly from src/F_B122.C to all 693 historical bytes.
Nine resource registrations feed eight word-offset expansion loops and one
unmodified pointer slot. The final loop fills the remaining descriptor slots
from slot zero and updates g720. Existing DATA/BSS publics cover every reference;
no storage aliases or layout changes were required. F_B40F supplied the proven
far-pointer declaration pattern; each F_B122 table and selector was independently
read from its original instructions.

F_28AC now compiles directly from src/F_28AC.C to all 218 historical bytes.
The two coordinate loads use (unsigned)(char far *)p[n]. With pinned Turbo C,
this preserves XOR AH,AH and XOR DX,DX before assigning the low word to SI/DI.
The pointer intermediate is never dereferenced. Plain integer and long casts
compile to 214 bytes; both direct compilation and -B reproduce the full 218
bytes with the pointer intermediate. No -B flag is needed in production.
This is a compiler-specific exact reconstruction, not evidence that the original
author used that expression. The 22 controlled probe results and source template
are preserved in coordinate-conversion-evidence.json.

Both source modules reach the linker unchanged, with no inline ASM, byte
instructions, padding, object correction, or relocation reordering. Their
original ASM remains reference material. These recoveries add 911 bytes of
ordinary C, bringing the documented nine recoveries to 3,745 bytes and reducing
production TASM-source modules from 44 to 42. The remaining C review candidates
are F_25B3, F_8BAB, and F_B99F; F_25B3 can now use the coordinate conversion
finding as a reference. Arithmetic module relocation ordering is still held.

Validation: both canonical build integration tests pass, including a fresh
fixture-independent construction and full fixture comparison. The resulting
EXE SHA256 is 1259348425483d8d97fd8821860b47cfdf58fc8029711eb0ed0e78ab33807a10,
with all 106 relocations in their original order. Queue audit and readiness
checks pass; the unrelated unresolved runtime trailer byte remains unchanged.

## F_25B3: moving-record update and redraw

src/F_25B3.C now reproduces all 761 historical bytes as ordinary Turbo C.
The ten three-byte records carry a flag/count byte and two coordinate bytes.
The four conditional redraw paths preserve the cell checks, movement updates,
sprite calls, temporary g96 state, and corresponding board-cell writes.
The local-frame order and unsigned SI/DI coordinates match the original.

The coordinate conversion uses the already evidenced F_28AC far-pointer
intermediate, without dereferencing it. Combining flag assignment and its
low-nibble test as if ((flag = *p) & 15) reproduces TEST AL,0Fh; splitting
those expressions instead tests the stack local and emits two extra bytes.
The decrement/toggle sequence remains ordinary C. No inline assembly, padding,
object transformation, storage relocation, or new public alias was needed.

This adds 761 bytes, bringing the ten documented C recoveries to 4,506 bytes.
Production retains 291 source objects and 440 linker inputs, with 41 remaining
TASM-source modules. The earlier ASM is retained as a reference. The remaining
large C review candidates are F_8BAB and F_B99F; the arithmetic relocation-order
constraint and the unresolved runtime trailer byte are unchanged.

Validation: both canonical integration tests pass, including fixture-independent
construction and a fresh full comparison. The EXE SHA256 remains
1259348425483d8d97fd8821860b47cfdf58fc8029711eb0ed0e78ab33807a10, and all 106
relocations retain their exact order. Compiler objects reach TLINK unchanged.

## F_8BAB: grid selection and placement state machine

src/F_8BAB.C reproduces all 1,275 historical bytes directly with pinned Turbo C.
The complete candidate matched on its first compile, including both sparse-key
switch tables, nested selection/placement loops, redraw and blink state,
completion handling, saved runtime settings, and the common return path.

Two-byte piece records contain signed character kind and rotation fields,
consistent with the existing F_90A6/F_929E/F_969D consumers. The empty kind is
-1. The 4-by-6 table's row addressing, whole-record copies and argument passing
compile naturally to the observed far accesses. Eleven int locals reproduce
the 22-byte frame, with row/column register variables in SI/DI. Assignments in
the key and completion tests preserve their observed register use. Existing
publics and storage bindings are reused; no storage or module boundaries move.

No inline assembly, explicit jump-table bytes, ORG, padding, or compiler-object
correction is needed. The original ASM remains a reference. This proves an
exact C reconstruction, not the original author's source text. The eleven
documented recoveries now total 5,781 bytes. Production retains 291 source
objects and 440 linker inputs, with 40 remaining TASM-source modules.
F_B99F is the remaining large C state-machine candidate in the held inventory.

Validation: both canonical integration tests pass, covering fresh full EXE
comparison, all 106 ordered relocations, unchanged compiler objects, and
fixture-independent construction. The EXE SHA256 remains
1259348425483d8d97fd8821860b47cfdf58fc8029711eb0ed0e78ab33807a10.

## F_B99F: board movement, collision, and redraw loop

src/F_B99F.C reproduces all 1,857 historical bytes with direct Turbo C output.
Its 148-byte local frame consists of a 128-byte snapshot of four 32-byte records,
eight int locals, and an unsigned long deadline. SI is the record index/action
selector; DI is the movement direction. The ordinary C loop preserves horizontal
movement, jumping and falling, collision responses, damage/invulnerability
rendering, timed board changes, and both return paths.

The record snapshots call the pinned library's movmem source/destination API,
not memmove. Record addresses passed without a prototype naturally reproduce
the original CX:BX pointer construction. Direct flag access through a near byte
view of the same record array preserves the historical DS addressing. No new
storage aliases or layout changes were required. The pointer at B0F0 is
expressed as gb07c[29], reusing the established descriptor-array owner.

The reset chain g40ce=direction=key=0 preserves both assignment order and exact
register use. Combining the action==2 and g72c==0 conditions preserves the jump
into the following else-if test when the second condition fails. The timed
exit skips the second f1ecd call via the common frame-end label; reconstructing
that branch as unconditional fallthrough would change behavior. All of these
relations are checked by the complete byte comparison, not source appearance.

No inline assembly, byte directives, padding, generated-assembly edits, or object
corrections are used. The ASM reference remains available. The twelve documented
C recoveries total 7,638 bytes; production retains 291 source objects and 440
linker inputs, with 39 remaining TASM-source modules. The four large held C
candidates (F_B122, F_25B3, F_8BAB, F_B99F) are now all recovered, as is the
previously held F_28AC. Remaining ASM needs separate origin/feasibility review;
the arithmetic module's relocation-order constraint is still unresolved.

Validation: both canonical integration tests pass, including fixture-independent
construction, full EXE comparison, unchanged compiler objects, and all 106
relocations in their exact order. SHA256 remains
1259348425483d8d97fd8821860b47cfdf58fc8029711eb0ed0e78ab33807a10.

## Five ordinary C helpers in two existing shared modules

src/MRKCELL.C reproduces both fixed-rectangle display helpers, 60 and 61
bytes, as one 121-byte compiler object. The first helper calls f03ba; the
second calls f03b7. Both then call f03b4 using the shared C34E/C350 coordinates.
The different first-call targets were verified against the historical operands.

src/VOXCHAN.C reproduces the three state-table helpers, 64, 30 and 43
bytes, as one 137-byte compiler object. Two-byte records expose the signed
selector at offset one. SI carries its sign-extended value. Unsigned value
clamping and ordinary array accesses preserve every emitted instruction.

All five functions were first compared individually, then both complete shared
objects were compared against their full original extents. Their function
publics remain at offsets 0/60 and 0/64/94. The unused _f_98cb_end and
_f_dad7_end publics were reconstruction-only epilogue markers; no active source
or binding uses them. They were removed from the public index, which acceptance
checks again against fresh compiler objects. Historical ASM and old capsules
remain reference material, outside production.

These recoveries add 258 bytes without splitting modules or moving storage.
Production retains 291 source objects and 440 linker inputs, with 37 remaining
TASM-source modules. The documented C recoveries total 7,896 bytes across 17
functions. No inline ASM, explicit byte emission, padding, or object correction
was introduced. The remaining hardware/live-register routines require separate
review; for example F_643A has an explicit STI and F_6BCF is an interrupt handler.
Those properties alone do not prove historical source language.

Validation: both canonical integration tests pass, covering fresh full EXE
comparison, all 106 ordered relocations, unchanged compiler objects, and
fixture-independent construction. SHA256 remains
1259348425483d8d97fd8821860b47cfdf58fc8029711eb0ed0e78ab33807a10.

## M_CB5C_CD23: row-setting callbacks and two menus

src/OPTIONS.C reproduces five functions as one exact 641-byte ordinary C
module: F_CB5C (97 bytes), F_CBBB (90), F_CC17 (85), F_CC6B (184), and F_CD23
(185). Each matched independently on its first probe; the combined compilation
also matches all bytes and entry offsets 0, 97, 187, 272, and 456.

The 27-byte menu records expose word settings at offsets 13, 15, 17 and 19.
The three callbacks initialize a dialog flag, apply accepted results, and update
the associated sound/music state. The two remaining routines display ten records
and wait for Enter or Escape. Far-pointer prototypes preserve the observed
address construction for dialog and row arguments. Existing storage symbols
cover all references without adding aliases or changing record ownership.

The reconstruction-only _f_cb5c_end, _f_cbbb_end and _f_cc17_end epilogue labels
have no active callers or bindings. They are removed from the public index;
fresh acceptance still checks every actual compiler public. Original function
entries and shared-module boundaries remain intact. ASM and capsule references
remain outside production. No inline ASM, explicit bytes, padding, or object
corrections are used.

Production now has 36 TASM-source modules, 291 source objects and 440 linker
inputs. The documented recoveries total 8,537 bytes across 22 functions.

Validation: both canonical integration tests pass, including fresh full EXE
comparison, all 106 ordered relocations, unchanged compiler objects, and
fixture-independent construction. SHA256 remains
1259348425483d8d97fd8821860b47cfdf58fc8029711eb0ed0e78ab33807a10.

## F_699E: keyboard interrupt handler

src/F_699E.C reproduces all 380 bytes using Turbo C's interrupt function
extension and native __sti__, __inportb__, and __outportb__ intrinsics. The
interrupt-qualified saved-vector pointer produces PUSHF plus the far call.
The compiler generates all register saves, the DS setup, sparse switch table,
and IRET. There is no inline assembly, explicit byte emission, or padding.
The -B path preserves the historical short-branch relaxation bytes.

The local word precedes the scan byte in declaration order to reproduce the
four-byte stack frame. Existing _b856 and _gb72 storage symbols cover the mode
byte and adjacent modifier state without adding storage aliases. All original
entry points, module boundaries, and relocation ordering remain unchanged.

The pinned Turbo C distribution's DOS.H documents these intrinsics, including
__sti__ as the implementation of enable(). Port arguments must have the signed
int prototype: the initially tested unsigned prototype is rejected as a bad
inline-function call. The historical ASM comment that IRET and a CS-relative
switch necessarily imply handwritten assembly was incorrect and is corrected.
This result also warrants revisiting the other C-shaped routines containing STI.

Production has 35 TASM-source modules, 291 source objects, and 440 linker inputs.
The documented C recoveries total 8,917 bytes across 23 functions.

Validation: both canonical integration tests pass, including fresh full EXE
comparison, all 106 ordered relocations, unchanged compiler objects, and
fixture-independent construction. Queue and handover audits pass. SHA256 remains
1259348425483d8d97fd8821860b47cfdf58fc8029711eb0ed0e78ab33807a10.

## F_643A and F_652A: persistent record writer and BIOS retry helper

src/F_643A.C reproduces all 240 bytes of the indexed record writer. It reads
adjacent file offsets, writes the record payload, and retries after refreshing
metadata when the shared error flag is set. Turbo C's native __sti__ intrinsic
reproduces the final interrupt enable. The existing _ga5e storage symbol plus
0x69 addresses the original DGROUP:0AC7 message; no storage alias is added.
The local declaration order preserves the original twelve-byte stack frame.

src/F_652A.C reproduces all 66 bytes of the BIOS disk retry helper. Turbo C's
register pseudo-variables (_AH, _DL, _ES, _BX, and the other byte registers)
and __int__ intrinsic reproduce the BIOS register setup and INT instructions.
The ordinary C loop and 512-byte local buffer also reproduce the repeated
SS/address calculations exactly. The previous assumption that these operations
required inline assembly was too restrictive.

Both routines match using direct Turbo C compilation. Neither uses inline
assembly, explicit byte emission, padding, or object corrections. Their
original entry points, extents, and link positions are preserved.
Production now has 33 TASM-source modules, 291 source objects, and 440 linker
inputs. The documented C recoveries total 9,223 bytes across 25 functions.

Validation: both canonical integration tests pass, including fresh full EXE
comparison, all 106 ordered relocations, unchanged compiler objects, and
fixture-independent construction. Handover and queue audits pass. SHA256 remains
1259348425483d8d97fd8821860b47cfdf58fc8029711eb0ed0e78ab33807a10.

## F_4F63: DOS handle-2 output

src/DOSWRT2.C reproduces all 51 bytes with direct Turbo C compilation. Ordinary
locals and register variables reproduce the strlen call, length decrement,
far-pointer decomposition, and stack frame. Turbo C's register pseudo-variables
set BX, CX, DX, AH, and DS; __int__ emits the DOS interrupt instruction.
There is no inline assembly, explicit byte emission, padding, or object change.

The historical behavior is preserved precisely: the write count is strlen(text)
minus one, and DS is loaded from the buffer segment without a separate restore.
The function remains at its original entry and extent with its existing strlen
binding. No new storage or public symbol aliases are needed.

Production now has 32 TASM-source modules, 291 source objects, and 440 linker
inputs. The documented C recoveries total 9,274 bytes across 26 functions.

Validation: both canonical integration tests pass, including fresh full EXE
comparison, all 106 ordered relocations, unchanged compiler objects, and
fixture-independent construction. Handover and queue audits pass. SHA256 remains
1259348425483d8d97fd8821860b47cfdf58fc8029711eb0ed0e78ab33807a10.

## F_6BCF: timer interrupt C probe, held

The timer handler was tested with the same native interrupt and I/O facilities
that recovered F_699E. With the counter increment separated from its comparison,
the C body reproduces the original operations. The remaining difference is the
outer PUSHF/POPF pair: the native interrupt function emits 85 bytes versus the
original 87. Preserving _FLAGS in a local emits 89 bytes, adding POP SI after
PUSHF and PUSH SI before POPF. This is not an exact replacement.

The two source probes and bound byte results are retained in timer-c-probes.json.
Production remains ASM, with no binary patching, manual byte emission, or changed
acceptance criteria. This result does not prove that no exact C formulation
exists. It identifies the specific remaining compiler-code-generation obstacle.
There are still 32 production TASM-source modules; no executable inputs changed.

## Interface consolidation: dialog descriptors and save-slot records

No executable inputs changed in behavior; 45 production C sources now share
typed interfaces. include/DIALOG.H defines the 20-byte dialog descriptor for
f7e07, f8480 and f86c9 and every caller passes a typed object. include/C470.H
merges all partial 27-byte record views for gc470/gc360 and retires RECORD27.H.
F_5321/F_5382 gained the typed intro driver interface, fa036 its variadic
prototype, f652a/f7343 int parameters, and gc0cc one interrupt-vector type.
Each module was proven with tools/probe_module.py before installation, and
full acceptance remains byte-identical with 106 ordered relocations. Details
and the remaining census categories are in interface-audit.md.

## Readability groups: four multi-source C modules

Four recipes now compile contiguous, flag-identical member sources as single
translation units: C_01BC_0355 (video/BIOS initialization, 9 members),
C_7D91_880A (dialog layout, drawing and input, 12 members), C_A09D_A24E and
C_A33F_AD0E (the save-slot record table and its menus, 7 and 11 members,
split around F_A28D which owns private DATA). Each group was probed as a whole
module before activation, and full acceptance remains byte-identical with all
106 relocations: 256 source modules and 405 linker inputs, unchanged compiler
objects. Historical module identity is not established by these groups; they
are readability groupings whose bytes the pinned compiler reproduces.

Concatenation exposed two declaration conflicts that were resolved in the
members: F_0232 now declares g3904/gbe as int arrays like its neighbours, and
F_8267/F_8378 index the LAYOUT.H geometry words through same-address casts
instead of private array declarations. The public index was regenerated from a
fresh compile session, and the acceptance test derives the module count from
the plan and resolves recovered functions inside grouped modules.

## Nineteen further readability groups

A classification pass over the remaining contiguous, flag-identical runs
produced nineteen more recipes, each a coherent subsystem proven byte-exact as
one translation unit: board and sprite rendering (C_200F_2A70), board scroll
and animation (C_31C4_3986), program lifecycle (C_49E3_4A93), the intro
chapter (C_5321_56C6), tile hit-testing (C_5A3B_6021), the DOS critical-error
handler (C_622C_625D), indexed file I/O (C_6266_68AA), the IRQ1 keyboard pair
(C_695E_697D), C_6990_6997, the HUD panel (C_6FC3_747B), the grid puzzle
(C_8A37_969D), score-panel pieces (C_984C_9871, C_9962_99E2), the round-end
sequence (C_9A0E_9D79), level play (C_AF45_C15E), the player-select screen
(C_CDDD_D344), the rectangle table (C_D85F_D8F0), OPL bring-up (C_D99B_DA49),
OPL voice control (C_DB60_DDC7) and OPL register writes (C_E095_E54D). F_9908
and F_9D8E stay standalone: F_9908 needs its char pointer view of gc34e for
exact bytes, and F_9D8E owns native DATA the acceptance test inspects.

Concatenation surfaced and fixed conflicting extern types and duplicate
file-scoped tags in 31 members (type-consistent redeclarations, renamed tags,
suppressed duplicate includes); no statement order or semantics changed. Two
edits keep a byte view through a cast where neighbours declare a word
(gc04a/gc04c in F_5E98, gc132 in F_8BAB); those belong to the open
signed/width review. Production is now 127 source modules and 276 linker
inputs; acceptance remains byte-identical with all 106 relocations in order.

## Readable names

`python tools/rename_symbol.py old=new ... [--refresh-index]` renames a C-level
symbol across production sources, headers, production TASM, manifest and
structural bindings, module and data recipes, and records the original
reconstruction name and address in docs/current/symbol-names.json. Object
bytes never change, so the public index is regenerated from a fresh compile
session and full acceptance is rerun. Turbo C keeps 32 significant characters
including the OMF underscore, so names are limited to 31 characters.

254 symbols now carry behavior-based names proposed with cited evidence and
reviewed before application: 39 globals (energy meter, keyboard state, slot
counts, OPL driver state, and the dialog descriptors named after the dialogs
they show) and 215 functions covering the dialog routines and descriptors, the save-slot
table (slot_table, current_slot) and its menus, the option toggles, the HUD
panel and energy meter, the player-select screen, the round-end sequence, the
intro chapter, board and level play, the grid puzzle, video initialization, and
the OPL sound driver, resource and file I/O, DOS and BIOS services, timers,
sprites and text drawing. Functions whose purpose the code does not establish keep
their address names. Acceptance after the renames remains byte-identical with
all 106 relocations in order.

## Merged module sources

`python tools/merge_module.py <ID> src/NAME.C` turns a grouped module into one
source file: the members are written in link order under banners that keep the
original region ids and addresses, the recipe and the manifest regions point at
the new file, and the member files are removed. The compiled text is the same
concatenation, so bytes do not change. All 26 grouped C modules now live in
named files (DIALOG.C, HUD.C, LEVEL.C, PUZZLE.C, SLOTS.C, SLOTMENU.C,
RESOURCE.C, OPLREG.C, ...); each probes exact and full acceptance remains
byte-identical with all 106 relocations. The dialog geometry block and 12 more
globals carry readable names (290 renamed symbols in total).

Sixteen declaration differences remain deliberately local because a probe
shows the other spelling changes bytes: signed versus unsigned byte views
(gbfcc, voice_level_table), a far pointer versus int view (score_panel_x in
F_9908), and the word-split pushes of ui_gfx_blob in DIALOG.C.

## Named source files

`python tools/rename_source.py old=new ...` renames a production source file and
rewrites every reference (manifest regions, structural modules, module, data and
historical wave recipes, tests), then regenerates the plan. 46 standalone C
sources and the arithmetic assembler module now carry names derived from their
defined functions (TURNLOOP.C, CMDLINE.C, OPTIONS.C, SNDSTART.C, MUSIC.ASM, ...);
21 C files and 30 assembler files keep address names because their routines
have no established role yet. Every renamed module probes exact except
M_DDD9_DF98, whose library far call the single-module binder cannot resolve;
full acceptance covers it and remains byte-identical.
