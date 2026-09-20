# M_DDD9_DF98 symbolic arithmetic candidate

`asm/M_DDD9_DF98.ASM` is a complete 700-byte compatible reconstructed TASM
module for `F_DDD9`, `F_DE7E`, `F_DEFA`, and `F_DF98`. It emits the original
four public offsets and all 17 `_TEXT` fixup sites, targets, and addends.

The source emits its `ORG` contributions from high to low offset. TASM 1.0
therefore emits the complete historical descending FIXUPP sequence directly;
the arithmetic FIXUPP record reorder is not fundamentally necessary.

It is not yet canonical: the six reconstructed DGROUP data references in the
`F_DEFA` part currently use TASM external-frame fixups which overflow in the
whole link. Turbo C's corresponding object uses a group-relative frame. The
remaining bounded task is to express those six data operands with TASM's
matching group-relative OMF frame while preserving the already exact 700-byte
object and descending FIXUPP sequence. No final EXE or OMF object is patched.
