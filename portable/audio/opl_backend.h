/* opl_backend.h -- private Nuked-OPL3 wrapper for portable/audio/
 * audio_mixer.c (Milestone F).  Not a public portable/include header:
 * only files inside portable/audio/ may include it (docs/portable/
 * architecture.md "Layering": "No file outside portable/audio/ may
 * include Nuked-OPL3 headers" -- this header is the boundary that keeps
 * that true, since it never exposes opl3_chip/opl3.h to its callers).
 *
 * The chip is a single process-wide instance (the game only ever needs
 * one OPL); there is nothing to create/destroy, only reset/write/generate.
 */
#ifndef PORTABLE_AUDIO_OPL_BACKEND_H
#define PORTABLE_AUDIO_OPL_BACKEND_H

#include <stdint.h>

/* Reset the chip for rendering at `sample_rate` samples/sec.  Leaves OPL3
 * mode disabled (register 0x105 is never written) -- the game programs a
 * plain OPL2 register set, bank 0 only. */
void opl_backend_reset(uint32_t sample_rate);

/* One (reg, val) write, routed to bank 0 (reg is the historical 0x00-0xF5
 * OPL2 register number; Nuked-OPL3 addresses bank 0 at 0x000-0x0FF, so no
 * translation is needed as long as reg stays a plain uint8_t). */
void opl_backend_write(uint8_t reg, uint8_t val);

/* One mono sample in roughly [-1, 1], taken from Nuked's left channel to
 * match the historical OPL playback path. */
float opl_backend_generate(void);

#endif /* PORTABLE_AUDIO_OPL_BACKEND_H */
