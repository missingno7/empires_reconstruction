/* audio_mixer.c -- event ring + PCM render (see audio.h for the full
 * timeline scheme, backend mapping and known limitations).
 *
 * Producer side (audio_mixer_push_event): portable/audio/sound_driver.c's
 * four sound_backend_* hooks, called from the 236.7 Hz timer thread.
 * Consumer side (audio_render): whatever pulls PCM -- normally
 * portable/platform/sdl3/audio_sdl.c's SDL audio callback, or a test.
 * The two sides only ever touch the mutex-protected ring below; the OPL
 * chip (opl_backend.c) and PC-speaker synth (speaker_synth.c) state is
 * touched only from the render side, inside audio_render(), so it needs
 * no locking of its own.
 */
#include "audio.h"

#include "opl_backend.h"
#include "speaker_synth.h"
#include "sync.h"
#include "timer.h" /* TIMER_PIT_DIVISOR -- the exact PIT ratio, see audio.h */

#include <string.h>

/* Generous relative to the driver's real event rate (SOUND.ASM services at
 * most 4 voices per tick, each capable of a handful of OPL writes) -- see
 * audio_mixer_dropped_events(). */
#define EVENT_RING_CAPACITY 2048

typedef struct {
    uint64_t sample_pos;
    int kind;
    uint16_t a;
    uint16_t b;
    int origin;                 /* enum audio_event_origin */
} queued_event;

static queued_event s_ring[EVENT_RING_CAPACITY];
static uint32_t s_ring_head;   /* next free slot (producer) */
static uint32_t s_ring_tail;   /* next slot to consume (render side) */
static uint32_t s_ring_count;
static sync_mutex s_ring_mutex;
static bool s_mutex_inited;

static int s_sample_rate = 48000;
static uint64_t s_cursor;               /* absolute samples rendered so far */
static uint64_t s_events_applied;
static uint64_t s_dropped_events;

static PcSpeaker s_speaker;

static void ensure_mutex(void)
{
    if (!s_mutex_inited) {
        sync_mutex_init(&s_ring_mutex);
        s_mutex_inited = true;
    }
}

/* Output gains.  The OPL backend yields +-1.0 full scale and the speaker
 * synth a +-1.0 square wave; both are far hotter than an AdLib line-out and
 * a PC-speaker cone next to each other, so the defaults sit well below unity
 * (a full-scale square wave has 3 dB more RMS than a sine of the same
 * amplitude, hence the extra drop on the speaker).  On top sit the user's
 * two volume knobs, music and effects (empires.json audio.*). */
#define AUDIO_GAIN_OPL_DEFAULT      0.25f
#define AUDIO_GAIN_SPEAKER_DEFAULT  0.06f
static float s_gain_opl = AUDIO_GAIN_OPL_DEFAULT;
static float s_gain_speaker = AUDIO_GAIN_SPEAKER_DEFAULT;
static float s_gain_music = 1.0f;
static float s_gain_effects = 1.0f;
static int s_speaker_origin = AUDIO_ORIGIN_MUSIC;   /* who wrote to the speaker last (render side) */

static float pct_to_gain(int percent)
{
    if (percent < 0) percent = 0;
    if (percent > 200) percent = 200;
    return (float)percent / 100.0f;
}

void audio_mixer_set_music_volume(int percent)
{
    ensure_mutex();
    sync_mutex_lock(&s_ring_mutex);
    s_gain_music = pct_to_gain(percent);
    sync_mutex_unlock(&s_ring_mutex);
}

void audio_mixer_set_effects_volume(int percent)
{
    ensure_mutex();
    sync_mutex_lock(&s_ring_mutex);
    s_gain_effects = pct_to_gain(percent);
    sync_mutex_unlock(&s_ring_mutex);
}

int audio_mixer_music_volume(void)
{
    float gain;
    ensure_mutex();
    sync_mutex_lock(&s_ring_mutex); gain = s_gain_music; sync_mutex_unlock(&s_ring_mutex);
    return (int)(gain * 100.0f + 0.5f);
}

