/* game_funcs.h -- prototypes for every historical function this port must
 * provide: one per src/*.C function DEFINITION and per C-callable asm/*.ASM
 * PUBLIC symbol, EXCLUDING functions the portable service headers already
 * declare (gfx.h, resource.h, timer.h, input.h, sound.h, decode.h) and
 * functions whose entire historical file is replaced outright by a
 * hand-written portable subsystem (see the "excluded" sections of
 * docs/portable/funcs-inventory.md for the full rationale/evidence).
 *
 * GENERATED FILE -- do not edit by hand.  Regenerate with:
 *   python tools/portable/inventory/extract_prototypes.py
 *
 * Types follow docs/portable/tu-porting-rules.md sec 2 (dos_* aliases,
 * `T far *` -> `T *`, K&R implicit-int -> dos_int with a marker comment).
 * `struct NAME` types come from game_structs.h.
 */
#ifndef PORTABLE_GAME_FUNCS_H
#define PORTABLE_GAME_FUNCS_H

#include "dos_types.h"
#include "game_structs.h"

/* ---- provided by portable services; NOT redeclared here ----
 * See the "provided by services" table in docs/portable/funcs-inventory.md
 * for the full name -> header mapping (gfx.h, resource.h, timer.h, input.h,
 * sound.h, decode.h) and for the whole-file replacements (VIDEO.C,
 * RESOURCE.C, CRITERR.C, STARTUP.C, LIB_RAND.C) and single-function
 * replacements (PLAYERSL.C's ui_gfx_alloc).
 */

/* ---- historical struct tags used below but NOT covered by game_structs.h's
 * curated set (see docs/portable/funcs-inventory.md sec 7 for provenance) ---- */
struct E {  /* src/INTRO.C: locally-defined struct, not part of game_structs.h's curated set */
    dos_int f0;
    dos_int f2;
    dos_int f4;
    dos_int f6;
    dos_int f8;
    dos_int fa;
    dos_int fc;
    dos_int fe;
};
struct P {  /* src/INTRO.C: locally-defined struct, not part of game_structs.h's curated set */
    dos_int a;
    dos_int b;
};
struct input {  /* src/DIALOG.C: locally-defined struct, not part of game_structs.h's curated set */
    dos_char * title;
    dos_char flag;
    dos_char ** records;
    dos_char count;
    dos_int a;
    dos_int b;
    dos_int c;
    dos_int d;
};
struct piece_desc {  /* src/PUZZLE.C: locally-defined struct, not part of game_structs.h's curated set */
    dos_char a;
    dos_char b;
};

/* ==== src/ANIMFRAM.C ==== */
dos_int anim_frame_advance(dos_int n);

/* ==== src/ANIMSTEP.C ==== */
void anim_step_loop(dos_int a, dos_int b, dos_int c, dos_int d, dos_int e, dos_int f);

/* ==== src/BOARD.C ==== */
void resource_icon_table_load(void);
void resource_scoreboard_unpack(void);
void sprite_load_boot_sheets(void);
void resource_stripe_table_load(void);
void hud_arena_init(void);
void sprite_draw_cursor(void);
void board_redraw_view(void);
void board_scan_wipe_effect(dos_int i);
dos_int f250c(dos_int i);  /* K&R: implicit int */
dos_int point_in_hotspot_rect(dos_int x, dos_int y);
void board_update_moving_records(void);
void board_mark_record_cells(void);
void f2986(dos_uchar * p);
dos_char * record_field_skip_n(dos_int n);
dos_uchar * record_table_level4_ptr(void);
void board_redraw_paint(void);
void board_scroll_transition(void);
void board_record_index_select(void);
dos_int f32fa(dos_int a);  /* K&R: implicit int */
void board_run_unit_script(dos_uchar * s);
void board_advance_unit_moves(dos_int a);
void board_record_complete(void);

