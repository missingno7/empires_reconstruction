/* decode.c -- semantic transcription of asm/DECODE.ASM (module
 * M_6D86_6F4B).  See portable/include/decode.h for the public contract.
 *
 * The ASM addresses offsets through segment:offset far pointers; the port
 * uses flat pointers instead, so every `si`/`di`/`bx`/`bp` cursor below is
 * modelled as a dos_uint byte offset relative to the start of the buffer it
 * indexes (offset 0 == the pointer argument itself).  16-bit truncation and
 * signed/unsigned comparisons the ASM performs on those registers are kept
 * explicit via dos_types.h helpers wherever the original instruction was
 * signed (cbw/cmp+jg/jl/jle) or wrapped (neg/inc/add on 16-bit registers).
 */
#include "decode.h"

/* ------------------------------------------------------------------ */
/* F_6D86 (asm/DECODE.ASM lines 35-64) -- PackBits-style RLE expansion. */
/* ------------------------------------------------------------------ */
dos_uint rle_packbits_decode(const uint8_t *src, uint8_t *dst, dos_uint max_len)
{
    /* si/di mirror the ASM registers as offsets from src/dst (offset 0).
     * bx = si(start=0) + max_len, per `mov bx,si; ...; add bx,ax`. */
    dos_uint si = 0;
    dos_uint di = 0;
    const dos_uint bx = max_len;

    for (;;) {
        /* lodsb; cbw -- sign-extend the control byte. */
        dos_int ax = (dos_int)(dos_char)src[si++];

        if (ax > 0) {
            /* L_lit: cx = ax (no +1 despite the module banner comment --
             * the code itself uses the control byte value directly);
             * rep movsb copies cx literal bytes. */
            dos_uint cx = (dos_uint)ax;
            while (cx--) {
                dst[di++] = src[si++];
            }
        } else {
            /* neg ax; inc ax -> cx = 1 - n; lodsb the repeat byte; rep
             * stosb writes it cx times. */
            dos_uint cx = (dos_uint)(dos_int)(-ax + 1);
            uint8_t b = src[si++];
            while (cx--) {
                dst[di++] = b;
            }
        }

        /* Both branches end with `cmp si,bx` using a SIGNED 16-bit
         * comparison (jl/jge); continue while (int16)si < (int16)bx. */
        if (!((dos_int)si < (dos_int)bx)) {
            break;
        }
    }

    return di; /* ax = di - dx (dx was dst's start, i.e. offset 0) */
}

/* ------------------------------------------------------------------ */
/* F_6DCC (asm/DECODE.ASM lines 66-233) -- variable-width pair-span
 * decompressor.  L_bit1/L_fill1/L_shift1 (and their L_*2 mirrors) become
 * lz_read_code(); L_head/L_sym1/L_sym2/L_go1/L_lit1/L_go2/L_lit2/L_exit
 * become the outer loop below, run twice per L_head (the ASM literally
 * duplicates the symbol-read/dispatch code for sym1 and sym2; they are
 * identical, so the port runs one implementation twice per head instead of
 * duplicating it). */
/* ------------------------------------------------------------------ */

/* Shared bit-reader step (L_bit1/L_fill1/L_shift1).  Reads `bitlen` bits
 * MSB-first from the byte stream, refilling *bitbuf from src[(*si)++] when
 * exhausted.  If inleft reaches 0 mid-read, the ASM abandons the remaining
 * bits (falls straight to L_done*) without a further shift; the caller
 * checks *inleft after the call, exactly as L_chk1/L_chk2 do -- but only
 * AFTER the ax==0x100 (width bump) check, so this helper does not decide
 * whether to exit on its own. */
static dos_uint lz_read_code(const uint8_t *src, dos_uint *si,
                              dos_int *bitcnt, dos_uchar *bitbuf,
                              dos_uint bitlen, dos_uint *inleft)
{
    dos_uint ax = 0;
    dos_uint cx = bitlen;
    dos_int dx = *bitcnt;
    dos_uchar bl = *bitbuf;

    while (cx != 0) {
        dx = (dos_int)(dx - 1);
        if (dx < 0) {
            /* L_fill1/L_fill2: refill from the input; the ASM reads this
             * byte unconditionally, even when it turns out to be the one
             * garbage byte past the meaningful data that inleft==0 below
             * discards -- do not add a bound to "fix" that. */
            bl = src[(*si)++];
            dx = 7;
            *inleft = dos_usub16(*inleft, 1);
            if (*inleft == 0) {
                /* jne L_shift1 not taken: store state, skip the shift, and
                 * abandon any remaining bits of this code. */
                break;
            }
        }
        /* L_shift1/L_shift2: rcl bl,1 ; rcl ax,1.  The carry flag entering
         * `rcl bl,1` is always 0 here (the preceding `cmp dx,0` never sets
         * CF, and none of mov/inc/dec touch it), so this is a plain
         * MSB-first bit extraction, not a rotate needing carry state. */
        {
            dos_uint bit = (dos_uint)((bl >> 7) & 1u);
            bl = (dos_uchar)(bl << 1);
            ax = (dos_uint)((ax << 1) | bit);
        }
        cx--;
    }

    *bitcnt = dx;
    *bitbuf = bl;
    return ax;
}

