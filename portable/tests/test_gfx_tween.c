/* test_gfx_tween.c -- frame interpolation (portable/gfx/gfx_tween.c):
 * capture of tagged blits, publish, and composition at intermediate
 * alphas, on a gfx_framebuffer_init(display_mode=5) framebuffer. */
#include "gfx.h"
#include "gfx_tween.h"

#include <stdio.h>
#include <string.h>

extern dos_char display_mode;

static int s_failures;
#define CHECK(cond) \
    do { if (!(cond)) { fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); s_failures++; } } while (0)

/* A 4x2 opaque sprite (2 packed bytes per row): colour table[1] = marker. */
static uint8_t s_bitmap[0x22 + 4];
static uint8_t s_rectq[4096];

static void make_bitmap(uint8_t marker)
{
    memset(s_bitmap, 0, sizeof s_bitmap);
    s_bitmap[0x11] = marker;
    s_bitmap[0x20] = 2;             /* bytes per row -> 4 pixels */
    s_bitmap[0x21] = 2;             /* rows */
    s_bitmap[0x22] = 0x11; s_bitmap[0x23] = 0x11;
    s_bitmap[0x24] = 0x11; s_bitmap[0x25] = 0x11;
}

static void setup(void)
{
    display_mode = 5;
    gfx_framebuffer_init();
    memset(gfx_framebuffer(), 7, (size_t)gfx_row_bytes() * GFX_ROWS);   /* background 7 */
    memset(gfx_vram, 7, sizeof gfx_vram);
    g94 = 0; g96 = 199; g98 = 0; g9a = 159;
    gbc = 1;
    rect_queue_write_ptr = s_rectq;
    gfx_tween_set_enabled(true);
}

static void teardown(void)
{
    gfx_tween_set_enabled(false);
    gfx_framebuffer_shutdown();
}

/* Draw the tagged sprite at (x, y), present it, publish the frame. */
static void frame(int tag, dos_int x, dos_int y, double now_ms, double deadline_ms)
{
    gfx_tween_tag = tag;
    gfx_copy_rect(x, y, s_bitmap, 0);
    gfx_tween_tag = 0;
    gfx_box(0, 0, 320, 200);
    gfx_tween_frame_publish(now_ms, deadline_ms, gfx_vram_generation);
}

static void erase(dos_int x, dos_int y)
{
    /* the game restores the background under the old sprite each frame */
    gfx_color_select(0);
    for (int r = 0; r < 2; r++)
        memset(g3924[y + r] + x, 7, 4);
}

static int count_marker(const uint8_t *frame, uint8_t marker, int *min_x, int *min_y)
{
    int n = 0;
    *min_x = 999; *min_y = 999;
    for (int y = 0; y < 200; y++)
        for (int x = 0; x < 320; x++)
            if (frame[y * 320 + x] == marker) {
                n++;
                if (x < *min_x) *min_x = x;
                if (y < *min_y) *min_y = y;
            }
    return n;
}

static void test_interpolates_between_frames(void)
{
    uint8_t out[320 * 200];
    int n, mx, my;
    setup();
    make_bitmap(0x42);

    frame(GFX_TWEEN_TAG_PLAYER, 10, 20, 1000.0, 1100.0);
    erase(10, 20);
    frame(GFX_TWEEN_TAG_PLAYER, 30, 40, 1100.0, 1200.0);

    /* alpha 0.5 -> (20, 30) */
    CHECK(gfx_tween_compose(out, 1150.0, gfx_vram_generation));
    n = count_marker(out, 0x42, &mx, &my);
    CHECK(n == 8);
    CHECK(mx == 20 && my == 30);
    /* the old position was erased (background restored from the capture) */
    CHECK(out[40 * 320 + 30] == 7 || (mx == 30));

    /* alpha 0 -> previous position */
    CHECK(gfx_tween_compose(out, 1100.0, gfx_vram_generation));
    n = count_marker(out, 0x42, &mx, &my);
    CHECK(n == 8 && mx == 10 && my == 20);

    /* alpha 1 -> the frame exactly as presented */
    CHECK(gfx_tween_compose(out, 1200.0, gfx_vram_generation));
    CHECK(memcmp(out, gfx_vram, sizeof out) == 0);
    n = count_marker(out, 0x42, &mx, &my);
    CHECK(n == 8 && mx == 30 && my == 40);

    /* odd x rounds down to the packed pair, like the driver */
    CHECK(gfx_tween_compose(out, 1125.0, gfx_vram_generation));   /* alpha .25 -> x=15 -> col 14, y=25 */
    n = count_marker(out, 0x42, &mx, &my);
    CHECK(n == 8 && mx == 14 && my == 25);
    teardown();
}

