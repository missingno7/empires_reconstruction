/* level.c -- portable port of src/LEVEL.C: level play (display init,
 * movement loop, chapter driver).
 * One translation unit; the sections below were the separate member
 * sources of grouped module C_AF45_C15E and keep their original ids.
 *
 * generator sizing note: actor_record_table is generated as `dos_uchar[1]`
 * (portable/generated/game_state.h; docs/portable/state-map.md's alias
 * type conflict table shows the historical extern was only ever
 * `char g0b3ae[]`, undimensioned).  This file indexes it far past one byte
 * (as a flat struct-actor_rec array, `actor_record_table[i*32+7]` for i up
 * to 11) exactly as the historical code did -- reported to the supervisor
 * as a generator undersizing issue (the real object needs to be at least
 * ~360 bytes); using the generated symbol here reproduces the historical
 * indexing arithmetic unchanged (no semantics differ), but the DECLARED
 * bound of 1 byte is unsafe until the generator is fixed.
 */
#include "game.h"

/* ---- F_AF45 (original code at 0xAF45) ---- */
dos_int menu_wait_key_animated(void)
{
    gfx_wipe_rect(g13ef[0] * 0x12, 0x190, 0x12, 0x21, 0x04, 0x55);
    gfx_copy_rect_flip_h(0x04, 0x55, 0x12, 0x21, 0x12a, 0x55);
    gfx_wipe_rect(0x04, 0x55, 0x12, 0x21, 0x04, 0x11d);
    gfx_wipe_rect(0x12a, 0x55, 0x12, 0x21, 0x12a, 0x11d);
    gfx_box(0x04, 0x55, 0x12, 0x21);
    gfx_box(0x12a, 0x55, 0x12, 0x21);
    timer_deadline_arm(0x17);
    while (!keyboard_poll_nonblocking()) {
        if (timer_deadline_reached()) {
            if (++g13ef[0] >= 3)
                g13ef[0] = 0;
            gfx_wipe_rect(g13ef[0] * 0x12, 0x190, 0x12, 0x21, 0x04, 0x55);
            gfx_copy_rect_flip_h(0x04, 0x55, 0x12, 0x21, 0x12a, 0x55);
            gfx_box(0x04, 0x55, 0x12, 0x21);
            gfx_box(0x12a, 0x55, 0x12, 0x21);
            timer_deadline_arm(0x17);
        }
    }
    /* The historical `return;` after the call returns whatever AX held:
       the key just read (SLOTMENU.C switches on it). */
    return keyboard_read_blocking_hotkeys();
}

/* ---- F_B09A (original code at 0xB09A) ---- */
dos_int fb09a(void)
{
    resource_load_record(0x47);
    gfx_blit_bitmap(6, 200, ui_gfx_shadow_a);
    gfx_wipe_rect(6, 200, 0x134, 0x90, 6, 0x158);
    resource_load_record(0x48);
    g96 = 400;
    gfx_copy_rect(0x72, 0xd3, ui_gfx_shadow_a, 0);
    g96 = 0x9f;
    gfx_wipe_rect(6, 200, 0x134, 0x90, 6, 16);
    return 0;   /* PORT: value unused (K&R implicit int) */
}

/* ---- F_B122 (original code at 0xB122) ----
 * Expand the board descriptor resources into byte-addressed far pointers.
 */
void board_resource_expand(void)
{
    dos_int i, j;
    i = 0;
    resource_load_record(0x52); memmove(tile_width_table, ui_gfx_shadow_a, 0x54);
    resource_load_record(0x53); memmove(tile_height_table, ui_gfx_shadow_a, 0x54);
    resource_load_record_alloc(0x49, (uint8_t **)&gc5a8);
    for (j = 0; j < 12; j++) resource_ptr_table[i++] = gc5a8 + ((dos_int *)gc5a8)[j] + 2;
    resource_load_record_alloc(0x4a, (uint8_t **)&gc580);
    for (j = 0; j < 8; j++) resource_ptr_table[i++] = gc580 + ((dos_int *)gc580)[j] + 2;
    resource_load_record_alloc(0x4b, (uint8_t **)&gc5ac);
    for (j = 0; j < 10; j++) resource_ptr_table[i++] = gc5ac + ((dos_int *)gc5ac)[j] + 2;
    resource_load_record_alloc(0x4c, (uint8_t **)&gc59a);
    for (j = 0; j < 17; j++) resource_ptr_table[i++] = gc59a + ((dos_int *)gc59a)[j] + 2;
    resource_load_record_alloc(0x4d, (uint8_t **)&gc5a4);
    for (j = 0; j < 4; j++) resource_ptr_table[i++] = gc5a4 + ((dos_int *)gc5a4)[j] + 2;
    resource_load_record_alloc(0x4e, (uint8_t **)&gc58a);
    resource_ptr_table[i++] = gc58a;
    resource_load_record_alloc(0x4f, (uint8_t **)&gc58e);
    for (j = 0; j < 12; j++) resource_ptr_table[i++] = gc58e + ((dos_int *)gc58e)[j] + 2;
    resource_load_record_alloc(0x50, (uint8_t **)&gc584);
    for (j = 0; j < 4; j++) resource_ptr_table[i++] = gc584 + ((dos_int *)gc584)[j] + 2;
    resource_load_record_alloc(0x51, (uint8_t **)&gc59e);
    for (j = 0; j < 1; j++) resource_ptr_table[i++] = gc59e + ((dos_int *)gc59e)[j] + 2;
    while (i < 84) resource_ptr_table[i++] = resource_ptr_table[0];
    g720 = 2;
}

