/* primitives.c -- software graphics primitives, semantic transcription of
 * asm/RUNTIME_BLOCK.ASM (the mode-4/mode-13h dispatch slot's routines) plus
 * the present path (rt_083b / rt_089f).  See docs/portable/architecture.md
 * "Video model" and portable/include/gfx.h for the contract.
 *
 * Conventions used throughout this file:
 *  - 16-bit x86 registers are modelled as uint16_t (unsigned ops: shr/add/
 *    sub/mul as the ASM used them) or int16_t (only where the ASM used a
 *    signed test: js/jle/jg/sar) locals named after the register, so each
 *    block can be checked line-by-line against the cited ASM range.
 *  - GFX_ROW_BYTES (0xA0) is hard-coded exactly as every `add di,0a0h` /
 *    `mov si,0a0h` in the ASM is, never derived from a table.
 *  - Where the ASM decodes an address purely to add a per-row/per-group
 *    stride to a register that our C also uses as a raw pointer, we walk a
 *    second "inner" pointer and advance the outer row pointer by the FULL
 *    stride once per row; this is arithmetically identical to the ASM's
 *    "advance by (stride - consumed)" pattern (the two differ only in
 *    whether the consumed part was added before or after the leftover
 *    remainder) and keeps the C readable.  Any place this equivalence is
 *    not obviously safe is called out in a comment.
 */
#include "gfx.h"

#include <string.h>

/* ---- runtime_transparency_mask (asm/RUNTIME_BLOCK.ASM ~3148-3164).
 * mask(v) = (hi_nibble(v)==0 ? 0xF0 : 0) | (lo_nibble(v)==0 ? 0x0F : 0);
 * computed rather than tabulated (verified against the 256-byte dump: e.g.
 * v=0x00 -> 0xFF, v=0x01 -> 0xF0, v=0x10 -> 0x0F, v=0x11 -> 0x00). */
static uint8_t gfx_transparency_mask(uint8_t v)
{
    uint8_t m = 0;
    if ((v & 0xF0u) == 0) m = (uint8_t)(m | 0xF0u);
    if ((v & 0x0Fu) == 0) m = (uint8_t)(m | 0x0Fu);
    return m;
}

/* Shared even/odd-start 1bpp painter used by gfx_draw_char and
 * gfx_blit_image (both reuse the same unrolled 8-slot dispatch chain in
 * the ASM: runtime_even_pixel_dispatch / runtime_odd_pixel_dispatch at
 * ~3084-3090, entered mid-chain for a partial final byte).  Rather than
 * reproduce the self-modified computed-jmp machinery, this paints pixel
 * index k=0..count-1 using the closed-form byte/nibble placement the
 * dispatch chain provably reduces to (derived from and checked against
 * every rt_149x/rt_151x/rt_18bx label in RUNTIME_BLOCK.ASM lines
 * 2957-3090 and 3415-3473): even-start pixel k lands in byte k/2, high
 * nibble when k is even, low when odd; odd-start pixel k lands in byte
 * (k+1)/2, LOW nibble when k is even (k=0 is the byte's low nibble, the
 * high nibble of that first byte belongs to column x-1) and high when odd.
 * A byte is fetched every 8 pixels (MSB first); the last byte of a row may
 * be partial (1..7 bits) -- the ASM's `ah = bl & 2` trick exists only to
 * make the dispatch chain stop after exactly that many bits, which this
 * loop does directly via `n`. */
/* Returns the number of source bytes consumed (ceil(count/8), 0 if
 * count==0), since that need not equal any destination byte-stride the
 * caller separately computes (e.g. gfx_draw_char's dest stride is
 * (width+1)>>1, a different quantity from ceil(width/8)). */
static uint16_t gfx_paint_bits(uint8_t *base, const uint8_t *bits, uint16_t count,
                                int odd_start, uint8_t hi_color, uint8_t lo_color)
{
    uint16_t k = 0;
    uint16_t remaining = count;
    uint16_t consumed = 0;
    while (remaining > 0) {
        uint8_t byte = bits[consumed++];
        uint16_t n = (uint16_t)(remaining < 8u ? remaining : 8u);
        for (uint16_t i = 0; i < n; i++) {
            int bit = (byte >> (7 - i)) & 1;
            if (bit) {
                uint16_t byte_index = odd_start ? (uint16_t)((k + 1u) >> 1) : (uint16_t)(k >> 1);
                int high_nibble = odd_start ? (k & 1u) : !(k & 1u);
                uint8_t *p = base + byte_index;
                if (high_nibble) *p = (uint8_t)((*p & 0x0Fu) | hi_color);
                else             *p = (uint8_t)((*p & 0xF0u) | lo_color);
            }
            k++;
        }
        remaining = (uint16_t)(remaining - n);
    }
    return consumed;
}