static void test_teleport_and_unmatched_draw_at_current(void)
{
    uint8_t out[320 * 200];
    int n, mx, my;
    setup();
    make_bitmap(0x42);
    frame(GFX_TWEEN_TAG_PLAYER, 10, 20, 0.0, 100.0);
    erase(10, 20);
    frame(GFX_TWEEN_TAG_PLAYER, 200, 20, 100.0, 200.0);      /* 190 px jump: teleport */
    CHECK(gfx_tween_compose(out, 150.0, gfx_vram_generation));
    n = count_marker(out, 0x42, &mx, &my);
    CHECK(n == 8 && mx == 200 && my == 20);

    /* a tag with no counterpart in the previous frame draws where it is */
    erase(200, 20);
    gfx_tween_tag = GFX_TWEEN_TAG_ACTOR(3);
    gfx_copy_rect(100, 100, s_bitmap, 0);
    gfx_tween_tag = 0;
    frame(GFX_TWEEN_TAG_PLAYER, 210, 20, 200.0, 300.0);
    CHECK(gfx_tween_compose(out, 250.0, gfx_vram_generation));
    CHECK(out[100 * 320 + 100] == 0x42 && out[101 * 320 + 103] == 0x42);
    n = count_marker(out, 0x42, &mx, &my);
    CHECK(n == 16);
    teardown();
}

static void test_live_present_falls_back(void)
{
    uint8_t out[320 * 200];
    setup();
    make_bitmap(0x42);
    frame(GFX_TWEEN_TAG_PLAYER, 10, 20, 0.0, 100.0);
    erase(10, 20);
    frame(GFX_TWEEN_TAG_PLAYER, 20, 20, 100.0, 200.0);
    /* an unrelated present (a dialog) after the publish: held briefly, then live */
    gfx_box(0, 0, 320, 200);
    CHECK(gfx_tween_compose(out, 150.0, gfx_vram_generation));      /* within the hold window */
    CHECK(!gfx_tween_compose(out, 250.0, gfx_vram_generation));     /* gave up: show live VRAM */
    /* a new publish re-arms it */
    frame(GFX_TWEEN_TAG_PLAYER, 30, 20, 300.0, 400.0);
    CHECK(gfx_tween_compose(out, 350.0, gfx_vram_generation));
    teardown();
}

static void test_presenter_resume_after_live_mode(void)
{
    uint8_t out[320 * 200];
    int n, mx, my;
    setup();
    make_bitmap(0x42);

    /* These frames were published while the presenter was effectively in
     * live-VRAM mode.  Resuming must discard them rather than use stale
     * presenter history as an interpolation origin. */
    frame(GFX_TWEEN_TAG_PLAYER, 10, 20, 0.0, 100.0);
    erase(10, 20);
    frame(GFX_TWEEN_TAG_PLAYER, 30, 20, 100.0, 200.0);
    gfx_tween_presenter_resume();

    /* The first fresh frame establishes a new base. */
    erase(30, 20);
    frame(GFX_TWEEN_TAG_PLAYER, 50, 20, 200.0, 300.0);
    CHECK(gfx_tween_compose(out, 250.0, gfx_vram_generation));
    n = count_marker(out, 0x42, &mx, &my);
    CHECK(n == 8 && mx == 50 && my == 20);

    /* The following fresh pair must interpolate normally again. */
    erase(50, 20);
    frame(GFX_TWEEN_TAG_PLAYER, 70, 20, 300.0, 400.0);
    CHECK(gfx_tween_compose(out, 350.0, gfx_vram_generation));
    n = count_marker(out, 0x42, &mx, &my);
    CHECK(n == 8 && mx == 60 && my == 20);
    teardown();
}

