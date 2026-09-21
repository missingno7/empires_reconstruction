/* gfx_vga.c -- 8bpp software graphics driver (display selector 5), semantic
 * transcription of docs/portable/reference/AE000_002-vga-runtime.lst (the
 * VGA replacement runtime overlay for asm/RUNTIME_BLOCK.ASM).  See
 * docs/portable/architecture.md "Video model" and portable/gfx/gfx_drivers.h
 * for the contract.
 *
 * Framebuffer model: 320 x 488 logical pixels, ONE byte per pixel (a VGA DAC
 * index 0..255 into the 256-entry palette g11e), 320 (0x140) bytes per row.
 * Every primitive addresses rows through g3924[] with the stride hard-coded
 * 0x140, exactly as every `add di,0x140` / `sub di,0x141` in the listing is
 * -- never derived from gfx_row_bytes() or any other table.  Coordinates and
 * sizes are historical 16-bit ints; no bounds checks are added beyond what
 * the listing itself performs.
 *
 * Bitmap art stays packed 4bpp on disk; blitters that draw art (blit_bitmap,
 * copy_rect) expand nibbles through the bitmap's own 16-byte VGA colour
 * table (bitmap+0x10) at draw time -- see their own comments below.
 *
 * Conventions, matching gfx_planar.c:
 *  - 16-bit x86 registers are modelled as uint16_t (unsigned ops) or
 *    int16_t (only where the listing used a signed test: js/jle/jg/sar),
 *    locals named after the register, so each block can be checked
 *    line-by-line against the cited .lst address range.
 *  - Where the listing uses `push reg` / do work / `pop reg` to save a
 *    row-start pointer across an inner loop that also advances that same
 *    register, the C keeps a separate "outer" (row-start) and "inner"
 *    (walking) pointer instead of literally modelling the stack -- this is
 *    the identical simplification gfx_planar.c already uses.
 */
#include "gfx.h"
#include "gfx_drivers.h"

#include <string.h>

/* Row stride for display selector 5, hard-coded exactly as every
 * `0x140`/`0x13f`/`0x141` literal in the listing is. */
#define VGA_ROW_BYTES 0x140

/* ---- Shared even-start 1bpp painter used by vga_draw_char and
 * vga_blit_image (both reuse the same unrolled 8-slot dispatch chain in the
 * listing: 0783-07D3 for draw_char / 081D-086D for blit_image, entered
 * mid-chain for a partial final byte via the `loop` instruction after each
 * pixel).  Unlike gfx_planar.c's gfx_paint_bits, VGA pixels are whole bytes
 * (1 byte per pixel, no nibble packing), so painting bit k just writes
 * base[k] -- there is no even/odd-start distinction here.
 *
 * Returns the number of source bytes consumed (ceil(count/8), 0 if
 * count==0). */
static uint16_t vga_paint_bits(uint8_t *base, const uint8_t *bits, uint16_t count, uint8_t color)
{
    uint16_t k = 0;
    uint16_t remaining = count;
    uint16_t consumed = 0;
    while (remaining > 0) {
        uint8_t byte = bits[consumed++];
        uint16_t n = (uint16_t)(remaining < 8u ? remaining : 8u);
        for (uint16_t i = 0; i < n; i++) {
            /* `shl al,1` then `jae` tests the bit that was just shifted out
             * of the top (the MSB before the shift); compute it first, then
             * shift, to match without needing the carry flag. */
            uint8_t bit = (uint8_t)(byte & 0x80u);
            byte = (uint8_t)(byte << 1);
            if (bit) base[k] = color;
            k++;
        }
        remaining = (uint16_t)(remaining - n);
    }
    return consumed;
}

/* Append a 4-byte dirty-rect record: word1=(y<<8)|(col&0xFF),
 * word2=(rows<<8)|(bytes&0xFF) -- identical record shape to
 * gfx_planar.c's, and still HALF units (x>>1, w>>1) even though VGA rows
 * are full bytes per pixel; the listing computes it that way (`shr ax,1`
 * on x/w before packing into the record) in every routine that appends
 * one, so it is transcribed literally rather than "fixed". */
static void vga_dirty_queue_append(uint16_t y, uint16_t col, uint16_t rows, uint16_t bytes)
{
    dos_wr16(rect_queue_write_ptr, (uint16_t)((y << 8) | (col & 0xFFu)));
    rect_queue_write_ptr += 2;
    dos_wr16(rect_queue_write_ptr, (uint16_t)((rows << 8) | (bytes & 0xFFu)));
    rect_queue_write_ptr += 2;
}

