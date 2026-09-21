/* hudpmsg.c -- src/HUDPMSG.C: draw the "prompt" HUD panel (the small text
 * box near the bottom of the screen used for continue/select/confirm
 * prompts and free-form messages).
 *
 * hud_prompt_kind (historical name `gb83` -- docs/history/
 * naming-research-2026-09-21.md; DS offset not resolvable from any of the
 * 7 symbol sources) is defined by src/PROMPTS.C's `int hud_prompt_kind = 0;`
 * initializer and merely externed by src/HUD.C and this file; now declared
 * in game_data.h ("defined in portable/game/prompts.c"), so the local
 * forward-reference extern this file used to carry is no longer needed.
 */
#include "game.h"

/* F_? -- draw the prompt panel: blit the caller's message bitmap, clear the
 * response strip when a response is expected (a != 0), then frame the box. */
void hud_prompt_message_draw(dos_char *p, dos_int a)
{
    hud_prompt_kind = 5;
    gc0f2 = p;
    gc0fc = 1;
    gc0ec = a;
    gfx_blit_bitmap(6, 0xa2, (const uint8_t *)p);   /* PORT: dos_char* text/bitmap ptr cast to gfx.h's const uint8_t* */
    if (a) {
        gfx_color_select(1);
        gfx_clear_rect(13, 0xb5, 0x6a, 12);
    }
    gfx_box(6, 0xa2, 0x134, 0x24);
}
