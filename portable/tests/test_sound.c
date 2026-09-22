/* test_sound.c -- unit tests for portable/audio/sound_driver.c, the
 * portable reimplementation of asm/SOUND.ASM (the game-owned sound state
 * machine, Phase 12 part 1).
 *
 * Every expected event below is hand-derived from asm/SOUND.ASM's own
 * semantics (docs/current/sound-state.md, docs/portable/asm-module-
 * inventory.md sec 9) and from the constant game data
 * portable/generated/game_data.c ships (notetab[2]==32508 etc.), NOT from
 * running sound_driver.c and recording its output -- see this file's
 * per-test comments for the arithmetic.
 *
 * Test (a)/(b) build a minimal per-voice command stream in a small local
 * buffer, publish it via sound_set_resource_blocks() as the voice block
 * (what snd_seg2:snd_base2 historically pointed at -- see
 * portable/include/sound.h's header comment), and point
 * voice_stream_cursor_table[0]/voice_stream_base_table[0] at it, exactly
 * the state sound_voice_table_reload() would have computed from a real
 * resource load. The stream is two commands:
 *
 *   byte 0x100: 0x4D 0x05  -- secondary command 0xD, ah=4 ("scale a raw
 *                             0..63 value by 4"): g1788 = 5*4 = 20.
 *   byte 0x102: 0x03 0x80  -- ordinary note command, opcode al=3 (so
 *                             notetab[al-1] == notetab[2] == 32508),
 *                             ah=0 (no octave shift); arg_byte 0x80 has
 *                             bit 0x80 set, so sound_command_value_derive
 *                             returns g1788+g178a == 20 unmodified
 *                             (decode_scaled_value's "direct value"
 *                             branch) -> v_ctr[0] becomes 20.
 *   byte 0x104: 0x0F       -- terminator (unused by tests (a)/(b), which
 *                             stop before the voice reaches it).
 *
 * sound_voice_table_prime() (run automatically by the first
 * sound_tick_entry() call, since snd_mode starts at 1, not 2) primes
 * v_len[0]=6, so the note-off countdown (v_ctr == v_len-1, i.e. 5) lands
 * on tick 15: v_ctr is 20 right after the note command decodes on tick 1
 * and is decremented by one every tick from then on (19, 18, ..., landing
 * on 5 after the 15th decrement, i.e. at the end of tick 15).
 */
#include "game.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

static int g_failures = 0;

static void fail(const char *test, const char *what)
{
    fprintf(stderr, "test_sound: FAIL %s: %s\n", test, what);
    g_failures++;
}

static void check(const char *test, int cond, const char *what)
{
    if (!cond) fail(test, what);
}

/* Local backing storage for sound_resource_block/sound_voice_block
 * (portable/include/sound.h) -- stand-ins for the real resource_ptr/gc5da
 * buffers portable/game/plrldpub.c allocates. 1024 bytes is far more than
 * any test here needs (the real staging block is 0x620 == 1568 bytes). */
static uint8_t s_resource_buf[1024];
static uint8_t s_voice_buf[1024];

/* Reset every DS:175E..1E96 sound-cluster object this driver touches to a
 * known baseline, rewire the resource blocks to freshly-zeroed local
 * buffers, and clear the event log. Tests share one process (ctest links
 * one executable), so each test must start from a clean slate. */
static void reset_sound_state(dos_int backend_mode)
{
    dos_int i;

    snd_on = 0;
    snd_mode = 0;
    snd_hi = -1;
    snd_backend_mode = backend_mode;
    snd_nvoices = 1;
    snd_flag2 = 0;
    sound_enabled = 1;
    music_enabled = 1;
    mus_flag = 0;

    for (i = 0; i < 4; i++) {
        v_a[i] = 0;
        v_b[i] = 0;
        v_ctr[i] = 0;
        v_hold[i] = 0;
        v_len[i] = 0;
        g17a4[i] = 0;
        voice_rest_table[i] = 0;
        voice_stream_cursor_table[i] = 0;
        voice_stream_base_table[i] = 0;
    }
    memset(sound_region_17C4, 0, sizeof(sound_region_17C4));
    memset(g17f4, 0, sizeof(g17f4));

    g1788 = 0;
    g178a = 0;

    mus_ptr = 0;
    mus_arg = -1;
    snd_len = 0;
    snd_delay = 0;
    stream_note_delay = 0;
    snd_one = 0;
    g1e84 = 0;
    g1e86 = 0;
    memset(g1e8c, 0, sizeof(g1e8c));

    /* snd_base/snd_seg/snd_base2/snd_seg2 are left at their generated
       zero default throughout -- portable/game/plrldpub.c no longer
       writes them either (see that file and sound.h's header comment);
       the driver addresses sound_resource_block/sound_voice_block
       directly instead. */
    memset(s_resource_buf, 0, sizeof(s_resource_buf));
    memset(s_voice_buf, 0, sizeof(s_voice_buf));
    sound_set_resource_blocks(s_resource_buf, s_voice_buf);

    /* OPLVOICE.C bank state (only relevant to the backend-mode-2 test,
       harmless to reset unconditionally). voice_bank_retune_on() (via
       voice_set_frequency(), oplreg.c) indexes gca24[]/tab_ix[]/tab_oct[]
       -- BSS pointer/index tables that stay NULL/zero until something
       primes them; music_reset_tuning_tables() (src/MUSIC.C, normally
       run once from opl_init() at startup) is that real prerequisite, so
       this is not a test-only workaround. */
    memset(en, 0, sizeof(en)); /* dos_char en[9] */
    music_reset_tuning_tables();

    sound_event_log_clear();
}

