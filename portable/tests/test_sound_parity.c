/* test_sound_parity.c -- differential parity test for portable/audio/
 * sound_driver.c against the REAL asm/SOUND.ASM machine code, executed
 * once ahead of time under Unicorn (tools/portable/sound_oracle/emu.py +
 * gen_scenarios.py) and captured into the line-based text fixtures this
 * file reads: portable/tests/fixtures/sound_scenario_*.txt.
 *
 * Each fixture holds: the backend mode and real AE000 resource/voice
 * block bytes the scenario was driven with, the initial and final
 * historical-layout DGROUP sound-region snapshot (sound.h's
 * SOUND_DRIVER_SNAPSHOT_SIZE, filled/read via sound_driver_snapshot()/
 * sound_driver_restore()), the ordered C-facing routine call sequence,
 * a tick count, and the REAL asm/SOUND.ASM's ordered backend-event log
 * (sound.h's enum sound_event_kind numbering, which this file's own
 * driver run must reproduce exactly via sound_event_log_get()).
 *
 * This file only DRIVES the C port and COMPARES; it does not re-derive
 * expected values from first principles (that is what the oracle did).
 * Regenerate the fixtures with:
 *   python tools/portable/sound_oracle/gen_scenarios.py
 * (requires assets/AE000.DAT and assets/AEPROG.EXE, which this test
 * itself does NOT need -- the fixtures are committed, same convention as
 * tools/portable/oracle/'s fixtures).
 */
#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures = 0;

static void fail(const char *scenario, const char *what)
{
    fprintf(stderr, "test_sound_parity: FAIL %s: %s\n", scenario, what);
    g_failures++;
}

static void check(const char *scenario, int cond, const char *what)
{
    if (!cond) fail(scenario, what);
}

/* ---- local resource/voice block storage (sound.h's ES:DI model) ---- */
#define RESOURCE_BLOCK_CAP 16384
#define VOICE_BLOCK_SIZE 0x620

static uint8_t g_resource_block[RESOURCE_BLOCK_CAP];
static uint8_t g_voice_block[VOICE_BLOCK_SIZE];

/* ---- tiny hex helpers (no dependency on a project-wide hex codec) ---- */