/* Entry 0 of the 20-entry jump table (VIDEO.H order), .lst 03D8-03DD
 * (`mov ax,0x13; int 0x10; ret`): a BIOS INT 10h video-mode set.  It has no
 * portable meaning (SDL3 owns mode selection instead -- see
 * portable/platform/sdl3) and is not one of the gfx.h primitives, so it is
 * not transcribed as a function here; noted for completeness only. */

/* ---- gfx_box (present): .lst 03DE-0424.  Unlike the planar driver's
 * gfx_box (which unpacks 4bpp nibble pairs into 8bpp VRAM bytes), the VGA
 * framebuffer IS already 8bpp/1-byte-per-pixel at the same 320-byte row
 * stride as gfx_vram, so this is a literal per-row byte copy: `rep movsb`
 * w bytes per row from g3924[y]+x to VRAM offset y*320+x, for h rows (the
 * y*320 built as the classic (y<<8)+(y<<6) shl/xchg trick, same as
 * gfx_planar.c's gfx_box and RUNTIME_BLOCK's rt_083b use for the same
 * purpose -- see 03EA-0408). */
void vga_box(dos_int x, dos_int y, dos_int w, dos_int h)
{
    uint16_t yv = (uint16_t)y;
    uint16_t xv = (uint16_t)x;
    uint16_t wv = (uint16_t)w;
    uint16_t rows = (uint16_t)h;
    uint8_t *dst = gfx_vram + (size_t)yv * GFX_VRAM_W + xv;      /* es:di, 03E5-0406 */
    const uint8_t *src = g3924[yv] + xv;                            /* ds:si, 03F3-0408 */

    for (uint16_t r = 0; r < rows; r++) {                              /* 0415-041E */
        memcpy(dst, src, wv);                                            /* 0417: rep movsb */
        dst += VGA_ROW_BYTES;                                              /* 0419: add di,ax (ax=0x140-w) + movsb's own +w */
        src += VGA_ROW_BYTES;                                                /* 041B: add si,ax, same */
    }
    gfx_vram_generation++;
}

/* ---- gfx_bar: .lst 0425-0446. */
void vga_bar(dos_int x, dos_int y, dos_int n)
{
    uint8_t *di = g3924[(uint16_t)y] + (uint16_t)x;   /* 042C-0437 */
    uint8_t al = (uint8_t)result;                        /* 043D */
    uint16_t cx = (uint16_t)n;                              /* 043A */
    memset(di, al, cx);                                        /* 0440: rep stosb */
}

/* ---- gfx_vline: .lst 0447-046E. */
void vga_vline(dos_int x, dos_int y, dos_int n)
{
    uint8_t *di = g3924[(uint16_t)y] + (uint16_t)x;   /* 044E-0459 */
    uint8_t al = (uint8_t)result;                        /* 045F */
    uint16_t cx = (uint16_t)n;                              /* 045C */
    for (uint16_t k = 0; k < cx; k++) {                        /* 0465-0468 */
        *di = al;
        di += VGA_ROW_BYTES;                                     /* bx=0x13f; stosb(+1)+add di,bx(+0x13f) = +0x140 */
    }
}

/* ---- gfx_clear_rect: .lst 046F-049F. */
void vga_clear_rect(dos_int x, dos_int y, dos_int w, dos_int h)
{
    uint8_t *di = g3924[(uint16_t)y] + (uint16_t)x;   /* 0476-0481 */
    uint8_t al = (uint8_t)result;                        /* 0487 */
    uint16_t wv = (uint16_t)w;                              /* 048A */
    uint16_t rows = (uint16_t)h;                              /* 0484 */
    for (uint16_t r = 0; r < rows; r++) {                        /* 0492-0499 */
        memset(di, al, wv);                                        /* 0494: rep stosb */
        di += VGA_ROW_BYTES;                                          /* 0490/0496: si=0x140-w, add di,si (+ rep's own +w) */
    }
}

/* ---- gfx_fill_rect: .lst 04A0-04D3.  XOR 0x0F (low nibble only, literally
 * `mov al,0xf` -- NOT 0xFF) on every byte of the w x h rect. */
void vga_fill_rect(dos_int x, dos_int y, dos_int w, dos_int h)
{
    uint8_t *di = g3924[(uint16_t)y] + (uint16_t)x;   /* 04A7-04B2 */
    uint16_t wv = (uint16_t)w;                              /* 04BA */
    uint16_t rows = (uint16_t)h;                              /* 04B5 */
    for (uint16_t r = 0; r < rows; r++) {                        /* 04C2-04CD */
        for (uint16_t k = 0; k < wv; k++) di[k] ^= 0x0Fu;           /* 04C4-04C8 */
        di += VGA_ROW_BYTES;                                          /* 04BD/04CA: si=0x140-w, add di,si */
    }
}