/* Writes the two-command synthetic voice stream described in this file's
 * header comment into the voice block at offset 0x100, and arms voice 0's
 * cursor at it (as sound_voice_table_reload() would have computed). */
static void arm_synthetic_voice_stream(void)
{
    s_voice_buf[0x100] = 0x4D;
    s_voice_buf[0x101] = 0x05;
    s_voice_buf[0x102] = 0x03;
    s_voice_buf[0x103] = 0x80;
    s_voice_buf[0x104] = 0x0F;

    voice_stream_cursor_table[0] = 0x100;
    voice_stream_base_table[0] = 0x100;
    snd_mode = 1; /* "reloaded, not yet primed" -- matches sound_voice_table_reload()'s exit state */
}

/* ---------------------------------------------------------------------
 * (a) backend mode 0 (PC speaker): expect PIT divisor 32508 (notetab[2])
 * and a speaker-gate-on event on tick 1, then a speaker-gate-off event
 * on tick 15 when the note's hold counter reaches v_len-1 -- see this
 * file's header comment for the full arithmetic.
 * --------------------------------------------------------------------- */
static void test_backend_mode0_pit_and_gate(void)
{
    const char *t = "backend_mode0_pit_and_gate";
    dos_int tick;

    reset_sound_state(0);
    arm_synthetic_voice_stream();

    for (tick = 1; tick <= 15; tick++)
        sound_tick_entry();

    check(t, sound_event_log_count() == 3, "expected exactly 3 backend events over 15 ticks");
    if (sound_event_log_count() == 3) {
        const struct sound_event *e0 = sound_event_log_get(0);
        const struct sound_event *e1 = sound_event_log_get(1);
        const struct sound_event *e2 = sound_event_log_get(2);

        check(t, e0->kind == SOUND_EVENT_PIT_DIVISOR && e0->a == 32508 && e0->tick == 1,
              "event0 should be PIT divisor 32508 (notetab[2]) on tick 1");
        check(t, e1->kind == SOUND_EVENT_SPEAKER_GATE && e1->a == 1 && e1->b == 0 && e1->tick == 1,
              "event1 should be speaker-gate ON (plain PC-speaker semantics) on tick 1");
        check(t, e2->kind == SOUND_EVENT_SPEAKER_GATE && e2->a == 0 && e2->b == 0 && e2->tick == 15,
              "event2 should be speaker-gate OFF on tick 15 (v_ctr reaches v_len-1 == 5)");
    }

    check(t, v_ctr[0] == 5, "v_ctr[0] should have counted down to v_len[0]-1 == 5 after 15 ticks");
    check(t, v_a[0] != 0, "voice 0 should still be armed (only the terminator clears v_a)");
}

/* ---------------------------------------------------------------------
 * (b) backend mode 2 (queued OPL bank): the SAME synthetic stream must
 * produce OPL register events instead of PIT-divisor/speaker-gate
 * events. sound_control_value_select's mode-2 branch computes
 * result = ((al-1) + (ah*12)) & 0xFF = ((3-1) + 0) & 0xFF = 2, stores it
 * via sound_pit_divisor_program (mode 2: no hardware event, just queues
 * voice_retune_base_table[0]=2), then voice_enable (mode 2) calls
 * OPLVOICE.C's voice_bank_retune_on(bank=0, base=2). With en[0]=1 (set
 * here) and en[1]=en[2]=0, that runs exactly:
 *   voice_key_off(0):        opl_write(0xB0, 0); opl_write(0xA0, 0);
 *   voice_set_frequency(0,..): opl_write(0xA0, <f-number lo>);
 *                              opl_write(0xB0, <f-number hi | block>);
 * -- the first two writes' register AND value are pinned exactly
 * (voice_key_off always zeroes both registers, unconditionally); the
 * last two writes' register numbers are pinned (voice_set_frequency
 * always targets v+0xA0 then v+0xB0) but their values depend on
 * OPLREG.C's note/frequency tables, out of SOUND.ASM's own scope, so
 * this test does not re-derive them.
 * --------------------------------------------------------------------- */
