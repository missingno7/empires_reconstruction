/* sound_driver.c -- portable reimplementation of asm/SOUND.ASM (2492
 * bytes, 41 routines): the game-owned sound state machine.
 *
 * Phase 12 part 1 (docs/portable/architecture.md "Audio model"). Every
 * routine is translated for MEANING, not instruction-for-instruction: no
 * register variables, no segment arithmetic, SI (voice index x2 in the
 * original register ABI) becomes a plain `dos_int voice` (0..3) parameter,
 * ES:DI (the live command-stream far pointer) becomes an explicit 16-bit
 * byte offset into one of the two real blocks sound.h declares
 * (sound_resource_block / sound_voice_block -- the actual buffers
 * portable/game/plrldpub.c allocates and portable/game/rescache.c fills;
 * see sound.h's header comment). Every observable effect -- the bytes
 * written to the DGROUP tables, the order and arguments of the backend
 * event calls, 16-bit wraparound, signed vs unsigned compares -- is
 * reproduced exactly; see this file's per-routine comments for the ASM
 * evidence, and the port report (delivered alongside this change) for
 * every place the ASM's own semantics were ambiguous and the decision
 * made here.
 *
 * asm/SOUND.ASM's public routines are the ONLY 8 with a src/*.C caller
 * (docs/portable/asm-module-inventory.md sec 9 "C-facing entry points");
 * the other 33 are internal register-ABI helpers with no C caller and
 * become `static` functions here, in the same order/grouping the ASM
 * file's own "former module" banners use.
 */
#include "game.h"
#include "sound_driver_internal.h"
#include "trace.h" /* a defensive iteration cap tripping is worth a trace line -- see the two "GUARD HIT" call sites below */
#include "audio.h" /* Milestone F: the four sound_backend_* hooks below also
                     * forward to the PCM mixer -- see their bodies and
                     * audio.h's header comment for the timeline scheme. */

/* ===========================================================================
 * Command-stream memory (sound.h) and event log
 * =========================================================================== */

/* Real pointers to the historical far-pointer targets (sound.h's header
 * comment has the full story): sound_resource_block is what snd_seg:
 * snd_base pointed at (the loaded "record 0x41" sound resource,
 * portable/game/plrldpub.c's resource_ptr), sound_voice_block is what
 * snd_seg2:snd_base2 pointed at (gc5da, the 0x620-byte staging block
 * portable/game/rescache.c fills per voice table before every
 * sound_voice_table_reload() call). Both start NULL -- see
 * resource_byte()/voice_byte() below for why that is safe, not a crash
 * risk. */
uint8_t *sound_resource_block;
uint8_t *sound_voice_block;

void sound_set_resource_blocks(uint8_t *resource, uint8_t *staging)
{
    sound_resource_block = resource;
    sound_voice_block = staging;
}

/* Every ES:DI-style byte/word read in this file goes through one of these
 * four helpers instead of indexing sound_resource_block/sound_voice_block
 * directly. A NULL block (no resource loaded yet -- the exact state a
 * fresh boot, or a tick serviced before player_record_load_publish() has
 * ever run, is in) reads as 0xFF: the SAME sentinel byte
 * sound_voice_table_prime()'s "is this entry the 0xFF end marker"
 * check and every command-stream dispatcher's "0xF = terminator" opcode
 * already use, so "nothing loaded" behaves exactly like "an
 * already-exhausted stream" -- inert, not a crash, not a spin. See
 * portable/tests/test_sound.c's "unarmed_tick_returns_promptly" test. */
static uint8_t resource_byte(uint16_t off)
{
    return sound_resource_block ? sound_resource_block[off] : 0xFF;
}

static uint8_t voice_byte(uint16_t off)
{
    return sound_voice_block ? sound_voice_block[off] : 0xFF;
}

static uint16_t resource_word(uint16_t off)
{
    return (uint16_t)(resource_byte(off) | ((uint16_t)resource_byte((uint16_t)(off + 1)) << 8));
}

static uint16_t voice_word(uint16_t off)
{
    return (uint16_t)(voice_byte(off) | ((uint16_t)voice_byte((uint16_t)(off + 1)) << 8));
}

#define SOUND_EVENT_LOG_CAPACITY 4096u

static struct sound_event g_event_log[SOUND_EVENT_LOG_CAPACITY];
static size_t g_event_log_total;   /* total pushes ever (saturating index math below) */
static uint32_t g_tick;            /* count of sound_tick_entry() calls so far */

static void event_log_push(int kind, uint16_t a, uint16_t b)
{
    struct sound_event *slot = &g_event_log[g_event_log_total % SOUND_EVENT_LOG_CAPACITY];
    slot->tick = g_tick;
    slot->kind = kind;
    slot->a = a;
    slot->b = b;
    g_event_log_total++;
}

void sound_event_log_clear(void)
{
    g_event_log_total = 0;
    g_tick = 0;
}

size_t sound_event_log_count(void)
{
    return g_event_log_total < SOUND_EVENT_LOG_CAPACITY ? g_event_log_total : SOUND_EVENT_LOG_CAPACITY;
}

const struct sound_event *sound_event_log_get(size_t index)
{
    size_t count = sound_event_log_count();
    size_t base = (g_event_log_total > SOUND_EVENT_LOG_CAPACITY) ? (g_event_log_total - count) : 0;

    if (index >= count)
        return NULL;
    return &g_event_log[(base + index) % SOUND_EVENT_LOG_CAPACITY];
}

/* ---- the four backend hooks (sound.h) ----
 *
 * Each hook does two things: (1) the original default impl -- append to
 * the in-memory event log portable/tests/test_sound.c inspects (tick =
 * g_tick, this file's own "calls to sound_tick_entry() so far" counter);
 * (2) Milestone F addition -- forward the same event to the PCM mixer
 * (portable/audio/audio_mixer.c) via audio_mixer_push_event(), tagged
 * with the CURRENT timer_ticks (portable/game/timer.c increments
 * timer_ticks before it ever calls sound_tick_entry(), so this is exactly
 * the tick this hardware write belongs to -- see audio.h's header
 * comment for the full timeline scheme). AUDIO_EVENT_* (audio.h) and
 * enum sound_event_kind (sound.h) are numerically identical by contract
 * (both 0=OPL write, 1=PIT divisor, 2=speaker gate, 3=nibble write), so
 * the kind value is passed straight through. */

void sound_backend_opl_write(uint8_t reg, uint8_t val)
{
    event_log_push(SOUND_EVENT_OPL_WRITE, reg, val);
    audio_mixer_push_event(SOUND_EVENT_OPL_WRITE, (uint32_t)timer_ticks, reg, val);
}

void sound_backend_pit_divisor(uint16_t divisor)
{
    event_log_push(SOUND_EVENT_PIT_DIVISOR, divisor, 0);
    audio_mixer_push_event(SOUND_EVENT_PIT_DIVISOR, (uint32_t)timer_ticks, divisor, 0);
}

void sound_backend_speaker_gate(int enabled, int tandy_mode)
{
    event_log_push(SOUND_EVENT_SPEAKER_GATE, (uint16_t)(enabled != 0), (uint16_t)(tandy_mode != 0));
    audio_mixer_push_event(SOUND_EVENT_SPEAKER_GATE, (uint32_t)timer_ticks,
                            (uint16_t)(enabled != 0), (uint16_t)(tandy_mode != 0));
}

void sound_backend_nibble_port_write(uint8_t value)
{
    event_log_push(SOUND_EVENT_NIBBLE_WRITE, value, 0);
    audio_mixer_push_event(SOUND_EVENT_NIBBLE_WRITE, (uint32_t)timer_ticks, value, 0);
}

/* opl_write() (sound.h): the new backend primitive src/OPLREG.C's ported
 * callers (oplreg.c, oplinit.c, oplvoice.c, voxchan.c) already call
 * instead of the historical port-I/O opl_register_write(). Routes through
 * the SAME event hook opl_register_write() below uses, so one log
 * captures every OPL write regardless of which C entry point produced
 * it (task requirement 3). */
