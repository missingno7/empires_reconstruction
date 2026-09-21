/* sndfxtgl.c -- ported from src/SNDFXTGL.C (Ctrl+S sound-effects toggle).
 * No hardware fragments; calls the asm/SOUND.ASM C-facing entry points
 * (sound_stop_reset, sound_voices_reset -- sound.h, stubbed in
 * portable/audio/sound_stub.c per tu-porting-rules.md sec 5) and
 * music_resume_if_valid (src/RESCACHE.C's port, prototyped in
 * game_funcs.h).
 *
 * Replaces the placeholder sound_effects_toggle() stub that used to live in
 * portable/game/keyboard_stubs.c (removed in the same change as this file).
 *
 * tbl[n_sel] (include/TBL.H's struct tbl_entry, fields d13/f15 at byte
 * offsets +13/+15) is the historical name for the SAME DS:C470 storage
 * game_structs.h's struct c470_record already names slot_table[] with
 * (docs/portable/state-map.md's "Alias type conflicts" table); offsets
 * +13/+15 there are c470_record's own `sound`/`music` fields (see that
 * struct's field comments: "+0D copied to sound_enabled", "+0F copied to
 * music_enabled") -- exactly SNDFXTGL.C's d13/f15, so this file uses those
 * canonical field names instead of re-deriving tbl_entry's. */
#include "game.h"

void sound_effects_toggle(void)
{
    if (music_enabled) {
        sound_stop_reset(); sound_voices_reset();
        music_enabled = sound_enabled = 0;
        if (current_slot >= 0) slot_table[current_slot].music = slot_table[current_slot].sound = 0;
    } else {
        music_enabled = sound_enabled = 1;
        if (current_slot >= 0) slot_table[current_slot].music = slot_table[current_slot].sound = 1;
        music_resume_if_valid();
    }
}