dos_uint lz_decompress(uint8_t *src, uint8_t *dst, dos_uint src_len)
{
    /* mov ax,[bp+0ch]; mov bp,si; mov bx,si; sub ax,2; inc ax; ...
     * add si,2 -- bp (table base) coincides with src offset 0 in this flat
     * model, so table math below needs no separate base term. */
    dos_uint si = 2;
    dos_uint di = 0;
    dos_uint bx = 0; /* table write cursor, relative to src */
    dos_uint inleft = dos_usub16(src_len, 1); /* (src_len - 2) + 1 */
    dos_int bitcnt = 0;
    dos_uchar bitbuf = 0;
    dos_uint bitlen = 9;

    for (;;) {
        int pass;

        /* L_head: mov [bx],di; add bx,2 */
        dos_wr16(src + bx, (uint16_t)di);
        bx = dos_uadd16(bx, 2);

        for (pass = 0; pass < 2; pass++) {
            dos_uint ax;

            for (;;) {
                ax = lz_read_code(src, &si, &bitcnt, &bitbuf, bitlen, &inleft);
                if (ax == 0x100u) {
                    /* inc bitlen; jmp L_sym1/L_sym2 -- retry regardless of
                     * inleft; the ASM checks inleft only in L_chk1/L_chk2,
                     * which come after this comparison. */
                    bitlen++;
                    continue;
                }
                break;
            }

            if (inleft == 0) {
                /* L_chk1/L_chk2 -> L_exit */
                return di;
            }

            if ((dos_int)ax <= 0xFF) {
                /* L_lit1/L_lit2: stosb */
                dst[di++] = (uint8_t)ax;
            } else {
                /* idx = code - 0x101; table entries are 2 bytes each,
                 * written at L_head, and indexed directly (bp == src
                 * offset 0 in this flat model). */
                dos_uint idx = dos_usub16(ax, 0x101u);
                dos_uint off = (dos_uint)(idx << 1);
                dos_uint start = dos_rd16(src + off);
                dos_uint end = dos_rd16(src + off + 2);
                dos_uint len = dos_usub16(end, start);
                dos_uint k;

                /* `test cx,1; je even; movsb; even: shr cx,1; rep movsw`
                 * (cld).  Transcribed literally: one leading byte when the
                 * length is odd, then WORD moves, each of which reads both
                 * source bytes before writing.  For a span that ends before
                 * di (every well-formed stream) this equals a byte copy; for
                 * a degenerate overlap at distance 1 the word move reads the
                 * not-yet-written byte, exactly as the 8086 did. */
                if (len & 1u) {
                    dst[di] = dst[start];
                    di = dos_uadd16(di, 1);
                    start = dos_uadd16(start, 1);
                }
                for (k = 0; k < (dos_uint)(len >> 1); k++) {
                    uint8_t b0 = dst[start];
                    uint8_t b1 = dst[(dos_uint)(start + 1u)];
                    dst[di] = b0;
                    dst[(dos_uint)(di + 1u)] = b1;
                    di = dos_uadd16(di, 2);
                    start = dos_uadd16(start, 2);
                }
            }
        }
    }
}

/* ------------------------------------------------------------------ */
/* F_6EFF (asm/DECODE.ASM lines 235-286) -- in-place planar (display_mode
 * != 2) 4bpp sprite/tile expansion. */