/* ---- F_03A2 / bar: asm/RUNTIME_BLOCK.ASM 2255-2294 (runtime_bar_primitive). */
void gfx_bar(dos_int x, dos_int y, dos_int n)
{
    uint8_t al = (uint8_t)result;                       /* 2262 */
    uint16_t cx = (uint16_t)n;                            /* 2263 */
    uint8_t *rowptr = g3924[(uint16_t)y];                   /* 2264-2268 */
    uint16_t bx = (uint16_t)x;
    int odd = bx & 1u;                                        /* 2270: shr bx,1 -> CF = x&1 */
    bx = (uint16_t)(bx >> 1);
    uint8_t *di = rowptr;
    if (odd) {                                                  /* 2271: jae skips this block when x even */
        uint8_t dl = (uint8_t)(al & 0x0Fu);                        /* 2272-2273 */
        di[bx] = (uint8_t)((di[bx] & 0xF0u) | dl);                    /* 2274-2275 */
        di += 1;                                                        /* 2276 */
        cx = (uint16_t)(cx - 1u);                                          /* 2277 */
    }
    di += bx;                                                             /* 2279 */
    uint16_t fill_bx = 0;
    int cx_odd = (int)(cx & 1u);                                            /* 2281: shr cx,1 -> CF */
    cx = (uint16_t)(cx >> 1);
    if (cx_odd) fill_bx = 1;                                                  /* 2282-2283 */
    for (uint16_t k = 0; k < cx; k++) { *di++ = al; }                           /* 2285: rep stosb */
    if (fill_bx != 0) {                                                          /* 2286-2287 */
        uint8_t hi = (uint8_t)(al & 0xF0u);                                        /* 2288 */
        *di = (uint8_t)((*di & 0x0Fu) | hi);                                          /* 2289-2290 */
    }
}

/* ---- F_03A5 / vline: asm/RUNTIME_BLOCK.ASM 2296-2326 (runtime_f03a5). */
void gfx_vline(dos_int x, dos_int y, dos_int n)
{
    uint8_t al = (uint8_t)result;                        /* 2301 */
    uint16_t cx = (uint16_t)n;                             /* 2302 */
    uint8_t *di = g3924[(uint16_t)y];                        /* 2303-2306 */
    uint16_t bx = (uint16_t)x;
    int odd = bx & 1u;                                         /* 2308: shr bx,1 -> CF */
    bx = (uint16_t)(bx >> 1);
    uint8_t keep_mask, paint;
    if (odd) {                                                    /* 2309: jb rt_0fab */
        paint = (uint8_t)(al & 0x0Fu);
        keep_mask = 0xF0u;
    } else {
        paint = (uint8_t)(al & 0xF0u);
        keep_mask = 0x0Fu;
    }
    di += bx;                                                        /* 2317 */
    for (uint16_t k = 0; k < cx; k++) {                                  /* 2319-2323 */
        *di = (uint8_t)((*di & keep_mask) | paint);
        di += GFX_ROW_BYTES;
    }
}

/* ---- F_03A8 / clear: asm/RUNTIME_BLOCK.ASM 2328-2391 (runtime_clear_primitive,
 * falling into the shared rt_1034 epilogue). */
void gfx_clear_rect(dos_int x, dos_int y, dos_int w, dos_int h)
{
    uint8_t al = (uint8_t)result;                          /* 2334 */
    uint8_t *rowptr = g3924[(uint16_t)y];                     /* 2337-2340 */
    uint16_t bx = (uint16_t)x;
    int xodd = bx & 1u;                                          /* 2342: shr bx,1 -> CF */
    bx = (uint16_t)(bx >> 1);
    uint16_t rows = (uint16_t)h;
    uint16_t wlocal = (uint16_t)w;
    uint8_t *di = rowptr;

    if (xodd) {                                                     /* 2343: jae skips this block */
        uint8_t dl = (uint8_t)(al & 0x0Fu);                             /* 2348-2349 */
        uint8_t *p = rowptr + bx;                                          /* 2346 */
        for (uint16_t r = 0; r < rows; r++) {                                /* 2350-2354 */
            *p = (uint8_t)((*p & 0xF0u) | dl);
            p += GFX_ROW_BYTES;
        }
        di = rowptr + 1;                                                       /* 2355-2356 */
        wlocal = (uint16_t)(wlocal - 1u);                                         /* 2357: dec word ptr [bp+8] */
    }
    di += bx;                                                                       /* 2359 */
    int wodd = (int)(wlocal & 1u);                                                    /* 2361-2362: shr bx,1 -> CF */
    uint16_t bytes = (uint16_t)(wlocal >> 1);
    uint16_t fill_stride = (uint16_t)(GFX_ROW_BYTES - bytes);                            /* 2367 */
    uint8_t *row_start = di;
    for (uint16_t r = 0; r < rows; r++) {                                                   /* 2368-2373 */
        memset(di, al, bytes);
        di += fill_stride + bytes;
    }
    if (wodd) {                                                                                /* 2374-2375 */
        /* 2377: sub di,si undoes the fill loop's final advance, landing back
         * on the LAST drawn row, one past its main run; the trailing pass
         * then walks UPWARD (sub di,si) painting the high nibble. */
        di = row_start + (size_t)(rows - 1u) * GFX_ROW_BYTES + bytes;                             /* 2377 */
        uint8_t dl = (uint8_t)(al & 0xF0u);                                                          /* 2378-2379 */
        for (uint16_t r = 0; r < rows; r++) {                                                          /* 2382-2386 */
            *di = (uint8_t)((*di & 0x0Fu) | dl);
            di -= GFX_ROW_BYTES;
        }
    }
}