void opl_write(dos_uint reg, dos_uint val)
{
    sound_backend_opl_write((uint8_t)reg, (uint8_t)val);
}

dos_int opl_detect(void)
{
    return 1;
}

/* ===========================================================================
 * forward declarations of the 33 internal (no C caller) routines, grouped
 * exactly as asm/SOUND.ASM's own "former module" banners group them.
 * =========================================================================== */

static void sound_voice_pump_loop(void);
static void sound_voice_table_prime(void);
static void sound_voice_service_loop(void);
static void sound_command_stream_dispatch(dos_int voice);
static void sound_control_value_select(dos_int voice, dos_int al, dos_int ah);
static dos_int sound_command_value_derive(dos_int voice, dos_int arg_byte);
static void sound_control_block_advance(void);
static void sound_secondary_cmd_dispatch(dos_int voice, dos_int ah, dos_int al);
static void sound_command_flags_update(dos_int voice, dos_int al);
static void voice_percent_scale_store(dos_int al, dos_int ah);
static void sound_param_scale4(dos_int al);
static void f_c5a8(dos_int voice, dos_int al);
static void sound_table_word_select_store(dos_int voice, dos_int al);
static void f_c5c6(dos_int voice, dos_int al);
static void voice_command_decode_apply(dos_int voice, dos_int arg_byte, dos_int ah);
static void voice_enable(dos_int voice);
static void voice_disable(dos_int voice);
static void sound_pit_divisor_program(dos_int voice, dos_int ax);
static void sound_voices_reset_and_service(void);
static void opl_port_write_byte(dos_int al);
static void sound_tick_step(void);
static void sound_stream_command_step(void);
static void sound_note_dispatch(dos_int note_al, dos_int shift_ah);
static void sound_stream_delay_decode(dos_int arg_byte);
static void sound_ctlblock_command_dispatch(dos_int ah, dos_int al);
static void sound_ctlblock_flags_latch(dos_int al);
static void stream_percent_scale_store(dos_int al, dos_int ah);
static void stream_base_value_set(dos_int al);
static void stream_note_delay_set(dos_int al);
static void stream_note_program(dos_int arg_byte, dos_int ah);
static void speaker_gate_on(void);
static void speaker_gate_off(void);
static void pit_channel2_set_divisor(dos_int ax);

/* Per-voice queued PIT-divisor/tone value awaiting delivery to the 9-voice
 * OPL bank (voice_bank_retune_on). DS:1766 in the ASM; sound-state.md
 * confirms no data public exists at that address (never promoted past an
 * ASM-internal equate), so it stays driver-owned storage here rather than
 * a generated object. */
static dos_int voice_retune_base_table[SOUND_VOICE_COUNT_MAX];

/* ===========================================================================
 * Differential-oracle snapshot/restore (sound.h's header comment has the
 * full contract). SOUND_BASE is DS:175E, the buffer's own byte 0;
 * SOUND_OFF(addr) turns a historical DS address from docs/current/
 * sound-state.md's field map into a buffer offset. Every field below is
 * listed in the exact order that field map uses. Kept in one file (not a
 * generic loop over a table) because the per-field width/array-count
 * varies and this reads directly against the field map while auditing.
 * =========================================================================== */

#define SOUND_SNAPSHOT_BASE 0x175EU
#define SOUND_OFF(addr) ((size_t)((addr) - SOUND_SNAPSHOT_BASE))
#define SOUND_REQUEST_COUNT_OFF ((size_t)(SOUND_DRIVER_SNAPSHOT_SIZE - 2))

static void snap_w(uint8_t *img, unsigned addr, dos_int value)
{
    dos_wr16(&img[SOUND_OFF(addr)], (uint16_t)value);
}

static dos_int snap_r(const uint8_t *img, unsigned addr)
{
    return (dos_int)dos_rd16(&img[SOUND_OFF(addr)]);
}

static void snap_w_arr(uint8_t *img, unsigned addr, const dos_int *arr, int n)
{
    int i;
    for (i = 0; i < n; i++)
        dos_wr16(&img[SOUND_OFF(addr) + (size_t)i * 2], (uint16_t)arr[i]);
}

static void snap_r_arr(const uint8_t *img, unsigned addr, dos_int *arr, int n)
{
    int i;
    for (i = 0; i < n; i++)
        arr[i] = (dos_int)dos_rd16(&img[SOUND_OFF(addr) + (size_t)i * 2]);
}

void sound_driver_snapshot(uint8_t *dgroup_image)
{
    memset(dgroup_image, 0, SOUND_DRIVER_SNAPSHOT_SIZE);

    snap_w(dgroup_image, 0x175E, snd_base);
    snap_w(dgroup_image, 0x1760, snd_seg);
    snap_w(dgroup_image, 0x1762, snd_base2);
    snap_w(dgroup_image, 0x1764, snd_seg2);
    snap_w_arr(dgroup_image, 0x1766, voice_retune_base_table, SOUND_VOICE_COUNT_MAX);
    snap_w(dgroup_image, 0x176E, sound_enabled);
    snap_w(dgroup_image, 0x1770, snd_on);
    snap_w(dgroup_image, 0x1772, music_enabled);
    snap_w(dgroup_image, 0x1774, mus_flag);
    snap_w(dgroup_image, 0x1776, snd_flag2);
    snap_w(dgroup_image, 0x1778, snd_backend_mode);
    snap_w(dgroup_image, 0x177A, snd_nvoices);
    snap_w_arr(dgroup_image, 0x177C, v_b, SOUND_VOICE_COUNT_MAX);
    snap_w(dgroup_image, 0x1784, snd_mode);
    snap_w(dgroup_image, 0x1786, snd_hi);
    snap_w(dgroup_image, 0x1788, g1788);
    snap_w(dgroup_image, 0x178A, g178a);
    snap_w_arr(dgroup_image, 0x178C, voice_stream_cursor_table, SOUND_VOICE_COUNT_MAX);
    snap_w_arr(dgroup_image, 0x1794, voice_stream_base_table, SOUND_VOICE_COUNT_MAX);
    snap_w_arr(dgroup_image, 0x179C, v_ctr, SOUND_VOICE_COUNT_MAX);
    snap_w_arr(dgroup_image, 0x17A4, g17a4, SOUND_VOICE_COUNT_MAX);
    snap_w_arr(dgroup_image, 0x17AC, v_a, SOUND_VOICE_COUNT_MAX);
    snap_w_arr(dgroup_image, 0x17B4, v_hold, SOUND_VOICE_COUNT_MAX);
    snap_w_arr(dgroup_image, 0x17BC, v_len, SOUND_VOICE_COUNT_MAX);
    memcpy(&dgroup_image[SOUND_OFF(0x17C4)], sound_region_17C4, sizeof sound_region_17C4);
    snap_w_arr(dgroup_image, 0x17EC, voice_rest_table, SOUND_VOICE_COUNT_MAX);
    memcpy(&dgroup_image[SOUND_OFF(0x17F4)], g17f4, sizeof g17f4);
    {
        int i;
        for (i = 0; i < 12; i++)
            dos_wr16(&dgroup_image[SOUND_OFF(0x17FC) + (size_t)i * 2], (uint16_t)notetab[i]);
    }
    memcpy(&dgroup_image[SOUND_OFF(0x1814)], note_divisors_octave, sizeof note_divisors_octave);
    /* 0x182C (sound_dispatch_182C) and 0x1832 (sound_dispatch_1832): real C
     * pointer arrays, not byte-comparable with the historical raw words --
     * left zero (sound.h's header comment). */
    snap_w(dgroup_image, 0x1830, (dos_int)opl_port);
    /* 0x187A..0x1E84: unrelated other-subsystem DGROUP state, left zero. */
    snap_w(dgroup_image, 0x1E84, g1e84);
    snap_w(dgroup_image, 0x1E86, g1e86);
    snap_w(dgroup_image, 0x1E88, mus_ptr);
    snap_w(dgroup_image, 0x1E8A, mus_arg);
    memcpy(&dgroup_image[SOUND_OFF(0x1E8C)], g1e8c, sizeof g1e8c);
    snap_w(dgroup_image, 0x1E8E, snd_len);
    snap_w(dgroup_image, 0x1E90, snd_delay);
    snap_w(dgroup_image, 0x1E92, stream_note_delay);
    snap_w(dgroup_image, 0x1E94, snd_one);

    dos_wr16(&dgroup_image[SOUND_REQUEST_COUNT_OFF], (uint16_t)sound_request_count);
}