/* ------------------------------------------------------------------ */
void sprite_decode_4bpp_planar(uint8_t *q)
{
    int i;
    dos_uchar w, h;
    dos_uint count;
    dos_uint cx;
    dos_uint k;
    uint8_t *p;

    /* clear_upper_nibbles: q[0..15] as 8 little-endian words &= 0x0F0F;
     * this table doubles as the two xlatb lookup tables below. */
    for (i = 0; i < 8; i++) {
        dos_uint word = (dos_uint)(dos_rd16(q + i * 2) & 0x0F0Fu);
        dos_wr16(q + i * 2, (uint16_t)word);
    }

    /* add di,10h -- skip the remaining 16 bytes of the 32-byte header. */
    w = q[32];
    h = q[33];
    /* mul ah: AL*AH, unsigned byte*byte -> word, no truncation needed
     * (max 255*255 = 65025 fits in 16 bits). */
    count = (dos_uint)((dos_uint)w * (dos_uint)h);
    p = q + 34;

    /* expand_next_byte ... loop expand_next_byte: an x86 `loop` is
     * bottom-tested (decrement CX, continue while CX != 0), so with
     * count == 0 it would still execute once and then wrap CX to 0xFFFF --
     * reproduced here with a do/while-style loop rather than a pre-tested
     * `for`. */
    cx = count;
    k = 0;
    /* Deliberate deviation: a 0 x N record makes the 8086 `loop` run 65536
     * times through the rest of the segment.  No archive record is
     * degenerate (all 220 are covered by the golden tests); in the port that
     * would be a heap overflow, so the empty case is a no-op. */
    if (cx == 0)
        return;
    for (;;) {
        uint8_t b = p[k];
        uint8_t t_lo = q[b & 0x0Fu];
        uint8_t t_hi = q[(uint8_t)(b >> 4) & 0x0Fu];
        p[k] = (uint8_t)((t_hi << 4) | t_lo);
        k++;

        cx = dos_usub16(cx, 1);
        if (cx == 0) {
            break;
        }
    }
}

/* ------------------------------------------------------------------ */
/* F_6F4B (asm/DECODE.ASM lines 288-393) -- in-place mode-13h
 * (display_mode == 2) 4bpp sprite/tile expansion. */
/* ------------------------------------------------------------------ */
static uint8_t rotl8(uint8_t v, dos_uchar n)
{
    /* rol al,cl -- cl is always 0 or 4 in this routine. */
    n = (dos_uchar)(n & 7u);
    if (n == 0) {
        return v;
    }
    return (uint8_t)((uint8_t)(v << n) | (uint8_t)(v >> (8 - n)));
}

void sprite_decode_4bpp_mode13h(uint8_t *q)
{
    int i;
    dos_uchar w, h;
    dos_uint count;
    dos_uchar cl;
    dos_uchar row;
    uint8_t *p;

    /* xor al,al; stosb -- q[0] = 0. */
    q[0] = 0;

    /* header_loop: remap q[1..15] from VGA-attribute form to mode-13h form.
     * `and ax,0c030h; shr ax,1; shr ax,1; shr al,1; shr al,1; add
     * ax,1001h; or al,ah` decomposes cleanly into per-byte shifts because
     * the 0xC030 mask already zeroes AH's low 6 bits, so the two `shr
     * ax,1` (a real 16-bit shift) never carries a set bit from AH into
     * AL -- verified: AH' = (p&0xC0)>>2, AL' = (p&0x30)>>4, then
     * +0x1001 and OR. */
    for (i = 1; i <= 15; i++) {
        dos_uchar p8 = q[i];
        dos_uchar ah = (dos_uchar)(((p8 & 0xC0u) >> 2) + 0x10u);
        dos_uchar al = (dos_uchar)(((p8 & 0x30u) >> 4) + 1u);
        q[i] = (dos_uchar)(al | ah);
    }

    /* add di,10h -- skip to the width/height byte pair. */
    w = q[32];
    h = q[33];
    count = (dos_uint)((dos_uint)w * (dos_uint)h); /* mul ah */
    p = q + 34;

    cl = 0;
    row = w; /* mov dl,dh (dh was loaded from al == w before the mul) */
    if (count == 0)   /* same deliberate deviation as the planar decoder */
        return;

    /* decode_loop: bp (repurposed as the pixel counter) is decremented and
     * tested with `jz`, i.e. a do/while loop -- with count == 0 it still
     * runs once (bp wraps to 0xFFFF), matching the bottom-tested ASM. */
    for (;;) {
        uint8_t b = *p;
        uint8_t hi_idx = (uint8_t)((b >> 4) & 0x0Fu);
        uint8_t lo_idx = (uint8_t)(b & 0x0Fu);
        uint8_t hi = rotl8(q[hi_idx], cl);
        uint8_t lo = rotl8(q[lo_idx], cl);

        *p = (uint8_t)((hi & 0xF0u) | (lo & 0x0Fu));
        p++;

        count = dos_usub16(count, 1);
        if (count == 0) {
            break;
        }

        row = (dos_uchar)(row - 1);
        if (row != 0) {
            continue;
        }
        row = w;
        cl = (dos_uchar)(cl ^ 4);
    }
}
