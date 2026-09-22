/* audio.h -- host-independent PCM mixer (Milestone F, docs/portable/
 * architecture.md "Audio model").  Turns the timestamped backend events
 * portable/audio/sound_driver.c already produces (sound.h's four
 * sound_backend_* hooks: OPL register writes, PIT ch2 divisor, speaker
 * gate, raw nibble-port writes) into rendered PCM, independent of any
 * host audio API.  Only portable/platform/sdl3/audio_sdl.c may call into
 * an actual audio device; this header and portable/audio/audio_mixer.c
 * know nothing about SDL.
 *
 * ---------------------------------------------------------------------
 * Timeline scheme
 * ---------------------------------------------------------------------
 * The sound driver runs on the dedicated 236.7 Hz timer thread
 * (portable/game/timer.c: `++timer_ticks; ...; sound_tick_entry();`) and
 * emits zero or more backend events *synchronously inside that call*, in
 * order.  portable/audio/sound_driver.c's four sound_backend_* hook
 * bodies each forward their event to audio_mixer_push_event() below,
 * tagged with the CURRENT `timer_ticks` value at the moment of the call
 * (timer.c increments timer_ticks before it ever calls sound_tick_entry(),
 * so this is exactly the tick the event belongs to -- not an approximation
 * derived by polling later).
 *
 * audio_mixer_push_event() converts that tick index to an absolute sample
 * position on the mixer's own timeline (sample 0 == tick 0, i.e. audio
 * start is assumed to coincide with the game clock's epoch -- true in
 * practice since main.c starts the audio device before the timer thread
 * exists) using exact 64-bit integer math:
 *
 *     sample_pos = (tick + AUDIO_LATENCY_TICKS) * TIMER_PIT_DIVISOR
 *                  * sample_rate / 1193182
 *
 * (TIMER_PIT_DIVISOR/1193182 is the exact PIT ratio timer.h's
 * TIMER_TICK_HZ is derived from -- see timer.h.  Using the integer ratio
 * instead of dividing by the rounded 236.6975 Hz float avoids rounding
 * drift over a long session.)  AUDIO_LATENCY_TICKS (3 ticks, ~13 ms) is a
 * fixed scheduling margin: it is *added* to every event's raw tick-based
 * sample position, so an event is always scheduled a few ticks into the
 * mixer's future relative to when it logically occurred.  This guarantees
 * the event has already been pushed into the ring (production is
 * effectively instantaneous relative to the render side) by the time the
 * render cursor reaches its scheduled sample -- i.e. it absorbs
 * cross-thread scheduling jitter (OS thread wake latency, the SDL audio
 * callback's own period) without ever applying an event late.  The cost
 * is a constant ~13 ms of audio latency, which is inaudible as lag.
 *
 * The event ring (audio_mixer.c) is a small mutex-protected FIFO (sync.h):
 * the timer thread pushes, the audio render thread (audio_render(), called
 * from whatever pulls PCM -- an SDL callback, a feeder thread, or a test)
 * drains it in sample-position order.  audio_render() advances its own
 * absolute sample cursor by `frames` every call; for each stretch of
 * output it first applies every queued event whose scheduled sample_pos
 * is <= the current cursor (updating the OPL chip / PC-speaker synth
 * state), then renders samples with whatever state is now current.  If
 * the cursor is ahead of every queued event (nothing due yet), it just
 * renders with the current state -- exactly the "renders with the current
 * state" fallback the design calls for.
 *
 * Known limitation (documented, not fixed here): the mixer's sample clock
 * (the audio device's own playback rate) and the game's tick clock
 * (sync_now_ns(), portable/compat/clock.c) are two independent clocks.
 * AUDIO_LATENCY_TICKS absorbs short-term scheduling jitter between them
 * but not long-run drift (typical consumer audio hardware clocks can be
 * off by tens of PPM from the system clock); over a very long session
 * this could accumulate to audible mistiming.  Out of scope for
 * Milestone F -- a full fix would need clock-domain resampling.
 *
 * ---------------------------------------------------------------------
 * Backend mapping (task requirement 3 "anything the driver emits that
 * you could not map to a backend"):
 *   AUDIO_EVENT_OPL_WRITE     -> portable/audio/opl_backend.c (Nuked-OPL3,
 *                                 bank 0 only -- the game only ever
 *                                 programs an OPL2 register set).
 *   AUDIO_EVENT_PIT_DIVISOR,
 *   AUDIO_EVENT_SPEAKER_GATE  -> portable/audio/speaker_synth.c (one
 *                                 phase-continuous square-wave synth).
 *                                 The Tandy/PCjr gate mode
 *                                 (sound_backend_speaker_gate's
 *                                 tandy_mode==1) reuses the SAME synth --
 *                                 an intentional approximation.  The real
 *                                 hardware routes tandy_mode through the
 *                                 Tandy/PCjr 3-voice PSG instead of the PC
 *                                 speaker gate; emulating that chip is out
 *                                 of scope for this milestone, so a
 *                                 tandy-gated note sounds like a plain
 *                                 speaker square wave rather than the
 *                                 PSG's tone.
 *   AUDIO_EVENT_NIBBLE_WRITE  -> logged only (audio_mixer_events_applied_
 *                                 count() still counts it), never
 *                                 synthesised: it is backend-mode-1's
 *                                 packed-nibble PIT-divisor/voice-reset
 *                                 byte and the paused-mode single-register
 *                                 control writes (sound.h's header
 *                                 comment) -- none of backend mode 1
 *                                 (Tandy/PCjr queued mode) or the
 *                                 pause-overlay renderer's byte-level
 *                                 protocol is reverse-engineered to
 *                                 register-level meaning yet, so there is
 *                                 nothing concrete to synthesise from a
 *                                 raw byte.
 */