/* ---- F_03AB / fill: asm/RUNTIME_BLOCK.ASM 2392-2454 (runtime_f03ab). */
void gfx_fill_rect(dos_int x, dos_int y, dos_int w, dos_int h)
{
    uint16_t rows = (uint16_t)h;
    uint8_t *rowptr = g3924[(uint16_t)y];                      /* 2401-2404 */
    uint16_t xcol = (uint16_t)((uint16_t)x >> 1);
    int xodd = ((uint16_t)x) & 1u;                                /* 2406: shr bx,1 -> CF */
    uint16_t wlocal = (uint16_t)w;
    uint8_t *di;

    if (xodd) {                                                      /* 2407: jae skips this block */
        uint8_t *p = rowptr + xcol;                                     /* 2410 */
        for (uint16_t r = 0; r < rows; r++) {                              /* 2412-2415 */
            *p ^= 0x0Fu;
            p += GFX_ROW_BYTES;
        }
        di = rowptr + 1;                                                     /* 2416-2417 */
        wlocal = (uint16_t)(wlocal - 1u);                                       /* 2418 */
    } else {
        di = rowptr;
    }
    di += xcol;                                                                    /* 2420 */
    uint8_t *row1_after = di;                                                        /* 2421: push di */
    int wodd = (int)(wlocal & 1u);                                                     /* 2424: shr cx,1 -> CF */
    uint16_t bytes = (uint16_t)(wlocal >> 1);                                             /* 2428 */

    if (bytes != 0) {                                                                        /* 2429: jcxz skip */
        uint8_t *p = di;
        for (uint16_t r = 0; r < rows; r++) {                                                   /* 2430-2438 */
            for (uint16_t k = 0; k < bytes; k++) { p[k] ^= 0xFFu; }
            p += GFX_ROW_BYTES;
        }
    }

    di = row1_after + bytes;                                                                       /* 2440-2441: pop di; add di,bx */
    if (wodd) {                                                                                        /* 2442-2443 */
        for (uint16_t r = 0; r < rows; r++) {                                                             /* 2444-2449 */
            *di ^= 0xF0u;
            di += GFX_ROW_BYTES;
        }
    }
}

/* ---- F_03AE / save: asm/RUNTIME_BLOCK.ASM 2455-2493 (runtime_f03ae).
 * buf layout: word bytes, word rows, then bytes*rows raw planar data. */
void gfx_save_rect(dos_int x, dos_int y, dos_int w, dos_int h, uint8_t *buf)
{
    uint16_t ax = (uint16_t)x;
    uint16_t bx = (uint16_t)(ax + (uint16_t)w);                 /* 2464 */
    bx = (uint16_t)(bx - 1u);                                      /* 2466 */
    ax = (uint16_t)(ax >> 1);                                        /* 2467 */
    bx = (uint16_t)(bx >> 1);                                          /* 2468 */
    bx = (uint16_t)(bx - ax);                                            /* 2469 */
    bx = (uint16_t)(bx + 1u);                                              /* 2470: bx = inclusive byte width */
    uint16_t rows = (uint16_t)h;                                              /* 2471 */
    const uint8_t *si = g3924[(uint16_t)y] + ax;                                 /* 2472-2476 */
    uint8_t *di = buf;
    dos_wr16(di, bx);                                                              /* 2478 */
    dos_wr16(di + 2, rows);                                                          /* 2479 */
    di += 4;                                                                           /* 2480 */
    uint16_t stride_rem = (uint16_t)(GFX_ROW_BYTES - bx);                                 /* 2481-2482 */
    for (uint16_t r = 0; r < rows; r++) {                                                    /* 2483-2488 */
        memcpy(di, si, bx);
        di += bx;
        si += bx;
        si += stride_rem;
    }
}

/* ---- F_03B1 / restore: asm/RUNTIME_BLOCK.ASM 2494-2526 (runtime_f03b1). */
void gfx_restore_rect(dos_int x, dos_int y, const uint8_t *buf)
{
    uint16_t ax = (uint16_t)((uint16_t)x >> 1);                    /* 2502-2503 */
    uint8_t *di = g3924[(uint16_t)y] + ax;                            /* 2504-2508 */
    const uint8_t *si = buf;
    uint16_t bx = dos_rd16(si); si += 2;                                /* 2510-2511 */
    uint16_t rows = dos_rd16(si); si += 2;                                /* 2512-2513 */
    uint16_t stride_rem = (uint16_t)(GFX_ROW_BYTES - bx);                    /* 2514-2515 */
    for (uint16_t r = 0; r < rows; r++) {                                       /* 2516-2521 */
        memcpy(di, si, bx);
        si += bx;
        di += bx;
        di += stride_rem;
    }
}

/* Append a 4-byte dirty-rect record: word1=(y<<8)|(col&0xFF),
 * word2=(rows<<8)|(bytes&0xFF).  Shared shape used by gfx_wipe_rect,
 * gfx_blit_bitmap and gfx_copy_rect's forward path. */
static void gfx_dirty_queue_append(uint16_t y, uint16_t col, uint16_t rows, uint16_t bytes)
{
    dos_wr16(rect_queue_write_ptr, (uint16_t)((y << 8) | (col & 0xFFu)));
    rect_queue_write_ptr += 2;
    dos_wr16(rect_queue_write_ptr, (uint16_t)((rows << 8) | (bytes & 0xFFu)));
    rect_queue_write_ptr += 2;
}

/* ---- F_03B4 / wipe: asm/RUNTIME_BLOCK.ASM 2527-2585 (runtime_f03b4). */
void gfx_wipe_rect(dos_int sx, dos_int sy, dos_int w, dos_int h, dos_int dx, dos_int dy)
{
    uint8_t *di = g3924[(uint16_t)dy] + ((uint16_t)dx >> 1);          /* 2535-2541 */
    const uint8_t *si = g3924[(uint16_t)sy] + ((uint16_t)sx >> 1);      /* 2542-2548 */
    uint16_t bytes = (uint16_t)((uint16_t)w >> 1);                        /* 2549-2550 */
    uint16_t rows = (uint16_t)h;                                            /* 2551 */
    uint16_t stride_rem = (uint16_t)(GFX_ROW_BYTES - bytes);                  /* 2552-2553 */
    for (uint16_t r = 0; r < rows; r++) {                                        /* 2554-2563: rep movsw + rep movsb */
        memcpy(di, si, bytes);
        di += bytes; si += bytes;
        di += stride_rem; si += stride_rem;
    }
    if (gbc == 1 && (uint16_t)dy < 0xC8u) {                                          /* 2565-2569 */
        gfx_dirty_queue_append((uint16_t)dy, (uint16_t)((uint16_t)dx >> 1),
                                (uint16_t)h, bytes);                                     /* 2570-2580 */
    }
}