void sound_driver_restore(const uint8_t *dgroup_image)
{
    snd_base = (dos_uint)snap_r(dgroup_image, 0x175E);
    snd_seg = (dos_uint)snap_r(dgroup_image, 0x1760);
    snd_base2 = (dos_uint)snap_r(dgroup_image, 0x1762);
    snd_seg2 = (dos_uint)snap_r(dgroup_image, 0x1764);
    snap_r_arr(dgroup_image, 0x1766, voice_retune_base_table, SOUND_VOICE_COUNT_MAX);
    sound_enabled = snap_r(dgroup_image, 0x176E);
    snd_on = snap_r(dgroup_image, 0x1770);
    music_enabled = snap_r(dgroup_image, 0x1772);
    mus_flag = snap_r(dgroup_image, 0x1774);
    snd_flag2 = snap_r(dgroup_image, 0x1776);
    snd_backend_mode = snap_r(dgroup_image, 0x1778);
    snd_nvoices = snap_r(dgroup_image, 0x177A);
    snap_r_arr(dgroup_image, 0x177C, v_b, SOUND_VOICE_COUNT_MAX);
    snd_mode = snap_r(dgroup_image, 0x1784);
    snd_hi = snap_r(dgroup_image, 0x1786);
    g1788 = snap_r(dgroup_image, 0x1788);
    g178a = snap_r(dgroup_image, 0x178A);
    snap_r_arr(dgroup_image, 0x178C, voice_stream_cursor_table, SOUND_VOICE_COUNT_MAX);
    snap_r_arr(dgroup_image, 0x1794, voice_stream_base_table, SOUND_VOICE_COUNT_MAX);
    snap_r_arr(dgroup_image, 0x179C, v_ctr, SOUND_VOICE_COUNT_MAX);
    snap_r_arr(dgroup_image, 0x17A4, g17a4, SOUND_VOICE_COUNT_MAX);
    snap_r_arr(dgroup_image, 0x17AC, v_a, SOUND_VOICE_COUNT_MAX);
    snap_r_arr(dgroup_image, 0x17B4, v_hold, SOUND_VOICE_COUNT_MAX);
    snap_r_arr(dgroup_image, 0x17BC, v_len, SOUND_VOICE_COUNT_MAX);
    memcpy(sound_region_17C4, &dgroup_image[SOUND_OFF(0x17C4)], sizeof sound_region_17C4);
    snap_r_arr(dgroup_image, 0x17EC, voice_rest_table, SOUND_VOICE_COUNT_MAX);
    memcpy(g17f4, &dgroup_image[SOUND_OFF(0x17F4)], sizeof g17f4);
    /* notetab/note_divisors_octave: compile-time constants, never restored
     * from a scenario fixture (the fixture's own copy is only there so the
     * oracle's byte-for-byte snapshot has something to compare against a
     * known-constant span; see gen_scenarios.py). sound_dispatch_182C/1832
     * and the 187A..1E84 gap: never touched, per this file's snapshot
     * comment. */
    opl_port = (dos_uint)snap_r(dgroup_image, 0x1830);
    g1e84 = snap_r(dgroup_image, 0x1E84);
    g1e86 = snap_r(dgroup_image, 0x1E86);
    mus_ptr = snap_r(dgroup_image, 0x1E88);
    mus_arg = snap_r(dgroup_image, 0x1E8A);
    memcpy(g1e8c, &dgroup_image[SOUND_OFF(0x1E8C)], sizeof g1e8c);
    snd_len = snap_r(dgroup_image, 0x1E8E);
    snd_delay = snap_r(dgroup_image, 0x1E90);
    stream_note_delay = snap_r(dgroup_image, 0x1E92);
    snd_one = snap_r(dgroup_image, 0x1E94);

    sound_request_count = (dos_int)dos_rd16(&dgroup_image[SOUND_REQUEST_COUNT_OFF]);
}

/* ===========================================================================
 * former asm/M_C1A0_C232.ASM -- sound-tick entry, voice-pump, voice-table
 * scanner.
 * =========================================================================== */

/* F_C1A0 -- per-tick service entry. Callers: src/TIMER.C (timer.c's
 * timer_service_tick, under the historical
 * `!sound_request_count && (sound_enabled || music_enabled)` gate). */
void sound_tick_entry(void)
{
    g_tick++;

    v_b[0] = 0;
    v_b[1] = 0;
    v_b[2] = 0;
    v_b[3] = 0;

    if (snd_on != 0) {
        sound_tick_step();
        if (snd_mode == 0)
            return;
        /* ASM: `cmp snd_backend_mode,0 / jne tick_l1` -- v_b[0]=1 only for
           backend mode 0 (PC speaker); every other mode falls straight
           through to the pump call below untouched. */
        if (snd_backend_mode == 0)
            v_b[0] = 1;
        sound_voice_pump_loop();
    } else if (snd_mode != 0) {
        sound_voice_pump_loop();
    }
}

/* F_C1F7 -- pump loop: rescan the voice table in mode 2, dispatch one
 * command batch per voice, then optionally rewind every cursor back to
 * its reload baseline and re-prime (mode 0 + snd_flag2 set). ASM caveat:
 * nothing in this loop or its callees clears snd_flag2 or forces
 * snd_mode away from 0, so a persistent snd_flag2==1 while snd_mode==0
 * (e.g. no voice's command-stream entry ever arms, per
 * sound_voice_table_prime's 0xFF check) genuinely re-loops in the
 * original binary too (an external `g1776`/`g1784` write is what
 * normally breaks the cycle -- see sound-state.md's DS:1776 caveat).
 * This port keeps that behaviour but adds SOUND_LOOP_GUARD_MAX as a
 * defensive iteration cap (sound_driver_internal.h) so the caller's
 * thread never spins here for long enough to read as a freeze on a
 * degenerate/not-yet-loaded state; see the port report. */
static void sound_voice_pump_loop(void)
{
    int guard;

    if (snd_mode != 2)
        sound_voice_table_prime();

    for (guard = 0; guard < SOUND_LOOP_GUARD_MAX; guard++) {
        sound_voice_service_loop();

        if (snd_mode != 0 || snd_flag2 == 0)
            break;

        {
            dos_int i;
            for (i = 0; i < SOUND_VOICE_COUNT_MAX; i++)
                voice_stream_cursor_table[i] = voice_stream_base_table[i];
        }
        sound_voice_table_prime();
    }
    if (guard >= SOUND_LOOP_GUARD_MAX - 1)
        EMPIRES_TRACE("sound_voice_pump_loop GUARD HIT nvoices=%d mode=%d flag2=%d on=%d nv3=%d cur0=%d",
                      (int)snd_nvoices, (int)snd_mode, (int)snd_flag2, (int)snd_on,
                      (int)v_a[0], (int)voice_stream_cursor_table[0]);
}

/* F_C232 -- voice-table scanner: (re)prime every configured voice's
 * per-voice control slots from the entry at its (already reloaded)
 * command-stream cursor, unless that entry is the 0xFF end marker. */
static void sound_voice_table_prime(void)
{
    dos_int voice;
    dos_int count = snd_nvoices;

    snd_mode = 2;
    g178a = 0;

    if (count > SOUND_VOICE_COUNT_MAX) count = SOUND_VOICE_COUNT_MAX;
    if (count < 0) count = 0;

    for (voice = 0; voice < count; voice++) {
        dos_uint cursor = (dos_uint)voice_stream_cursor_table[voice];

        v_a[voice] = 0;
        if (voice_byte((uint16_t)cursor) != 0xFF) {
            v_a[voice] = 2;
            v_ctr[voice] = 0;
            g17a4[voice] = 1;
            v_hold[voice] = 0;
            voice_rest_table[voice] = 0;
            sound_region_word_set(g17f4, 0, voice, 0);
            v_len[voice] = 6;
        }
    }
}

