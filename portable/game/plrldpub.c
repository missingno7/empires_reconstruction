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

    /* PORT: snd_seg/snd_base historically held the segment (gc5e0) and
       offset (gc5de) halves of the far pointer resource_load_record_alloc
       just wrote.  Once far pointers flatten to real pointers there is no
       segment half; per the brief's PLRLDPUB.C note ("the port has snd_base
       as a real pointer") and docs/portable/state-map.md's "Segment-half
       aliases" section (the same no-portable-object pattern documented for
       gc5dc below), snd_seg is zeroed and snd_base carries the pointer's
       low 16 bits.  portable/include/sound.h already documents the whole
       snd_* interface as stub-only pending the Wave 4 sound driver, so this
       is a harmless placeholder, not a working handoff. */
    snd_seg = 0;
    snd_base = (dos_uint)(uintptr_t)resource_ptr;

    /* PORT: gc5dc (the historical segment half of gc5da, DS:C5DC) has no
       portable object at all -- state-map.md: "gc5dc: historical segment
       word of gc5da (DS:c5da) at DS:c5dc; no portable object" -- now that
       gc5da is a real dos_char* (portable/generated/game_state.h).  Same
       zero-segment/low-16-bits-of-pointer treatment as above. */
    snd_seg2 = 0;
    snd_base2 = (dos_uint)(uintptr_t)gc5da;

    sound_backend_select_init();
}