/* ---- F_03B7 / copy_rect_flip_v: asm/RUNTIME_BLOCK.ASM 2586-2633 (runtime_f03b7). */
void gfx_copy_rect_flip_v(dos_int sx, dos_int sy, dos_int w, dos_int h, dos_int dx, dos_int dy)
{
    uint8_t *di = g3924[(uint16_t)dy] + ((uint16_t)dx >> 1);          /* 2594-2600 */
    const uint8_t *si = g3924[(uint16_t)sy] + ((uint16_t)sx >> 1);      /* 2601-2607 */
    uint16_t bytes = (uint16_t)((uint16_t)w >> 1);                        /* 2608-2609 */
    uint16_t rows = (uint16_t)h;                                            /* 2610 */
    /* 2611-2618: `xchg ah,al` on (rows-1) then two adds is a byte-swap
     * multiply-by-0xA0 trick valid while rows-1 < 256 (true for any legal
     * rect here); land di on the LAST destination row. */
    di += (size_t)(uint16_t)(rows - 1u) * GFX_ROW_BYTES;
    for (uint16_t r = 0; r < rows; r++) {                                      /* 2620-2628 */
        memcpy(di, si, bytes);
        si += GFX_ROW_BYTES;
        di -= GFX_ROW_BYTES;
    }
}

/* ---- F_03BA / copy_rect_flip_h: asm/RUNTIME_BLOCK.ASM 2634-2682 (runtime_f03ba). */
void gfx_copy_rect_flip_h(dos_int sx, dos_int sy, dos_int w, dos_int h, dos_int dx, dos_int dy)
{
    uint8_t *di = g3924[(uint16_t)dy] + ((uint16_t)dx >> 1);          /* 2642-2648 */
    const uint8_t *si = g3924[(uint16_t)sy] + ((uint16_t)sx >> 1);      /* 2649-2655 */
    uint16_t bytes = (uint16_t)((uint16_t)w >> 1);                        /* 2656-2657 */
    uint16_t rows = (uint16_t)h;                                            /* 2658 */
    di += bytes; di -= 1u;                                                    /* 2659-2660: right edge of dest rect */
    for (uint16_t r = 0; r < rows; r++) {
        uint8_t *rowdi = di;
        for (uint16_t k = 0; k < bytes; k++) {                                     /* 2663-2671 */
            uint8_t al = *si++;
            al = (uint8_t)((al << 4) | (al >> 4));                                    /* rol al,1 x4: nibble swap */
            *rowdi-- = al;
        }
        si += (GFX_ROW_BYTES - bytes);                                                   /* 2672-2673 */
        di += GFX_ROW_BYTES;                                                                /* 2674-2675 (net after the -bytes/+bytes above) */
    }
}

/* ---- F_03BD / copy_rect_flip_hv: asm/RUNTIME_BLOCK.ASM 2683-2739 (runtime_f03bd). */
void gfx_copy_rect_flip_hv(dos_int sx, dos_int sy, dos_int w, dos_int h, dos_int dx, dos_int dy)
{
    uint8_t *di = g3924[(uint16_t)dy] + ((uint16_t)dx >> 1);          /* 2691-2697 */
    const uint8_t *si = g3924[(uint16_t)sy] + ((uint16_t)sx >> 1);      /* 2698-2704 */
    uint16_t bytes = (uint16_t)((uint16_t)w >> 1);                        /* 2705-2706 */
    uint16_t rows = (uint16_t)h;                                            /* 2707 */
    di += (size_t)(uint16_t)(rows - 1u) * GFX_ROW_BYTES;                       /* 2708-2715: byte-swap trick, last row */
    di += bytes; di -= 1u;                                                       /* 2716-2717: right edge */
    for (uint16_t r = 0; r < rows; r++) {
        uint8_t *rowdi = di;
        for (uint16_t k = 0; k < bytes; k++) {                                        /* 2720-2728 */
            uint8_t al = *si++;
            al = (uint8_t)((al << 4) | (al >> 4));
            *rowdi-- = al;
        }
        si += (GFX_ROW_BYTES - bytes);                                                    /* 2729-2730 */
        di -= GFX_ROW_BYTES;                                                                 /* 2731-2732 (net) */
    }
}

/* ---- F_03C0 / copy_rect_split: asm/RUNTIME_BLOCK.ASM 2740-2816 (runtime_f03c0).
 * Turns SOURCE ROWS into DESTINATION COLUMNS: consumes source rows in
 * pairs; row #1 of each pair supplies the LOW nibble of two vertically
 * adjacent (0xA0 apart) destination bytes, row #2 supplies the HIGH
 * nibble of the same two bytes; each outer step then moves one BYTE
 * column to the left and repeats for h>>1 pairs. */
