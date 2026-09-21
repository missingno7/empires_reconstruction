#include "G2FD2.H"
extern unsigned char voice_level_table[];
extern char voice_byte_table[];
extern void fc898();
extern void voice_write_level();
void voice_channel_set_gate(int slot,unsigned value)
{
 register int i;
 value=value?127:0;
 i=ui_panel_glyph_records[slot].b1;
 if(value>127) value=127;
 voice_level_table[i]=value;voice_write_level(i);
}

void voice_channel_level_refresh(int slot)
{
 register int i;
 i=ui_panel_glyph_records[slot].b1;voice_write_level(i);
}

void voice_channel_apply_freq(int slot)
{
 register int i;
 i=ui_panel_glyph_records[slot].b1;fc898(voice_byte_table[i]+64,63);
}
