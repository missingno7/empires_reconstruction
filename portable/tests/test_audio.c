/* test_audio.c -- unit tests for portable/audio/audio_mixer.c,
 * opl_backend.c and speaker_synth.c (Milestone F, docs/portable/
 * architecture.md "Audio model").  No SDL: only the host-independent
 * audio.h API is exercised (portable/audio/audio_mixer.c never includes
 * SDL headers, and opl_backend.c/speaker_synth.c are private to
 * portable/audio/ -- not reachable from here, same as sound_driver.c's
 * own private sound_driver_internal.h is not reachable from
 * test_sound.c).  Every event is pushed through audio_mixer_push_event()
 * exactly as portable/audio/sound_driver.c's hooks do, and inspected
 * through audio_render()'s output plus the module's test/diagnostic
 * hooks (audio_mixer_events_applied_count()).
 */
#include "audio.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define TEST_SAMPLE_RATE 48000

static int g_failures = 0;

static void fail(const char *test, const char *what)
{
    fprintf(stderr, "test_audio: FAIL %s: %s\n", test, what);
    g_failures++;
}

static void check(const char *test, int cond, const char *what)
{
    if (!cond)
        fail(test, what);
}

/* ---------------------------------------------------------------------
 * (a) PC-speaker synth: PIT divisor 1193 @ 48000 Hz should give a square
 * wave at 1193182/1193 ~= 1000.15 Hz (counted via zero crossings), and
 * gating the speaker off should silence the output.
 * --------------------------------------------------------------------- */
static void test_speaker_synth(void)
{
    const char *t = "speaker_synth";

    audio_mixer_init(TEST_SAMPLE_RATE);

    audio_mixer_push_event(AUDIO_EVENT_PIT_DIVISOR, 0, 1193, 0);
    audio_mixer_push_event(AUDIO_EVENT_SPEAKER_GATE, 0, 1, 0);   /* on, plain PC-speaker gate */
    audio_mixer_push_event(AUDIO_EVENT_SPEAKER_GATE, 300, 0, 0); /* off, ~300 ticks (~1.27s) later */

    uint64_t on_sample = audio_mixer_ticks_to_samples(0 + AUDIO_LATENCY_TICKS);
    uint64_t off_sample = audio_mixer_ticks_to_samples(300 + AUDIO_LATENCY_TICKS);

    int total = (int)off_sample + TEST_SAMPLE_RATE / 4; /* off_sample + 250ms tail */
    int16_t *buf = (int16_t *)malloc(sizeof(int16_t) * (size_t)total);
    audio_render(buf, total);

    /* Zero crossings over a clean 0.5s window well inside the "on" period. */
    int win_start = (int)on_sample + 100;
    int win_len = TEST_SAMPLE_RATE / 2;
    int crossings = 0;
    for (int i = win_start + 1; i < win_start + win_len; i++) {
        bool prev_pos = buf[i - 1] >= 0;
        bool cur_pos = buf[i] >= 0;
        if (prev_pos != cur_pos)
            crossings++;
    }
    double expected_freq = 1193182.0 / 1193.0; /* ~1000.15 Hz */
    double expected = 2.0 * expected_freq * ((double)win_len / TEST_SAMPLE_RATE);
    check(t, crossings > (int)(expected * 0.85) && crossings < (int)(expected * 1.15),
          "divisor 1193 @ 48kHz should give a ~1000Hz square wave (zero-crossing count out of range)");

    /* Well after gate-off (past AUDIO_LATENCY_TICKS' worth of margin),
     * output must be exactly silent -- the mix has nothing else feeding
     * it (no OPL note played in this test). */
    int tail_start = (int)off_sample + TEST_SAMPLE_RATE / 20; /* +50ms margin */
    bool all_zero = true;
    for (int i = tail_start; i < total; i++) {
        if (buf[i] != 0) {
            all_zero = false;
            break;
        }
    }
    check(t, all_zero, "speaker gate off should silence output");

    free(buf);
}

/* ---------------------------------------------------------------------
 * (b) Event scheduling: push events at ticks 0, 10, 20 (kind
 * AUDIO_EVENT_NIBBLE_WRITE -- counted by audio_mixer_events_applied_
 * count() but with no audio side effect, so it isolates the scheduling
 * mechanism from the OPL/speaker backends) and check each is applied
 * exactly when the render cursor reaches its scheduled sample position,
 * not before.
 * --------------------------------------------------------------------- */
static void render_up_to(uint64_t *rendered, uint64_t target)
{
    int16_t buf[256];
    while (*rendered < target) {
        uint64_t remain = target - *rendered;
        int chunk = remain < 256 ? (int)remain : 256;
        audio_render(buf, chunk);
        *rendered += (uint64_t)chunk;
    }
}

static void test_event_scheduling(void)
{
    const char *t = "event_scheduling";
    uint64_t rendered = 0;
    int16_t buf[16];

    audio_mixer_init(TEST_SAMPLE_RATE);

    audio_mixer_push_event(AUDIO_EVENT_NIBBLE_WRITE, 0, 1, 0);
    audio_mixer_push_event(AUDIO_EVENT_NIBBLE_WRITE, 10, 2, 0);
    audio_mixer_push_event(AUDIO_EVENT_NIBBLE_WRITE, 20, 3, 0);

    uint64_t s0 = audio_mixer_ticks_to_samples(0 + AUDIO_LATENCY_TICKS);
    uint64_t s10 = audio_mixer_ticks_to_samples(10 + AUDIO_LATENCY_TICKS);
    uint64_t s20 = audio_mixer_ticks_to_samples(20 + AUDIO_LATENCY_TICKS);

    render_up_to(&rendered, s0 - 1);
    check(t, audio_mixer_events_applied_count() == 0, "tick-0 event applied too early");
    audio_render(buf, 8);
    rendered += 8;
    check(t, audio_mixer_events_applied_count() == 1, "tick-0 event not applied at its scheduled sample");

    render_up_to(&rendered, s10 - 1);
    check(t, audio_mixer_events_applied_count() == 1, "tick-10 event applied too early");
    audio_render(buf, 8);
    rendered += 8;
    check(t, audio_mixer_events_applied_count() == 2, "tick-10 event not applied at its scheduled sample");

    render_up_to(&rendered, s20 - 1);
    check(t, audio_mixer_events_applied_count() == 2, "tick-20 event applied too early");
    audio_render(buf, 8);
    rendered += 8;
    check(t, audio_mixer_events_applied_count() == 3, "tick-20 event not applied at its scheduled sample");
}