void gfx_copy_rect_split(dos_int sx, dos_int sy, dos_int w, dos_int h, dos_int dx, dos_int dy)
{
    uint8_t *di0 = g3924[(uint16_t)dy] + ((uint16_t)dx >> 1);         /* 2748-2754 */
    const uint8_t *si = g3924[(uint16_t)sy] + ((uint16_t)sx >> 1);      /* 2755-2761 */
    uint16_t bytes = (uint16_t)((uint16_t)w >> 1);                        /* 2762-2763 */
    uint16_t pairs = (uint16_t)((uint16_t)h >> 1);                          /* 2764-2765 */
    di0 += pairs;                                                              /* 2766 */
    di0 -= 1u;                                                                    /* 2767 */

    for (uint16_t r = 0; r < pairs; r++) {
        uint8_t *p = di0;
        for (uint16_t k = 0; k < bytes; k++) {                                       /* 2771-2785: low_loop */
            uint8_t b = *si++;
            uint8_t hi_val = (uint8_t)(b >> 4);
            p[0]              = (uint8_t)((p[0] & 0xF0u) | hi_val);
            p[GFX_ROW_BYTES]  = (uint8_t)((p[GFX_ROW_BYTES] & 0xF0u) | (b & 0x0Fu));
            p += 2u * GFX_ROW_BYTES;
        }
        si += (GFX_ROW_BYTES - bytes);                                                  /* 2786-2787 */
        p = di0;
        for (uint16_t k = 0; k < bytes; k++) {                                             /* 2791-2805: high_loop */
            uint8_t b = *si++;
            uint8_t lo_shifted = (uint8_t)(b << 4);
            p[0]              = (uint8_t)((p[0] & 0x0Fu) | (b & 0xF0u));
            p[GFX_ROW_BYTES]  = (uint8_t)((p[GFX_ROW_BYTES] & 0x0Fu) | lo_shifted);
            p += 2u * GFX_ROW_BYTES;
        }
        si += (GFX_ROW_BYTES - bytes);                                                    /* 2806-2807 */
        di0 -= 1u;                                                                           /* 2809-2810 */
    }
}

/* ---- F_03C3 / copy_rect_split_flip_v: asm/RUNTIME_BLOCK.ASM 2817-2898
 * (runtime_f03c3).  Same row-pair transpose as gfx_copy_rect_split but the
 * per-byte pair walks UPWARD (0xA0 subtracted) and the outer step moves
 * one column to the RIGHT; the nibble roles are also swapped between the
 * two passes (see the ASM: this "low_loop" writes the HIGH nibble, this
 * "high_loop" writes the LOW nibble -- opposite of copy_rect_split). */
void gfx_copy_rect_split_flip_v(dos_int sx, dos_int sy, dos_int w, dos_int h, dos_int dx, dos_int dy)
{
    uint8_t *di0 = g3924[(uint16_t)dy] + ((uint16_t)dx >> 1);         /* 2825-2831 */
    const uint8_t *si = g3924[(uint16_t)sy] + ((uint16_t)sx >> 1);      /* 2832-2838 */
    uint16_t worig = (uint16_t)w;                                          /* 2839-2840 */
    uint16_t bytes = (uint16_t)(worig >> 1);                                 /* 2841 */
    uint16_t pairs = (uint16_t)((uint16_t)h >> 1);                             /* 2842-2843 */
    /* 2844-2850: byte-swap multiply-by-0xA0 trick, by (w-1) this time. */
    di0 += (size_t)(uint16_t)(worig - 1u) * GFX_ROW_BYTES;

    for (uint16_t r = 0; r < pairs; r++) {
        uint8_t *p = di0;
        for (uint16_t k = 0; k < bytes; k++) {                                    /* 2853-2867: low_loop (writes HIGH nibble) */
            uint8_t b = *si++;
            uint8_t lo_shifted = (uint8_t)(b << 4);
            p[0]                          = (uint8_t)((p[0] & 0x0Fu) | (b & 0xF0u));
            p[-(ptrdiff_t)GFX_ROW_BYTES]  = (uint8_t)((p[-(ptrdiff_t)GFX_ROW_BYTES] & 0x0Fu) | lo_shifted);
            p -= 2u * GFX_ROW_BYTES;
        }
        si += (GFX_ROW_BYTES - bytes);                                                 /* 2868-2869 */
        p = di0;
        for (uint16_t k = 0; k < bytes; k++) {                                            /* 2873-2887: high_loop (writes LOW nibble) */
            uint8_t b = *si++;
            uint8_t hi_val = (uint8_t)(b >> 4);
            p[0]                          = (uint8_t)((p[0] & 0xF0u) | hi_val);
            p[-(ptrdiff_t)GFX_ROW_BYTES]  = (uint8_t)((p[-(ptrdiff_t)GFX_ROW_BYTES] & 0xF0u) | (b & 0x0Fu));
            p -= 2u * GFX_ROW_BYTES;
        }
        si += (GFX_ROW_BYTES - bytes);                                                      /* 2888-2889 */
        di0 += 1u;                                                                             /* 2891-2892 */
    }
}

/* ---- F_03C6 / draw_char: asm/RUNTIME_BLOCK.ASM 2899-3090 (runtime_f03c6,
 * plus runtime_even_pixel_dispatch/runtime_odd_pixel_dispatch).  Font
 * metrics come from four byte tables at gc0e4 (width), gc0e6/gc0e2
 * (data-offset low/high byte), all indexed by glyph and relative to the
 * gc0e0 base; glyph bits live at gc0e0+gc0de+dataOffset. */