int audio_mixer_effects_volume(void)
{
    float gain;
    ensure_mutex();
    sync_mutex_lock(&s_ring_mutex); gain = s_gain_effects; sync_mutex_unlock(&s_ring_mutex);
    return (int)(gain * 100.0f + 0.5f);
}

void audio_mixer_init(int sample_rate)
{
    ensure_mutex();

    sync_mutex_lock(&s_ring_mutex);
    s_sample_rate = sample_rate > 0 ? sample_rate : 48000;
    s_ring_head = s_ring_tail = s_ring_count = 0;
    s_dropped_events = 0;
    sync_mutex_unlock(&s_ring_mutex);

    s_cursor = 0;
    s_events_applied = 0;
    memset(&s_speaker, 0, sizeof s_speaker);
    opl_backend_reset((uint32_t)s_sample_rate);
}

void audio_mixer_shutdown(void)
{
    /* Nothing owns a host resource here (see audio.h); kept for symmetry
     * with audio_sdl_init()/audio_sdl_shutdown(). */
}

uint64_t audio_mixer_ticks_to_samples(uint32_t ticks)
{
    /* Exact ratio, 64-bit integer math (task requirement): samples =
     * ticks * TIMER_PIT_DIVISOR * sample_rate / 1193182. TIMER_PIT_DIVISOR
     * (0x13B1 = 5041) and 1193182 together are the exact fraction
     * timer.h's TIMER_TICK_HZ (~236.6975 Hz) is derived from -- using the
     * integer ratio instead of dividing by that rounded float avoids
     * rounding drift over a long session. */
    return ((uint64_t)ticks * (uint64_t)TIMER_PIT_DIVISOR * (uint64_t)s_sample_rate) / 1193182ull;
}

void audio_mixer_push_event(int kind, uint32_t tick, uint16_t a, uint16_t b)
{
    audio_mixer_push_event_from(kind, tick, a, b, AUDIO_ORIGIN_MUSIC);
}

void audio_mixer_push_event_from(int kind, uint32_t tick, uint16_t a, uint16_t b, int origin)
{
    ensure_mutex();

    /* Scheduled a fixed AUDIO_LATENCY_TICKS into the future relative to
     * the tick it logically occurred on -- see audio.h's timeline scheme
     * for why. */
    uint64_t sample_pos = audio_mixer_ticks_to_samples((uint32_t)((uint64_t)tick + AUDIO_LATENCY_TICKS));

    sync_mutex_lock(&s_ring_mutex);
    if (s_ring_count >= EVENT_RING_CAPACITY) {
        /* The render side has stalled for long enough to fill the ring;
         * drop the oldest queued event to make room rather than block the
         * timer thread (audio glitches, not game-visible freezes, are the
         * acceptable failure mode here). */
        s_ring_tail = (s_ring_tail + 1) % EVENT_RING_CAPACITY;
        s_ring_count--;
        s_dropped_events++;
    }
    {
        queued_event *slot = &s_ring[s_ring_head];
        slot->sample_pos = sample_pos;
        slot->kind = kind;
        slot->a = a;
        slot->b = b;
        slot->origin = origin;
        s_ring_head = (s_ring_head + 1) % EVENT_RING_CAPACITY;
        s_ring_count++;
    }
    sync_mutex_unlock(&s_ring_mutex);
}

/* Applies one already-dequeued event to the OPL chip / PC-speaker synth
 * state.  Only ever called from audio_render() (the render side), so
 * s_speaker/opl_backend need no locking. */
