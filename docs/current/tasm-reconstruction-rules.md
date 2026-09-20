# TASM reconstruction rules

These rules are project-specific and exercised by
`python -m unittest discover -s tests -p test_tasm_reconstruction_rules.py`.
The tests compile actual examples with the pinned TASM 1.0 / Turbo C 2.0 tools.

## BRANCH_WIDTH

`jmp near ptr label` is **not sufficient** to preserve E9. The regression emits
`EB 02 90 90 C3` for a near jump followed by NOP and RET. Use this narrowly
approved helper where the oracle requires E9:

```asm
JMP_NEAR macro target
 db 0e9h
 dw target-$-2
endm
```

The equivalent tested helper text is available in `tools/tasm_rules.py`.
Runtime metric instrumentation accepts only the exact approved macro definitions.
Both helpers are already installed in the runtime header. A cheap model can
use them inside its assigned range without changing shared declarations. Ordinary short conditional branches should use local labels.
A same-segment CALL helper uses E8 and the same displacement expression; adding
new external fixups to the current zero-fixup runtime contract is not allowed.

## ABSOLUTE_MEMORY

For a numeric absolute memory operand, use an explicit segment:
`mov di,word ptr ds:[0c0e4h]` emits `8B 3E E4 C0`. Bare numeric brackets
(`mov di,word ptr [0c0e4h]`) emit the immediate load `BF E4 C0` with pinned
TASM 1.0. The production-line validation card exposed this failure and its
first-byte diagnostic is now automatically recognized. This is why Capstone
syntax is a guide, not a drop-in assembler source format.

## EXTRN_TYPES

Declare every external type explicitly, preferably one per line:

```asm
extrn function_a:near
extrn function_b:near
```

The pinned regression confirms E8 offset16 self-relative calls. A `:far` call
uses 9A and a pointer32 fixup. `extrn a,b:near` does **not** apply near to both:
the first call becomes a CS-relative indirect memory call. A declaration with
`:near` explicitly repeated for each comma-separated name is valid. Older
narrative evidence described this failure more broadly; this is the tested rule.

## DGROUP_FRAME

Declare `_DATA`, `_BSS` and `DGROUP group _DATA,_BSS` explicitly, including empty
segments. Use `offset DGROUP:symbol` for a group-framed data reference where the
oracle requires it. A matching byte and target alone do not prove the same fixup:
frame kind/name, target, addend, width, self-relative flag and emission order matter.
`ObjectModule.linker_fixups` preserves this complete ordered contract; `.fixups`
is the older compatibility view.

## FIXUP_ORDER and ORG_ORDER

TLINK relocation order follows object and contribution emission order. Do not sort
FIXUPP records by address during validation. TASM `ORG 5; call far_target; ORG 0;
call far_target` emits contiguous final code but fixups at sites **6, then 1**.
`asm/M_DDD9_DF98.ASM` applies this rule to the real arithmetic module. Preserve
its source contribution ordering. Runtime card mode rejects ORG/alignment and
include directives: changes to that topology belong to the supervisor.

## TC_INLINE_REGISTERS and POINTER_ABI

Turbo C can add register preservation when it sees SI/DI in inline ASM. The
regression changes `8B F0 C3` to `56 8B F0 5E C3` merely by expressing MOV SI,AX
symbolically in C. Recover runtime assembly in the native `.ASM` source.

Under the project's `-mc` model, default code pointers are near and default data
pointers are far. Explicit `char near *` vs `char far *` changes the dereference
from MOV to LES/ES code. `f()` is an unspecified argument list, not proof of zero
arguments. The ABI census flags declaration conflicts and preserves unknowns;
it does not prove that a source-level cleanup compiles identically.

## BOUNDARY, QUALITY, PUBLICS and FRESHNESS

A candidate must retain its exact extent and every public offset, including the
end public. Symbolic success means fewer assembler-measured unresolved bytes.
Changing DB to DW or adding a comment does not establish typed data. Typed data
needs a bounded, evidence-backed entry in `recipes/runtime/oracle.json` approved
by a supervisor. Unreachable bytes are UNKNOWN, not automatically DATA or CODE.

FAST compares the whole object and the card's edit boundary. ACCEPTANCE always
builds fresh and compares full EXE SHA and all 106 ordered relocations. Cached
objects are for FAST/research only, keyed by source, headers, flags, metadata,
tool identity and compiler-driver identity. Cache payload hashes are verified.
Old success files and historical probe receipts cannot authorize promotion.

Failure patterns and reusable resolutions live in
`recipes/runtime/failure-patterns.json`. Add a focused regression before treating
a newly diagnosed behavior as another known rule.

## UNROLLED_TRANSFER

Both bodies below were assembled with pinned TASM 1.0 and compared against all
80 historical repetitions in their respective runtime ranges. They create no
fixups. Cards now normally end at whole-body boundaries (six 15-byte bodies or
four 21-byte bodies); later partial promotions can leave partial bodies.
Transcribe only the selected card's instructions, not an entire enclosing loop.

Planar transfer, 0307–07B7, 15 bytes per body:

```asm
lodsw
mov bx,ax
shr bx,1
shr bx,1
shr bx,1
shr bx,1
mov ah,es:[bx]
stosb
```

Packed transform, 089F–0F2F, 21 bytes per body:

```asm
lodsw
mov ch,ah
mov bx,ax
rol bx,1
rol bx,1
rol bx,1
rol bx,1
mov ah,bl
stosw
mov al,ch
mov ah,bh
stosw
```

Keep these in native ASM. A C wrapper can introduce register saves. A normal
loop is not an equivalent reconstruction of computed-entry unrolled code.