dos_int gfx_draw_char(dos_int x, dos_int y, dos_int glyph)
{
    uint16_t g = (uint16_t)glyph;
    uint16_t width = gc0e0[g + gc0e4];                       /* 2911-2914 */
    uint8_t  lo  = gc0e0[g + gc0e6];                            /* 2915-2916 */
    uint8_t  hib = gc0e0[g + gc0e2];                              /* 2917-2918 */
    const uint8_t *si = gc0e0 + gc0de + (uint16_t)((hib << 8) | lo); /* 2919-2920 */
    uint16_t rows = (uint16_t)dialog_line_height;                  /* 2921 */
    uint8_t color = (uint8_t)result;                                  /* 2922-2923 */
    uint8_t dh = (uint8_t)(color & 0xF0u);                              /* 2941-2944 */
    uint8_t dl = (uint8_t)(color & 0x0Fu);
    int odd = ((uint16_t)x) & 1u;                                            /* 2929-2932 */
    uint16_t xcol = (uint16_t)((uint16_t)x >> 1);                              /* 2934 */
    /* 2939-2940 in the ASM computes a row stride of (0xA0 - (width+1)>>1)
     * to add to a `di` that has already walked forward ceil(width/2) bytes
     * during painting, landing on row y+row's g3924 base exactly because
     * rows are contiguous 0xA0 apart.  gfx_paint_bits never moves its
     * `base` argument (a deliberate simplification, see its own comment),
     * so here we address each row directly through g3924 instead of
     * reproducing that remainder arithmetic against a stationary base. */
    for (uint16_t row = 0; row < rows; row++) {                                         /* 2945-3009 (even) / 3011-3074 (odd) */
        uint8_t *row_base = g3924[(uint16_t)(y + (dos_int)row)] + xcol;                     /* 2924-2927, 2934 */
        si += gfx_paint_bits(row_base, si, width, odd, dh, dl);   /* consumes ceil(width/8) src bytes */
    }
    return (dos_int)width;                                                                /* 3075-3076: mov ax,bx; shr ax,1 */
}

/* ---- F_03C9 / blit: asm/RUNTIME_BLOCK.ASM 3091-3146 (runtime_blit_primitive).
 * bitmap+0x20 holds a (bytesPerRow, rows) byte pair; data follows at +0x22. */
void gfx_blit_bitmap(dos_int x, dos_int y, const uint8_t *bitmap)
{
    uint8_t *di = g3924[(uint16_t)y] + ((uint16_t)x >> 1);       /* 3099-3105 */
    const uint8_t *si = bitmap + 0x20;                              /* 3106-3107 */
    uint16_t bytes = si[0];                                           /* 3108-3110 */
    uint16_t rows  = si[1];                                             /* 3111-3112 */
    si += 2;
    uint16_t stride_rem = (uint16_t)(GFX_ROW_BYTES - bytes);              /* 3114-3115 */
    for (uint16_t r = 0; r < rows; r++) {                                    /* 3116-3124: rep movsw + rep movsb */
        memcpy(di, si, bytes);
        di += bytes; si += bytes;
        di += stride_rem;
    }
    if (gbc == 1 && (uint16_t)y < 0xC8u) {                                      /* 3127-3131 */
        gfx_dirty_queue_append((uint16_t)y, (uint16_t)((uint16_t)x >> 1),
                                rows, bytes);                                       /* 3132-3141 */
    }
}

/* ---- F_03CC / copy: asm/RUNTIME_BLOCK.ASM 3165-3372 (runtime_copy_primitive
 * + rt_17c7 flip path).  bitmap+0x20 holds a (bytesPerRow, rows) pair,
 * data follows at +0x22; g94/g96 clip rows, g98/g9a clip byte-columns.
 * Transparency: dst = (dst & mask(src)) | src via gfx_transparency_mask. */