/* ===========================================================================
 * former asm/M_C27D_C567.ASM -- voice service, command dispatch, control
 * helpers.
 * =========================================================================== */

/* F_C27D -- per-voice service loop. ASM detail: this is a post-test
 * (do-while) loop over SI -- voice 0's body always runs at least once,
 * even when snd_nvoices==0 (the loop bound is only checked after the
 * first iteration). Reproduced faithfully; the SOUND_VOICE_COUNT_MAX
 * clamp below is a defensive bound the ASM's register loop never needed
 * (SI simply kept incrementing), added because C array indexing has no
 * hardware fault to rely on if snd_nvoices were ever corrupted past 4 --
 * every real writer (sound_backend_select_init/sound_voices_reset) only
 * ever sets it to 0, 1 or 4. */
static void sound_voice_service_loop(void)
{
    dos_int voice = 0;

    do {
        if (voice < SOUND_VOICE_COUNT_MAX && v_a[voice] != 0) {
            if (v_ctr[voice] == 0)
                sound_command_stream_dispatch(voice);

            if (v_a[voice] != 0) {
                v_ctr[voice] = dos_sub16(v_ctr[voice], 1);
                if (snd_backend_mode != 1 && v_hold[voice] != 1) {
                    dos_int len_minus_1 = dos_sub16(v_len[voice], 1);
                    if (v_ctr[voice] == len_minus_1)
                        voice_disable(voice);
                }
            }
        }
        voice++;
    } while (voice < snd_nvoices && voice < SOUND_VOICE_COUNT_MAX);

    if (!(v_a[0] || v_a[1] || v_a[2] || v_a[3]))
        sound_voices_reset_and_service();
    else if (snd_backend_mode == 1)
        sound_control_block_advance();
}

/* F_C2EA -- command-stream dispatcher: split the byte at the voice's
 * command-stream cursor into a low-nibble opcode (0/D/E/F get dedicated
 * handlers, anything else falls to control_value_select+command_value_
 * derive), advance the cursor by 2, and keep dispatching while the
 * just-derived v_ctr stays 0 (zero-duration commands execute back to
 * back within the same tick). */
static void sound_command_stream_dispatch(dos_int voice)
{
    int guard;
    for (guard = 0; guard < SOUND_LOOP_GUARD_MAX; guard++) {
        dos_uint cursor = (dos_uint)voice_stream_cursor_table[voice];
        uint8_t cmd_byte = voice_byte((uint16_t)cursor);
        uint8_t arg_byte = voice_byte((uint16_t)(cursor + 1));
        dos_int al = (dos_int)(cmd_byte & 0x0F);
        dos_int ah = (dos_int)(cmd_byte >> 4);

        if (al == 0x0F) { /* C2EA_command_f -- terminator: no cursor advance */
            v_a[voice] = 0;
            voice_disable(voice);
            return;
        }

        if (al == 0) { /* C2EA_command_zero -- explicit note-off */
            voice_disable(voice);
            (void)sound_command_value_derive(voice, (dos_int)arg_byte);
            voice_rest_table[voice] = 1;
        } else if (al == 0x0D) { /* C2EA_command_d -- secondary command */
            sound_secondary_cmd_dispatch(voice, ah, (dos_int)arg_byte);
            /* ASM computes `cmp v_ctr,0` here but the result is never
               branched on (the next instruction is an unconditional jmp
               to the shared advance code) -- dead compare, omitted. */
        } else if (al == 0x0E) { /* C2EA_command_e -- note/frequency decode */
            voice_command_decode_apply(voice, (dos_int)arg_byte, ah);
        } else { /* default -- ordinary note command */
            sound_control_value_select(voice, al, ah);
            (void)sound_command_value_derive(voice, (dos_int)arg_byte);
            voice_rest_table[voice] = 0;
        }

        voice_stream_cursor_table[voice] = dos_add16(voice_stream_cursor_table[voice], 2);
        if (v_ctr[voice] != 0)
            return;
    }
    EMPIRES_TRACE("sound_command_stream_dispatch GUARD HIT voice=%d cur=%d byte0=%02x",
                  (int)voice, (int)voice_stream_cursor_table[voice],
                  (unsigned)voice_byte((uint16_t)voice_stream_cursor_table[voice]));
}

/* F_C359 -- select and submit one value from the sound control state
 * (notetab/voice_table_b lookup or the mode-2 queued-index shape),
 * branching on snd_backend_mode. `al` is the dispatcher's masked opcode
 * (1..0x0C, 0/0xD/0xE/0xF are handled by the caller), `ah` its argument
 * nibble (an octave-shift count in modes 0/1, a bank-index term in mode
 * 2). */
static void sound_control_value_select(dos_int voice, dos_int al, dos_int ah)
{
    if (snd_backend_mode == 1) {
        if (voice == 3) { /* C359_reset_voice: SI==6 was voice index 3 */
            if (v_b[3] == 0) {
                dos_int reg = dos_add16(dos_sub16(al, 1), 0xE0);
                opl_port_write_byte(reg);
            }
            return;
        }
        {
            dos_int shift = (ah == 0) ? (dos_int)0 : dos_sub16(ah, 1);
            dos_int idx = dos_sub16(al, 1);
            dos_uint word;
            dos_int value;

            if (idx < 0) idx = 0;
            if (idx > 11) idx = 11;
            word = dos_rd16(&note_divisors_octave[(int)idx * 2]);
            value = (dos_int)((dos_uint)word >> (shift & 0xFF));
            if (value > 0x3FF)
                value = (dos_int)((dos_uint)value >> 1);
            sound_pit_divisor_program(voice, value);
        }
        return;
    }

    if (snd_backend_mode == 2) {
        dos_int a = dos_sub16(al, 1);
        dos_int term = dos_i16((int32_t)(uint8_t)(ah * 12)); /* ah*8 + ah*4, byte-truncated */
        dos_int queued = dos_i16((int32_t)(uint8_t)dos_add16(a, term));
        sound_pit_divisor_program(voice, queued);
        voice_enable(voice);
        return;
    }

    /* default (backend 0; backend "3" is a transient probe value that
       sound_backend_select_init always collapses back to 1 before any
       command stream can run -- see the port report). */
    {
        dos_int idx = dos_sub16(al, 1);
        dos_uint word;
        dos_int value;

        if (idx < 0) idx = 0;
        if (idx > 11) idx = 11;
        word = notetab[(int)idx];
        value = (dos_int)((dos_uint)word >> (ah & 0xFF));
        sound_pit_divisor_program(voice, value);
        voice_enable(voice);
    }
}

/* Shared 0..3 "progress index" decode: F_C3DB (sound_command_value_derive,
 * per-voice, feeds v_ctr via g17f4) and F_C9A4 (sound_stream_delay_decode,
 * scalar, feeds snd_delay via g1e8c) run the byte-identical algorithm
 * against different state (sound-state.md: "the scalar twin at DS:1E8C
 * runs the byte-identical state machine"). `arg_byte` is the command's
 * raw argument byte (es:[di+1]); `base` is g1788+g178a or g1e84+g1e86;
 * `progress_word` is a 2-byte little-endian word (g17f4[voice*2] or
 * g1e8c). Returns the value to store into v_ctr[voice]/snd_delay.
 *
 * ASM note: when `arg_byte`'s bit 0x80 is set ("direct value"), the ASM
 * computes `al & 0x7F` but the result (in AL/AX) is never read again --
 * the store always uses BX, which in this branch is `base` UNMODIFIED.
 * That dead computation is omitted here. */
