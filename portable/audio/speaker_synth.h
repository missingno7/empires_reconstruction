/* speaker_synth.h -- phase-continuous PC-speaker square-wave synth
 * (private to portable/audio/audio_mixer.c, Milestone F).
 *
 * Models PIT channel 2 in mode 3 (square wave generator) as the game's
 * SOUND.ASM backend-mode-0 (plain PC speaker) and backend-mode-1
 * (Tandy/PCjr gate, approximated -- see audio.h's header comment) paths
 * drive it: a 16-bit divisor sets the frequency (1193182 / divisor), and
 * the port-0x61 gate bit enables/disables the speaker being connected to
 * that square wave.
 */
#ifndef PORTABLE_AUDIO_SPEAKER_SYNTH_H
#define PORTABLE_AUDIO_SPEAKER_SYNTH_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool enabled;
    uint16_t divisor;
    double phase;   /* 0..1, one full square-wave cycle */
} PcSpeaker;

/* One mono sample in [-1, 1] at `sample_rate` samples/sec, advancing
 * `spk->phase`.  Square wave (+1 for the first half cycle, -1 for the
 * second) while enabled with a nonzero divisor; silence otherwise.
 *
 * PIT ch2 mode-3 semantics: channel 2's GATE input is port 0x61 bit 0.
 * Real mode-3 hardware forces OUT high and reloads the counter the
 * instant GATE goes low, i.e. disabling produces a defined state rather
 * than freezing mid-cycle -- modelled here by resetting phase to 0 so the
 * next enable always starts a clean cycle. */
float speaker_synth_generate(PcSpeaker *spk, int sample_rate);

#endif /* PORTABLE_AUDIO_SPEAKER_SYNTH_H */