void gfx_copy_rect(dos_int x, dos_int y, const uint8_t *bitmap, dos_int flip)
{
    const uint8_t *si = bitmap + 0x20;                                 /* 3173-3174 */
    uint16_t header_bytes = si[0];                                        /* 3175 */
    uint16_t header_rows  = si[1];
    si += 2;                                                                /* 3176 */

    uint16_t cx = header_bytes;           /* clipped byte-width */
    uint16_t dxr = header_rows;             /* clipped row count */
    int16_t yv = (int16_t)y;                  /* di, used as a plain value until the row lookup */

    int16_t ax = (int16_t)(g96 - yv);            /* 3182-3183 */
    if (ax < 0) return;                             /* 3184: js rt_1767 (bail, nothing drawn) */
    ax = (int16_t)(ax + 1);                           /* 3185 */
    if ((uint16_t)dxr > (uint16_t)ax) dxr = (uint16_t)ax;  /* 3186-3188: jbe skip / clamp */

    ax = (int16_t)(g94 - yv);                            /* 3190-3191 */
    if (ax > 0) {                                           /* 3192: jle skip this block */
        dxr = (uint16_t)(dxr - (uint16_t)ax);                  /* 3193 */
        if ((int16_t)dxr <= 0) return;                            /* 3194: jbe rt_1767 (bail) */
        yv = (int16_t)(yv + ax);                                     /* 3195 */
        si += (uint8_t)ax * (uint8_t)cx;                               /* 3196-3197: mul cl (AL*CL, 8-bit) */
    }

    int16_t bp_extra = 0;

    if (flip == 0) {                                                        /* 3200-3202 */
        uint16_t bxcol = (uint16_t)((int16_t)x >> 1);                          /* 3204: sar bx,1 (signed) */
        ax = (int16_t)(g9a - (int16_t)bxcol);                                    /* 3207-3208 */
        if (ax < 0) return;                                                        /* 3209 */
        ax = (int16_t)(ax + 1);                                                      /* 3210 */
        if ((uint16_t)cx > (uint16_t)ax) {                                             /* 3211-3212 */
            bp_extra = (int16_t)(cx - (uint16_t)ax);                                      /* 3213 */
            cx = (uint16_t)ax;                                                              /* 3214 */
        }
        ax = (int16_t)(g98 - (int16_t)bxcol);                                                 /* 3217-3218 */
        if (ax > 0) {                                                                           /* 3219: jle skip */
            cx = (uint16_t)((int16_t)cx - ax);                                                     /* 3220 */
            if ((int16_t)cx > 0) {                                                                    /* 3221: ja */
                bxcol = (uint16_t)(bxcol + (uint16_t)ax);                                                 /* 3231 */
                bp_extra = (int16_t)(bp_extra + ax);                                                        /* 3232 */
                si += (uint16_t)ax;                                                                           /* 3233 */
            } else {
                return;                                                                                          /* cx<=0: rt_1767 bail */
            }
        }
        if (gbc == 1 && (uint16_t)yv < 0xC8u) {                                                                    /* 3237-3240 */
            gfx_dirty_queue_append((uint16_t)yv, bxcol, dxr, cx);                                                     /* 3241-3249 */
        }
        uint8_t *di = g3924[(uint16_t)yv] + bxcol;                                                                       /* 3250-3255 */
        for (uint16_t row = 0; row < dxr; row++) {                                                                          /* 3258-3274 */
            uint8_t *rdi = di;
            for (uint16_t k = 0; k < cx; k++) {
                uint8_t src = *si++;
                uint8_t mask = gfx_transparency_mask(src);
                *rdi = (uint8_t)((*rdi & mask) | src);
                rdi++;
            }
            di += GFX_ROW_BYTES;
            si += bp_extra;
        }
    } else {                                                                      /* rt_17c7: flip path, 3282-3372 */
        /* 3285-3289: bx = ((x + 2*bytesPerRow) >> 1) - 1, the rightmost dest
         * byte column of the (unclipped) mirrored rect. */
        uint16_t rightcol = (uint16_t)((uint16_t)(((uint16_t)x + 2u * header_bytes) >> 1) - 1u);
        ax = (int16_t)((int16_t)rightcol - g98);                                       /* 3290 */
        if (ax < 0) return;                                                              /* 3291 */
        ax = (int16_t)(ax + 1);                                                            /* 3292 */
        if ((uint16_t)cx > (uint16_t)ax) {                                                   /* 3293-3294 */
            bp_extra = (int16_t)(cx - (uint16_t)ax);                                            /* 3295 */
            cx = (uint16_t)ax;                                                                    /* 3296 */
        }
        ax = (int16_t)((int16_t)rightcol - g9a);                                                    /* 3299-3300 */
        if (ax > 0) {                                                                                  /* 3301: jbe skip */
            cx = (uint16_t)((int16_t)cx - ax);                                                            /* 3302 */
            if ((int16_t)cx > 0) {                                                                           /* 3303: ja */
                rightcol = (uint16_t)(rightcol - (uint16_t)ax);                                                  /* 3313 */
                bp_extra = (int16_t)(bp_extra + ax);                                                               /* 3314 */
                si += (uint16_t)ax;                                                                                  /* 3315 */
            } else {
                return;                                                                                                /* rt_17f0 bail */
            }
        }
        if (gbc == 1 && (uint16_t)yv < 0xC8u) {                                                                          /* 3319-3322 */
            uint8_t leftcol = (uint8_t)((uint8_t)rightcol - (uint8_t)cx + 1u);                                              /* 3323-3327 */
            gfx_dirty_queue_append((uint16_t)yv, leftcol, dxr, cx);                                                            /* 3328-3333 */
        }
        uint8_t *di = g3924[(uint16_t)yv] + rightcol;                                                                            /* 3337-3341 */
        for (uint16_t row = 0; row < dxr; row++) {                                                                                  /* 3344-3365 */
            uint8_t *rdi = di;
            for (uint16_t k = 0; k < cx; k++) {
                uint8_t src = *si++;                              /* 3348-3349: mov al,[si];inc si (SI still walks forward) */
                src = (uint8_t)((src << 4) | (src >> 4));           /* 3350-3353: ror al,1 x4 (nibble swap) */
                uint8_t mask = gfx_transparency_mask(src);
                *rdi = (uint8_t)((*rdi & mask) | src);
                rdi--;                                                  /* 3358: stosb under std -- DI decrements */
            }
            di += GFX_ROW_BYTES;
            si += bp_extra;
        }
    }
}

/* ---- F_03CF / blit_image: asm/RUNTIME_BLOCK.ASM 3373-3473 (runtime_f03cf).
 * image = [bytesPerRow][rows][1bpp bits...]; always the even-x-start path
 * (di is unconditionally pre-decremented in the ASM); bx=bytes*4 there
 * means pixel_count = bytes*2 (task contract), i.e. bytesPerRow is the
 * DESTINATION planar byte width, not a literal source-bitmap byte count. */
void gfx_blit_image(dos_int x, dos_int y, const uint8_t *image)
{
    uint8_t color = (uint8_t)result;                          /* 3383 */
    uint8_t dh = (uint8_t)(color & 0xF0u);                       /* 3384-3385 */
    uint8_t dl = (uint8_t)(color & 0x0Fu);
    const uint8_t *si = image;
    uint16_t bytes = si[0];                                         /* 3397-3398 */
    uint16_t rows  = si[1];                                            /* 3403-3404 */
    si += 2;
    uint16_t pixel_count = (uint16_t)(bytes * 2u);                         /* 3401-3402: bx=bytes*4 (=2*pixels) */
    uint16_t xcol = (uint16_t)((uint16_t)x >> 1);                            /* 3392-3394 */
    /* See gfx_draw_char's comment: address each row directly through
     * g3924 instead of reproducing the ASM's (0xA0-bytes) remainder
     * against a `di` that (unlike here) it kept walking forward. */
    for (uint16_t row = 0; row < rows; row++) {                               /* 3405 dec di folded into gfx_paint_bits' even-start formula */
        uint8_t *row_base = g3924[(uint16_t)(y + (dos_int)row)] + xcol;
        si += gfx_paint_bits(row_base, si, pixel_count, 0 /* even start */, dh, dl);
    }
}