static dos_int decode_scaled_value(dos_int arg_byte, dos_int base, uint8_t *progress_word)
{
    dos_int ah = (dos_int)(arg_byte & 0xFF);
    dos_int bx = base;

    if (ah & 0x80)
        return bx;

    {
        int shift = ah & 7;
        bx = (dos_int)((dos_uint)bx >> shift);
        if (ah & 8) {
            dos_int half = (dos_int)((dos_uint)bx >> 1);
            bx = dos_add16(bx, half);
        }
    }
    {
        dos_int idx = (dos_int)dos_rd16(progress_word);

        if (ah & 0x10) {
            if (idx != 3) {
                idx = (idx == 1) ? (dos_int)3 : dos_add16(idx, 1);
                dos_wr16(progress_word, (uint16_t)idx);
            }
        } else if (idx != 0) {
            idx = dos_add16(idx, 1);
            dos_wr16(progress_word, (uint16_t)idx);
        }
    }
    return bx;
}

/* F_C3DB -- derive a command value from the argument byte at
 * es:[di+1], store it into v_ctr, latch voice_pending_table. Returns the
 * stored value (unused by most callers; F_C914's default-note branch in
 * the twin scalar routine reads it, this per-voice one never does, kept
 * for symmetry/testability). */
static dos_int sound_command_value_derive(dos_int voice, dos_int arg_byte)
{
    dos_int bx = decode_scaled_value(arg_byte, dos_add16(g1788, g178a), &g17f4[(int)voice * 2]);

    v_ctr[voice] = bx;
    sound_region_word_set(sound_region_17C4, SOUND_REGION_VOICE_PENDING_OFF, voice, 1);
    return bx;
}

/* Resolve which of the 36 constant OPL-byte-stream blobs
 * (sound_dispatch_1832) an encoded state_cursor/state_cursor_next word
 * currently names, and read one byte at the encoded position -- see
 * sound_driver_internal.h's header comment for why a (table:6,pos:10)
 * encoding replaces the historical raw pointer word. Returns 0xFF (the
 * blobs' own sentinel byte) for an out-of-range index/position so a
 * corrupt cursor behaves like an already-exhausted stream instead of
 * reading out of bounds. */
static uint8_t stream_byte_at(unsigned index, unsigned pos)
{
    size_t count = sizeof(sound_dispatch_1832) / sizeof(sound_dispatch_1832[0]);

    if (index >= count || pos > SOUND_STREAM_POS_MASK)
        return 0xFF;
    return ((const uint8_t *)sound_dispatch_1832[index])[pos];
}

/* F_C440 -- advance the pause-overlay byte-stream renderer for all four
 * voice slots (LOW confidence on ultimate purpose, mechanism fully traced
 * per sound-state.md's DS:17C4 row): only reached from
 * sound_voice_service_loop when EVERY voice is idle and snd_backend_mode
 * ==1. Each slot either (a) resets/advances its byte-stream cursor and
 * renders the next byte, (b) renders a fixed override byte
 * (state_text), (c) renders a blank glyph (0x0F), or (d) renders
 * nothing this tick -- each rendered byte is OR'd with a per-slot OPL
 * register base (0x90/0xB0/0xD0/0xF0) and written via
 * opl_port_write_byte. */
static void sound_control_block_advance(void)
{
    dos_int voice;
    uint8_t reg_base = 0x90;

    for (voice = 0; voice < SOUND_VOICE_COUNT_MAX; voice++, reg_base = (uint8_t)(reg_base + 0x20)) {
        dos_int render_byte = -1; /* -1 = "no render this tick" (C440_slot_done) */
        int go_select_char = 0;

        if (v_b[voice] != 0 || v_a[voice] == 0)
            continue;

        {
            dos_int pending = sound_region_word_get(sound_region_17C4, SOUND_REGION_VOICE_PENDING_OFF, voice);

            if (pending == 1) {
                sound_region_word_set(sound_region_17C4, SOUND_REGION_VOICE_PENDING_OFF, voice, 0);
                {
                    dos_int idx = sound_region_word_get(g17f4, 0, voice);

                    if (idx < 2) {
                        sound_region_word_set(sound_region_17C4, SOUND_REGION_STATE_CURSOR_MODE_OFF, voice, 0);
                        {
                            dos_int cur = sound_region_word_get(sound_region_17C4, SOUND_REGION_STATE_CURSOR_OFF, voice);
                            sound_region_word_set(sound_region_17C4, SOUND_REGION_STATE_CURSOR_NEXT_OFF, voice, cur);
                        }
                        go_select_char = 1;
                    } else if (idx == 3) {
                        go_select_char = 1;
                    } else {
                        sound_region_word_set(g17f4, 0, voice, 0);
                        go_select_char = 1;
                    }
                }
            } else if (voice_rest_table[voice] == 1) {
                render_byte = 0x0F;
            } else {
                dos_int idx = sound_region_word_get(g17f4, 0, voice);

                if (idx == 1 || idx == 3 || v_hold[voice] == 1)
                    go_select_char = 1;
                else if (v_ctr[voice] <= v_len[voice]) /* ASM `jng` -- signed <= */
                    render_byte = 0x0F;
                else
                    go_select_char = 1;
            }
        }

        if (go_select_char) {
            dos_int mode = sound_region_word_get(sound_region_17C4, SOUND_REGION_STATE_CURSOR_MODE_OFF, voice);

            if (mode == 0) {
                dos_int cursor_next = sound_region_word_get(sound_region_17C4, SOUND_REGION_STATE_CURSOR_NEXT_OFF, voice);
                unsigned idxv = sound_stream_cursor_index((uint16_t)cursor_next);
                unsigned posv = sound_stream_cursor_pos((uint16_t)cursor_next);
                uint8_t byte = stream_byte_at(idxv, posv);

                sound_region_word_set(sound_region_17C4, SOUND_REGION_STATE_CURSOR_NEXT_OFF, voice,
                                       (dos_int)sound_stream_cursor_encode(idxv, posv + 1));

                if (byte != 0xFF) {
                    render_byte = (dos_int)byte;
                } else {
                    sound_region_word_set(sound_region_17C4, SOUND_REGION_STATE_CURSOR_MODE_OFF, voice, 1);
                    render_byte = -1;
                }
            } else if (mode > 1) {
                render_byte = 0x0F;
            } else {
                render_byte = sound_region_word_get(sound_region_17C4, SOUND_REGION_STATE_TEXT_OFF, voice);
            }
        }

        if (render_byte >= 0) {
            dos_int reg = dos_add16(render_byte & 0xFF, reg_base);
            opl_port_write_byte(reg);
        }
    }
}

/* F_C501 -- dispatch one secondary command by its AH selector. */
static void sound_secondary_cmd_dispatch(dos_int voice, dos_int ah, dos_int al)
{
    if (ah == 0)
        sound_command_flags_update(voice, al);
    else if (ah <= 2)
        voice_percent_scale_store(al, ah);
    else if (ah == 3)
        f_c5a8(voice, al);
    else if (ah == 4)
        sound_param_scale4(al);
    else if (ah == 5)
        sound_table_word_select_store(voice, al);
    else if (ah == 6)
        f_c5c6(voice, al);
}

/* F_C549 -- update the paired command-control flags (v_hold/v_len) from
 * AL: al==0 sets the hold flag; anything else clears it and latches the
 * new note length. */
static void sound_command_flags_update(dos_int voice, dos_int al)
{
    if (al == 0) {
        v_hold[voice] = 1;
    } else {
        v_hold[voice] = 0;
        v_len[voice] = (dos_int)((dos_uint)al & 0xFF);
    }
}

/* Shared signed percentage scale-and-store: F_C567 (voice_percent_scale_
 * store, the 1788/178A pair) and F_CA51 (stream_percent_scale_store, the
 * 1E84/1E86 pair) are byte-identical algorithms over different state.
 * `al` is the raw 0..99-ish percent, `ah` selects the sign (1 = keep
 * positive, anything else negates). The historical `mul`/`div` are
 * unsigned 16x16->32-bit; done here in a wider type so the C port never
 * needs to reproduce the 8086's divide-overflow fault (only reachable
 * with out-of-range game data that was never authored -- see port
 * report). */
