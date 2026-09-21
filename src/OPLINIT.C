/* src/OPLINIT.C: OPL bring-up and global settings.
   One translation unit; the sections below were the separate member
   sources of grouped module C_D99B_DA49 and keep their original ids. */

extern void voice_level_table_reset();
extern void music_reset_tuning_tables();
extern void opl_set_depth_wrapper();
extern void opl_set_depth_and_nts();
extern void voice_key_off();
extern void music_set_tempo();
extern void opl_set_enabled();
extern void opl_set_depth_flags();
extern void fc898();
extern int opl_enabled;                       /* DS:CA22 */
extern char voice_byte_table[];                    /* DS:2FE4, byte per voice */
extern int music_tempo;                       /* DS:C6C1 */
extern int music_tempo_scaled;                       /* DS:CA6B */
extern void opl_set_note_select();
extern char opl_am_depth;                      /* DS:C6AA */
extern char opl_vib_depth;                      /* DS:C6B5 */
extern char opl_note_select;                      /* DS:C6B4 */

/* ---- F_D99B (original code at 0xD99B) ---- */
/* F_D99B -- bring the OPL chip up: clear the note table, reset, silence the
   nine voices one at a time, then enable. */
void opl_init()
{
    register int i;

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
/* F_D9D9 -- a one-call wrapper around opl_set_depth_flags.  The frame is not optional: the
   original pushes BP, and TC 2.0 emits a frame only when the function has a
   parameter or a local, so this one has a parameter the body ignores. */
void opl_set_depth_wrapper(a)
int a;
{
    opl_set_depth_flags();
}


/* ---- F_D9E1 (original code at 0xD9E1) ---- */
/* F_D9E1 -- turn the whole OPL bank on or off: latch the enable word, silence
   all 18 voices' 0xE0 registers, then write the enable word to register 1. */
void opl_set_enabled(f)
int f;
{
    register int i;

    opl_enabled = f ? 0x20 : 0;
    for (i = 0; i < 0x12; i++) fc898(voice_byte_table[i] + 0xe0, 0);
    fc898(1, opl_enabled);
}


/* ---- F_DA20 (original code at 0xDA20) ---- */
/* F_DA20 -- clamp the tempo to 1..12, latch it, and recompute the derived
   word.  `jbe`/`jae` make the clamp UNSIGNED and the `mul` is an unsigned
   int multiply (tc20-codegen rule 5); the second statement re-reads the
   global rather than reusing the register, TC 2.0 doing no CSE (rule 6). */
void music_set_tempo(v)
register unsigned v;
{
    if (v > 12) v = 12;
    if (v < 1) v = 1;
    music_tempo = v;
    music_tempo_scaled = music_tempo * 0x19;
}


/* ---- F_DA49 (original code at 0xDA49) ---- */
/* F_DA49 -- latch three bytes of global voice state and re-emit them. */
void opl_set_depth_and_nts(a, b, c)
char a;
char b;
char c;
{
    opl_am_depth = a;
    opl_vib_depth = b;
    opl_note_select = c;
    opl_set_depth_flags();
    opl_set_note_select();
}
