/* main.c -- SDL3 executable entry point.
 *
 * Threading model (docs/portable/architecture.md, "Timing model"):
 *   - the GAME thread runs the historical game_main(): boot, intro, menus,
 *     levels; it blocks in timer_wait_ticks()/keyboard reads exactly where
 *     the DOS program busy-waited;
 *   - the TIMER thread (platform/sdl3/clock.c) delivers 236.7 Hz ticks and
 *     services the sound state machine, like the historical INT 8;
 *   - this MAIN thread pumps SDL events into the keyboard service (the
 *     historical INT 9) and uploads the presented VRAM at display rate.
 *
 * Options: --assets DIR (AE000.DAT/AE001.DAT location, default: the exe's
 * directory, then "assets"), --saves DIR (save-slot overlays), --demo
 * (primitive test scene instead of the game), --selftest (exit after
 * ~300 ms), --dump-vram FILE (write the presented frame as PPM at exit),
 * --deterministic (no tick thread: the game's own waits/polls advance the
 * 236.7 Hz clock, and --script/--selftest-ms/--dump-interval run on that
 * virtual time, so a run is reproducible for regression tests).
 * Historical switches (-E/-C/-T/-M/-V, -I, -S?) pass through to
 * cmdline_parse_args().
 *
 * Only files in portable/platform/sdl3/ may include <SDL3/SDL.h>.
 */
#include <SDL3/SDL.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "trace.h"
#include "sync.h"
#include "input_sdl.h"
#include "video_sdl.h"
#include "audio_sdl.h"

static Uint64 s_selftest_ms = 300;
static bool s_deterministic;   /* --deterministic: manual ticks, virtual time for scripts/dumps */

/* Virtual milliseconds in deterministic mode (derived from timer_ticks at
 * the historical rate), wall clock otherwise. */
static Uint64 now_ms(void)
{
    if (s_deterministic)
        return (Uint64)((double)timer_ticks * 1000.0 / TIMER_TICK_HZ);
    return SDL_GetTicks();
}

static volatile bool s_game_finished = false;

/* --script "ms:scan[:ascii[:hold_ms]],..." injects historical make/break pairs for
 * headless bring-up runs and replay tests.  Each entry fires once the game
 * has been continuously asking for a key (empty polls or blocking waits)
 * for at_ms, i.e. it is idle waiting for the user. */
typedef struct { Uint64 at_ms; uint8_t scan; uint8_t ascii; Uint64 hold_ms; char marker[32]; int repeat; int timed; } script_key;
static uint8_t s_held_scan; static Uint64 s_release_at;
#define SCRIPT_MAX 1024
static script_key s_script[SCRIPT_MAX];
static int s_script_n, s_script_next;


static void parse_script(const char *spec)
{
    while (*spec && s_script_n < SCRIPT_MAX) {
        char *end;
        script_key k;
        memset(&k, 0, sizeof k);
        if (*spec == '@') {                       /* wait for a trace marker */
            size_t n = strcspn(spec + 1, ",");
            if (n >= sizeof k.marker) n = sizeof k.marker - 1;
            memcpy(k.marker, spec + 1, n);
            s_script[s_script_n++] = k;
            spec += 1 + n;
            if (*spec == ',') spec++;
            continue;
        }
        if (*spec == '*') {                       /* repeat until the next marker fires */
            k.repeat = 1;
            spec++;
        }
        if (*spec == '+') {                       /* wall-clock delay, not idle-gated */
            k.timed = 1;
            spec++;
        }
        k.at_ms = (Uint64)strtoull(spec, &end, 10);
        if (*end != ':') break;
        k.scan = (uint8_t)strtoul(end + 1, &end, 16);
        k.ascii = 0;
        if (*end == ':')
            k.ascii = (uint8_t)strtoul(end + 1, &end, 16);
        if (*end == ':')                          /* hold duration (ms) before the break */
            k.hold_ms = (Uint64)strtoull(end + 1, &end, 10);
        s_script[s_script_n++] = k;
        spec = (*end == ',') ? end + 1 : end;
    }
    if (*spec)
        fprintf(stderr, "script: %d entries parsed, remaining text ignored: %.40s\n", s_script_n, spec);
}

/* --script-file FILE: the same syntax, whitespace/newlines allowed between
 * entries, '#' comments to end of line. */
static void parse_script_file(const char *path)
{
    FILE *f = fopen(path, "rb");
    char *text, *out;
    long n;
    if (!f) { fprintf(stderr, "script: cannot open %s\n", path); return; }
    fseek(f, 0, SEEK_END); n = ftell(f); fseek(f, 0, SEEK_SET);
    text = (char *)malloc((size_t)n + 1);
    if (!text) { fclose(f); return; }
    n = (long)fread(text, 1, (size_t)n, f);
    fclose(f);
    text[n] = 0;
    out = text;
    for (char *in = text; *in; in++) {          /* strip comments and whitespace */
        if (*in == '#') { while (*in && *in != 0x0a) in++; if (!*in) break; continue; }
        if (*in == ' ' || *in == 0x09 || *in == 0x0d || *in == 0x0a) continue;
        *out++ = *in;
    }
    *out = 0;
    parse_script(text);
    free(text);
}