/* ---- gfx_save_rect: .lst 04D4-050A.  buf layout: word w, word h, then
 * rows*w raw bytes (no per-row padding, unlike the g3924 source rows). */
void vga_save_rect(dos_int x, dos_int y, dos_int w, dos_int h, uint8_t *buf)
{
    const uint8_t *si = g3924[(uint16_t)y] + (uint16_t)x;   /* 04DB-04E6 */
    uint8_t *di = buf;
    uint16_t wv = (uint16_t)w;                                  /* 04EC */
    uint16_t rows = (uint16_t)h;                                  /* 04F2 */
    dos_wr16(di, wv);      di += 2;                                  /* 04EF */
    dos_wr16(di, rows);    di += 2;                                    /* 04F5 */
    for (uint16_t r = 0; r < rows; r++) {                                 /* 04FF-0504 */
        memcpy(di, si, wv);
        di += wv;
        si += VGA_ROW_BYTES;                                                /* 04F8-04FB: ax=0x140-w, add si,ax (+ movsb's own +w) */
    }
}

/* ---- gfx_restore_rect: .lst 050B-053B. */
void vga_restore_rect(dos_int x, dos_int y, const uint8_t *buf)
{
    uint8_t *di = g3924[(uint16_t)y] + (uint16_t)x;   /* 0512-051D */
    const uint8_t *si = buf;
    uint16_t wv = dos_rd16(si);    si += 2;              /* 0523-0524 */
    uint16_t rows = dos_rd16(si);  si += 2;                /* 0526-0527 */
    for (uint16_t r = 0; r < rows; r++) {                     /* 0530-0535 */
        memcpy(di, si, wv);
        si += wv;
        di += VGA_ROW_BYTES;                                     /* 0529-052C: ax=0x140-w, add di,ax (+ movsb's own +w) */
    }
}

/* ---- gfx_wipe_rect: .lst 053C-05A6.  Row-for-row framebuffer copy, then
 * (gbc==1 && dy<0xC8) a dirty record in HALF units (x>>1, w>>1) even though
 * VGA rows are full bytes per pixel -- 058E/0596 `shr ax,1` on both. */
void vga_wipe_rect(dos_int sx, dos_int sy, dos_int w, dos_int h, dos_int dx, dos_int dy)
{
    uint8_t *di = g3924[(uint16_t)dy] + (uint16_t)dx;          /* 0544-054F */
    const uint8_t *si = g3924[(uint16_t)sy] + (uint16_t)sx;      /* 0552-055D */
    uint16_t wv = (uint16_t)w;                                     /* 0560 */
    uint16_t rows = (uint16_t)h;                                     /* 0563 */
    for (uint16_t r = 0; r < rows; r++) {                               /* 056D-0574 */
        memcpy(di, si, wv);
        di += VGA_ROW_BYTES;                                              /* 0566-0569: ax=0x140-w, add di,ax (+w) */
        si += VGA_ROW_BYTES;
    }
    if (gbc == 1 && (uint16_t)dy < 0xC8u) {                                  /* 0577-0585 */
        vga_dirty_queue_append((uint16_t)dy, (uint16_t)((uint16_t)dx >> 1),
                                (uint16_t)h, (uint16_t)((uint16_t)w >> 1));     /* 058B-059D */
    }
}

/* ---- gfx_copy_rect_flip_v: .lst 05A7-05F4.  Dest starts at row dy+(h-1)
 * ((h-1)*0x140 via the xchg/shr trick, 05CD-05DB) and walks UP one row
 * (0x140) per source row consumed walking DOWN. */
void vga_copy_rect_flip_v(dos_int sx, dos_int sy, dos_int w, dos_int h, dos_int dx, dos_int dy)
{
    uint16_t wv = (uint16_t)w;
    uint16_t rows = (uint16_t)h;
    uint8_t *di = g3924[(uint16_t)dy] + (uint16_t)dx + (size_t)(uint16_t)(rows - 1u) * VGA_ROW_BYTES; /* 05CA-05DB */
    const uint8_t *si = g3924[(uint16_t)sy] + (uint16_t)sx;                                              /* 05AE-05C7 */
    for (uint16_t r = 0; r < rows; r++) {                                                                    /* 05DD-05EE */
        memcpy(di, si, wv);
        si += VGA_ROW_BYTES;                                                                                    /* 05E1-05E5 */
        di -= VGA_ROW_BYTES;                                                                                       /* 05E7-05EB */
    }
}

