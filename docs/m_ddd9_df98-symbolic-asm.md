# M_DDD9_DF98 symbolic arithmetic candidate

`asm/M_DDD9_DF98.ASM` is a complete 700-byte compatible reconstructed TASM
module for `F_DDD9`, `F_DE7E`, `F_DEFA`, and `F_DF98`. It emits the original
four public offsets and all 17 `_TEXT` fixup sites, targets, and addends.

The source emits its `ORG` contributions from high to low offset. TASM 1.0
therefore emits the complete historical descending FIXUPP sequence directly;
the arithmetic FIXUPP record reorder is not fundamentally necessary.

The six reconstructed `F_DEFA` DATA references use explicit `DGROUP` operands
and the same group-relative frame as Turbo C. The module is canonical in the
exact build: it links without an OMF transform and preserves the byte-identical
executable. It remains a compatible reconstructed module rather than a proven
historical translation unit.