/* ---- F_B3D7 (original code at 0xB3D7) ---- */
void chapter_map_backdrop_draw(void)
{
    resource_load_record(84);
    gfx_blit_bitmap(0, 0, ui_gfx_shadow_a);
    gfx_wipe_rect(0, 0, 320, 200, 0, 200);
}

/* ---- F_B40F (original code at 0xB40F) ---- */
void level_expand_descriptors(void)
{
    dos_int i, j;
    i = 0;
    resource_load_record(0x57); memmove(tile_width_table, ui_gfx_shadow_a, 0x54);
    resource_load_record(0x58); memmove(tile_height_table, ui_gfx_shadow_a, 0x54);
    resource_load_record_alloc(0x55, (uint8_t **)&gc592);
    for (j = 0; j < 2; j++) resource_ptr_table[i++] = gc592 + ((dos_int *)gc592)[j] + 2;
    resource_load_record_alloc(0x56, (uint8_t **)&gc596);
    for (j = 0; j < 12; j++) resource_ptr_table[i++] = gc596 + ((dos_int *)gc596)[j] + 2;
    while (i < 84) resource_ptr_table[i++] = resource_ptr_table[0];
}

/* ---- F_B4FB (original code at 0xB4FB) ---- */
dos_int level_actor_sprite_dims_init(void)
{
    setmem(actor_sprite_dims_table, 672, 0);
    actor_sprite_dims_table[72] = 3;  actor_sprite_dims_table[73] = 9;
    actor_sprite_dims_table[74] = 13; actor_sprite_dims_table[75] = 17;
    actor_sprite_dims_table[76] = 6;  actor_sprite_dims_table[77] = 9;
    actor_sprite_dims_table[78] = 16; actor_sprite_dims_table[79] = 17;
    actor_sprite_dims_table[416] = 15; actor_sprite_dims_table[417] = 5;
    actor_sprite_dims_table[418] = 47; actor_sprite_dims_table[419] = 34;
    actor_sprite_dims_table[420] = 12; actor_sprite_dims_table[421] = 5;
    actor_sprite_dims_table[422] = 44; actor_sprite_dims_table[423] = 34;
    return 0;   /* PORT: value unused (K&R implicit int) */
}

/* ---- F_B55E (original code at 0xB55E) ---- */
void menu_resources_free(void)
{
    farfree(g7352); farfree(g7356); farfree(g735a); farfree(sprite_tile_bank);
}

/* ---- F_B593 (original code at 0xB593) ---- */
void level_free_descriptor_table(void)
{
    farfree(gc5a8);
    farfree(gc580);
    farfree(gc5ac);
    farfree(gc59a);
    farfree(gc5a4);
    farfree(gc58a);
    farfree(gc58e);
    farfree(gc584);
    farfree(gc59e);
    g720 = 0;
}

/* ---- F_B60F (original code at 0xB60F) ---- */
struct actor_rec { dos_char a, b; dos_int x, y; dos_char c, d, e; dos_char rest[23]; };

dos_int chapter_map_sprites_wipe(void)
{
    dos_int x, y;
    dos_int i, r, b;
    struct actor_rec *p;

    p = (struct actor_rec *)(actor_record_table + 1);
    for (i = 0; i < actor_record_table[0]; i++, p++) {
        if (!p->e && p->b == board_record_index) {
            r = (x = p->x) + tile_width_table[(dos_uchar)p->c] - 1;
            b = (y = p->y) + tile_height_table[(dos_uchar)p->c] - 1;
            if (x < 0) x = 0;
            if (y < 0) y = 0;
            if (r >= 0) gfx_wipe_rect(x, y + 200, r - x + 1, b - y + 1, x, y);
        }
    }
    return 0;   /* PORT: value unused (K&R implicit int) */
}

