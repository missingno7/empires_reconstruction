/* oplinit.c -- ported from src/OPLINIT.C (OPL bring-up and global
 * settings).  All five functions are pure state-plus-dispatch; the only
 * hardware-facing call any of them makes is voice_key_off(), already
 * ported in oplreg.c to call opl_write() internally.
 */
#include "game.h"

/* ---- F_D99B (original code at 0xD99B) ---- */
/* F_D99B -- bring the OPL chip up: clear the note table, reset, silence the
   nine voices one at a time, then enable. */
void opl_init(void)
{
    dos_int i;

    voice_level_table_reset();
    music_reset_tuning_tables();
    opl_set_depth_wrapper(0);
    opl_set_depth_and_nts(0, 0, 0);
    for (i = 0; i < 9; i++)
        voice_key_off(i);
    music_set_tempo(1);
    opl_set_enabled(1);
}


/* ---- F_D9D9 (original code at 0xD9D9) ---- */
/* F_D9D9 -- a one-call wrapper around opl_set_depth_flags.  The frame is not
   optional: the original pushes BP, and TC 2.0 emits a frame only when the
   function has a parameter or a local, so this one has a parameter the body
   ignores. */
void opl_set_depth_wrapper(dos_int a)
{
    (void)a;
    opl_set_depth_flags();
}


/* ---- F_D9E1 (original code at 0xD9E1) ---- */
/* F_D9E1 -- turn the whole OPL bank on or off: latch the enable word,
   silence all 18 voices' 0xE0 registers, then write the enable word to
   register 1. */
void opl_set_enabled(dos_int f)
{
    dos_int i;

    opl_enabled = f ? 0x20 : 0;
    for (i = 0; i < 0x12; i++) opl_write(voice_byte_table[i] + 0xe0, 0);
    opl_write(1, opl_enabled);
}


/* ---- F_DA20 (original code at 0xDA20) ---- */
/* F_DA20 -- clamp the tempo to 1..12, latch it, and recompute the derived
   word.  int-semantics-inventory.md fact 11 (src/OPLINIT.C:71-79): the
   clamp is UNSIGNED (`jbe`/`jae`) -- v stays dos_uint.  music_tempo and
   music_tempo_scaled are both generated as dos_int (matching OPLINIT.C's
   own `extern int music_tempo`/`music_tempo_scaled` at file scope, not
   `unsigned`); fact 11's dos_u16() truncation note is conditioned on the
   result feeding a dos_uint storage location, which is not the case here,
   and the multiply's operands stay well within 16-bit range (<= 12*0x19),
   so no helper is needed for the derived-word store. */
void music_set_tempo(dos_uint v)
{
    if (v > 12) v = 12;
    if (v < 1) v = 1;
    music_tempo = v;
    music_tempo_scaled = music_tempo * 0x19;
}


/* ---- F_DA49 (original code at 0xDA49) ---- */
/* F_DA49 -- latch three bytes of global voice state and re-emit them. */
void opl_set_depth_and_nts(dos_char a, dos_char b, dos_char c)
{
    opl_am_depth = a;
    opl_vib_depth = b;
    opl_note_select = c;
    opl_set_depth_flags();
    opl_set_note_select();
}