/* ---- gfx_copy_rect_flip_h: .lst 05F5-063D.  Per row, byte-for-byte
 * mirrored copy (di starts at dx+w-1, std; no nibble swap -- VGA pixels are
 * whole bytes). */
void vga_copy_rect_flip_h(dos_int sx, dos_int sy, dos_int w, dos_int h, dos_int dx, dos_int dy)
{
    uint16_t wv = (uint16_t)w;
    uint16_t rows = (uint16_t)h;
    uint8_t *row_start = g3924[(uint16_t)dy] + (uint16_t)dx + wv - 1u;   /* 05FC-0620 */
    const uint8_t *si = g3924[(uint16_t)sy] + (uint16_t)sx;                /* 060A-0615 */
    for (uint16_t r = 0; r < rows; r++) {                                     /* 0621-0636 */
        uint8_t *di = row_start;
        for (uint16_t k = 0; k < wv; k++) {                                      /* 0623-0627 */
            *di-- = *si++;
        }
        si += VGA_ROW_BYTES - wv;                                                   /* 0629-062D: next source row */
        row_start += VGA_ROW_BYTES;                                                    /* 062F-0633: next dest row, same right edge */
    }
}

/* ---- gfx_copy_rect_flip_hv: .lst 063E-0693.  Both: dest starts at the
 * bottom-right corner (row dy+(h-1), column dx+w-1); rows walk UP, each row
 * mirrored (columns walk right-to-left) while the source walks normally. */
void vga_copy_rect_flip_hv(dos_int sx, dos_int sy, dos_int w, dos_int h, dos_int dx, dos_int dy)
{
    uint16_t wv = (uint16_t)w;
    uint16_t rows = (uint16_t)h;
    uint8_t *row_start = g3924[(uint16_t)dy] + (uint16_t)dx
                          + (size_t)(uint16_t)(rows - 1u) * VGA_ROW_BYTES + wv - 1u;   /* 0661-0676 */
    const uint8_t *si = g3924[(uint16_t)sy] + (uint16_t)sx;                              /* 0645-065E */
    for (uint16_t r = 0; r < rows; r++) {                                                   /* 067C-068D */
        uint8_t *di = row_start;
        for (uint16_t k = 0; k < wv; k++) {                                                    /* 067D-0683 */
            *di = *si++;
            di -= 1;                                                                              /* movsb(+1) + sub di,2(-2) = -1 */
        }
        si += (uint16_t)(VGA_ROW_BYTES - wv);                                                        /* 0685: add si,ax (ax=0x140-w) -- next source row */
        row_start -= VGA_ROW_BYTES;                                                                  /* 0688: next dest row (up) */
    }
}

/* ---- gfx_copy_rect_split: .lst 0694-06DA.  Turns SOURCE ROWS into
 * DESTINATION COLUMNS (a transpose): dest column base di = g3924[dy]+dx+h-1
 * (06BD-06BF, a plain +h-1, NOT a *0x140 offset -- di is still a byte
 * position within row dy at this point).  For each of h source rows: for
 * w bytes, `movsb` then `di += 0x13f` (net +0x140 -- one dest ROW down per
 * source byte, i.e. one source row becomes a dest column going down); then
 * the saved column-start di is restored and decremented by 1 (one dest
 * column to the LEFT) for the next source row. */
void vga_copy_rect_split(dos_int sx, dos_int sy, dos_int w, dos_int h, dos_int dx, dos_int dy)
{
    uint16_t wv = (uint16_t)w;
    uint16_t rows = (uint16_t)h;                                                     /* dx register, outer counter, 06BA */
    uint8_t *di0 = g3924[(uint16_t)dy] + (uint16_t)dx + rows - 1u;                       /* 06A2-06BF */
    const uint8_t *si = g3924[(uint16_t)sy] + (uint16_t)sx;                                /* 06A9-06B4 */

    for (uint16_t r = 0; r < rows; r++) {                                                     /* 06C5-06D4 */
        uint8_t *di = di0;                                                                        /* 06C5: push di */
        for (uint16_t k = 0; k < wv; k++) {                                                          /* 06C6-06CD */
            *di = *si++;
            di += VGA_ROW_BYTES;                                                                        /* movsb(+1) + add di,0x13f(+0x13f) = +0x140 */
        }
        si += (uint16_t)(VGA_ROW_BYTES - wv);                                                          /* 06CF: add si,ax (ax=0x140-w) -- next source row */
        di0 -= 1u;                                                                                     /* 06D1-06D2: pop di (discarded); di0-- */
    }
}