static void test_backend_mode2_opl_events(void)
{
    const char *t = "backend_mode2_opl_events";
    dos_int tick;

    reset_sound_state(2);
    arm_synthetic_voice_stream();
    en[0] = 1; /* OPLVOICE.C: enable bank-0 voice 0 so voice_bank_retune_on acts */

    for (tick = 1; tick <= 3; tick++)
        sound_tick_entry();

    check(t, sound_event_log_count() == 4, "expected exactly 4 OPL register events (voice_key_off + voice_set_frequency)");
    if (sound_event_log_count() == 4) {
        const struct sound_event *e0 = sound_event_log_get(0);
        const struct sound_event *e1 = sound_event_log_get(1);
        const struct sound_event *e2 = sound_event_log_get(2);
        const struct sound_event *e3 = sound_event_log_get(3);

        check(t, e0->kind == SOUND_EVENT_OPL_WRITE && e0->a == 0xB0 && e0->b == 0 && e0->tick == 1,
              "event0 should be voice_key_off's first write: reg 0xB0 <- 0");
        check(t, e1->kind == SOUND_EVENT_OPL_WRITE && e1->a == 0xA0 && e1->b == 0 && e1->tick == 1,
              "event1 should be voice_key_off's second write: reg 0xA0 <- 0");
        check(t, e2->kind == SOUND_EVENT_OPL_WRITE && e2->a == 0xA0 && e2->tick == 1,
              "event2 should be voice_set_frequency's f-number write: reg 0xA0");
        check(t, e3->kind == SOUND_EVENT_OPL_WRITE && e3->a == 0xB0 && e3->tick == 1,
              "event3 should be voice_set_frequency's block/key write: reg 0xB0");
    }

}

/* ---------------------------------------------------------------------
 * (c) sound_stop_reset: clears the armed state and gates off. Per
 * F_CB48: snd_on <- 0, mus_arg <- -1 (invalidate the cached stream
 * index/priority key), then an unconditional speaker_gate_off() call --
 * ALWAYS a plain (tandy_mode 0) gate-off event, regardless of
 * snd_backend_mode (the single-stream cluster's speaker calls are never
 * backend-mode-gated -- see sound_driver.c's note above sound_note_
 * dispatch).
 * --------------------------------------------------------------------- */
static void test_sound_stop_reset(void)
{
    const char *t = "sound_stop_reset";

    reset_sound_state(2); /* pick a non-zero backend mode to prove the gate-off is unconditional */
    snd_on = 5;
    mus_arg = 7;

    sound_stop_reset();

    check(t, snd_on == 0, "sound_stop_reset should clear snd_on");
    check(t, mus_arg == -1, "sound_stop_reset should invalidate mus_arg back to -1");
    check(t, sound_event_log_count() == 1, "sound_stop_reset should emit exactly one backend event");
    if (sound_event_log_count() == 1) {
        const struct sound_event *e0 = sound_event_log_get(0);
        check(t, e0->kind == SOUND_EVENT_SPEAKER_GATE && e0->a == 0 && e0->b == 0,
              "sound_stop_reset's event should be an unconditional (tandy_mode 0) speaker-gate-off");
    }
}

/* ---------------------------------------------------------------------
 * Bonus: opl_write()/opl_register_write() (task requirement 3) must
 * route through the same sound_backend_opl_write() hook, so ONE log
 * captures OPL traffic from both entry points.
 * --------------------------------------------------------------------- */
static void test_opl_write_shared_log(void)
{
    const char *t = "opl_write_shared_log";

    reset_sound_state(0);

    opl_write(5, 10);
    opl_register_write(6, 11); /* the historical *timed* entry point -- settling reads dropped, forwards to opl_write() */

    check(t, sound_event_log_count() == 2, "opl_write and opl_register_write should share one event log");
    if (sound_event_log_count() == 2) {
        const struct sound_event *e0 = sound_event_log_get(0);
        const struct sound_event *e1 = sound_event_log_get(1);
        check(t, e0->kind == SOUND_EVENT_OPL_WRITE && e0->a == 5 && e0->b == 10, "opl_write(5,10) event");
        check(t, e1->kind == SOUND_EVENT_OPL_WRITE && e1->a == 6 && e1->b == 11, "opl_register_write(6,11) event");
    }

    check(t, opl_detect() == 1, "opl_detect() should always report 1 (no real hardware to probe)");
}