static void game_thread_fn(void *arg)
{
    (void)arg;
    game_main();            /* video_mode_select -> boot -> game_run -> shutdown */
    s_game_finished = true;
}

/* Draw a scene that exercises the primitives with the real gfx_* calls. */
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
    gfx_clear_rect(11, 71, 61, 31);
    gfx_fill_rect(20, 80, 41, 13);
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
    gfx_box(0, 0, 320, 200);
}

/* Write the presented VRAM as a binary PPM (DAC expanded to 8-bit RGB). */
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

static bool file_exists(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f)
        return false;
    fclose(f);
    return true;
}

/* Default asset directory: the executable's own directory if it holds
 * AE000.DAT, else "assets" (the repository layout). */
static void choose_asset_dir(char *out, size_t n, const char *explicit)
{
    if (explicit) {
        snprintf(out, n, "%s", explicit);
        return;
    }
    const char *base = SDL_GetBasePath();
    char probe[1024];
    if (base) {
        snprintf(probe, sizeof probe, "%sAE000.DAT", base);
        if (file_exists(probe)) {
            snprintf(out, n, "%s", base);
            return;
        }
    }
    snprintf(out, n, "assets");
}


static const char *s_dump_path;
static Uint64 s_dump_interval, s_next_dump, s_start_ticks;
static int s_dump_index;

/* One bring-up step: scripted input and periodic frame dumps.  Runs on the
 * main thread every frame in real-time mode, or from the tick observer (game
 * thread, virtual time) in deterministic mode. */
static void bringup_step(void)
{
    /* Scripted input: an entry fires once the game has been asking for
     * a key (empty polls / blocking waits) continuously for at_ms. */
    {
        static Uint64 idle_since, last_fire, last_ask; static uint32_t last_empty_seen;
        Uint64 now = now_ms();
        if (input_empty_reads != last_empty_seen || input_blocked)
            last_ask = now;                 /* the game asked for a key */
        if (now - last_ask > 500)
            idle_since = now;               /* no request for 500 ms: not idle-waiting */
        last_empty_seen = input_empty_reads;
        /* A marker entry is satisfied when the game emits that trace;
         * a '*' key entry before it keeps firing (when idle) until then. */
        {
            static unsigned seen_seq;
            int cur = s_script_next;
            if (cur < s_script_n && s_script[cur].repeat && cur + 1 < s_script_n &&
                s_script[cur + 1].marker[0])
                cur = cur + 1;              /* look at the marker first */
            if (cur < s_script_n && s_script[cur].marker[0]) {
                unsigned end = empires_trace_seq;
                if (end - seen_seq > EMPIRES_TRACE_RING)
                    seen_seq = end - EMPIRES_TRACE_RING;
                while (seen_seq != end) {
                    const char *m = empires_trace_ring[seen_seq % EMPIRES_TRACE_RING];
                    seen_seq++;
                    if (m && strncmp(m, s_script[cur].marker, strlen(s_script[cur].marker)) == 0) {
                        s_script_next = cur + 1;
                        idle_since = now;
                        last_fire = now;
                        break;
                    }
                }
            }
            if (s_script_next < s_script_n && !s_script[s_script_next].marker[0] &&
                now - (s_script[s_script_next].timed ? last_fire : idle_since) >= s_script[s_script_next].at_ms) {
                const script_key *k = &s_script[s_script_next];
                if (!k->repeat)
                    s_script_next++;
                if (s_held_scan) { input_key_event(s_held_scan, false, 0); s_held_scan = 0; }
                input_key_event(k->scan, true, k->ascii);
                if (k->hold_ms) { s_held_scan = k->scan; s_release_at = now + k->hold_ms; }
                else input_key_event(k->scan, false, 0);
                idle_since = now;
                last_fire = now;
            }
        }
        if (s_held_scan && now >= s_release_at) {
            input_key_event(s_held_scan, false, 0);
            s_held_scan = 0;
        }
    }
    if (s_dump_interval && s_dump_path && now_ms() - s_start_ticks >= s_next_dump) {
        char path[1100];
        snprintf(path, sizeof path, "%s.%03d.ppm", s_dump_path, s_dump_index++);
        dump_vram_ppm(path);
        s_next_dump += s_dump_interval;
    }
}

static volatile bool s_virtual_deadline_hit;

static void deterministic_tick_observer(void)
{
    bringup_step();
    if (s_selftest_ms && now_ms() - s_start_ticks >= s_selftest_ms) {
        /* Freeze the game thread here so the final frame is fixed before
         * the main thread exits (keeps replays byte-reproducible). */
        s_virtual_deadline_hit = true;
        for (;;)
            sync_sleep_ns(10000000ull);
    }
}