/* ==== src/DIALOG.C ==== */
dos_int dialog_draw_panel(struct gc0fe_record * p);  /* K&R: implicit int */
void menu_list_source_set_default(void);
void menu_list_source_set_players(void);
void dialog_layout(struct dialog * p);
void dialog_draw_button(dos_int n);
void dialog_fill_box(dos_int n);
void dialog_blink_box(dos_int a);
void dialog_draw_shadow(void);
void dialog_restore_screen(void);
void dialog_draw(struct dialog * p, dos_int first);
dos_int dialog_run(struct dialog * p);
dos_int dialog_list_run(struct input * p);

/* ==== src/DLGELOST.C ==== */
void dialog_energy_lost_show(void);

/* ==== src/FONT.C ==== */
void sprite_sheet_select(dos_int i);
dos_int sprite_sheet_index_get(void);
dos_int dialog_line_height_get(void);
dos_int text_line_width(dos_char * s);
void text_draw_wrapped(dos_int x, dos_int y, dos_char * s);

/* ==== src/FSTPCPY.C ==== */
dos_char * far_stpcpy_capped(dos_char * dst, dos_char * src);

/* ==== src/GAME.C ==== */
dos_int turn_loop_run(void);
void board_terrain_resources_load(dos_int di);
void menu_backdrop_paint(void);
dos_int level_driver_run(void);
void blitter_patch_variant(void);
void boot_init_seed_rand(void);
void campaign_chapter_advance(dos_int i);
void game_shutdown(void);
void game_run(void);

/* ==== src/HELPMENU.C ==== */
dos_int dialog_list_pick(dos_int a, dos_int n, dos_char * p);  /* K&R: implicit int */
dos_int help_topic_keyboard_show(void);
dos_int help_topic_playing_show(void);
dos_int help_topic_obstacles_show(void);
dos_int help_topic_puzzles_show(void);

/* ==== src/HINTDLG.C ==== */
void tutorial_hint_dialog_show(dos_int n);

/* ==== src/HITTEST.C ==== */
void cursor_trail_arm(void);
void board_raycast_step(void);
void sprite_slots_redraw(void);
dos_int board_raycast_hit_test(dos_int x, dos_int y);  /* K&R: implicit int */
void board_unit_script_trigger(void);

/* ==== src/HUD.C ==== */
dos_int hud_panel_clear(void);  /* implicit int */
void hud_icons_load(void);
void hud_panel_open(void);
void ui_overlay_show(void);
void ui_overlay_hide(void);
void ui_overlay_reset(void);
void hud_draw_meter(void);
void hud_scroll_reset(void);
dos_int hud_tab_get(void);
dos_int hud_tab_next(void);
void hud_tab_draw(void);
dos_int hud_scroll_move(dos_int n);
void energy_set(dos_int value);
dos_int energy_adjust(dos_int n);
void energy_draw(void);
void f7417(void);
void hud_panel_node_marker_draw(void);
void hud_frame_draw(void);

/* ==== src/HUDPMSG.C ==== */
void hud_prompt_message_draw(dos_char * p, dos_int a);

/* ==== src/INTRO.C ==== */
void intro_play_script(struct E * ev, dos_int count, dos_int step, struct P * q);
void intro_animate_step(struct E * ev, dos_int step, struct P * q, dos_int * idx, dos_ulong * when, dos_int dx0, dos_int dy0, dos_int * ph, dos_int * pi, dos_int * pj, dos_int * pk);
void splash_draw_and_clear(void);
dos_int intro_wait_key(void);
void resource_ptr_table_build(void);
void bitmap_blit_topleft(void);
void intro_title_picture_redisplay(void);
dos_int intro_run_chapter(void);

/* ==== src/KEYBOARD.C ==== */
void keyboard_irq_handler(void);

/* ==== src/LEVEL.C ==== */
dos_int menu_wait_key_animated(void);
dos_int fb09a(void);  /* implicit int */
void board_resource_expand(void);
void chapter_map_backdrop_draw(void);
void level_expand_descriptors(void);
dos_int level_actor_sprite_dims_init(void);  /* implicit int */
void menu_resources_free(void);
void level_free_descriptor_table(void);
dos_int chapter_map_sprites_wipe(void);  /* implicit int */
dos_int level_chapter_driver(void);  /* implicit int */
dos_int fb772(void);  /* implicit int */
void level_display_init(void);
void level_exit_transition_run(void);
dos_int level_run_loop(void);
dos_int level_play(void);  /* implicit int */
dos_int level_play_chapter(void);  /* implicit int */

