/* score.c -- portable port of src/SCORE.C: score panel.
 * One translation unit; the sections below were the separate member
 * sources of grouped module C_9962_99E2 and keep their original ids.
 */
#include "game.h"

/* ---- F_9962 (original code at 0x9962) ---- */
void f9962(void)
{
    gfx_copy_rect_split_flip_v(4, 360, 32, 28, 80, 360);
    gfx_wipe_rect(80, 362, 30, 29, score_panel_x + 6, score_panel_y);
}

/* ---- F_99A2 (original code at 0x99A2) ---- */
void f99a2(void)
{
    gfx_copy_rect_split(4, 360, 32, 28, 80, 360);
    gfx_wipe_rect(80, 362, 30, 29, score_panel_x + 5, score_panel_y);
}

/* ---- F_99E2 (original code at 0x99E2) ---- */
/* generator type issue: g1271 (DS:1271, 48 bytes) is generated as
 * uint8_t[48] (portable/generated/game_data.h); the historical extern is
 * `struct score_pos { int a,b; } g1271[]` (12 records of 4 bytes).  The
 * byte span and per-record layout agree exactly (48 = 12 * 4), so this
 * reinterprets the generated flat buffer as the historical record array
 * without changing any indexing arithmetic (tu-porting-rules.md's "use the
 * generated one and adapt indexing" case, not a semantics change). */
struct score_pos { dos_int a, b; };

void score_set_position(dos_int i)
{
    const struct score_pos *recs = (const struct score_pos *)g1271;
    score_panel_x = recs[i].a;
    score_panel_y = recs[i].b;
}