/* ---------------------------------------------------------------------
 * (c) OPL backend: configure a simple note on channel 0 and check the
 * rendered buffer is non-silent while held, then key it off and check it
 * decays to (near-)silence within a few hundred ms.
 *
 * The task's register list (0x20/0x40/0x60/0x80/0xA0/0xB0) names register
 * *classes*; 0x20/0x40/0x60/0x80 are per-operator, so both of channel 0's
 * operators are written (base and base+3, standard OPL2 slot layout), and
 * 0xC0 (connection/feedback) is set to additive (bit0=1) so both
 * operators contribute to the audible output regardless of the exact
 * modulator/carrier arrangement -- a plain FM patch (0xC0 left at its
 * post-reset 0) would need the *carrier* operator specifically configured
 * with a nonzero attack rate to ever produce sound, which is incidental
 * detail this test does not want to depend on.
 * --------------------------------------------------------------------- */
static void opl_write_evt(uint32_t tick, uint8_t reg, uint8_t val)
{
    audio_mixer_push_event(AUDIO_EVENT_OPL_WRITE, tick, reg, val);
}

static void test_opl_backend(void)
{
    const char *t = "opl_backend";
    const uint16_t fnum = 0x2AE;
    const uint8_t block = 4;
    const uint32_t t_on = 0;
    const uint32_t t_off = 60; /* ~253ms after note-on: well into a settled sustain */

    audio_mixer_init(TEST_SAMPLE_RATE);

    opl_write_evt(t_on, 0x20, 0x20);
    opl_write_evt(t_on, 0x23, 0x20); /* EGT=1 (sustained envelope), no AM/VIB */
    opl_write_evt(t_on, 0x40, 0x00);
    opl_write_evt(t_on, 0x43, 0x00); /* TL=0 (loudest), KSL=0 */
    opl_write_evt(t_on, 0x60, 0xF0);
    opl_write_evt(t_on, 0x63, 0xF0); /* AR=15 (fastest attack), DR=0 */
    opl_write_evt(t_on, 0x80, 0x0C);
    opl_write_evt(t_on, 0x83, 0x0C); /* SL=0 (loudest sustain), RR=12 (fast-ish release) */
    opl_write_evt(t_on, 0xC0, 0x01); /* connection=1 (additive), feedback=0 */
    opl_write_evt(t_on, 0xA0, (uint8_t)(fnum & 0xFF));
    opl_write_evt(t_on, 0xB0, (uint8_t)(0x20 | (block << 2) | (fnum >> 8))); /* key on */

    opl_write_evt(t_off, 0xB0, (uint8_t)((block << 2) | (fnum >> 8))); /* key off, same pitch bits */

    uint64_t note_on_sample = audio_mixer_ticks_to_samples(t_on + AUDIO_LATENCY_TICKS);
    uint64_t note_off_sample = audio_mixer_ticks_to_samples(t_off + AUDIO_LATENCY_TICKS);

    int total = (int)note_off_sample + TEST_SAMPLE_RATE; /* +1s tail past key-off */
    int16_t *buf = (int16_t *)malloc(sizeof(int16_t) * (size_t)total);
    audio_render(buf, total);

    /* Non-silence while the note is held. */
    int win_start = (int)note_on_sample + 50;
    int win_len = 2000;
    bool any_nonzero = false;
    for (int i = win_start; i < win_start + win_len && i < total; i++) {
        if (buf[i] != 0) {
            any_nonzero = true;
            break;
        }
    }
    check(t, any_nonzero, "a held OPL note should produce non-silent output");

    /* Decay to (near-)silence within ~700ms of key-off: scan 50ms windows
     * starting at key-off, looking for the first one that stays quiet. */
    int decay_deadline = (int)note_off_sample + (TEST_SAMPLE_RATE * 7) / 10;
    if (decay_deadline > total)
        decay_deadline = total;
    int quiet_win = TEST_SAMPLE_RATE / 20; /* 50ms */
    bool decayed = false;
    for (int start = (int)note_off_sample; start + quiet_win <= total && start <= decay_deadline; start += TEST_SAMPLE_RATE / 100) {
        bool all_quiet = true;
        for (int i = start; i < start + quiet_win; i++) {
            if (buf[i] > 24 || buf[i] < -24) {
                all_quiet = false;
                break;
            }
        }
        if (all_quiet) {
            decayed = true;
            break;
        }
    }
    check(t, decayed, "key-off should decay to near-silence within ~700ms");

    free(buf);
}

int main(void)
{
    test_speaker_synth();
    test_event_scheduling();
    test_opl_backend();

    if (g_failures) {
        fprintf(stderr, "test_audio: %d failure(s)\n", g_failures);
        return 1;
    }
    printf("test_audio: OK\n");
    return 0;
}