/* ---------------------------------------------------------------------
 * Regression test for the intro freeze: sound_tick_entry() must return
 * promptly even when nothing is armed, matching the exact global state
 * observed at the freeze site (via EMPIRES_TRACE instrumentation):
 *   sound_enabled=1, music_enabled=1 (the slot's "on" value),
 *   snd_on=0 (sound_start()'s embedded sound_stop_reset() call clears
 *     it), mus_flag=0, snd_flag2=1 (the intro sets it directly, per
 *     sound-state.md's DS:1776 caveat: several non-sound C call sites
 *     touch this address too), snd_backend_mode=2, snd_nvoices=4 (a
 *     real sound_backend_select_init() already ran), snd_mode=1 (a real
 *     sound_voice_table_reload() already ran, priming has not happened
 *     yet this tick) -- and, critically, "before any stream is armed":
 *     sound_resource_block/sound_voice_block still NULL (no
 *     player_record_load_publish() has wired them in yet).
 *
 * Root cause (found via the exact trace above): sound_command_stream_
 * dispatch() was reading command bytes from a stand-in arena that was
 * never connected to the real resource data (the old "synthetic 64KB
 * arena" design this driver no longer uses -- see sound.h/plrldpub.c);
 * every byte in it read as 0x00 ("note off, derive a zero-duration
 * value"), which is a legal command that never terminates and never
 * produces a nonzero v_ctr, so the per-voice dispatch loop advanced its
 * cursor forever. Two changes fix it: (1) sound_resource_block/
 * sound_voice_block are real pointers now, so once a real loader wires
 * them in the data is genuine, terminator-bearing resource content, not
 * a disconnected stand-in; (2) a NULL block (this test's exact "nothing
 * loaded yet" state) now reads as 0xFF -- the same sentinel
 * sound_voice_table_prime()'s own "is this voice armed" check already
 * uses -- so priming leaves v_a at 0 for every voice instead of wrongly
 * arming one against garbage data. SOUND_LOOP_GUARD_MAX
 * (sound_driver_internal.h) additionally caps every dispatch loop so a
 * still-degenerate state (e.g. snd_flag2 staying pinned nonzero, which
 * asm/SOUND.ASM itself has no internal way to clear -- see
 * sound_voice_pump_loop's port comment) can only cost a bounded, small
 * amount of work per tick instead of reading as a freeze. */
static void test_unarmed_tick_returns_promptly(void)
{
    const char *t = "unarmed_tick_returns_promptly";
    clock_t start;
    double elapsed_s;
    int tick;

    reset_sound_state(2);
    sound_set_resource_blocks(NULL, NULL); /* "before any stream is armed" -- nothing loaded yet */

    sound_enabled = 1;
    music_enabled = 1; /* stands in for "whatever the slot table set" */
    snd_on = 0;
    mus_flag = 0;
    snd_flag2 = 1;    /* the intro's direct write, per sound-state.md's DS:1776 caveat */
    snd_nvoices = 4;  /* sound_backend_select_init() already ran for backend mode 2 */
    snd_mode = 1;     /* sound_voice_table_reload() already ran; not yet (re)primed this tick */

    start = clock();
    for (tick = 0; tick < 2000; tick++) /* ~8.4s of real 236.7 Hz ticks */
        sound_tick_entry();
    elapsed_s = (double)(clock() - start) / (double)CLOCKS_PER_SEC;

    check(t, elapsed_s < 2.0, "2000 unarmed ticks should complete in well under 2s of CPU time, not spin");
    check(t, !(v_a[0] || v_a[1] || v_a[2] || v_a[3]),
          "no voice should ever get armed against a NULL (nothing-loaded) block");
    check(t, sound_event_log_count() == 0, "an unarmed tick should never emit a single backend event");
}

int main(void)
{
    test_backend_mode0_pit_and_gate();
    test_backend_mode2_opl_events();
    test_sound_stop_reset();
    test_opl_write_shared_log();
    test_unarmed_tick_returns_promptly();

    if (g_failures) {
        fprintf(stderr, "test_sound: %d failure(s)\n", g_failures);
        return 1;
    }
    printf("test_sound: OK\n");
    return 0;
}
