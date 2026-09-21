/* decode.h -- semantic port of asm/DECODE.ASM (module M_6D86_6F4B).
 *
 * Four pure data transforms.  Offsets and lengths are 16-bit exactly as the
 * 8086 routines computed them; buffers are ordinary flat pointers.  Behavior
 * is specified instruction-by-instruction by asm/DECODE.ASM and must not be
 * "improved" (e.g. no early termination that the original lacked).
 */
#ifndef PORTABLE_DECODE_H
#define PORTABLE_DECODE_H

#include "dos_types.h"

/* F_6D86 -- PackBits-style RLE.  Signed control byte n: n > 0 -> copy n
 * literal bytes; n <= 0 -> repeat the next byte (1 - n) times.  Loops until
 * the source cursor reaches src + max_len (the `cmp si,bx` checks in the
 * ASM).  Returns bytes written. */
dos_uint rle_packbits_decode(const uint8_t *src, uint8_t *dst, dos_uint max_len);

/* F_6DCC -- variable-width pair-span decompressor.  src holds the record
 * payload (2-byte size header + bit stream, src_len bytes).  The routine
 * overwrites the FRONT of src with its {start,end} output-offset table
 * exactly as the original did (src is not const).  Returns the number of
 * bytes written to dst (the final 16-bit output offset). */
dos_uint lz_decompress(uint8_t *src, uint8_t *dst, dos_uint src_len);

/* F_6EFF -- in-place 4bpp sprite record fix-up for display modes != 2:
 * masks the 16 palette bytes to their low nibble and remaps every packed
 * pixel byte through that table (both nibbles). */
void sprite_decode_4bpp_planar(uint8_t *q);

/* F_6F4B -- in-place 4bpp sprite record fix-up for display mode 2 (CGA-class):
 * rewrites the 15-byte palette to the mode-13h form and expands pixels via
 * two table lookups with the alternating nibble rotation. */
void sprite_decode_4bpp_mode13h(uint8_t *q);

#endif