static void test_disabled_and_untagged_capture_nothing(void)
{
    uint8_t out[320 * 200];
    int n, mx, my;
    setup();
    make_bitmap(0x42);
    /* untagged draws are background: never moved */
    gfx_copy_rect(10, 20, s_bitmap, 0);
    gfx_box(0, 0, 320, 200);
    gfx_tween_frame_publish(0.0, 100.0, gfx_vram_generation);
    gfx_copy_rect(30, 20, s_bitmap, 0);
    gfx_box(0, 0, 320, 200);
    gfx_tween_frame_publish(100.0, 200.0, gfx_vram_generation);
    CHECK(gfx_tween_compose(out, 150.0, gfx_vram_generation));
    n = count_marker(out, 0x42, &mx, &my);
    CHECK(n == 16 && mx == 10);
    CHECK(memcmp(out, gfx_vram, sizeof out) == 0);

    gfx_tween_set_enabled(false);
    CHECK(!gfx_tween_compose(out, 150.0, gfx_vram_generation));
    gfx_tween_tag = GFX_TWEEN_TAG_PLAYER;
    gfx_copy_rect(50, 20, s_bitmap, 0);          /* must not touch anything while disabled */
    gfx_tween_tag = 0;
    gfx_framebuffer_shutdown();
}

/* Edge geometry: a 32x48 sprite drawn partially outside the play window
 * (clip g94..g96 = rows 16..159, g98..g9a = packed columns 4..155), at
 * negative x, past the right edge, and below the bottom, both mirrored
 * and not.  Composed at alpha 0.5 between two identical frames the result
 * must equal the frame the game presented. */
static uint8_t s_big[0x22 + 16 * 48];
static void test_edge_clipping_matches_game(void)
{
    static const dos_int xs[] = { -16, -8, -2, 0, 3, 8, 280, 296, 300, 304, 311 };
    static const dos_int ys[] = { 0, 10, 16, 100, 130, 144, 158 };
    uint8_t out[320 * 200];
    int bad = 0;
    for (int flip = 0; flip < 2; flip++)
        for (size_t xi = 0; xi < sizeof xs / sizeof xs[0]; xi++)
            for (size_t yi = 0; yi < sizeof ys / sizeof ys[0]; yi++) {
                setup();
                g94 = 16; g96 = 159; g98 = 4; g9a = 155;
                memset(s_big, 0, sizeof s_big);
                s_big[0x11] = 0x42; s_big[0x12] = 0x43;
                s_big[0x20] = 16; s_big[0x21] = 48;
                for (int i = 0; i < 16 * 48; i++) s_big[0x22 + i] = (uint8_t)((i & 1) ? 0x12 : 0x21);
                gfx_tween_tag = GFX_TWEEN_TAG_PLAYER;
                gfx_copy_rect(xs[xi], ys[yi], s_big, flip);
                gfx_tween_tag = 0;
                gfx_box(0, 0, 320, 200);
                gfx_tween_frame_publish(0.0, 100.0, gfx_vram_generation);
                gfx_tween_tag = GFX_TWEEN_TAG_PLAYER;
                gfx_copy_rect(xs[xi], ys[yi], s_big, flip);
                gfx_tween_tag = 0;
                gfx_box(0, 0, 320, 200);
                gfx_tween_frame_publish(100.0, 200.0, gfx_vram_generation);
                if (!gfx_tween_compose(out, 150.0, gfx_vram_generation) || memcmp(out, gfx_vram, sizeof out) != 0) {
                    fprintf(stderr, "edge mismatch: x=%d y=%d flip=%d\n", (int)xs[xi], (int)ys[yi], flip);
                    bad++;
                }
                teardown();
            }
    CHECK(bad == 0);
}