/* ---- gfx_copy_rect_split_flip_v: .lst 06DB-072B.  Same row/column
 * transpose as split, but vertically flipped: the initial di uses the
 * *0x140 multiply trick with (w-1) this time (070E-070F: di = g3924[dy]+dx
 * + (w-1)*0x140, a ROW offset, landing on row dy+(w-1)); each source row's
 * w bytes are written walking UP (di -= 0x140 net per byte, 0719-071E) and
 * the saved di is incremented by 1 (one column to the RIGHT) between source
 * rows -- per the task contract, the outer counter really is h (dx
 * register, 06FE) and the inner per-row byte count really is w (bx/cx,
 * 0701-0717), matching split's own outer/inner roles exactly. */
void vga_copy_rect_split_flip_v(dos_int sx, dos_int sy, dos_int w, dos_int h, dos_int dx, dos_int dy)
{
    uint16_t wv = (uint16_t)w;                                                              /* bx, 0704 */
    uint16_t rows = (uint16_t)h;                                                              /* dx register, outer counter, 06FE */
    uint8_t *di0 = g3924[(uint16_t)dy] + (uint16_t)dx + (size_t)(uint16_t)(wv - 1u) * VGA_ROW_BYTES; /* 06E9-070F */
    const uint8_t *si = g3924[(uint16_t)sy] + (uint16_t)sx;                                     /* 06F0-06FB */

    for (uint16_t r = 0; r < rows; r++) {                                                          /* 0716-0725 */
        uint8_t *di = di0;                                                                             /* 0716: push di */
        for (uint16_t k = 0; k < wv; k++) {                                                               /* 0717-071E */
            *di = *si++;
            di -= VGA_ROW_BYTES;                                                                             /* movsb(+1) + sub di,0x141(-0x141) = -0x140 */
        }
        si += (uint16_t)(VGA_ROW_BYTES - wv);                                                             /* 0720: add si,ax (ax=0x140-w) -- next source row */
        di0 += 1u;                                                                                        /* 0722-0723: pop di (discarded); di0++ */
    }
}

/* ---- gfx_draw_char: .lst 072C-07E3.  Font state exactly like the planar
 * version (gc0e0 base pointer, gc0e4 width table, gc0e6 lo/gc0e2 hi data
 * offset, dialog_line_height rows).  Per row: cx=width pixels; bits are
 * taken MSB-first from successive source bytes; bit=1 writes the colour
 * byte (`ah`, the low byte of `result` -- 075A-075D `mov ax,[0x40c8];
 * mov ah,al`) at the pixel; bit=0 leaves the destination untouched; the
 * unrolled `loop`-after-each-pixel chain (0777-07D3) means a row ends after
 * exactly `width` pixels and the NEXT row starts on a fresh source byte
 * (glyph rows are ceil(width/8) bytes each).  Rows advance by 0x140
 * (07D6).  Unlike the planar driver, x is used directly with no >>1 (VGA is
 * 1 byte per pixel, not 2 pixels per byte) and the destination byte written
 * is the FULL colour byte, not a nibble.  Returns width unshifted (07DD:
 * `mov ax,bx`, no `shr ax,1` -- the planar version's return value IS
 * halved, this one is not). */
dos_int vga_draw_char(dos_int x, dos_int y, dos_int glyph)
{
    uint16_t g = (uint16_t)glyph;
    uint16_t width = gc0e0[g + gc0e4];                                /* 073B-073F */
    uint8_t  lo  = gc0e0[g + gc0e6];                                     /* 0742-0746 */
    uint8_t  hib = gc0e0[g + gc0e2];                                       /* 0749-074D */
    const uint8_t *si = gc0e0 + gc0de + (uint16_t)((hib << 8) | lo);         /* 0750-0754 */
    uint16_t rows = (uint16_t)dialog_line_height;                              /* 0756 */
    uint8_t color = (uint8_t)result;                                              /* 075A-075D */
    uint8_t *row_di0 = g3924[(uint16_t)y] + (uint16_t)x;                            /* 0760-076B */

    for (uint16_t row = 0; row < rows; row++) {                                        /* 07D5-07DB */
        si += vga_paint_bits(row_di0, si, width, color);   /* the unrolled 8-slot dispatch chain, 0776-07D3 */
        row_di0 += VGA_ROW_BYTES;                                                             /* 07D6 */
    }
    return (dos_int)width;                                                                       /* 07DD */
}

