/* main.c -- SDL3 executable entry point.
 *
 * Milestone C state: the software framebuffer (portable/gfx) is presented
 * through video_sdl.h.  Until the ported game logic exists, `--demo` (the
 * default) draws a primitive test scene with the real gfx_* calls so the
 * present path (packed 4bpp -> mode-13h VRAM -> g41e DAC -> texture) can be
 * checked by eye.  `--selftest` runs the loop for ~300 ms and exits 0.
 *
 * Only files in portable/platform/sdl3/ may include <SDL3/SDL.h>.
 */
#include <SDL3/SDL.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "gfx.h"
#include "game_data.h"
#include "video_sdl.h"

#define SELFTEST_DURATION_MS 300

/* Draw a scene that exercises the primitives the port has today: the 16
 * logical colours through gfx_color_select, bars, vlines, rect fills, a
 * save/restore round trip and the XOR fill, then present it. */
static void draw_demo_scene(void)
{
    dos_int i;
    static uint8_t saved[4 + 40 * 20];

    gfx_color_select(0);
    gfx_clear_rect(0, 0, 320, 200);
    for (i = 0; i < 16; i++) {
        gfx_color_select(i);
        gfx_clear_rect((dos_int)(i * 20), 8, 20, 40);
    }
    gfx_color_select(15);
    rect_border_draw(4, 60, 312, 100);
    gfx_color_select(4);
    gfx_clear_rect(11, 71, 61, 31);          /* odd x / odd w: nibble clipping */
    gfx_fill_rect(20, 80, 41, 13);           /* XOR inside the block */
    gfx_color_select(2);
    for (i = 0; i < 40; i++)
        gfx_vline((dos_int)(100 + i * 2), (dos_int)(70 + (i & 7)), 30);
    gfx_save_rect(11, 71, 40, 20, saved);
    gfx_restore_rect(200, 110, saved);
    gfx_restore_rect(201, 135, saved);
    gfx_copy_rect_flip_h(11, 71, 40, 20, 240, 110);
    gfx_copy_rect_flip_v(11, 71, 40, 20, 240, 135);
    gfx_color_select(14);
    for (i = 0; i < 300; i += 3)
        gfx_set_pixel((dos_int)(10 + i), (dos_int)(180 + ((i / 3) & 3)));
    gfx_box(0, 0, 320, 200);                 /* present everything */
}

/* Write the presented VRAM as a binary PPM (DAC expanded to 8-bit RGB) so
 * the pixel output can be inspected without a display. */
static void dump_vram_ppm(const char *path)
{
    FILE *f = fopen(path, "wb");
    if (!f)
        return;
    fprintf(f, "P6\n%d %d\n255\n", GFX_VRAM_W, GFX_VRAM_H);
    for (int i = 0; i < GFX_VRAM_W * GFX_VRAM_H; i++) {
        const uint8_t *rgb6 = gfx_dac + gfx_vram[i] * 3;
        uint8_t rgb[3];
        for (int c = 0; c < 3; c++)
            rgb[c] = (uint8_t)((rgb6[c] << 2) | (rgb6[c] >> 4));
        fwrite(rgb, 1, 3, f);
    }
    fclose(f);
}

int main(int argc, char **argv)
{
    bool selftest = false;
    const char *dump_path = NULL;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--selftest") == 0)
            selftest = true;
        else if (strcmp(argv[i], "--dump-vram") == 0 && i + 1 < argc)
            dump_path = argv[++i];
    }

    if (!sdl_video_init("Empires (portable)")) {
        sdl_video_shutdown();
        return 1;
    }

    gfx_framebuffer_init();
    color_lookup_tables_init();
    video_load_palette(&g41e[0][0]);         /* src/VIDEO.C: mode 4 loads g41e */
    draw_demo_scene();
    if (dump_path)
        dump_vram_ppm(dump_path);

    Uint64 start_ticks = SDL_GetTicks();
    bool quit = false;
    while (!quit) {
        quit = sdl_video_poll_events();
        sdl_video_present(gfx_vram, gfx_dac);
        if (selftest && (SDL_GetTicks() - start_ticks) >= SELFTEST_DURATION_MS)
            quit = true;
    }

    gfx_framebuffer_shutdown();
    sdl_video_shutdown();
    return 0;
}
