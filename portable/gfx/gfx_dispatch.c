/* gfx_dispatch.c -- public gfx_<name>() primitives (portable/include/gfx.h),
 * each dispatching to the packed-4bpp driver (gfx_planar.c, display
 * selectors 1/3/4) or the 8bpp VGA driver (gfx_vga.c, display selector 5)
 * by display_mode, exactly as src/VIDEO.C's own runtime-slot dispatch
 * (RUNTIME_BLOCK.ASM vs AE000_002) picked a driver at load time.  See
 * docs/portable/architecture.md "Video model" and gfx_drivers.h.
 *
 * portable/resource owns display_mode (DS:BFCD); mirrors the same
 * extern-plus-alias pattern framebuffer.c uses.
 */
#include "gfx.h"
#include "gfx_drivers.h"

extern dos_char display_mode;

void gfx_box(dos_int x, dos_int y, dos_int w, dos_int h)
{
    if (display_mode == 5) vga_box(x, y, w, h);
    else                   planar_box(x, y, w, h);
}

void gfx_bar(dos_int x, dos_int y, dos_int n)
{
    if (display_mode == 5) vga_bar(x, y, n);
    else                   planar_bar(x, y, n);
}

void gfx_vline(dos_int x, dos_int y, dos_int n)
{
    if (display_mode == 5) vga_vline(x, y, n);
    else                   planar_vline(x, y, n);
}

void gfx_clear_rect(dos_int x, dos_int y, dos_int w, dos_int h)
{
    if (display_mode == 5) vga_clear_rect(x, y, w, h);
    else                   planar_clear_rect(x, y, w, h);
}

void gfx_fill_rect(dos_int x, dos_int y, dos_int w, dos_int h)
{
    if (display_mode == 5) vga_fill_rect(x, y, w, h);
    else                   planar_fill_rect(x, y, w, h);
}

void gfx_save_rect(dos_int x, dos_int y, dos_int w, dos_int h, uint8_t *buf)
{
    if (display_mode == 5) vga_save_rect(x, y, w, h, buf);
    else                   planar_save_rect(x, y, w, h, buf);
}

void gfx_restore_rect(dos_int x, dos_int y, const uint8_t *buf)
{
    if (display_mode == 5) vga_restore_rect(x, y, buf);
    else                   planar_restore_rect(x, y, buf);
}

void gfx_wipe_rect(dos_int sx, dos_int sy, dos_int w, dos_int h, dos_int dx, dos_int dy)
{
    if (display_mode == 5) vga_wipe_rect(sx, sy, w, h, dx, dy);
    else                   planar_wipe_rect(sx, sy, w, h, dx, dy);
}

void gfx_copy_rect_flip_v(dos_int sx, dos_int sy, dos_int w, dos_int h, dos_int dx, dos_int dy)
{
    if (display_mode == 5) vga_copy_rect_flip_v(sx, sy, w, h, dx, dy);
    else                   planar_copy_rect_flip_v(sx, sy, w, h, dx, dy);
}

void gfx_copy_rect_flip_h(dos_int sx, dos_int sy, dos_int w, dos_int h, dos_int dx, dos_int dy)
{
    if (display_mode == 5) vga_copy_rect_flip_h(sx, sy, w, h, dx, dy);
    else                   planar_copy_rect_flip_h(sx, sy, w, h, dx, dy);
}

void gfx_copy_rect_flip_hv(dos_int sx, dos_int sy, dos_int w, dos_int h, dos_int dx, dos_int dy)
{
    if (display_mode == 5) vga_copy_rect_flip_hv(sx, sy, w, h, dx, dy);
    else                   planar_copy_rect_flip_hv(sx, sy, w, h, dx, dy);
}

void gfx_copy_rect_split(dos_int sx, dos_int sy, dos_int w, dos_int h, dos_int dx, dos_int dy)
{
    if (display_mode == 5) vga_copy_rect_split(sx, sy, w, h, dx, dy);
    else                   planar_copy_rect_split(sx, sy, w, h, dx, dy);
}

void gfx_copy_rect_split_flip_v(dos_int sx, dos_int sy, dos_int w, dos_int h, dos_int dx, dos_int dy)
{
    if (display_mode == 5) vga_copy_rect_split_flip_v(sx, sy, w, h, dx, dy);
    else                   planar_copy_rect_split_flip_v(sx, sy, w, h, dx, dy);
}

dos_int gfx_draw_char(dos_int x, dos_int y, dos_int glyph)
{
    if (display_mode == 5) return vga_draw_char(x, y, glyph);
    else                   return planar_draw_char(x, y, glyph);
}

void gfx_blit_bitmap(dos_int x, dos_int y, const uint8_t *bitmap)
{
    if (display_mode == 5) vga_blit_bitmap(x, y, bitmap);
    else                   planar_blit_bitmap(x, y, bitmap);
}

void gfx_copy_rect(dos_int x, dos_int y, const uint8_t *bitmap, dos_int flip)
{
    if (display_mode == 5) vga_copy_rect(x, y, bitmap, flip);
    else                   planar_copy_rect(x, y, bitmap, flip);
}

void gfx_blit_image(dos_int x, dos_int y, const uint8_t *image)
{
    if (display_mode == 5) vga_blit_image(x, y, image);
    else                   planar_blit_image(x, y, image);
}

void gfx_set_pixel(dos_int x, dos_int y)
{
    if (display_mode == 5) vga_set_pixel(x, y);
    else                   planar_set_pixel(x, y);
}

dos_int gfx_get_pixel(dos_int x, dos_int y)
{
    if (display_mode == 5) return vga_get_pixel(x, y);
    else                   return planar_get_pixel(x, y);
}
