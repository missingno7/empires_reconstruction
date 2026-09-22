/* src/PLRLDPUB.C: publish the loaded player/sound resource pointers. */
#include "game.h"

/* ---- F_D555 (original code at 0xD555) ---- */
/* F_D555 -- allocate the 0x620-byte staging block, read record 0x41 into a
   second far block through F_684A, publish both addresses into the DS:175E
   table and repaint.  The four publications are FOUR WORD copies, not two far
   pointer copies: a far pointer assignment would have opened `les`, and the
   image opens `mov ax,[C5E0]`. */
void player_record_load_publish()
{
    /* PORT: farmalloc -> malloc (tu-port-agent-brief.md / tu-porting-rules.md
       sec 6). */
    dos_uchar *resource_ptr;   /* PORT: stands in for the historical far
                                   pointer OUT param the old F_684A wrote as
                                   an offset:segment pair at &gc5de.  The
                                   ported resource_load_record_alloc
                                   (portable/include/resource.h) takes a
                                   single `uint8_t **`, and gc5de/gc5e0 are
                                   generated (portable/generated/game_data.h)
                                   as two independent dos_uint scalars, not a
                                   pointer -- they can no longer receive the
                                   call's result directly. */

    gc5da = malloc((size_t)0x620L);
    resource_load_record_alloc(0x41, &resource_ptr);

    /* PORT: snd_seg:snd_base and snd_seg2:snd_base2 historically held the
       far-pointer segment:offset pair resource_load_record_alloc's result
       and gc5da (the 0x620-byte staging block) lived at.  Once far
       pointers flatten to real pointers there is no segment half and no
       portable 16-bit offset can carry a 64-bit pointer's value either --
       those four historical words are left untouched at their generated
       zero default (never written here) and portable/audio/sound_driver.c
       (sound_set_resource_blocks()) owns the real pointers directly
       instead, exactly like every other far-pointer flattening in this
       tree (resource.h's own uint8_t** pattern). */
    sound_set_resource_blocks(resource_ptr, (uint8_t *)gc5da);

    sound_backend_select_init();
}