/* ---- gfx_blit_image: .lst 07E4-087B.  image = [byte n][byte rows]
 * [bits...]; pixel count per row = n*2 (0801-0807: `bl=al; shl bx,1`),
 * colour = result low byte (080B: `ah=cl`, cl the low byte of the word read
 * at 07EB), same bit painter as draw_char, rows advance 0x140. */
void vga_blit_image(dos_int x, dos_int y, const uint8_t *image)
{
    uint8_t color = (uint8_t)result;                       /* 07EB (cl) */
    const uint8_t *si = image;
    uint16_t n = si[0];                                        /* 0805 */
    uint16_t rows = si[1];                                        /* 0809 */
    si += 2;                                                         /* 0800 lodsw already consumed the header word */
    uint16_t pixel_count = (uint16_t)(n * 2u);                          /* 0807 */
    uint8_t *row_di0 = g3924[(uint16_t)y] + (uint16_t)x;                   /* 07EF-07FA */

    for (uint16_t row = 0; row < rows; row++) {                               /* 086F-0875 */
        si += vga_paint_bits(row_di0, si, pixel_count, color);   /* the unrolled 8-slot dispatch chain, 0810-086D */
        row_di0 += VGA_ROW_BYTES;                                                /* 0870 */
    }
}

/* ---- gfx_blit_bitmap: .lst 087C-08F1.  table = bitmap+0x10 (16 bytes);
 * bitmap+0x20 = (packed_bytes, rows); data at +0x22.  Each packed byte b
 * produces two adjacent destination bytes via `stosw` (little-endian word:
 * al=table[hi] written first at [di], ah=table[lo] at [di+1] -- left pixel
 * = high nibble).  Dirty record in HALF units, as elsewhere. */
void vga_blit_bitmap(dos_int x, dos_int y, const uint8_t *bitmap)
{
    uint8_t *row_di0 = g3924[(uint16_t)y] + (uint16_t)x;   /* 0883-088E */
    const uint8_t *table = bitmap + 0x10;                      /* 0895-0898 */
    const uint8_t *si = bitmap + 0x20;                            /* 089A */
    uint16_t packed_bytes = si[0];                                   /* 089D, al */
    uint16_t rows = si[1];                                              /* 089D, ah */
    si += 2;                                                               /* data starts at +0x22 */

    for (uint16_t r = 0; r < rows; r++) {                                     /* 08A7-08C4 */
        uint8_t *di = row_di0;
        for (uint16_t k = 0; k < packed_bytes; k++) {                            /* 08A9-08BB */
            uint8_t b = *si++;
            uint8_t hi = (uint8_t)(b >> 4);
            uint8_t lo = (uint8_t)(b & 0x0Fu);
            di[0] = table[hi];                                                       /* al after the second xlatb */
            di[1] = table[lo];                                                          /* ah, saved before the first xlatb */
            di += 2;
        }
        row_di0 += VGA_ROW_BYTES;                                                          /* 08BF: pop di restores row start, then add di,0x140 */
    }

    if (gbc == 1 && (uint16_t)y < 0xC8u) {                                                     /* 08C8-08D6 */
        vga_dirty_queue_append((uint16_t)y, (uint16_t)((uint16_t)x >> 1),
                                rows, packed_bytes);                                                /* 08DC-08E9 */
    }
}

/* ---- gfx_copy_rect: .lst 08F2-0AC4 (forward path 093E-09F6, flip path
 * 09F9-0ABF).  bitmap+0x10 = 16-byte VGA colour table; bitmap+0x20 =
 * (packed_bytes cl, rows dl); data at +0x22.  g94/g96 clip rows exactly as
 * gfx_planar.c's gfx_copy_rect (identical instructions, 0913-0930, down to
 * the 8-bit `mul cl` source-row skip).  g98/g9a clip byte-COLUMNS in
 * PACKED-PAIR units (one packed source byte = two destination pixels/
 * bytes): forward path bxcol = x>>1 (sar, signed, 093E), and the eventual
 * destination base is g3924[y] + 2*bxcol (0996-0998, the `shl bx,1` restores
 * byte scale after clipping in pair units) -- the same "clip in packed
 * units, then double for the byte address" shape as gfx_planar.c's
 * gfx_copy_rect, except each packed unit here is 2 whole destination BYTES
 * instead of 2 nibbles inside 1 byte. */