static void percent_scale_store(dos_int al, dos_int ah, dos_int base_val, dos_int *out)
{
    dos_uint pct = (dos_uint)((dos_uint)al & 0xFF);

    if (pct == 0) {
        *out = 0;
        return;
    }
    {
        uint32_t prod = (uint32_t)(uint16_t)base_val * (uint32_t)pct;
        uint32_t quot = prod / 100u;
        uint32_t rem = prod % 100u;
        dos_int scaled;

        if (rem > 0x31u) quot++;
        scaled = dos_i16((int32_t)quot);
        if (ah != 1) scaled = dos_i16(-(int32_t)scaled);
        *out = scaled;
    }
}

static void voice_percent_scale_store(dos_int al, dos_int ah)
{
    percent_scale_store(al, ah, g1788, &g178a);
}

/* ===========================================================================
 * former src/SNDSCL4.C -- F_C59A.
 * =========================================================================== */

/* F_C59A -- scale a raw 0..63 value by 4, store through g1788. */
static void sound_param_scale4(dos_int al)
{
    g1788 = (dos_int)(((dos_uint)al & 0xFF) << 2);
}

/* ===========================================================================
 * former asm/M_C5A8_C5C6.ASM -- adjacent SI-relative setters.
 * =========================================================================== */

/* F_C5A8 -- store AL (zero-extended) through voice_dur_table (g17a4). */
static void f_c5a8(dos_int voice, dos_int al)
{
    g17a4[voice] = (dos_int)((dos_uint)al & 0xFF);
}

/* F_C5B3 -- select an entry from the 36-entry OPL-byte-stream table
 * (sound_dispatch_1832) by AL (the secondary command's raw, unmasked
 * argument byte) and store it, encoded, into state_cursor. See
 * sound_driver_internal.h's header comment: `idx` here is what the ASM's
 * `mov bx,[bx+1832h]` pointer-table read would have selected; this port
 * clamps any AL >= the generator's captured table count (36) to "no
 * stream selected" (index 0x3F, an unreachable encoded index) rather
 * than reproducing the historical out-of-bounds DS read, which the
 * generator's fixed-size array cannot represent safely in C. */
static void sound_table_word_select_store(dos_int voice, dos_int al)
{
    size_t count = sizeof(sound_dispatch_1832) / sizeof(sound_dispatch_1832[0]);
    unsigned idx = (unsigned)((dos_uint)al & 0xFF);

    if (idx >= count)
        idx = SOUND_STREAM_INDEX_MASK; /* out of range: encodes to an invalid table */

    sound_region_word_set(sound_region_17C4, SOUND_REGION_STATE_CURSOR_OFF, voice,
                           (dos_int)sound_stream_cursor_encode(idx, 0));
}

/* F_C5C6 -- store AL (zero-extended) through state_text. */
static void f_c5c6(dos_int voice, dos_int al)
{
    sound_region_word_set(sound_region_17C4, SOUND_REGION_STATE_TEXT_OFF, voice, (dos_int)((dos_uint)al & 0xFF));
}

/* ===========================================================================
 * former asm/M_C5D1_C706.ASM -- command decode, speaker control, PIT
 * output.
 * =========================================================================== */

/* F_C5D1 -- decode one command byte (es:[di+1], `arg_byte`) and update
 * the selected voice's note/frequency state; `ah` is the dispatcher's
 * argument nibble (an octave-shift count). arg_byte==0 means "note off".
 * Backend-mode branches: 1 = Tandy-gate frequency formula (a fixed-point
 * approximation, not a table lookup), 2 = queue only (no hardware call
 * here -- voice_enable drains the queue later), default(0) = notetab[0]
 * plus a signed delta, matching sound_control_value_select's default
 * shape. v_ctr is always re-latched from g17a4 at the end, regardless of
 * branch. */
static void voice_command_decode_apply(dos_int voice, dos_int arg_byte, dos_int ah)
{
    dos_int dx = (dos_int)(arg_byte & 0xFF);

    if (dx == 0) {
        voice_disable(voice);
        voice_rest_table[voice] = 1;
    } else if (snd_backend_mode == 1) {
        dos_int bx = dos_mul16(dx, 3);
        bx = (dos_int)((dos_uint)bx >> 2);
        bx = dos_add16(bx, 1);
        {
            dos_int cl = (ah == 0) ? (dos_int)1 : ah;
            dos_int base = (dos_int)((dos_uint)0x0D70u >> (cl & 0xFF));

            if (cl > 4)
                bx = (dos_int)((dos_uint)bx >> (cl - 4));
            else if (cl < 4)
                bx = dos_i16((int32_t)((dos_uint)bx << (4 - cl)));

            {
                dos_int freq = dos_sub16(base, bx);
                if (freq > 0x400)
                    freq = (dos_int)((dos_uint)freq >> 1);
                sound_pit_divisor_program(voice, freq);
            }
        }
        sound_region_word_set(sound_region_17C4, SOUND_REGION_VOICE_PENDING_OFF, voice, 1);
        voice_rest_table[voice] = 0;
    } else if (snd_backend_mode == 2) {
        sound_region_word_set(sound_region_17C4, SOUND_REGION_VOICE_PENDING_OFF, voice, 1);
    } else {
        /* backend 0 (backend "3" never persists past sound_backend_select_
           init -- see port report). sound_dispatch_182C[0] == &notetab[0]
           (confirmed by game_data.c's own initializer). */
        dos_int shifted = dos_i16((int32_t)((dos_uint)dx << 7));
        dos_int delta = dos_i16(-(int32_t)shifted);
        dos_uint base_word = ((const dos_uint *)sound_dispatch_182C[0])[0];
        dos_int value = dos_add16((dos_int)base_word, delta);

        value = (dos_int)((dos_uint)value >> (ah & 0xFF));
        sound_pit_divisor_program(voice, value);
        voice_enable(voice);
    }

    v_ctr[voice] = g17a4[voice];
}

/* F_C678 -- enable a voice: open the speaker gate (mode 0/1) or hand its
 * queued divisor to the OPL bank (mode 2, `bank` == voice index, per
 * OPLVOICE.C's voice_bank_retune_on's own 0..2 bank range -- voice index
 * 3 is silently a no-op there, matching the ASM's own SI>>1 bank
 * derivation for a 4th "voice" slot the 9-voice OPL bank has no room
 * for). No-op entirely when the voice's per-tick output-suppressed gate
 * (v_b) is set. */
static void voice_enable(dos_int voice)
{
    if (v_b[voice] != 0)
        return;

    if (snd_backend_mode == 1)
        sound_backend_speaker_gate(1, 1);
    else if (snd_backend_mode == 2)
        voice_bank_retune_on((dos_uint)voice, voice_retune_base_table[voice]);
    else
        sound_backend_speaker_gate(1, 0);
}

/* F_C6B9 -- disable a voice, or submit an alternate-backend update.
 * Backend mode 1 does NOT use the speaker-gate port at all: it writes an
 * OPL voice-reset byte directly, then calls sound_pit_divisor_program(0),
 * which (still mode 1) emits two more nibble bytes -- three
 * nibble-port writes total for one disable call. */
static void voice_disable(dos_int voice)
{
    if (v_b[voice] != 0)
        return;

    if (snd_backend_mode == 1) {
        uint8_t si2 = (uint8_t)((uint8_t)voice * 2);
        uint8_t reg = (uint8_t)(((uint8_t)(si2 << 4)) + 0x90);
        reg = (uint8_t)(reg | 0x0F);
        opl_port_write_byte((dos_int)reg);
        sound_pit_divisor_program(voice, 0);
    } else if (snd_backend_mode == 2) {
        voice_bank_update((dos_int)voice);
    } else {
        sound_backend_speaker_gate(0, 0);
    }
}

/* F_C706 -- write a PIT divisor (mode 0), queue it for the OPL bank
 * (mode 2, into driver-owned voice_retune_base_table), or emit it as two
 * packed-nibble bytes for the Tandy-gate backend (mode 1). No-op when
 * the voice's v_b gate is set. */
