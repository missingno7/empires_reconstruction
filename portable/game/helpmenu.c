/* helpmenu.c -- portable port of src/HELPMENU.C: F1 help topic list dialog
 * and the four topic wrappers.
 * One translation unit; the sections below were the separate member
 * sources of grouped module C_D3DA_D49D and keep their original ids.
 */
#include "game.h"

/* ---- F_D3DA (original code at 0xD3DA) ----
 * RESOLVED: game_data.h now declares `extern struct input g22f0;`
 * (DS:22F0, 18 bytes, override-struct).  dlg_pick_desc's `.p`/`.n` fields
 * are struct input's `.title`/`.count` (see the porting-note cross-
 * reference this file originally shipped with); the pre-initialized
 * `.records` field (matching gc5ce) and the other fields are left as the
 * generator's static initializer set them, exactly as the historical code
 * only ever touched title/count itself.  gc5ce is now correctly generated
 * as `dos_char *gc5ce[3]` (game_state.h), so the offset-table expansion
 * loop below needs no adaptation either. */
dos_int dialog_list_pick(dos_int a, dos_int n, dos_char *p)
{
    dos_char *q;
    dos_int i, r;

    resource_load_record_alloc(64, (uint8_t **)&q);
    for (i = 0; i < n; i++)
        gc5ce[i] = q + ((dos_int *)q + a)[i] + 2;
    g22f0.title = p;
    g22f0.count = (dos_char)n;
    r = dialog_list_run(&g22f0);
    free(q);
    return r;
}

/* ---- F_D45C (original code at 0xD45C) ---- */
dos_int help_topic_keyboard_show(void)
{
    return dialog_list_pick(0, 2, g2302);
}

/* ---- F_D471 (original code at 0xD471) ---- */
dos_int help_topic_playing_show(void)
{
    return dialog_list_pick(2, 2, g2315);
}

/* ---- F_D487 (original code at 0xD487) ---- */
dos_int help_topic_obstacles_show(void)
{
    return dialog_list_pick(4, 2, g2326);
}

/* ---- F_D49D (original code at 0xD49D) ---- */
dos_int help_topic_puzzles_show(void)
{
    return dialog_list_pick(6, 3, g233b);
}
