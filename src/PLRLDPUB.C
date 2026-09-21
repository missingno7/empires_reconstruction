/* F_D555 -- allocate the 0x620-byte staging block, read record 0x41 into a
   second far block through F_684A, publish both addresses into the DS:175E
   table and repaint.  The four publications are FOUR WORD copies, not two far
   pointer copies: a far pointer assignment would have opened `les`, and the
   image opens `mov ax,[C5E0]`. */
extern char far *farmalloc();
extern void resource_load_record_alloc();
extern void sound_backend_select_init();
extern char far *gc5da;                 /* DS:C5DA offset, DS:C5DC segment */
extern unsigned gc5dc;                  /* DS:C5DC */
extern unsigned gc5de;                  /* DS:C5DE */
extern unsigned gc5e0;                  /* DS:C5E0 */
#include "SOUND.H"
extern unsigned g1760, g1764;           /* DS:1760, DS:1764 -- see SOUND.H's
                                            comment on why snd_seg/snd_seg2
                                            are not declared there */

void player_record_load_publish()
{
    gc5da = farmalloc(0x620L);
    resource_load_record_alloc(0x41, &gc5de);
    g1760 = gc5e0;
    snd_base = gc5de;
    g1764 = gc5dc;
    snd_base2 = (unsigned) gc5da;
    sound_backend_select_init();
}
