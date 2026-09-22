/* speaker_synth.c -- see speaker_synth.h. */
#include "speaker_synth.h"

#include <math.h>

#define SPEAKER_PIT_HZ 1193182.0

float speaker_synth_generate(PcSpeaker *spk, int sample_rate)
{
    if (!spk->enabled || spk->divisor == 0 || sample_rate <= 0) {
        spk->phase = 0.0;
        return 0.0f;
    }

    double freq = SPEAKER_PIT_HZ / (double)spk->divisor;
    double step = freq / (double)sample_rate;
    float sample = (spk->phase < 0.5) ? 1.0f : -1.0f;

    spk->phase = fmod(spk->phase + step, 1.0);

    return sample;
}