static void sound_pit_divisor_program(dos_int voice, dos_int ax)
{
    if (v_b[voice] != 0)
        return;

    if (snd_backend_mode == 1) {
        uint8_t si = (uint8_t)((uint8_t)voice * 2);
        uint8_t reg = (uint8_t)(((uint8_t)(si << 4)) + 0x80);
        reg = (uint8_t)(reg | (((uint16_t)(dos_uint)ax) & 0x0F));
        opl_port_write_byte((dos_int)reg);
        opl_port_write_byte((dos_int)(uint8_t)(((dos_uint)ax) >> 4));
    } else if (snd_backend_mode == 2) {
        voice_retune_base_table[voice] = ax;
    } else {
        sound_backend_pit_divisor((uint16_t)(dos_uint)ax);
    }
}

/* ===========================================================================
 * former src/F_C755.ASM.
 * =========================================================================== */

/* F_C755 -- reset every configured voice's armed flag and disable it;
 * unlike sound_voices_reset (F_C834) this does not touch v_b, snd_nvoices,
 * or apply the backend-mode==2 voice-count override. */
static void sound_voices_reset_and_service(void)
{
    dos_int voice;
    dos_int count = snd_nvoices;

    snd_mode = 0;
    snd_hi = -1;

    if (count > SOUND_VOICE_COUNT_MAX) count = SOUND_VOICE_COUNT_MAX;
    if (count < 0) count = 0;

    for (voice = 0; voice < count; voice++) {
        v_a[voice] = 0;
        voice_disable(voice);
    }
}

/* ===========================================================================
 * former asm/M_C77A_C898.ASM -- backend selection, voice-table reload,
 * OPL output.
 * =========================================================================== */

/* F_C77A -- select and initialise a sound backend. Caller: src/PLRLDPUB.C
 * (player_record_load_publish). ASM ambiguity: the backend-mode-1 branch
 * calls voice_enable() with whatever SI happened to hold; this C-ABI
 * entry point has no register convention at all, so voice 0 is used (the
 * only sensible reading -- see port report). */
void sound_backend_select_init(void)
{
    dos_int mode = snd_backend_mode;

    if (mode == 1) {
        opl_port = 0xC0;
        snd_nvoices = 4;
        voice_enable(0);
    } else if (mode == 2) {
        snd_nvoices = 4;
        opl_init();
    } else if (mode == 3) {
        opl_port = 0x205;
        snd_nvoices = 4;
        snd_backend_mode = 1;
    } else {
        snd_nvoices = 1;
    }
}

/* F_C7CB -- reload per-voice command-stream cursor/base tables for voice
 * table `v`, gated on music_enabled. snd_hi is latched unconditionally
 * (even when music is disabled), matching the ASM's write-before-check
 * order. */
void sound_voice_table_reload(dos_int v)
{
    snd_hi = v;
    if (music_enabled == 0)
        return;

    if (snd_backend_mode == 2) {
        (void)record_panel_rebuild();
        snd_nvoices = 4;
    }

    {
        int shift = (snd_backend_mode == 0) ? 1 : 3;
        uint16_t di = (uint16_t)(((uint16_t)(dos_uint)v) << shift);
        dos_int voice;
        dos_int count = snd_nvoices;

        di = (uint16_t)(di + (uint16_t)(dos_uint)snd_base2);

        if (count > SOUND_VOICE_COUNT_MAX) count = SOUND_VOICE_COUNT_MAX;
        if (count < 0) count = 0;

        for (voice = 0; voice < count; voice++) {
            dos_uint word = voice_word(di);
            dos_int value = (dos_int)dos_uadd16(word, (dos_uint)snd_base2);

            voice_stream_cursor_table[voice] = value;
            voice_stream_base_table[voice] = value;
            di = (uint16_t)(di + 2);
        }
    }

    snd_mode = 1;
}

/* F_C834 -- reset each configured voice, retain voice count. ASM
 * curiosity: the backend-mode==2 branch transiently writes snd_nvoices=4,
 * but the loop bound was already latched into a register beforehand and
 * the epilogue unconditionally restores snd_nvoices from the
 * pre-overwrite value -- nothing ever reads the global between the write
 * and the restore, so the overwrite is provably inert. Reproduced
 * faithfully (net effect: always zero) for documentation fidelity. */
void sound_voices_reset(void)
{
    dos_int orig_nvoices = snd_nvoices;
    dos_int count = orig_nvoices;
    dos_int voice;

    snd_mode = 0;
    snd_hi = -1;

    if (snd_backend_mode == 2)
        snd_nvoices = 4; /* see comment above: provably undone below */

    if (count > SOUND_VOICE_COUNT_MAX) count = SOUND_VOICE_COUNT_MAX;
    if (count < 0) count = 0;

    for (voice = 0; voice < count; voice++) {
        v_a[voice] = 0;
        v_b[voice] = 0;
        voice_disable(voice);
    }

    snd_nvoices = orig_nvoices;
}

/* F_C877 -- submit a disabled-voice update for every voice (backend mode
 * 2 always services 4 regardless of snd_nvoices, matching
 * sound_voices_reset's momentary override, except here it is NOT undone).
 * Caller: src/SNDREQ.C (sound_start). */
void sound_voices_disable_all(void)
{
    dos_int count = (snd_backend_mode == 2) ? (dos_int)4 : snd_nvoices;
    dos_int voice;

    if (count > SOUND_VOICE_COUNT_MAX) count = SOUND_VOICE_COUNT_MAX;
    if (count < 0) count = 0;

    for (voice = 0; voice < count; voice++)
        voice_disable(voice);
}

/* F_C898 -- write one OPL register+data pair. Historically this bracketed
 * the writes with six settling reads and a 35-iteration settling loop
 * (real-hardware OPL bus timing); per this task's requirement 1 those are
 * DROPPED and the write becomes a single opl_write() call, which routes
 * through the same sound_backend_opl_write() hook opl_write()'s other
 * callers (oplreg.c etc.) use, so one event log captures all OPL traffic
 * regardless of entry point. Callers: src/OPLINIT.C, src/OPLREG.C,
 * src/VOXCHAN.C. */
void opl_register_write(dos_int reg, dos_int val)
{
    opl_write((dos_uint)reg, (dos_uint)val);
}

/* ===========================================================================
 * former src/F_C8D4.ASM.
 * =========================================================================== */

/* F_C8D4 -- write one raw byte to the OPL/backend port with no
 * register-select/data split (the packed-nibble and paused-mode direct-
 * register paths). Routes to the dedicated nibble-port backend hook,
 * distinct from opl_write()'s register+data pair. */
static void opl_port_write_byte(dos_int al)
{
    sound_backend_nibble_port_write((uint8_t)(uint16_t)al);
}

/* ===========================================================================
 * former src/SNDTICK.C -- F_C8E2.
 * =========================================================================== */

/* F_C8E2 -- run one music-stream command via sound_stream_command_step
 * when due, then age the current note's delay counter and gate the
 * speaker off at its last tick (snd_delay == snd_len-1). */
static void sound_tick_step(void)
{
    if (snd_delay == 0) {
        sound_stream_command_step();
        if (snd_on == 0)
            return;
    }

    snd_delay = dos_sub16(snd_delay, 1);
    if (snd_one == 1)
        return;

    {
        dos_int len_minus_1 = dos_sub16(snd_len, 1);
        if (snd_delay == len_minus_1)
            speaker_gate_off();
    }
}

/* ===========================================================================
 * former asm/F_C914.ASM.
 * =========================================================================== */

/* F_C914 -- fetch the next music-stream command byte (sound_resource_block
 * at mus_ptr), split into opcode/argument nibbles, dispatch, and keep
 * dispatching while snd_delay stays 0 after advancing the cursor by 2.
 * The terminator (0xF) branch returns WITHOUT advancing mus_ptr or
 * looping, whichever of its two sub-paths (chain to the next cue, or
 * stop) it takes. */