/* ==== src/MARKERS.C ==== */
void score_panel_clear(void);
void roundend_draw_marker(void);

/* ==== src/MENULACT.C ==== */
dos_int menu_list_active(void);

/* ==== src/MENULDIS.C ==== */
void menu_list_disable(void);

/* ==== src/MENULEN.C ==== */
void menu_list_enable(void);

/* ==== src/MENULIST.C ==== */
void menu_list_draw(dos_int n);

/* ==== src/MENULOOP.C ==== */
void menu_loop_run(dos_int initial);

/* ==== src/MENULSRC.C ==== */
void menu_list_source_set(dos_char * p);

/* ==== src/MENURES.C ==== */
dos_int menu_resources_load(void);  /* implicit int */

/* ==== src/MRKCELL.C ==== */
void marker_cell_draw_highlight(void);
void marker_cell_draw_plain(void);

/* ==== src/MUSIC.C ==== */
dos_long music_note_to_divisor(dos_int a, dos_int b);
void music_build_octave_table(dos_uint * p, dos_int a, dos_int b);
void music_reset_tuning_tables(void);
void music_voice_frequency_lookup(dos_int i, dos_int val);

/* ==== src/NODEIDX.C ==== */
dos_int campaign_node_index(void);

/* ==== src/OPLINIT.C ==== */
void opl_init(void);
void opl_set_depth_wrapper(dos_int a);
void opl_set_enabled(dos_int f);
void music_set_tempo(dos_uint v);
void opl_set_depth_and_nts(dos_char a, dos_char b, dos_char c);

/* ==== src/OPLREG.C ==== */
void voice_set_field(dos_int v, dos_int k, dos_char val);
void voice_load_instrument(dos_int v, dos_char * q, dos_int w);
void voice_load_instrument_words(dos_int v, dos_char * q, dos_int w);
void voice_apply_field(dos_int u, dos_int k);
void voice_program_all(dos_int v);
void voice_write_level(dos_int i);
void opl_set_note_select(void);
void voice_write_connection(dos_int v);
void voice_write_op1_attack_decay(dos_int v);
void voice_write_op2_attack_decay(dos_int v);
void voice_write_envelope_flags(dos_int i);
void opl_set_depth_flags(void);
void voice_write_waveform(dos_int v);
void voice_set_frequency(dos_int v, dos_int note, dos_int flag);
void voice_key_off(dos_int v);

/* ==== src/OPLVOICE.C ==== */
void voice_bank_retune_on(dos_uint bank, dos_int base);
void voice_bank_update(dos_int bank);
void voice_update_tone(dos_int v);
void voice_level_table_reset(void);

/* ==== src/OPTIONS.C ==== */
dos_int options_toggle_option(void);
dos_int options_toggle_music(void);
dos_int options_toggle_sound(void);
dos_int slot_backup_list_show(void);
dos_int slot_list_show(void);

/* ==== src/PLAYERSL.C ==== */
void player_select_quit_confirm(void);
void player_select_menu_confirm(void);
void player_select_restart_confirm(void);
void player_select_mark(dos_int i);
dos_int player_select_draw_portraits(void);  /* implicit int */
dos_int player_select_draw_screen(void);  /* implicit int */
void player_select_load_flags(void);
void player_select_clear_highlight(void);
void player_select_draw_highlight(void);
dos_int player_select_choose_slot(void);  /* implicit int */
dos_int player_select_close_wipe(void);  /* implicit int */
dos_int player_select_run(dos_int a);  /* K&R: implicit int */

/* ==== src/PLRLDPUB.C ==== */
void player_record_load_publish(void);