#ifndef PORTABLE_AUDIO_H
#define PORTABLE_AUDIO_H

#include <stdint.h>

/* Mirrors sound.h's `enum sound_event_kind` exactly (same 4 values, same
 * order) -- kept as a small, independent enum so this header does not
 * need to include sound.h (audio_mixer.c stays a generic PCM engine, not
 * specifically wired to the SOUND.ASM port).  portable/audio/
 * sound_driver.c's forwarding calls pass its own enum's values here
 * directly; a comment at each call site ties the two together. */
enum audio_event_kind {
    AUDIO_EVENT_OPL_WRITE = 0,
    AUDIO_EVENT_PIT_DIVISOR = 1,
    AUDIO_EVENT_SPEAKER_GATE = 2,
    AUDIO_EVENT_NIBBLE_WRITE = 3
};

/* Fixed scheduling margin, in ticks, added to every event's sample
 * position -- see the timeline scheme above. ~3 ticks @ 236.7 Hz ~= 13 ms. */
#define AUDIO_LATENCY_TICKS 3

/* (Re)initialise the mixer for rendering at `sample_rate` samples/sec,
 * mono.  Resets the OPL chip, the PC-speaker synth, the event ring and the
 * render cursor.  Safe to call more than once (e.g. from independent
 * tests in one process). */
void audio_mixer_init(int sample_rate);

/* Release whatever audio_mixer_init() would need released.  Currently the
 * mixer owns no host resources (no heap, no device) -- provided for
 * symmetry with audio_sdl_init()/audio_sdl_shutdown() and forward
 * compatibility. */
void audio_mixer_shutdown(void);

/* Push one backend event, produced at tick `tick` (portable/game/timer.c's
 * timer_ticks at the moment of the historical hardware write).  Thread
 * -safe: called from the timer thread; audio_render() (below) drains the
 * ring from the render thread. `a`/`b` carry the same payload sound.h's
 * struct sound_event does: OPL (reg,val), PIT (divisor,-), speaker gate
 * (enabled,tandy_mode), nibble (value,-). */
void audio_mixer_push_event(int kind, uint32_t tick, uint16_t a, uint16_t b);

/* Render `frames` mono 16-bit PCM samples at the sample_rate given to
 * audio_mixer_init(), applying every event whose scheduled sample position
 * falls within this call as the render cursor reaches it.  Host
 * -independent: called from portable/platform/sdl3/audio_sdl.c's SDL
 * audio callback, or directly from tests. */
void audio_render(int16_t *out, int frames);

/* Pure helper: the same tick -> sample conversion audio_mixer_push_event()
 * uses internally (exact 64-bit ratio, see the timeline scheme above),
 * exposed so callers/tests don't duplicate the formula.  Uses the sample
 * rate passed to the most recent audio_mixer_init() call. Does NOT add
 * AUDIO_LATENCY_TICKS. */
uint64_t audio_mixer_ticks_to_samples(uint32_t ticks);

/* ---- test/diagnostic hooks (portable/tests/test_audio.c) ---- */

/* Cumulative count of events applied so far (any kind), incremented by
 * audio_render()'s consume loop at the moment each event's scheduled
 * sample position is reached -- lets a test observe *when* an event took
 * effect without depending on OPL/PC-speaker audio output. */
uint64_t audio_mixer_events_applied_count(void);

/* Cumulative count of events the ring had to drop because it was full
 * (the render side stalled for long enough that more than the ring's
 * capacity of events queued up).  Always 0 in normal operation. */
uint64_t audio_mixer_dropped_events(void);

#endif /* PORTABLE_AUDIO_H */