/* ---- F_B6CD (original code at 0xB6CD) ---- */
dos_int level_chapter_driver(void)
{
    dos_int i;
    menu_list_disable(); hud_panel_clear();
    board_record_index = gbc = raycast_trail_active = 0;
    g40ce = 1; g94 = g98 = 0; g96 = 0x1e8; g9a = 160;
    chapter_map_backdrop_draw(); level_expand_descriptors(); board_actors_draw(0);
    gfx_box(0, 0, 320, 200); gbc = 1;
    i = 0;
    do {
        timer_deadline_arm(24);
        rect_queue_write_ptr = ui_gfx_blob;
        chapter_map_sprites_wipe();
        sprite_script_frame_driver();
        if (rect_queue_write_ptr != ui_gfx_blob) rect_queue_flush();
        timer_deadline_wait();
        if (++i == 200) snd_flag2 = 0;
    } while (i < 200 || g1784);
    return 0;   /* PORT: value unused (K&R implicit int) */
}

/* ---- F_B772 (original code at 0xB772) ---- */
struct R_B772 { dos_char a, b; dos_int x, y; dos_char c; dos_char rest[25]; };

dos_int fb772(void)
{
    dos_int i, j;
    dos_int *q;
    dos_int n;

    gc588 = *(dos_uchar *)gb52f = 0;
    gc5a2 = 13;
    n = actor_record_table[0];
    j = n * 32 + 1;
    for (i = 5; i <= 11; i += 2) {
        if (actor_record_table[i * 32 + 1] == 0 && actor_record_table[i * 32 + 7] == 9) {
            q = (dos_int *)(actor_record_table + 14 + i * 32);
            q[0] = (i - 5) / 2 * 3 + j;
            q[2] = 0;
        }
    }
    return 0;   /* PORT: value unused (K&R implicit int) */
}

/* ---- F_B7F9 (original code at 0xB7F9) ---- */
void level_display_init(void)
{
    dos_int x, i;

    resource_load_record(0x48); gbc = 0;
    ((dos_uchar *)actor_state_table)[136] = 1;
    ((dos_uchar *)actor_state_table)[200] = 1;
    ((dos_uchar *)actor_state_table)[264] = 1;
    ((dos_uchar *)actor_state_table)[328] = 1;
    sound_stop_reset(); mus_flag = 1; stream_control_block_arm(25);
    for (x = 118; x <= 198; x += 4) {
        timer_deadline_arm(24);
        gfx_wipe_rect(x - 4, 355, 92, 117, x - 4, 27);
        gfx_copy_rect(x, 27, ui_gfx_shadow_a, 0);
        gb52f[33] += 4; gb52f[49] += 4; gb52f[65] += 4; gb52f[97] += 4; gb52f[129] += 4; gb52f[161] += 4;
        sprite_script_frame_driver(); sprite_draw_cursor(); gfx_box(x - 4, 27, 96, 117); timer_deadline_wait();
    }
    mus_flag = 0; sound_stop_reset(); sprite_draw_cursor();
    g96 = 488; gfx_copy_rect(x - 4, 355, ui_gfx_shadow_a, 0);
    gfx_wipe_rect(6, 344, 308, 144, 6, 200);
    g96 = 159; gbc = 1; gb6cf = 0; resource_record_cache_reset(69);
    for (i = 0; i < 20; i++) {
        timer_deadline_arm(24);
        rect_queue_write_ptr = ui_gfx_blob;
        sprite_table_wipe_active(); sprite_script_frame_driver(); sprite_draw_cursor();
        rect_queue_flush(); timer_deadline_wait();
    }
    for (i = 433; i <= 440; i++) board_records[i] = 7;
}

/* ---- F_B967 (original code at 0xB967) ---- */
void level_exit_transition_run(void)
{
    *board_records = 7;
    sprite_script_frame_driver();
    rect_queue_flush();
    while (!gb6cf) {
        timer_deadline_arm(24);
        rect_queue_write_ptr = ui_gfx_blob;
        sprite_table_wipe_active();
        sprite_script_frame_driver();
        rect_queue_flush();
        timer_deadline_wait();
    }
}