void crash_handler_install(void);

int main(int argc, char **argv)
{
    crash_handler_install();
    bool selftest = false, demo = false;
    const char *assets = NULL, *saves = NULL;
    char asset_dir[1024];
    sync_thread game_thread = { NULL };

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--selftest") == 0)
            selftest = true;
        else if (strcmp(argv[i], "--selftest-ms") == 0 && i + 1 < argc) {
            selftest = true;
            s_selftest_ms = (Uint64)strtoull(argv[++i], NULL, 10);
        }
        else if (strcmp(argv[i], "--demo") == 0)
            demo = true;
        else if (strcmp(argv[i], "--deterministic") == 0)
            s_deterministic = true;
        else if (strcmp(argv[i], "--dump-vram") == 0 && i + 1 < argc)
            s_dump_path = argv[++i];
        else if (strcmp(argv[i], "--assets") == 0 && i + 1 < argc)
            assets = argv[++i];
        else if (strcmp(argv[i], "--saves") == 0 && i + 1 < argc)
            saves = argv[++i];
        else if (strcmp(argv[i], "--script") == 0 && i + 1 < argc)
            parse_script(argv[++i]);
        else if (strcmp(argv[i], "--script-file") == 0 && i + 1 < argc)
            parse_script_file(argv[++i]);
        else if (strcmp(argv[i], "--dump-interval") == 0 && i + 1 < argc)
            s_dump_interval = (Uint64)strtoull(argv[++i], NULL, 10);
    }

    if (!sdl_video_init("Empires (portable)")) {
        sdl_video_shutdown();
        return 1;
    }
    audio_sdl_init(); /* logs and continues without audio on failure -- see audio_sdl.h */

    startup_set_args(argc, argv);
    choose_asset_dir(asset_dir, sizeof asset_dir, assets);
    resource_set_asset_dir(asset_dir);
    resource_set_save_dir(saves ? saves : asset_dir);

    if (s_deterministic) {
        timer_service_set_manual(true);
        timer_set_tick_observer(deterministic_tick_observer);
    }

    if (demo) {
        display_mode = 5;
        gfx_framebuffer_init();
        color_lookup_tables_init();
        video_load_palette(g11e);
        draw_demo_scene();
        s_game_finished = true;     /* nothing to wait for */
    } else {
        if (!sync_thread_start(&game_thread, game_thread_fn, NULL)) {
            fprintf(stderr, "cannot start the game thread\n");
            audio_sdl_shutdown();
            sdl_video_shutdown();
            return 1;
        }
    }

    s_start_ticks = now_ms();
    uint32_t presented_generation = 0;
    bool quit = false;
    while (!quit) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT)
                quit = true;
            else if (ev.type == SDL_EVENT_KEY_DOWN && ev.key.key == SDLK_RETURN &&
                     (ev.key.mod & SDL_KMOD_ALT) && !ev.key.repeat)
                sdl_video_toggle_fullscreen();      /* host convenience, not a game key */
            else if (ev.type == SDL_EVENT_KEY_DOWN || ev.type == SDL_EVENT_KEY_UP)
                input_sdl_handle_event(&ev);
        }
        if (!s_deterministic)
            bringup_step();
        if (gfx_vram_generation != presented_generation || demo) {
            presented_generation = gfx_vram_generation;
            sdl_video_present(gfx_vram, gfx_dac);
        } else {
            SDL_Delay(4);
        }
        if (selftest && (s_deterministic ? s_virtual_deadline_hit
                                        : (now_ms() - s_start_ticks) >= s_selftest_ms))
            quit = true;
        if (!demo && s_game_finished)
            quit = true;
    }

    if (s_dump_path)
        dump_vram_ppm(s_dump_path);
    if (getenv("EMPIRES_TRACE")) {
        size_t n = sound_event_log_count(), i, kinds[8] = {0};
        for (i = 0; i < n; i++) {
            const struct sound_event *e = sound_event_log_get(i);
            if (e && (unsigned)e->kind < 8) kinds[e->kind]++;
        }
        fprintf(stderr, "[trace] sound events logged: %zu (opl=%zu pit=%zu gate=%zu nibble=%zu)\n",
                n, kinds[0], kinds[1], kinds[2], kinds[3]);
    }

    if (!demo) {
        if (s_game_finished) {
            sync_thread_join(&game_thread);
        } else {
            /* The historical program only ends through its own menus; a
             * closed window ends the process outright, like a DOS reboot
             * would have.  Save slots are already on disk at that point. */
            fflush(stdout);
            audio_sdl_shutdown();
            sdl_video_shutdown();
            _Exit(0);
        }
    }
    gfx_framebuffer_shutdown();
    audio_sdl_shutdown();
    sdl_video_shutdown();
    return 0;
}