static void sound_stream_command_step(void)
{
    int guard;
    for (guard = 0; guard < SOUND_LOOP_GUARD_MAX; guard++) {
        uint8_t cmd_byte = resource_byte((uint16_t)mus_ptr);
        uint8_t arg_byte = resource_byte((uint16_t)(mus_ptr + 1));
        dos_int al = (dos_int)(cmd_byte & 0x0F);
        dos_int ah = (dos_int)(cmd_byte >> 4);

        if (al == 0x0F) {
            speaker_gate_off();
            if (mus_flag == 0) {
                sound_stop_reset();
            } else {
                stream_control_block_arm(mus_arg);
                snd_delay = dos_add16(snd_delay, 1);
            }
            return;
        }

        if (al == 0) {
            speaker_gate_off();
            sound_stream_delay_decode((dos_int)arg_byte);
        } else if (al == 0x0D) {
            sound_ctlblock_command_dispatch(ah, (dos_int)arg_byte);
            /* ASM computes `cmp snd_delay,0` here but never branches on
               it before the unconditional jmp to the shared advance code
               -- dead compare, omitted (same pattern as F_C2EA). */
        } else if (al == 0x0E) {
            stream_note_program((dos_int)arg_byte, ah);
        } else {
            sound_note_dispatch(al, ah);
            sound_stream_delay_decode((dos_int)arg_byte);
        }

        mus_ptr = dos_add16(mus_ptr, 2);
        if (snd_delay != 0)
            return;
    }
    EMPIRES_TRACE("sound_stream_command_step GUARD HIT mus_ptr=%d byte0=%02x mus_flag=%d",
                  (int)mus_ptr, (unsigned)resource_byte((uint16_t)mus_ptr), (int)mus_flag);
}

/* ===========================================================================
 * former src/SNDNOTE.C -- F_C988.
 * =========================================================================== */

/* F_C988 -- look up a PIT divisor for (note=al, octave-shift=ah) in
 * notetab and program it, then open the speaker gate. Used only by the
 * single music-stream cluster (unconditional PC-speaker semantics,
 * independent of snd_backend_mode -- see sound_control_value_select for
 * the per-voice, backend-aware twin). */
static void sound_note_dispatch(dos_int note_al, dos_int shift_ah)
{
    dos_int idx = dos_sub16(note_al, 1);
    dos_uint word;
    dos_int divisor;

    if (idx < 0) idx = 0;
    if (idx > 11) idx = 11;
    word = notetab[(int)idx];
    divisor = (dos_int)((dos_uint)word >> (shift_ah & 0xFF));
    pit_channel2_set_divisor(divisor);
    speaker_gate_on();
}

/* ===========================================================================
 * former asm/M_C9A4_CA91.ASM -- music-stream control-block command
 * decoder.
 * =========================================================================== */

/* F_C9A4 -- decode one music-stream command argument byte into a scaled
 * delay, store into snd_delay. See decode_scaled_value's comment for the
 * shared algorithm with sound_command_value_derive. */
static void sound_stream_delay_decode(dos_int arg_byte)
{
    snd_delay = decode_scaled_value(arg_byte, dos_add16(g1e84, g1e86), g1e8c);
}

/* F_CA03 -- dispatch one control-block command by AH selector. */
static void sound_ctlblock_command_dispatch(dos_int ah, dos_int al)
{
    if (ah == 0)
        sound_ctlblock_flags_latch(al);
    else if (ah <= 2)
        stream_percent_scale_store(al, ah);
    else if (ah == 3)
        stream_note_delay_set(al);
    else if (ah == 4)
        stream_base_value_set(al);
}

/* F_CA35 -- latch or clear the control block's one-shot/length pair. */
static void sound_ctlblock_flags_latch(dos_int al)
{
    if (al == 0) {
        snd_one = 1;
    } else {
        snd_one = 0;
        snd_len = (dos_int)((dos_uint)al & 0xFF);
    }
}

/* F_CA51 -- signed percentage scale-and-store, the 1E84/1E86 twin of
 * F_C567 (voice_percent_scale_store above). */
static void stream_percent_scale_store(dos_int al, dos_int ah)
{
    percent_scale_store(al, ah, g1e84, &g1e86);
}

/* F_CA83 -- scale a raw 0..63 value by 4, store through g1e84. */
static void stream_base_value_set(dos_int al)
{
    g1e84 = (dos_int)(((dos_uint)al & 0xFF) << 2);
}

/* F_CA91 -- store AL (zero-extended) through stream_note_delay. */
static void stream_note_delay_set(dos_int al)
{
    stream_note_delay = (dos_int)((dos_uint)al & 0xFF);
}

/* ===========================================================================
 * former asm/F_CA9B.ASM.
 * =========================================================================== */

/* F_CA9B -- look up a divisor in notetab[0] for a shifted note delta,
 * program the PIT and speaker gate (arg_byte==0: gate off only), then
 * copy the default note delay into snd_delay. Twin shape to
 * voice_command_decode_apply's default branch. */
static void stream_note_program(dos_int arg_byte, dos_int ah)
{
    dos_int dx = (dos_int)(arg_byte & 0xFF);

    if (dx == 0) {
        speaker_gate_off();
    } else {
        dos_int shifted = dos_i16((int32_t)((dos_uint)dx << 7));
        dos_int delta = dos_i16(-(int32_t)shifted);
        dos_int value = dos_add16((dos_int)notetab[0], delta);

        value = (dos_int)((dos_uint)value >> (ah & 0xFF));
        pit_channel2_set_divisor(value);
        speaker_gate_on();
    }
    snd_delay = stream_note_delay;
}

/* ===========================================================================
 * former src/SPKON.C / src/SPKOFF.C / src/PITDIV2.C -- always-unconditional
 * hardware helpers the single music-stream cluster uses (no snd_backend_
 * mode gating at all -- see architecture note above sound_note_dispatch).
 * =========================================================================== */

/* F_CAD0 -- open the PC-speaker gate. */
static void speaker_gate_on(void)
{
    sound_backend_speaker_gate(1, 0);
}

/* F_CADB -- close the PC-speaker gate. */
static void speaker_gate_off(void)
{
    sound_backend_speaker_gate(0, 0);
}

/* F_CAE6 -- program PIT channel 2 with the divisor in `ax`. */
static void pit_channel2_set_divisor(dos_int ax)
{
    sound_backend_pit_divisor((uint16_t)(dos_uint)ax);
}

/* ===========================================================================
 * former asm/F_CAF1.ASM.
 * =========================================================================== */

/* F_CAF1 -- arm the single music-stream cursor for a new cue index `n`.
 * Self-referential priority gate: only accepts n if it is <= the
 * currently armed mus_arg, compared UNSIGNED (ASM `ja`/jump-if-above) --
 * mus_arg's reset value is -1 (0xFFFF unsigned), so the very first call
 * after boot/reset always passes. Callers: ~15 src/*.C files (BOARD.C,
 * GAME.C, HITTEST.C, INTRO.C, LEVEL.C, PUZZLE.C, ROUNDEND.C, ...); also
 * asm/SPRITES.ASM's bytecode interpreter (ASM-to-ASM). */
void stream_control_block_arm(dos_int n)
{
    if (sound_enabled == 0)
        return;
    if ((dos_uint)n > (dos_uint)mus_arg)
        return;

    mus_arg = n;
    {
        uint16_t off = (uint16_t)((uint16_t)(dos_uint)n << 1);
        uint16_t di = (uint16_t)((uint16_t)(dos_uint)snd_base + off);
        dos_uint word = resource_word(di);

        mus_ptr = (dos_int)dos_uadd16(word, (dos_uint)snd_base);
    }
    snd_on = 2;
    snd_len = 6;
    stream_note_delay = 1;
    g1e86 = 0;
    snd_delay = 0;
    snd_one = 0;
    dos_wr16(g1e8c, 0);
}

/* ===========================================================================
 * former src/SNDSTOP.C.
 * =========================================================================== */

/* F_CB48 -- clear the on-flag and invalidate the cached stream index,
 * then close the speaker gate. Callers: ~11 src/*.C files. */
void sound_stop_reset(void)
{
    snd_on = 0;
    mus_arg = -1;
    speaker_gate_off();
}