/* Event-driven objects: a sprite published with its own window keeps
 * interpolating across an unrelated publish (an erase event with no ops),
 * and finds its previous position across publishes that did not draw it. */
static void test_event_driven_windows(void)
{
    uint8_t out[320 * 200];
    int n, mx, my;
    setup();
    make_bitmap(0x42);
    frame(GFX_TWEEN_TAG_INTRO, 10, 20, 0.0, 100.0);
    erase(10, 20);
    frame(GFX_TWEEN_TAG_INTRO, 30, 20, 100.0, 300.0);        /* 200 ms window */
    gfx_box(0, 0, 320, 200);
    gfx_tween_frame_publish(150.0, 150.0, gfx_vram_generation); /* erase event, no ops */
    CHECK(gfx_tween_compose(out, 200.0, gfx_vram_generation));  /* alpha .5 of the sprite's own window */
    n = count_marker(out, 0x42, &mx, &my);
    CHECK(n == 8 && mx == 20 && my == 20);

    /* next event: previous position found two publishes back */
    erase(30, 20);
    frame(GFX_TWEEN_TAG_INTRO, 50, 20, 300.0, 400.0);
    CHECK(gfx_tween_compose(out, 350.0, gfx_vram_generation));
    n = count_marker(out, 0x42, &mx, &my);
    CHECK(n == 8 && mx == 40 && my == 20);
    teardown();
}

/* Beam trail: 24 pixels, 8 new per frame.  At alpha .5 the four newest
 * head pixels are still hidden and the four oldest tail pixels of the
 * previous frame still linger. */
static void beam_frame(int head, double t0, double t1)
{
    /* pixel of absolute index c sits at (100 + 2c, 50); ages 0..23 from head */
    gfx_color_select(14);
    for (int age = 23; age >= 0; age--) {
        int c = head - age;
        gfx_tween_tag = GFX_TWEEN_TAG_BEAM(age);
        gfx_set_pixel((dos_int)(100 + 2 * c), 50);
        gfx_tween_tag = 0;
    }
    gfx_box(0, 0, 320, 200);
    gfx_tween_frame_publish(t0, t1, gfx_vram_generation);
}
static void test_beam_grows_and_shrinks(void)
{
    uint8_t out[320 * 200];
    setup();
    g3904[14] = 14;
    beam_frame(23, 0.0, 100.0);                    /* pixels c = 0..23 */
    for (int c = 0; c < 8; c++) g3924[50][100 + 2 * c] = 7;   /* game erases the tail */
    beam_frame(31, 100.0, 200.0);                  /* pixels c = 8..31 */
    CHECK(gfx_tween_compose(out, 150.0, gfx_vram_generation));
    /* visible: c = 4..27 (tail 4..7 lingering, head 28..31 not yet) */
    for (int c = 0; c < 36; c++) {
        bool on = out[50 * 320 + 100 + 2 * c] == 14;
        bool want = c >= 4 && c <= 27;
        if (on != want) { fprintf(stderr, "beam c=%d on=%d want=%d\n", c, on, want); s_failures++; }
    }
    CHECK(gfx_tween_compose(out, 200.0, gfx_vram_generation));
    CHECK(memcmp(out, gfx_vram, sizeof out) == 0);
    teardown();
}

int main(void)
{
    test_event_driven_windows();
    test_beam_grows_and_shrinks();
    test_edge_clipping_matches_game();
    test_interpolates_between_frames();
    test_teleport_and_unmatched_draw_at_current();
    test_live_present_falls_back();
    test_presenter_resume_after_live_mode();
    test_disabled_and_untagged_capture_nothing();
    if (s_failures) {
        fprintf(stderr, "test_gfx_tween: %d failure(s)\n", s_failures);
        return 1;
    }
    puts("test_gfx_tween: OK");
    return 0;
}