/* ==== src/PROMPTS.C ==== */
void hud_prompt_continue_draw(void);
void hud_prompt_continue_clear(void);
dos_int hud_prompt_select_draw(dos_char * p);  /* K&R: implicit int */
void hud_prompt_message_run(dos_char * p);
dos_int hud_prompt_confirm_draw(dos_char * p, dos_int a, dos_int b, dos_int c, dos_int e);  /* K&R: implicit int */

/* ==== src/PUZZLE.C ==== */
void puzzle_clear_grid(void);
dos_int puzzle_deal_pieces(void);
dos_int puzzle_piece_count(void);  /* implicit int */
dos_int puzzle_run(void);
dos_int puzzle_display_init(void);  /* implicit int */
void puzzle_free_resources(void);
dos_int puzzle_draw_piece(struct piece_desc r, dos_int a, dos_int b);  /* K&R: implicit int */
dos_int puzzle_draw_tray_piece(dos_int i);  /* K&R: implicit int */
void f9402(dos_int i);
dos_int f9440(dos_int i);  /* K&R: implicit int */
dos_int puzzle_cell_backing_swap(dos_int a, dos_int b, dos_int c, dos_int d);  /* K&R: implicit int */
dos_int puzzle_cell_highlight_draw(dos_int a, dos_int b, dos_int c, dos_int d);  /* K&R: implicit int */
dos_int puzzle_clear_cell(dos_int a, dos_int b);  /* K&R: implicit int */
dos_int puzzle_check_solved(void);  /* implicit int */

/* ==== src/RECTTAB.C ==== */
void record_table_delete_compact(dos_int key);
dos_int rect_table_hit_id(dos_int x, dos_int y, dos_int w, dos_int h);
dos_int record_panel_rebuild(void);  /* implicit int */

/* ==== src/RESCACHE.C ==== */
void resource_record_cache_load(dos_int v);
void resource_record_cache_reset(dos_int n);
void music_resume_if_valid(void);

/* ==== src/ROUNDEND.C ==== */
void roundend_marker_show(dos_int n, dos_int j);
void roundend_flash_panel_icons(void);
void roundend_round_setup(dos_int n);
void roundend_wait(void);

/* ==== src/SCORE.C ==== */
void f9962(void);
void f99a2(void);
void score_set_position(dos_int i);

/* ==== src/SCOREPNL.C ==== */
dos_int score_panel_draw(void);  /* implicit int */

/* ==== src/SHBMPHIT.C ==== */
dos_int shadow_bitmap_hit_test(dos_int x, dos_int y, dos_int w);

/* ==== src/SLOTCOPY.C ==== */
void slot_archive_and_delete(void);
dos_int slot_reset_for_new_game(void);  /* implicit int */

/* ==== src/SLOTMENU.C ==== */
dos_int slot_list_draw(void);  /* implicit int */
void slot_cursor_box(dos_int x, dos_int y, dos_int c);
dos_int slot_input_wait_key(dos_int a, dos_int b);  /* K&R: implicit int */
dos_int player_name_edit(void);  /* implicit int */
dos_int player_type_select(void);
dos_int confirm_quit_dialog(void);  /* implicit int */
dos_int player_slot_add_run(void);
dos_int slot_list_select_loop(void);
dos_int slot_menu_run(void);
dos_int slot_is_new_game(void);
dos_char slot_flags_get(void);

/* ==== src/SLOTROW.C ==== */
dos_int slot_row_draw(struct c470_record * p, dos_int y, dos_int a);

/* ==== src/SLOTS.C ==== */
dos_int slot_menu_draw_header(void);  /* implicit int */
void slot_table_save(void);
void slot_row_highlight(dos_int n);
void player_type_toggle_draw(void);
void quit_confirm_toggle_draw(void);
dos_int slot_find_free(void);
void slot_delete(dos_int at);

/* ==== src/SNDFXTGL.C ==== */
void sound_effects_toggle(void);

/* ==== src/SNDREQ.C ==== */
void sound_start(void);
void sound_request_count_dec(void);
void sound_request_count_clear(void);

/* ==== src/SPRPOOLD.C ==== */
void sprite_pool_draw_masked(dos_char * dst, dos_int b, dos_uint mask);

