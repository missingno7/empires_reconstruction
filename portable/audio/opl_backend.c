/* opl_backend.c -- Nuked-OPL3 wrapper (see opl_backend.h).
 *
 * The ONLY translation unit in the portable tree allowed to include
 * Nuked-OPL3's headers (docs/portable/architecture.md "Layering").
 *
 * The chip is reset with OPL3_Reset(chip, sample_rate) and driven with
 * OPL3_GenerateResampled(), i.e. Nuked-OPL3's own internal resampler
 * produces output directly at the mixer's sample rate; we never run the
 * chip at its native 49716 Hz and resample ourselves.  Nuked's resampler
 * is the reference implementation for this chip, so letting it do the
 * work is simpler and at least as accurate as a second, independent
 * resampling stage would be.
 *
 * OPL3 mode is never enabled: this backend never writes register 0x105
 * (OPL3_WriteReg's high bank/OPL3-enable register).  Every write the
 * driver makes targets a plain uint8_t register number in 0x00-0xF5,
 * which Nuked-OPL3 already addresses as bank 0 (0x000-0x0FF in its 9-bit
 * register space) with no translation needed -- matching the historical
 * game, which only ever programmed an OPL2.
 */
#include "opl_backend.h"

#include "opl3.h"

static opl3_chip s_chip;

void opl_backend_reset(uint32_t sample_rate)
{
    OPL3_Reset(&s_chip, sample_rate);
}

void opl_backend_write(uint8_t reg, uint8_t val)
{
    /* Normal hardware writes are delayed by the OPL bus.  Nuked's buffered
     * entry point models that delay; the immediate API is reserved for
     * callers that explicitly need to bypass it. */
    OPL3_WriteRegBuffered(&s_chip, (uint16_t)reg, val);
}

float opl_backend_generate(void)
{
    int16_t buf[2];
    OPL3_GenerateResampled(&s_chip, buf);
    /* The historical OPL path consumes the first (left) channel.  Nuked's
     * channel-sample-delay quirk means averaging L/R is not equivalent. */
    return (float)buf[0] / 32768.0f;
}