/* ---- F_B99F (original code at 0xB99F) ----
 * Board movement, jumping, collision response, and redraw loop.
 */
dos_int level_run_loop(void)
{
    dos_ulong deadline;
    struct gb3af_entry saved[4];
    dos_int key, hit, last, remaining, ended, delay, hurt, once;
    dos_int i, direction;

    gc5a2 = hurt = delay = ended = last = raycast_trail_active = 0;
    once = gc588 = gbc = 1;
    remaining = 2;
    for (i = 0; i < 4; i++) movmem(&actor_state_table[i * 2 + 4], &saved[i], 32);
    ((dos_uchar *)actor_state_table)[g1684[gc5a2] * 32] = 0;
    deadline = 0;
    for (;;) {
        timer_deadline_arm(24); g40ce = direction = key = 0;
        if (remaining) {
            if (((dos_uchar *)actor_state_table)[g1684[gc5a2] * 32]) {
                gc5a2++; if (g1684[gc5a2] < 0) gc5a2 = 0;
                ((dos_uchar *)actor_state_table)[g1684[gc5a2] * 32] = 0;
            }
        } else {
            if (!ended && !actor_state_table[24].flag) { ended = 1; delay = 20; }
            else if (delay && !--delay) {
                if (slot_is_new_game()) level_display_init();
                else {
                    gbc = 0; slot_reset_for_new_game(); snd_flag2 = 0; while (g1784) ; snd_flag2 = 1;
                    dialog_run(&g1670); longjmp(game_abort_jmpbuf, 1);
                }
            }
        }
        if (keyboard_poll_nonblocking()) {
            gbc = 0; key = keyboard_read_blocking_hotkeys();
            if (key == 13) hud_tab_next(); else if (key == 27) player_select_restart_confirm();
            gbc = 1;
        }
        rect_queue_write_ptr = ui_gfx_blob; board_redraw_view(); sprite_table_wipe_active();
        if (raycast_trail_active) sprite_slots_redraw();
        if (key_up_right_held) {
            cursor_facing_left = 0;
            if (!(board_collision_span_or(cursor_x + 28, cursor_y + 1, 39) & 7)) {
                cursor_x += g734; direction = 1;
                if (g72e <= 8) { if (++g72e > 8) g72e = 1; }
                if (gc588) {
                    for (i = 0; i < 4; i++) movmem(&saved[i], &actor_state_table[i * 2 + 4], 32);
                    fb772();
                }
            }
        } else if (key_up_left_held) {
            cursor_facing_left = 1;
            if (!(board_collision_span_or(cursor_x, cursor_y + 1, 39) & 7)) {
                cursor_x -= g734; direction = -1;
                if (g72e <= 8) { if (++g72e > 8) g72e = 1; }
                if (gc588) {
                    for (i = 0; i < 4; i++) movmem(&saved[i], &actor_state_table[i * 2 + 4], 32);
                    fb772();
                }
            }
        }
        if (g730) {
            if (!(shadow_bitmap_hit_test(cursor_x + 8, cursor_y - 1, 9) & 7)) {
                cursor_y -= g740[g730]; g730--; if (++g72e > 11) g72e = 11;
                if (!direction) g72e = 10;
            } else g730 = 0;
        } else if (!(shadow_bitmap_hit_test(cursor_x + 8, cursor_y + 47, 9) & 7)) {
            if (g732) cursor_y += 8; else { g732 = 1; cursor_y += 2; }
            g72e = (direction & 1) + 10;
        } else if (!(shadow_bitmap_hit_test(cursor_x + 8, cursor_y + 40, 9) & 7)) {
            cursor_y += ((cursor_y + 48) / 8) * 8 - (cursor_y + 40); g732 = 0;
            g72e = (direction & 1) + 10; stream_control_block_arm(11);
        } else if (key == 32) {
            if (hud_tab_get() == 1) {
                if (!(shadow_bitmap_hit_test(cursor_x + 8, cursor_y - 1, 9) & 7)) {
                    stream_control_block_arm(16); g730 = 8; g72e = 9; g734 = direction ? 8 : 4;
                } else goto walk;
            } else goto walk;
        } else if (key_up_held && key_up_released && !(shadow_bitmap_hit_test(cursor_x + 8, cursor_y - 1, 9) & 7)) {
            key_up_released = 0; g730 = 5; g72e = 9; g734 = direction ? 8 : 4; stream_control_block_arm(12);
        } else {
walk:
            if (direction) { if (g72e > 8) g72e = 1; } else g72e = 0;
            g734 = 4;
            if (cursor_y < 70 && cursor_x > 90 && cursor_x < 210) {
                level_exit_transition_run(); gbc = 0; return 1;
            }
        }
        if (key == 32) {
            if ((i = hud_tab_get()) == 2 && !hud_scroll_cooldown_ticks) {
                if (hud_scroll_move(-1) != -1) { hud_scroll_cooldown_ticks = 58; hurt = 0; stream_control_block_arm(0); }
                else stream_control_block_arm(17);
            } else if (!i) {
                if (!raycast_trail_active) { cursor_trail_arm(); stream_control_block_arm(20); }
                else stream_control_block_arm(23);
            }
        }
        if (hud_scroll_cooldown_ticks == 1) hud_scroll_cooldown_ticks = 0;
        sprite_script_frame_driver();
        if (once && !gc588 && actor_state_table[12].flag) {
            actor_state_table[12].rest[7] = 1; once = 0;
            g96 = 400; gfx_copy_rect(160, 274, (const uint8_t *)resource_ptr_table[29], 0); g96 = 159;
        }
        if (g40ce && last != g40ce) hit = actor_record_table[0] - g40ce; else hit = -1;
        last = g40ce;
        if (cursor_x < 8) cursor_x = 8; else if (cursor_x > 272) cursor_x = 272;
        if (raycast_trail_active) board_raycast_step();
        if (hurt) {
            hurt--;
            if (hurt > 26) gfx_copy_rect(cursor_x, cursor_y, (const uint8_t *)(resource_stripe_table + 14828), cursor_facing_left);
            else if (hurt & 1) gfx_copy_rect(cursor_x, cursor_y, (const uint8_t *)(resource_stripe_table + g72e * 674), cursor_facing_left);
        } else if (hud_scroll_cooldown_ticks) {
            gfx_copy_rect(cursor_x, cursor_y, (const uint8_t *)g96ee, 0); hud_scroll_cooldown_ticks--;
            gfx_copy_rect(cursor_x, cursor_y, (const uint8_t *)(resource_stripe_table + g72e * 674), cursor_facing_left);
        } else if (hit > 4 && hit <= 11) {
            gfx_copy_rect(cursor_x, cursor_y, (const uint8_t *)(resource_stripe_table + 14828), cursor_facing_left);
            hurt = 30; stream_control_block_arm(1); rect_queue_flush(); gbc = 0;
            if (energy_adjust(-2) <= 0) { board_record_complete(); return 0; }
            gbc = 1; goto frame_end;
        } else gfx_copy_rect(cursor_x, cursor_y, (const uint8_t *)(resource_stripe_table + g72e * 674), cursor_facing_left);
        if (hit == 13) {
            if (timer_ticks > deadline) {
                g730 = actor_state_table[13].flag = 0;
                if (--remaining < 0) remaining = 0;
                if (!remaining && !slot_is_new_game()) { rect_queue_flush(); resource_record_cache_reset(69); goto frame_end; }
            }
            deadline = timer_ticks + 500;
        }
        rect_queue_flush();
frame_end:
        timer_deadline_wait();
    }
    /* PORT: the historical source had an unreachable `gbc=0;return 1;`
     * epilogue after this point (the infinite for(;;) above only ever
     * exits via the returns inside it); dropped here since /W4 (C4702)
     * flags it and no code path can ever reach it, in the original or
     * the port. */
}

/* ---- F_C0E0 (original code at 0xC0E0) ---- */
dos_int level_play(void)
{
    dos_int r;
    gbc = raycast_trail_active = 0;
    board_records = (dos_char *)g43b4[board_record_index = 1].bytes;
    g40ce = 1;
    menu_resources_free();
    fb09a();
    board_resource_expand();
    level_actor_sprite_dims_init();
    board_actors_draw(0);
    cursor_x = 16; cursor_y = 112; cursor_facing_left = 0;
    sprite_draw_cursor();
    gfx_box(8, 16, 304, 144);
    resource_record_cache_reset(67);
    r = level_run_loop();
    level_free_descriptor_table();
    return r;
}

/* ---- F_C15E (original code at 0xC15E) ---- */
dos_int level_play_chapter(void)
{
    hud_scroll_reset();
    if (!slot_is_new_game()) hud_scroll_move(-3); else hud_scroll_move(-4);
    campaign_round_node_cursor = 42; g722 = 0;
    menu_backdrop_paint();
    if (!level_play()) return 0;
    level_chapter_driver();
    sound_voices_reset();
    return 1;
}