/* ==== src/STRCATF.C ==== */
dos_long str_concat_far_list(dos_char * dest, dos_char * first, ...);

/* ==== src/TICKDIV.C ==== */
dos_int tick_div8(void);

/* ==== src/TIMER.C ==== */
void timer_irq_handler(void);

/* ==== src/VALPAR.C ==== */
dos_int value_parity(dos_int n);

/* ==== src/VOXCHAN.C ==== */
void voice_channel_set_gate(dos_int slot, dos_uint value);
void voice_channel_level_refresh(dos_int slot);
void voice_channel_apply_freq(dos_int slot);

/* ==== src/VOXSLOAD.C ==== */
dos_int voice_slot_load_pair(dos_int i, dos_char * a);  /* K&R: implicit int */

/* ==== asm/ANIMROW.ASM ==== */
void anim_step_row_copy(dos_int src_row_idx, dos_int dst_off, dos_int src_off, dos_int count, dos_int dst_row_idx, dos_int arg6, dos_int row_stride, dos_int arg8);  /* caller: src/ANIMSTEP.C (2 sites, `anim_step_row_copy(a/N,b,q/N,d,e/N,f,i,stride)`); asm-module-inventory.md sec 5 leaves args 6 and 8 (its own '?' entries) unnamed -- AMBIGUOUS, named arg6/arg8 here pending a Wave-2 disassembly cross-check */

/* ==== asm/BOARDCOL.ASM ==== */
dos_int board_collision_span_or(dos_int x, dos_int y, dos_int h);  /* callers: src/GAME.C, src/LEVEL.C */

/* ==== asm/DRAWQ.ASM ==== */
void draw_queue_render_highlighted(void);  /* caller: src/BOARD.C */
void draw_queue_render(void);  /* caller: src/GAME.C */

/* ==== asm/DRAWQBUF.ASM ==== */
void draw_queue_reset(void);  /* caller: src/BOARD.C */
void draw_queue_append(dos_char attr, dos_int x, dos_int y, dos_int color, dos_int height);  /* callers: src/BOARD.C (5 sites) and asm/SPRDRAW.ASM (ASM-to-ASM); signature taken from src/BOARD.C's own `extern void draw_queue_append(char, int, int, int, int);` (line 409), which resolves asm-module-inventory.md sec 8's own flagged ambiguity about the exact parameter count -- the routine's raw AX return ("address of the record's final word" per the ASM header) is never used through this C-visible `void` signature, also per BOARD.C's own extern decl */

/* ==== asm/ICONANIM.ASM ==== */
void icon_list_animate_draw(void);  /* caller: src/GAME.C */
void icon_frame_reset_and_draw(void);  /* no C caller (grep-confirmed); asm-module-inventory.md sec 6: a direct-branch shared tail of icon_list_animate_draw, public but not an independently reachable entry point */

/* ==== asm/RECTQ.ASM ==== */
void rect_queue_flush(void);  /* no C caller found (grep-confirmed); asm-module-inventory.md sec 1 flags it as possibly dead code, reachable only from other ASM (F_233E's cast animation) -- kept as a checklist entry, not called from any src/*.C site */

/* ==== asm/SOUND.ASM ==== */
/* REGISTER ABI -- see docs/portable/asm-module-inventory.md sec 9
 * (SI = voice index x2, ES:DI = loaded sound resource far pointer);
 * ASM-to-ASM only, no C caller, no C prototype possible: */
