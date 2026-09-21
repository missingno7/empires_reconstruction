/* voxchan.c -- ported from src/VOXCHAN.C (per-slot voice-level/frequency
 * glue used by the player-slot UI panel).  Its one hardware-facing call
 * (opl_register_write) is rewritten to opl_write(); voice_write_level()
 * (oplreg.c) already goes through opl_write() itself.
 */
#include "game.h"

void voice_channel_set_gate(dos_int slot, dos_uint value)
{
    dos_int i;

    value = value ? 127 : 0;
    i = ui_panel_glyph_records[slot].b1;
    if (value > 127) value = 127;
    voice_level_table[i] = value;
    voice_write_level(i);
}

void voice_channel_level_refresh(dos_int slot)
{
    dos_int i;

    i = ui_panel_glyph_records[slot].b1;
    voice_write_level(i);
}

void voice_channel_apply_freq(dos_int slot)
{
    dos_int i;

    i = ui_panel_glyph_records[slot].b1;
    opl_write(voice_byte_table[i] + 64, 63);
}