static void apply_event(const queued_event *e)
{
    switch (e->kind) {
    case AUDIO_EVENT_OPL_WRITE:
        opl_backend_write((uint8_t)e->a, (uint8_t)e->b);
        break;
    case AUDIO_EVENT_PIT_DIVISOR:
        s_speaker.divisor = e->a;
        s_speaker_origin = e->origin;
        break;
    case AUDIO_EVENT_SPEAKER_GATE:
        /* e->b (tandy_mode) is deliberately ignored here -- both gate
         * modes drive the same square-wave synth (audio.h's documented
         * approximation). */
        s_speaker.enabled = e->a != 0;
        s_speaker_origin = e->origin;
        break;
    case AUDIO_EVENT_NIBBLE_WRITE:
        /* Logged (the applied-count below still advances) but not
         * synthesised -- see audio.h's backend mapping note. */
        break;
    default:
        break;
    }
    s_events_applied++;
}

/* Pops the front of the ring if its sample_pos is <= `upto`, returning
 * true and filling *out. Locks briefly; does not call apply_event() itself
 * so the mutex is never held while touching OPL/speaker state. */
static bool try_pop_due(uint64_t upto, queued_event *out)
{
    bool have = false;
    sync_mutex_lock(&s_ring_mutex);
    if (s_ring_count > 0 && s_ring[s_ring_tail].sample_pos <= upto) {
        *out = s_ring[s_ring_tail];
        s_ring_tail = (s_ring_tail + 1) % EVENT_RING_CAPACITY;
        s_ring_count--;
        have = true;
    }
    sync_mutex_unlock(&s_ring_mutex);
    return have;
}

/* sample_pos of the next queued event, or UINT64_MAX if the ring is
 * currently empty (renderer is ahead of every event -- render with the
 * current state, per the design). */
static uint64_t peek_next_sample_pos(void)
{
    uint64_t pos = UINT64_MAX;
    sync_mutex_lock(&s_ring_mutex);
    if (s_ring_count > 0)
        pos = s_ring[s_ring_tail].sample_pos;
    sync_mutex_unlock(&s_ring_mutex);
    return pos;
}

void audio_render(int16_t *out, int frames)
{
    int i = 0;
    float music_gain;
    float effects_gain;

    if (frames <= 0)
        return;

    /* Snapshot once per callback.  Setters may run on the game/main thread,
     * but the PCM loop never takes a mutex per sample. */
    ensure_mutex();
    sync_mutex_lock(&s_ring_mutex);
    music_gain = s_gain_music;
    effects_gain = s_gain_effects;
    sync_mutex_unlock(&s_ring_mutex);

    while (i < frames) {
        uint64_t cur = s_cursor + (uint64_t)i;

        /* Apply everything due at or before the current sample. */
        for (;;) {
            queued_event ev;
            if (!try_pop_due(cur, &ev))
                break;
            apply_event(&ev);
        }

        /* Render up to the next event (or the rest of the buffer if none
         * is queued) with the state as it now stands. */
        int run = frames - i;
        uint64_t next_pos = peek_next_sample_pos();
        if (next_pos != UINT64_MAX) {
            uint64_t remain = next_pos - cur;
            if (remain < (uint64_t)run)
                run = remain > 0 ? (int)remain : 1;
        }

        for (int j = 0; j < run; j++) {
            float opl_sample = opl_backend_generate();
            float spk_sample = speaker_synth_generate(&s_speaker, s_sample_rate);

            float spk_gain = s_gain_speaker * (s_speaker_origin == AUDIO_ORIGIN_EFFECTS ? effects_gain : music_gain);
            float mixed = opl_sample * s_gain_opl * music_gain + spk_sample * spk_gain;
            if (mixed > 1.0f)
                mixed = 1.0f;
            else if (mixed < -1.0f)
                mixed = -1.0f;

            out[i + j] = (int16_t)(mixed * 32767.0f);
        }
        i += run;
    }

    s_cursor += (uint64_t)frames;
}

uint64_t audio_mixer_events_applied_count(void)
{
    return s_events_applied;
}

uint64_t audio_mixer_dropped_events(void)
{
    uint64_t v;
    ensure_mutex();
    sync_mutex_lock(&s_ring_mutex);
    v = s_dropped_events;
    sync_mutex_unlock(&s_ring_mutex);
    return v;
}