/*   sound_voice_pump_loop (F_C1F7): rescan the voice table in mode 2, dispatch one command per voice */
/*   sound_voice_table_prime (F_C232): (re)prime every configured voice's per-voice table entries */
/*   sound_voice_service_loop (F_C27D): advance each voice's counter, retrigger on note-off */
/*   sound_command_stream_dispatch (F_C2EA): split/dispatch the command byte at [si+voice_stream_cursor_table] */
/*   sound_control_value_select (F_C359): select and submit one value from the sound control state */
/*   sound_command_value_derive (F_C3DB): derive a command value from ES:DI state, update its selector */
/*   sound_control_block_advance (F_C440): advance three two-word slots in the DS:177Ch control-state block */
/*   sound_secondary_cmd_dispatch (F_C501): dispatch one secondary stream command by its AH selector */
/*   sound_command_flags_update (F_C549): update the paired command-control flags (v_hold/v_len) from AL */
/*   voice_percent_scale_store (F_C567): signed scale-and-store for the 1788/178A state pair */
/*   sound_param_scale4 (F_C59A): scale a raw 0..63 value by 4, store through g1788 */
/*   f_c5a8 (F_C5A8): store AL (zero-extended) through SI at voice_dur_table (17A4) */
/*   sound_table_word_select_store (F_C5B3): select a word from the 1832h table by AL, store into state_cursor (17C4) */
/*   f_c5c6 (F_C5C6): store AL (zero-extended) through SI at state_text (17DC) */
/*   voice_command_decode_apply (F_C5D1): decode one command byte, update the selected SI-relative note/frequency state */
/*   voice_enable (F_C678): enable a voice: open the speaker gate (mode 0/1) or queue a value for the OPL bank (mode 2) */
/*   voice_disable (F_C6B9): disable a voice, or submit an alternate-backend voice update */
/*   sound_pit_divisor_program (F_C706): write a PIT divisor (mode 0), queue it (mode 2), or emit packed OPL nibbles */
/*   sound_voices_reset_and_service (F_C755): combined reset+immediate-service helper */
/*   opl_port_write_byte (F_C8D4): write one raw byte to the OPL data port */
/*   sound_tick_step (F_C8E2): run one music-stream command via F_C914 when due, age the delay counter */
/*   sound_stream_command_step (F_C914): fetch the next command byte, split into nibble sub-dispatch */
/*   sound_note_dispatch (F_C988): look up a PIT divisor for (note, octave-shift) in notetab, program it */
/*   sound_stream_delay_decode (F_C9A4): decode one ES:[DI+1] command byte into a scaled delay, store into snd_delay */
/*   sound_ctlblock_command_dispatch (F_CA03): dispatch one control-block command by AH selector */
/*   sound_ctlblock_flags_latch (F_CA35): latch/clear the control block's one-shot/length pair from AL */
/*   stream_percent_scale_store (F_CA51): signed percentage scale-and-store -- the 1E84/1E86 twin of F_C567 */
/*   stream_base_value_set (F_CA83): scale a raw 0..63 value by 4, store through g1e84 */
/*   stream_note_delay_set (F_CA91): store AL (zero-extended) through stream_note_delay (1E92) */
/*   stream_note_program (F_CA9B): look up a divisor in notetab for a shifted note delta, program it */
/*   speaker_gate_on (F_CAD0): open the PC-speaker gate (timer-2 output + speaker enable bits, port 0x61) */
/*   speaker_gate_off (F_CADB): close the PC-speaker gate */
/*   pit_channel2_set_divisor (F_CAE6): write AX's low then high byte to PIT channel 2 (port 0x42) */

/* ==== asm/SPRDRAW.ASM ==== */
void sprite_table_queue_draws(void);  /* caller: src/BOARD.C */
void animated_tile_tick(void);  /* caller: src/GAME.C; internally not a Turbo-C-shaped routine (BP used as a data register, stack patched mid-body per asm-module-inventory.md sec 4) but its external interface is a plain 0-arg call */
void sprite_record_adjust_draw(dos_int id);  /* caller: src/BOARD.C */

/* ==== asm/SPRITES.ASM ==== */
void play_window_wipe_clipped(dos_int x1, dos_int y1, dos_int w, dos_int h);  /* no C caller (ASM-internal, called only from sprite_table_wipe_active) */
void sprite_script_frame_driver(void);  /* callers: src/GAME.C, src/LEVEL.C */
void sprite_table_wipe_active(void);  /* callers: src/GAME.C, src/LEVEL.C */
void board_actors_draw(dos_int y0);  /* callers: src/BOARD.C, src/GAME.C, src/LEVEL.C */

#endif /* PORTABLE_GAME_FUNCS_H */
