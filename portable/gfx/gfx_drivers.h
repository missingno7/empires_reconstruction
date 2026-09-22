/* gfx_drivers.h -- private interface between gfx_dispatch.c and the two
 * concrete driver translation units:
 *
 *   gfx_planar.c  planar_<name>()  packed-4bpp driver, display selectors 1/3/4
 *                 (asm/RUNTIME_BLOCK.ASM).
 *   gfx_vga.c     vga_<name>()     8bpp driver, display selector 5
 *                 (docs/portable/reference/AE000_002-vga-runtime.lst).
 *
 * Both drivers share the DGROUP state declared in gfx.h (g3924, result,
 * gbc, rect_queue_write_ptr, g94..g9a, gc0de..gc0e8, gfx_vram, gfx_dac,
 * gfx_vram_generation) -- neither driver file defines any of that state,
 * framebuffer.c still owns it.  Not part of the public API: nothing
 * outside portable/gfx/ should call planar_<name>()/vga_<name>() directly;
 * external callers always go through gfx_<name>() (gfx.h, implemented in
 * gfx_dispatch.c), which selects a driver by display_mode.
 */
#ifndef PORTABLE_GFX_DRIVERS_H
#define PORTABLE_GFX_DRIVERS_H

#include "dos_types.h"

/* ---- packed-4bpp driver (gfx_planar.c), display selectors 1/3/4 ---- */
void    planar_box(dos_int x, dos_int y, dos_int w, dos_int h);
void    planar_bar(dos_int x, dos_int y, dos_int n);
void    planar_vline(dos_int x, dos_int y, dos_int n);
void    planar_clear_rect(dos_int x, dos_int y, dos_int w, dos_int h);
void    planar_fill_rect(dos_int x, dos_int y, dos_int w, dos_int h);
void    planar_save_rect(dos_int x, dos_int y, dos_int w, dos_int h, uint8_t *buf);
void    planar_restore_rect(dos_int x, dos_int y, const uint8_t *buf);
void    planar_wipe_rect(dos_int sx, dos_int sy, dos_int w, dos_int h, dos_int dx, dos_int dy);
void    planar_copy_rect_flip_v(dos_int sx, dos_int sy, dos_int w, dos_int h, dos_int dx, dos_int dy);
void    planar_copy_rect_flip_h(dos_int sx, dos_int sy, dos_int w, dos_int h, dos_int dx, dos_int dy);
void    planar_copy_rect_flip_hv(dos_int sx, dos_int sy, dos_int w, dos_int h, dos_int dx, dos_int dy);
void    planar_copy_rect_split(dos_int sx, dos_int sy, dos_int w, dos_int h, dos_int dx, dos_int dy);
void    planar_copy_rect_split_flip_v(dos_int sx, dos_int sy, dos_int w, dos_int h, dos_int dx, dos_int dy);
dos_int planar_draw_char(dos_int x, dos_int y, dos_int glyph);
void    planar_blit_bitmap(dos_int x, dos_int y, const uint8_t *bitmap);
void    planar_copy_rect(dos_int x, dos_int y, const uint8_t *bitmap, dos_int flip);
void    planar_blit_image(dos_int x, dos_int y, const uint8_t *image);
void    planar_set_pixel(dos_int x, dos_int y);
dos_int planar_get_pixel(dos_int x, dos_int y);

/* ---- 8bpp VGA driver (gfx_vga.c), display selector 5 ---- */
void    vga_box(dos_int x, dos_int y, dos_int w, dos_int h);
void    vga_bar(dos_int x, dos_int y, dos_int n);
void    vga_vline(dos_int x, dos_int y, dos_int n);
void    vga_clear_rect(dos_int x, dos_int y, dos_int w, dos_int h);
void    vga_fill_rect(dos_int x, dos_int y, dos_int w, dos_int h);
void    vga_save_rect(dos_int x, dos_int y, dos_int w, dos_int h, uint8_t *buf);
void    vga_restore_rect(dos_int x, dos_int y, const uint8_t *buf);
void    vga_wipe_rect(dos_int sx, dos_int sy, dos_int w, dos_int h, dos_int dx, dos_int dy);
void    vga_copy_rect_flip_v(dos_int sx, dos_int sy, dos_int w, dos_int h, dos_int dx, dos_int dy);
void    vga_copy_rect_flip_h(dos_int sx, dos_int sy, dos_int w, dos_int h, dos_int dx, dos_int dy);
void    vga_copy_rect_flip_hv(dos_int sx, dos_int sy, dos_int w, dos_int h, dos_int dx, dos_int dy);
void    vga_copy_rect_split(dos_int sx, dos_int sy, dos_int w, dos_int h, dos_int dx, dos_int dy);
void    vga_copy_rect_split_flip_v(dos_int sx, dos_int sy, dos_int w, dos_int h, dos_int dx, dos_int dy);
dos_int vga_draw_char(dos_int x, dos_int y, dos_int glyph);
void    vga_blit_bitmap(dos_int x, dos_int y, const uint8_t *bitmap);
void    vga_copy_rect(dos_int x, dos_int y, const uint8_t *bitmap, dos_int flip);

/* vga_copy_rect's two halves, shared with the frame-interpolation replay
 * (gfx_tween.c).  The clip step reproduces the listing's row/column clip
 * against explicit clip words (the historical g94/g96/g98/g9a) and fills
 * the geometry the draw step needs; the draw step paints through `rows`
 * (a row-pointer table with the 0x140 stride) instead of g3924. */
struct vga_copy_clip {
    uint16_t yv;            /* first dest row after clipping */
    uint16_t dxr;           /* rows to draw */
    uint16_t col;           /* leftmost dest packed-pair column (2 pixels each) */
    uint16_t startcol;      /* column the draw starts from (== col, or the right edge when mirrored) */
    uint16_t cx;            /* packed columns to draw per row */
    int16_t  bp_extra;      /* source bytes to skip per row (clipped-away width) */
    const uint8_t *si;      /* first source byte */
};
bool    vga_copy_rect_clip(dos_int x, dos_int y, const uint8_t *bitmap, dos_int flip,
                           dos_int c94, dos_int c96, dos_int c98, dos_int c9a,
                           struct vga_copy_clip *out);
void    vga_copy_rect_draw(uint8_t *const *rows, const struct vga_copy_clip *c,
                           const uint8_t *table, dos_int flip);
void    vga_blit_image(dos_int x, dos_int y, const uint8_t *image);
void    vga_set_pixel(dos_int x, dos_int y);
dos_int vga_get_pixel(dos_int x, dos_int y);

#endif