void vga_copy_rect(dos_int x, dos_int y, const uint8_t *bitmap, dos_int flip)
{
    const uint8_t *table = bitmap + 0x10;                                    /* 08F8-08FE (si pushed/popped as bx) */
    const uint8_t *si = bitmap + 0x20;                                          /* 08FF */
    uint16_t header_bytes = si[0];                                                 /* 0902 al */
    uint16_t header_rows  = si[1];                                                    /* 0902 ah */
    si += 2;                                                                             /* 0905 */

    uint16_t cx = header_bytes;           /* clipped packed byte-width */
    uint16_t dxr = header_rows;             /* clipped row count */
    int16_t yv = (int16_t)y;

    /* ---- row clip against g94/g96, identical to gfx_planar.c's gfx_copy_rect
     * (0913-0930). */
    int16_t ax = (int16_t)(g96 - yv);            /* 0913-0916 */
    if (ax < 0) return;                             /* 0918: js (bail, nothing drawn) */
    ax = (int16_t)(ax + 1);                           /* 091A */
    if ((uint16_t)dxr > (uint16_t)ax) dxr = (uint16_t)ax;  /* 091B-091F */

    ax = (int16_t)(g94 - yv);                            /* 0921-0924 */
    if (ax > 0) {                                           /* 0926: jle skip */
        dxr = (uint16_t)(dxr - (uint16_t)ax);                  /* 0928 */
        if ((int16_t)dxr <= 0) return;                            /* 092A: jbe bail */
        yv = (int16_t)(yv + ax);                                     /* 092C */
        si += (uint8_t)ax * (uint8_t)cx;                               /* 092E-0930: mul cl (AL*CL, 8-bit) */
    }

    int16_t bp_extra = 0;

    if (flip == 0) {                                                        /* 0935-093B */
        uint16_t bxcol = (uint16_t)((int16_t)x >> 1);                          /* 093E: sar bx,1 (signed) */
        ax = (int16_t)(g9a - (int16_t)bxcol);                                    /* 0943-0946 */
        if (ax < 0) return;                                                        /* 0948 */
        ax = (int16_t)(ax + 1);                                                      /* 094A */
        if ((uint16_t)cx > (uint16_t)ax) {                                             /* 094B-094F */
            bp_extra = (int16_t)(cx - (uint16_t)ax);                                      /* 0951 */
            cx = (uint16_t)ax;                                                              /* 0953 */
        }
        ax = (int16_t)(g98 - (int16_t)bxcol);                                                 /* 0955-0958 */
        if (ax > 0) {                                                                           /* 095A: jle skip */
            cx = (uint16_t)((int16_t)cx - ax);                                                     /* 095C */
            if ((int16_t)cx > 0) {                                                                    /* 095E: ja */
                bxcol = (uint16_t)(bxcol + (uint16_t)ax);                                                 /* 0964 */
                bp_extra = (int16_t)(bp_extra + ax);                                                        /* 0966 */
                si += (uint16_t)ax;                                                                           /* 0968 */
            } else {
                return;                                                                                          /* cx<=0: bail */
            }
        }
        if (gbc == 1 && (uint16_t)yv < 0xC8u) {                                                                    /* 096C-0977 */
            vga_dirty_queue_append((uint16_t)yv, (uint16_t)bxcol, dxr, cx);                                           /* 0979-0989 */
        }
        uint8_t *di = g3924[(uint16_t)yv] + (size_t)2u * bxcol;                                                        /* 098D-0998 */
        for (uint16_t row = 0; row < dxr; row++) {                                                                        /* 099C-09BF */
            uint8_t *rdi = di;
            for (uint16_t k = 0; k < cx; k++) {                                                                              /* 099E-09B4 */
                uint8_t b = *si++;
                uint8_t lo = (uint8_t)(b & 0x0Fu);
                if (lo != 0) {                                                                                                  /* 09A3: je -> 0x9d6 branch */
                    uint8_t hi = (uint8_t)(b >> 4);
                    if (hi == 0) {                                                                                                 /* 09AE: je -> 0x9c4 branch */
                        rdi[1] = table[lo];                                                                                            /* 09C4-09C5 */
                    } else {
                        rdi[0] = table[hi];                                                                                            /* 09B0-09B3 */
                        rdi[1] = table[lo];
                    }
                } else {
                    uint8_t hi = (uint8_t)(b >> 4);                                                                                    /* 09D6-09DC */
                    if (hi != 0) rdi[0] = table[hi];                                                                                      /* 09DE-09E3 */
                }
                rdi += 2;                                                                                                                  /* both sub-branches: di += 2 */
            }
            di += VGA_ROW_BYTES;                                                                                                              /* 09B8/09CA/09ED */
            si += bp_extra;                                                                                                                      /* 09BC/09CE/09F1 */
        }
    } else {                                                                      /* flip path, 09F9-0ABF */
        /* 09FC-0A02: bx = ((x + 2*header_bytes) >> 1) - 1, the rightmost
         * dest packed-pair column of the (unclipped) mirrored rect. */
        uint16_t rightcol = (uint16_t)((uint16_t)(((uint16_t)x + 2u * header_bytes) >> 1) - 1u);
        ax = (int16_t)((int16_t)rightcol - g98);                                       /* 0A03-0A05 */
        if (ax < 0) return;                                                              /* 0A09 */
        ax = (int16_t)(ax + 1);                                                            /* 0A0B */
        if ((uint16_t)cx > (uint16_t)ax) {                                                   /* 0A0C-0A10 */
            bp_extra = (int16_t)(cx - (uint16_t)ax);                                            /* 0A12 */
            cx = (uint16_t)ax;                                                                    /* 0A14 */
        }
        ax = (int16_t)((int16_t)rightcol - g9a);                                                    /* 0A16-0A18 */
        if (ax > 0) {                                                                                  /* 0A1C: jbe skip */
            cx = (uint16_t)((int16_t)cx - ax);                                                            /* 0A1E */
            if ((int16_t)cx > 0) {                                                                           /* 0A20: ja */
                rightcol = (uint16_t)(rightcol - (uint16_t)ax);                                                  /* 0A26 */
                bp_extra = (int16_t)(bp_extra + ax);                                                               /* 0A28 */
                si += (uint16_t)ax;                                                                                  /* 0A2A */
            } else {
                return;                                                                                                /* bail */
            }
        }
        if (gbc == 1 && (uint16_t)yv < 0xC8u) {                                                                          /* 0A2E-0A39 */
            uint8_t leftcol = (uint8_t)((uint8_t)rightcol - (uint8_t)cx + 1u);                                              /* 0A3B-0A43 */
            vga_dirty_queue_append((uint16_t)yv, leftcol, dxr, cx);                                                            /* 0A45-0A51 */
        }
        uint8_t *di = g3924[(uint16_t)yv] + (size_t)2u * rightcol;                                                               /* 0A55-0A60 */
        for (uint16_t row = 0; row < dxr; row++) {                                                                                  /* 0A64-0A8A */
            uint8_t *rdi = di;
            for (uint16_t k = 0; k < cx; k++) {                                                                                        /* 0A66-0A95 */
                uint8_t b = *si++;                                          /* 0A66-0A68: si still walks FORWARD (not mirrored) */
                uint8_t hi = (uint8_t)(b >> 4);
                if (hi != 0) {                                                                                                            /* 0A73: je -> 0xaa4 branch */
                    uint8_t lo = (uint8_t)(b & 0x0Fu);
                    if (lo == 0) {                                                                                                          /* 0A79: je -> 0xa8e branch */
                        rdi[1] = table[hi];                                                                                                    /* 0A8E */
                    } else {
                        rdi[0] = table[lo];                                                                                                    /* 0A7B-0A7E */
                        rdi[1] = table[hi];
                    }
                } else {
                    uint8_t lo = (uint8_t)(b & 0x0Fu);                                                                                         /* 0AA4-0AA7 */
                    if (lo != 0) rdi[0] = table[lo];                                                                                              /* 0AA9-0AAC */
                }
                rdi -= 2;                                                                                                                          /* both sub-branches: di -= 2 (std) */
            }
            di += VGA_ROW_BYTES;                                                                                                                      /* 0A83/0A99/0AB6 (plain add, direction flag doesn't affect it) */
            si += bp_extra;                                                                                                                              /* 0A87/0A9D/0ABA */
        }
    }
}

/* ---- gfx_set_pixel: .lst 0AC5-0ADF. */
void vga_set_pixel(dos_int x, dos_int y)
{
    uint8_t *di = g3924[(uint16_t)y];        /* 0AC9-0AD0 */
    di[(uint16_t)x] = (uint8_t)result;          /* 0AD4-0ADA */
}

/* ---- gfx_get_pixel: .lst 0AE0-0AF9. */
dos_int vga_get_pixel(dos_int x, dos_int y)
{
    const uint8_t *di = g3924[(uint16_t)y];     /* 0AE4-0AEF */
    return (dos_int)di[(uint16_t)x];               /* 0AF2-0AF4 */
}
