/* gfx.h -- software graphics primitives (asm/RUNTIME_BLOCK.ASM, display_mode 4).
 *
 * Framebuffer: 320 x 488 logical pixels, packed 4 bpp, 160 bytes per row,
 * high nibble = even x, low nibble = odd x.  Every primitive addresses rows
 * through gfx_rows[] (historical g3924) with the stride fixed at 0xA0.
 * Coordinates and sizes are historical 16-bit ints; no bounds checks are
 * added beyond what the original code performed.
 */
#ifndef PORTABLE_GFX_H
#define PORTABLE_GFX_H

#include <stdbool.h>

#include "dos_types.h"

#define GFX_ROW_BYTES       0xA0    /* packed-4bpp driver row stride (display selectors 1/3/4) */
#define GFX_ROW_BYTES_VGA   0x140   /* 8bpp VGA driver row stride (display selector 5) */
#define GFX_ROWS        488
#define GFX_VRAM_W      320
#define GFX_VRAM_H      200

/* ---- shared state (historical DGROUP objects, historical names kept) ---- */
extern uint8_t *g3924[GFX_ROWS];      /* DS:3924 row-pointer table */
extern dos_int  result;               /* DS:40C8: low byte = color in both nibbles */
extern dos_int  gbc;                  /* DS:00BC: 1 -> wipe/blit/copy append dirty records */
extern uint8_t *rect_queue_write_ptr; /* DS:40C4 (g40c4): next dirty record write position */
extern dos_int  g94, g96;             /* DS:94, DS:96: inclusive row clip bounds (gfx_copy_rect) */
extern dos_int  g98, g9a;             /* DS:98, DS:9A: inclusive BYTE-column clip bounds */
/* Font state read by gfx_draw_char (DS:C0DE..C0E8): a base pointer (the
 * historical segment word at DS:C0E0) plus table offsets relative to it. */
extern const uint8_t *gc0e0;          /* DS:C0E0: font resource base (was a segment) */
extern dos_uint gc0de;                /* DS:C0DE: offset of glyph bitmap area */
extern dos_uint gc0e2;                /* DS:C0E2: per-glyph high byte of data offset */
extern dos_uint gc0e4;                /* DS:C0E4: per-glyph pixel width */
extern dos_uint gc0e6;                /* DS:C0E6: per-glyph low byte of data offset */
extern dos_int  dialog_line_height;   /* DS:C0E8: glyph rows */

/* 8-bpp "VRAM" that gfx_box presents into (mode 13h, 320x200) and the
 * 256-entry 6-bit DAC palette loaded by video_load_palette. */
extern uint8_t gfx_vram[GFX_VRAM_W * GFX_VRAM_H];
extern uint8_t gfx_dac[256 * 3];
extern uint32_t gfx_vram_generation;  /* incremented by every gfx_box */

/* Allocate the framebuffer and fill g3924 (src/VIDEO.C video_alloc_framebuffer:
 * w = 0x140 for display_mode 5, 0x50 for display_mode 2, 0xA0 otherwise). */
void gfx_framebuffer_init(void);
void gfx_framebuffer_shutdown(void);
uint8_t *gfx_framebuffer(void);       /* row 0; rows are contiguous, gfx_row_bytes() apart */
dos_int gfx_row_bytes(void);          /* current row stride, set by the last gfx_framebuffer_init() */

/* Host-present synchronization for event-driven legacy animations. */
void gfx_present_sync_set_enabled(bool enabled);
void gfx_present_sync_begin(void);
void gfx_present_sync_wait(void);
bool gfx_present_sync_requested(void);
void gfx_present_sync_ack(uint32_t generation);

/* ---- primitives, 1:1 with include/VIDEO.H ---- */
void gfx_box(dos_int x, dos_int y, dos_int w, dos_int h);                       /* present rect to VRAM */
void gfx_bar(dos_int x, dos_int y, dos_int n);                                  /* horizontal span in gfx_result */
void gfx_vline(dos_int x, dos_int y, dos_int n);                                /* vertical span */
void gfx_clear_rect(dos_int x, dos_int y, dos_int w, dos_int h);                /* fill with gfx_result */
void gfx_fill_rect(dos_int x, dos_int y, dos_int w, dos_int h);                 /* XOR-invert */
void gfx_save_rect(dos_int x, dos_int y, dos_int w, dos_int h, uint8_t *buf);   /* header (bytes,rows) + rows */
void gfx_restore_rect(dos_int x, dos_int y, const uint8_t *buf);
void gfx_wipe_rect(dos_int sx, dos_int sy, dos_int w, dos_int h, dos_int dx, dos_int dy);
void gfx_copy_rect_flip_v(dos_int sx, dos_int sy, dos_int w, dos_int h, dos_int dx, dos_int dy);
void gfx_copy_rect_flip_h(dos_int sx, dos_int sy, dos_int w, dos_int h, dos_int dx, dos_int dy);
void gfx_copy_rect_flip_hv(dos_int sx, dos_int sy, dos_int w, dos_int h, dos_int dx, dos_int dy);
void gfx_copy_rect_split(dos_int sx, dos_int sy, dos_int w, dos_int h, dos_int dx, dos_int dy);
void gfx_copy_rect_split_flip_v(dos_int sx, dos_int sy, dos_int w, dos_int h, dos_int dx, dos_int dy);
dos_int gfx_draw_char(dos_int x, dos_int y, dos_int glyph);                     /* returns advance */
void gfx_blit_bitmap(dos_int x, dos_int y, const uint8_t *bitmap);              /* 32-byte header + (bytes,rows) + rows */
void gfx_copy_rect(dos_int x, dos_int y, const uint8_t *bitmap, dos_int flip);  /* clipped, transparent, optional mirror */
void gfx_blit_image(dos_int x, dos_int y, const uint8_t *image);                /* 1-bpp: (bytes_per_row, rows, bits...) */
void gfx_set_pixel(dos_int x, dos_int y);
dos_int gfx_get_pixel(dos_int x, dos_int y);

/* ---- src/VIDEO.C palette plumbing ---- */
extern dos_int cur_idx;               /* DS:3902 */
extern dos_int g3904[16];             /* DS:3904 */
extern dos_int gbe[16];               /* DS:00BE */
extern dos_int gfe[16];               /* DS:00FE */
void gfx_color_select(dos_int i);
dos_int cur_color_index_get(void);
void color_table_entry_set(dos_int index, dos_int value1, dos_int value2);
void color_lookup_tables_init(void);
void video_load_palette(const uint8_t *dac6_rgb256);   /* copies 768 bytes into gfx_dac */
void rect_border_draw(dos_int x, dos_int y, dos_int w, dos_int h);

#endif