static int hex_nibble(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

/* Decodes `hex` (NUL- or newline-terminated) into `out` (capacity
 * out_cap). Returns the decoded byte count, or -1 on a malformed string
 * or overflow. */
static long hex_decode(const char *hex, uint8_t *out, size_t out_cap)
{
    size_t n = 0;
    while (hex[0] != '\0' && hex[0] != '\n' && hex[0] != '\r') {
        int hi = hex_nibble(hex[0]);
        int lo = hex[1] ? hex_nibble(hex[1]) : -1;
        if (hi < 0 || lo < 0) return -1;
        if (n >= out_cap) return -1;
        out[n++] = (uint8_t)((hi << 4) | lo);
        hex += 2;
    }
    return (long)n;
}

/* ---- line-based fixture reader ---- */

#define LINE_CAP 65536

static char g_line[LINE_CAP];

static int read_line(FILE *f)
{
    if (!fgets(g_line, sizeof g_line, f)) return 0;
    size_t len = strlen(g_line);
    while (len > 0 && (g_line[len - 1] == '\n' || g_line[len - 1] == '\r')) {
        g_line[--len] = '\0';
    }
    return 1;
}

struct scenario {
    int backend_mode;
    long resource_len;
    long voice_len;
    uint8_t region_initial[SOUND_DRIVER_SNAPSHOT_SIZE];
    uint8_t region_final[SOUND_DRIVER_SNAPSHOT_SIZE];
    int ticks;
    /* call sequence, replayed verbatim (see dispatch_call()) */
    char call_lines[64][64];
    int call_count;
    /* expected event log */
    struct { long tick; int kind; long a; long b; } *events;
    int event_count;
};

static int parse_scenario(const char *path, struct scenario *sc)
{
    FILE *f = fopen(path, "r");
    if (!f) return 0;

    memset(sc, 0, sizeof *sc);
    long n;

    while (read_line(f)) {
        if (g_line[0] == '#' || g_line[0] == '\0') continue;

        if (strncmp(g_line, "BACKEND_MODE ", 13) == 0) {
            sc->backend_mode = atoi(g_line + 13);
        } else if (strncmp(g_line, "RESOURCE_HEX ", 13) == 0) {
            n = hex_decode(g_line + 13, g_resource_block, RESOURCE_BLOCK_CAP);
            if (n < 0) { fclose(f); return 0; }
            sc->resource_len = n;
        } else if (strncmp(g_line, "VOICE_HEX ", 10) == 0) {
            memset(g_voice_block, 0xFF, sizeof g_voice_block);
            n = hex_decode(g_line + 10, g_voice_block, VOICE_BLOCK_SIZE);
            if (n < 0) { fclose(f); return 0; }
            sc->voice_len = n;
        } else if (strncmp(g_line, "REGION_INITIAL_HEX ", 19) == 0) {
            n = hex_decode(g_line + 19, sc->region_initial, SOUND_DRIVER_SNAPSHOT_SIZE);
            if (n != SOUND_DRIVER_SNAPSHOT_SIZE) { fclose(f); return 0; }
        } else if (strncmp(g_line, "REGION_FINAL_HEX ", 17) == 0) {
            n = hex_decode(g_line + 17, sc->region_final, SOUND_DRIVER_SNAPSHOT_SIZE);
            if (n != SOUND_DRIVER_SNAPSHOT_SIZE) { fclose(f); return 0; }
        } else if (strcmp(g_line, "CALLS") == 0) {
            sc->call_count = 0;
            while (read_line(f) && strcmp(g_line, "END_CALLS") != 0) {
                if (sc->call_count >= (int)(sizeof sc->call_lines / sizeof sc->call_lines[0])) {
                    fclose(f);
                    return 0;
                }
                snprintf(sc->call_lines[sc->call_count], sizeof sc->call_lines[0], "%s", g_line);
                sc->call_count++;
            }
        } else if (strncmp(g_line, "TICKS ", 6) == 0) {
            sc->ticks = atoi(g_line + 6);
        } else if (strncmp(g_line, "EVENTS ", 7) == 0) {
            int count = atoi(g_line + 7);
            sc->events = calloc((size_t)count, sizeof *sc->events);
            sc->event_count = count;
            for (int i = 0; i < count; i++) {
                if (!read_line(f)) { fclose(f); return 0; }
                long tick, a, b;
                int kind;
                if (sscanf(g_line, "%ld %d %ld %ld", &tick, &kind, &a, &b) != 4) {
                    fclose(f);
                    return 0;
                }
                sc->events[i].tick = tick;
                sc->events[i].kind = kind;
                sc->events[i].a = a;
                sc->events[i].b = b;
            }
            if (!read_line(f) || strcmp(g_line, "END_EVENTS") != 0) { fclose(f); return 0; }
        }
    }

    fclose(f);
    return 1;
}

/* Maps one "CALLS" fixture line to the matching sound.h entry point. Kept
 * as an explicit dispatch (not a generic table) since the set of call
 * shapes is small and fixed -- see sound.h's "8 C-facing entry points". */
static void dispatch_call(const char *scenario_name, const char *line)
{
    int arg;
    if (strcmp(line, "select_init") == 0) {
        sound_backend_select_init();
    } else if (sscanf(line, "voice_table_reload %d", &arg) == 1) {
        sound_voice_table_reload((dos_int)arg);
    } else if (strcmp(line, "voice_cursors_direct") == 0) {
        /* C twin of emu.py's SoundMachine.poke_voice_cursors_direct() --
         * see gen_scenarios.py's mode2_queued_opl comment for why this
         * bypasses sound_voice_table_reload()'s record_panel_rebuild()
         * call (an unrelated UI subsystem, out of scope here). */
        /* voice_stream_cursor_table/voice_stream_base_table are declared
         * dos_int[4] in game_data.h (SOUND_VOICE_COUNT_MAX in
         * portable/audio/sound_driver_internal.h, a private header this
         * test does not include). */
        int voice;
        for (voice = 0; voice < 4; voice++) {
            voice_stream_cursor_table[voice] = 0;
            voice_stream_base_table[voice] = 0;
        }
    } else if (sscanf(line, "arm %d", &arg) == 1) {
        stream_control_block_arm((dos_int)arg);
    } else if (strcmp(line, "voices_reset") == 0) {
        sound_voices_reset();
    } else if (strcmp(line, "disable_all") == 0) {
        sound_voices_disable_all();
    } else if (strcmp(line, "stop_reset") == 0) {
        sound_stop_reset();
    } else {
        fail(scenario_name, line);
    }
}

static void run_one(const char *fixture_dir, const char *name)
{
    char path[1024];
    snprintf(path, sizeof path, "%s/sound_scenario_%s.txt", fixture_dir, name);

    struct scenario sc;
    if (!parse_scenario(path, &sc)) {
        fail(name, "could not parse fixture (missing/malformed -- regenerate with "
                    "python tools/portable/sound_oracle/gen_scenarios.py)");
        return;
    }

    sound_event_log_clear();
    sound_set_resource_blocks(sc.resource_len > 0 ? g_resource_block : NULL,
                               sc.voice_len > 0 ? g_voice_block : NULL);
    sound_driver_restore(sc.region_initial);

    for (int i = 0; i < sc.call_count; i++)
        dispatch_call(name, sc.call_lines[i]);
    for (int t = 0; t < sc.ticks; t++)
        sound_tick_entry();

    /* -- compare the event log -- */
    size_t got_count = sound_event_log_count();
    char what[256];
    if ((long)got_count != sc.event_count) {
        snprintf(what, sizeof what, "event count: got %zu, expected %d", got_count, sc.event_count);
        fail(name, what);
    }
    size_t compare_n = got_count < (size_t)sc.event_count ? got_count : (size_t)sc.event_count;
    size_t first_mismatch = compare_n;
    for (size_t i = 0; i < compare_n; i++) {
        const struct sound_event *ev = sound_event_log_get(i);
        if (!ev) { first_mismatch = i; break; }
        if ((long)ev->tick != sc.events[i].tick || ev->kind != sc.events[i].kind ||
            (long)ev->a != sc.events[i].a || (long)ev->b != sc.events[i].b) {
            first_mismatch = i;
            break;
        }
    }
    if (first_mismatch < compare_n) {
        const struct sound_event *ev = sound_event_log_get(first_mismatch);
        snprintf(what, sizeof what,
                 "first event divergence at index %zu: got {tick=%u kind=%d a=%u b=%u}, "
                 "expected {tick=%ld kind=%d a=%ld b=%ld}",
                 first_mismatch, ev ? ev->tick : 0u, ev ? ev->kind : -1,
                 ev ? (unsigned)ev->a : 0u, ev ? (unsigned)ev->b : 0u,
                 sc.events[first_mismatch].tick, sc.events[first_mismatch].kind,
                 sc.events[first_mismatch].a, sc.events[first_mismatch].b);
        fail(name, what);
    }

    /* -- compare the final historical-layout DGROUP sound region -- */
    uint8_t region_final[SOUND_DRIVER_SNAPSHOT_SIZE];
    sound_driver_snapshot(region_final);
    if (memcmp(region_final, sc.region_final, SOUND_DRIVER_SNAPSHOT_SIZE) != 0) {
        size_t off = 0;
        while (off < SOUND_DRIVER_SNAPSHOT_SIZE && region_final[off] == sc.region_final[off]) off++;
        snprintf(what, sizeof what,
                 "final region mismatch at snapshot byte %zu (DS:%04zX-ish): got 0x%02x, expected 0x%02x",
                 off, (size_t)0x175E + off, region_final[off], sc.region_final[off]);
        fail(name, what);
    }

    free(sc.events);
    printf("test_sound_parity: %s: %d calls, %d ticks, %d events -- %s\n",
           name, sc.call_count, sc.ticks, sc.event_count,
           g_failures == 0 ? "ok so far" : "see failures above");
}

int main(void)
{
    const char *fixture_dir = EMPIRES_FIXTURE_DIR;

    run_one(fixture_dir, "sanity_gate_off");
    run_one(fixture_dir, "mode0_basic");
    run_one(fixture_dir, "mode2_queued_opl");

    if (g_failures) {
        fprintf(stderr, "test_sound_parity: %d failure(s)\n", g_failures);
        return 1;
    }
    printf("test_sound_parity: all scenarios match the real asm/SOUND.ASM oracle\n");
    return 0;
}