/* ---- F_03D2 / set_pixel: asm/RUNTIME_BLOCK.ASM 3480-3512 (runtime_f03d2). */
void gfx_set_pixel(dos_int x, dos_int y)
{
    uint8_t *di = g3924[(uint16_t)y];                     /* 3485-3488 */
    uint16_t bx = (uint16_t)x;
    int odd = bx & 1u;                                        /* 3490: shr bx,1 -> CF */
    bx = (uint16_t)(bx >> 1);
    uint8_t color = (uint8_t)result;
    if (!odd) {                                                  /* fallthrough (even x): high nibble */
        uint8_t keep = (uint8_t)(di[bx] & 0x0Fu);                    /* 3493-3494 */
        uint8_t paint = (uint8_t)(color << 4);                          /* 3495-3499 */
        di[bx] = (uint8_t)(keep | paint);                                  /* 3507-3509 */
    } else {                                                          /* 3491: jb rt_1959 (odd x): low nibble */
        uint8_t keep = (uint8_t)(di[bx] & 0xF0u);                        /* 3502-3504 */
        uint8_t paint = (uint8_t)(color & 0x0Fu);                           /* 3505-3506 */
        di[bx] = (uint8_t)(keep | paint);
    }
}

/* ---- F_03D5 / get_pixel: asm/RUNTIME_BLOCK.ASM 3513-3540 (runtime_f03d5). */
dos_int gfx_get_pixel(dos_int x, dos_int y)
{
    const uint8_t *di = g3924[(uint16_t)y];               /* 3518-3521 */
    uint16_t bx = (uint16_t)x;
    int odd = bx & 1u;                                        /* 3523: shr bx,1 -> CF */
    bx = (uint16_t)(bx >> 1);
    uint8_t al = di[bx];
    if (!odd) {                                                  /* fallthrough (even x): high nibble -> low */
        return (dos_int)((al & 0xF0u) >> 4);                        /* 3526-3531 */
    }
    return (dos_int)(al & 0x0Fu);                                     /* 3524-3525, 3533-3536: jb rt_1997 (odd x) */
}

/* ---- gfx_box / present: asm/RUNTIME_BLOCK.ASM 1149-1197 (rt_083b setup,
 * dispatch slot 4 = mode 13h) + 1198-2253 (rt_089f unrolled transform and
 * row-tail; the self-modified computed jmp just unrolls exactly `groups`
 * iterations per row, reproduced here as a plain loop).  Converts the
 * packed 4bpp framebuffer rect to 8bpp VRAM bytes; gfx_vram byte v is
 * later resolved to a colour by the caller via the 256-entry DAC through
 * v (see docs/portable/architecture.md "Video model"). */
void gfx_box(dos_int x, dos_int y, dos_int w, dos_int h)
{
    uint16_t xv = (uint16_t)x;
    uint16_t wv = (uint16_t)w;
    uint16_t right = (uint16_t)(xv + wv - 1u);                       /* 1169-1172 */
    uint16_t xeven = (uint16_t)(xv & ~(uint16_t)1u);                    /* 1173-1175: (x>>1)<<1 */
    uint16_t xcol  = (uint16_t)(xv >> 1);                                 /* 1176 */
    /* 1177-1182: groups = ((x+w-1)>>2) - (x>>2) + 1 */
    uint16_t groups = (uint16_t)((uint16_t)(right >> 2) - (uint16_t)(xv >> 2) + 1u);

    const uint8_t *src = g3924[(uint16_t)y] + xcol;                        /* 1157-1160, 1177 */
    uint8_t *dst = gfx_vram + (size_t)(uint16_t)y * GFX_VRAM_W + xeven;       /* 1157-1168, 1173-1175 (y*320 via the y*64+y*256 shl trick) */
    uint16_t rows = (uint16_t)h;                                                 /* 1192 */

    for (uint16_t row = 0; row < rows; row++) {
        const uint8_t *s = src;
        uint8_t *d = dst;
        for (uint16_t g = 0; g < groups; g++) {                                       /* rt_089f body, 1199-1211 (one of 80 unrolled copies) */
            uint8_t b0 = s[0];
            uint8_t b1 = s[1];
            s += 2;
            /* rol bx,1 x4 on the 16-bit word (b1<<8|b0): see architecture
             * doc / task contract for the derivation of these four bytes. */
            d[0] = b0;
            d[1] = (uint8_t)((b0 << 4) | (b1 >> 4));
            d[2] = b1;
            d[3] = (uint8_t)((b1 << 4) | (b0 >> 4));
            d += 4;
        }
        src += GFX_ROW_BYTES;                                                            /* 2240: add si,dx (dx=0xA0-2*groups) + the 2*groups the inner loop already advanced == 0xA0 */
        dst += GFX_VRAM_W;                                                                  /* 2241: add di,bp (bp=0x140-4*groups) + the 4*groups the inner loop already advanced == 0x140 */
    }
    gfx_vram_generation++;
}
