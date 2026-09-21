# Proposed names for the 104 remaining address-named public functions

Research artifact only — no tracked file was edited. For each `_fXXXX` / `_f_XXXX`
public listed by the production plan, this proposes a behaviour-based C
identifier where the evidence supports it (own source banner/body, callers,
the data touched, and — for asm/SOUND.ASM — the OPL/PIT/speaker port and
voice-state comments already in the file). Where the only fact established
is a raw address ("writes DS:xxxx through SI") with no field name fixed
elsewhere, no name is proposed and the entry is marked LOW.

Confidence key: HIGH = the function's own banner/body states the behaviour
directly, or the naming follows a directly-matching sibling that is already
named. MEDIUM = behaviour inferred from code + call sites, mechanically
sound but some semantic detail (e.g. exact game-facing meaning) is not
nailed down. LOW = mechanical fact only; no name proposed.

---

## src/VIDEO.C (module C_01BC_0355)

| Address | Current | Proposed name | Conf. | Evidence |
|---|---|---|---|---|
| 0x020F | `_f020f` | `cur_color_index_get` | HIGH | `f020f(){return g3902;}` — `g3902` is the same storage as `cur_idx` (`extern int cur_idx; /* DS:3902 */`, `@SYM _cur_idx=0x3902`), the index `gfx_color_select` writes. Pure accessor. |
| 0x0232 | `_f0232` | `color_lookup_tables_init` | HIGH | `movmem(g9c,(char*)g3904,0x20); movmem(gde,(char*)gbe,0x20);` — copies the two 0x20-byte default tables into `g3904`/`gbe`, the exact arrays `gfx_color_select` indexes for mode 5 / mode 2. Called once from `video_alloc_framebuffer` before the palette load. `g9c`/`gde` have no other referents in the tree. |

## asm/F_1ECD.ASM, asm/F_1F91.ASM (standalone)

| Address | Current | Proposed name | Conf. | Evidence |
|---|---|---|---|---|
| 0x1ECD | `_f1ecd` | `rect_queue_flush` | HIGH | File's own header comment: drains the queued invalidation rectangles between `ui_gfx_blob` (read ptr) and `rect_queue_write_ptr` (write ptr), calling `gfx_box` per record, then resets the write pointer to the read pointer. |
| 0x1F91 | `_f1f91` | `board_collision_span_or` | HIGH | File's own header comment: ORs together the collision/attribute bytes of the 0x26-stride cells a clipped span covers in `board_records`, clamping x to 8..0x137 and y to 0x10..0x9F, and returns the accumulated byte. |

## src/BOARD.C (module C_200F_3986)

| Address | Current | Proposed name | Conf. | Evidence |
|---|---|---|---|---|
| 0x250C | `_f250c` | `board_record_start_move` | MEDIUM | `p=board_records+i*3+0x2ac` is the exact per-record flag byte `board_update_moving_records`/`board_mark_record_cells` iterate (`p=board_records+684`, i.e. `0x2ac`). Sets the low-nibble move counter to `6-a` and toggles direction bit `0x20` when a countdown is already running, else primes the counter to 6. Called from `board_run_unit_script`'s opcode dispatch (`f250c(c)` when the script byte has neither bit 0x10 nor 0x40 set) — i.e. it is the "kick off/continue this record's move" script opcode handler. |
| 0x2986 | `_f2986` | `board_record_draw_badge` | MEDIUM | Draws `gb1cc[0]` (base icon) at `(p[0]*2, p[1]+0xb8)` then, for `p[4]+5 <= i < 10` while `p[i]!=0`, draws `g893c[p[i]-1]` alongside it — a base sprite plus a row of up to 5 value/count icons, with the z-order variable `g00bc` cleared then restored around the draws. Exact icon semantics (what the 1..7 values represent) are not established. |
| 0x2A70 | `_f2a70` | `record_table_level4_ptr` | HIGH | Existing banner: "Walk the nested count-prefixed tables at `record_table_root` and return the near address of the fourth level." |
| 0x329F | `_f329f` | `board_record_index_select` | HIGH | `board_records=(char far*)&g43b4[board_record_index]; board_redraw_paint(); ...; f4eeb(0);` — re-points `board_records` at the slot named by `board_record_index` and repaints, i.e. switches the active board. |
| 0x32FA | `_f32fa` | `board_record_cell_flag_toggle` | MEDIUM | `p=record_table_root+a*4+1; p[3]=(p[3]+4)&7; n=p[2]+2; ...; board_records[j]^=16;` for `n` cells starting at the record's board position — toggles bit `0x10` across a run of board cells and advances the record's own 3-bit phase field. Called from `board_run_unit_script` for opcode bit `0x10` (highlight/selection-style toggle over a run of cells; exact visual meaning not established). |
| 0x3986 | `_f3986` | `board_record_complete` | MEDIUM-HIGH | Saves `board_record_index` to `g73e`, plays a zoom/reveal animation (`anim_step_loop`) around the current cursor rect, then `slot_table[current_slot].value++`, `energy_set(slot_table[current_slot].state = 4)`, and offers `confirm_quit_dialog()` (aborting via `longjmp` on yes). This is the "board record finished" handler: awards the slot's score, marks its state, plays the finish animation, then offers to quit. |

## asm/F_4AA8.ASM, F_4B0C.ASM, F_4E9F.ASM, src/F_4EEB.ASM (standalone)

| Address | Current | Proposed name | Conf. | Evidence |
|---|---|---|---|---|
| 0x4AA8 | `_f4aa8` | `play_window_wipe_clipped` | HIGH | Own banner: clip a rectangle to the play window (8..0x137, 0x10..0x9F) and hand the clipped rect to `wipe()`/`f03B4`, y biased by 0xB8. |
| 0x4B0C | `_f4b0c` | `sprite_script_frame_driver` | HIGH | Own banner: "the 'sprite script' bytecode interpreter's per-frame driver" — walks the 0x20-byte record table at `gB3AE`, renders/ticks/resumes each active sprite's bytecode program. |
| 0x4E9F | `_f4e9f` | `sprite_table_wipe_active` | HIGH | Own banner: walks the same `gB3AE` record table and, for records on the current board (`+1==gBFBA`) not already rendered (`+8!=1`), calls `f4AA8` to clip+wipe the record's rect — the "erase before redraw" companion to `f4b0c`. |
| 0x4EEB | `_f4eeb` | `board_actors_draw` | HIGH | Own banner (restated for the asm recovery of `F_4EEB.C`): walks `gb3ae`, and for every record matching `board_record_index` and not flagged `+8==1`, blits its sprite via `gfx_copy_rect` using `resource_ptr_table[record byte+6]`, with an extra `gfx_vline` strip when byte `+0x1A` is set. The main "draw all active actors for the current board" pass. |

## src/STARTUP.C (module C_4F63_520A)

| Address | Current | Proposed name | Conf. | Evidence |
|---|---|---|---|---|
| 0x50D2 | `f50d2` | `video_adapter_detect` | HIGH | Own banner: "F_50D2 video adapter detection ... (formerly src/VIDDET.C)". Body probes INT 10h/15h/EGA-RAM signatures and writes `display_mode`. |
| 0x53BF | `f53bf` | `sound_backend_probe` | HIGH | Own banner: "F_53BF sound-hardware probe". Sets `g1778` (the same selector `F_C678`/`F_C706` in SOUND.ASM branch on: 0=PC speaker, 1/2/3 = alternate backends) via `opl_detect()` and an INT 15h/port-0x203 hardware probe. |

## src/INTRO.C (module C_5321_56C6)

| Address | Current | Proposed name | Conf. | Evidence |
|---|---|---|---|---|
| 0x568C | `f568c` | `intro_slide_redisplay` | MEDIUM | `f7676(); anim_step_loop(0,232,200,152,0,32); bitmap_blit_topleft(); hud_prompt_continue_draw(); gfx_box(...)` — called only when the two-page intro caption loop times out with no keypress (`k==2 && key==-1`); clears the continue prompt, wipes/reveals the main picture area, redraws the intro bitmap and prompt, then reframes. Effectively "redisplay the current intro slide after an idle timeout". |

## src/RESOURCE.C (module C_6266_68AA)

| Address | Current | Proposed name | Conf. | Evidence |
|---|---|---|---|---|
| 0x6771 | `f6771` | `resource_sprite_sheet_decode_sequential` | HIGH | Called from the resource-load dispatch when the loaded record's kind byte `gc0cb==0`: walks concatenated 0x47-tagged sprite records back-to-back (stride `q[34]*q[35]+36`), decoding each in place via `f6f4b`/`f6eff` (the two 4bpp decoders). Sibling of the `gc0cb==0x47` single-record case and the `gc0cb==1` indexed case below. |
| 0x67DC | `f67dc` | `resource_sprite_sheet_decode_indexed` | HIGH | Called when `gc0cb==1`: walks a leading offset table (`t[i]`, count `(*t>>1)-1`) to locate each 0x47-tagged record and decode it via `f6f4b`/`f6eff` — the indexed-table sibling of `f6771`. |

## src/KEYIRQ.C (module C_695E_697D)

| Address | Current | Proposed name | Conf. | Evidence |
|---|---|---|---|---|
| 0x697D | `f697d` | `keyboard_irq_restore` | HIGH | Own comment: "Restore the INT 9 vector saved by `keyboard_irq_install`." Body: `setvect(9, int9_saved_vector)`, the exact undo of `keyboard_irq_install` (F_695E). |

## src/KEYBOARD.C (module C_6990_6B74)

| Address | Current | Proposed name | Conf. | Evidence |
|---|---|---|---|---|
| 0x6B1A | `f6b1a` | `keyboard_read_blocking_hotkeys` | HIGH | Own banner: "blocking INT 16h read with the F1..F10 hot-key check." |
| 0x6B4A | `f6b4a` | `keyboard_poll_nonblocking` | HIGH | Own banner: "non-blocking INT 16h keyboard poll." |

## asm/M_6D86_6DCC.ASM (standalone)

| Address | Current | Proposed name | Conf. | Evidence |
|---|---|---|---|---|
| 0x6D86 | `_f6d86` | `rle_packbits_decode` | HIGH | Own banner: "PackBits-style RLE expansion" — signed control bytes select run vs literal copy, returns bytes written. |
| 0x6DCC | `_f6dcc` | `lz_decompress` | HIGH | Own banner: "LZ-style decompressor (variable-width codes, growing from 9 bits...)" with a back-reference string table following the header. |

## asm/F_6EFF.ASM, F_6F4B.ASM (standalone)

| Address | Current | Proposed name | Conf. | Evidence |
|---|---|---|---|---|
| 0x6EFF | `_f6eff` | `sprite_decode_4bpp_planar` | HIGH | Own banner: decode a packed 4-bit-per-pixel sprite/tile record in place, the `display_mode!=2` (EGA/VGA planar) sibling of F_6F4B. |
| 0x6F4B | `_f6f4b` | `sprite_decode_4bpp_mode13h` | HIGH | Own banner: the `display_mode==2` (mode-13h) sibling of F_6EFF, remapping the palette bytes to mode-13h form before nibble expansion. |

## src/HUD.C (module C_6FC3_747B)

| Address | Current | Proposed name | Conf. | Evidence |
|---|---|---|---|---|
| 0x7417 | `f7417` | `hud_panel_animated_icon_draw` | MEDIUM | `gfx_blit_bitmap(244,175, ...->offsets[tick_div8()]+2)` — blits one frame of a tick-cycling (8-tick) icon strip at a fixed HUD position; called from the panel-open sequence (`F_6FDA`) alongside `f7443`. Exact icon subject not established. |
| 0x7443 | `f7443` | `hud_panel_node_marker_draw` | MEDIUM-HIGH | `i=campaign_node_index(); gfx_blit_bitmap(244+i*16,186, ...->offsets[i]+2)` — blits the icon for the current campaign node at a position offset by 16px per node, i.e. marks the current node along a row of node icons in the HUD. |

## src/PROMPTS.C (module C_75F3_7856)

| Address | Current | Proposed name | Conf. | Evidence |
|---|---|---|---|---|
| 0x7676 | `f7676` | `hud_prompt_continue_clear` | HIGH | `anim_step_loop(0x18,0x184,0x94,10,0x18,0xbc)` targets exactly the rect `hud_prompt_continue_draw` boxes (`0x18,0xbc,0x94,0xa`) — the reveal-wipe counterpart that erases the "... to Continue" prompt. |
| 0x7747 | `f7747` | `hud_prompt_message_run` | HIGH | `saved=menu_list_active(); menu_list_disable(); hud_prompt_confirm_draw(p,0,15,1,0); do key=f6b1a(); while(key!=13&&key!=27); if(saved) menu_list_enable(); hud_panel_open();` — draws a confirm-style message box and blocks until Enter/Esc, the simple one-button message-box wrapper around `hud_prompt_confirm_draw`. |

## src/DIALOG.C (module C_7D91_880A)

| Address | Current | Proposed name | Conf. | Evidence |
|---|---|---|---|---|
| 0x7DF1 | `f7df1` | `menu_list_source_set_default` | HIGH | `menu_list_source_set(g0d36)`; called from `GAME.C`'s level driver and again at the end of `player_select_run` (`PLAYERSL.C`) to put the menu list source back after `f7dfc` (below) temporarily redirected it — the "restore default" half of the pair. |
| 0x7DFC | `f7dfc` | `menu_list_source_set_player_select` | HIGH | `menu_list_source_set(g0d78)`; called only at the start of `player_select_run`, matched by `f7df1()` at that function's end — the player-select screen's menu-list source. |

## src/PUZZLE.C (module C_8A37_969D)

| Address | Current | Proposed name | Conf. | Evidence |
|---|---|---|---|---|
| 0x9402 | `f9402` | `puzzle_hint_message_show` | MEDIUM | `hud_prompt_message_draw(g0dc8+g0dc8[i]+2, i&&!face7())` — shows one of the cached instruction strings selected by `i` (0/1, matching whether the player is currently holding a piece); called at puzzle entry and after a piece is placed/picked up. |
| 0x9440 | `f9440` | `puzzle_stage_message_show` | MEDIUM | `hud_prompt_confirm_draw(gc136[i],0,11,1,1)` — shows the cached stage-indexed confirm message (`gc136[0]`/`gc136[1]`, loaded from the level's two staged text blocks); called for the intro stage (`++stage<2`) and again once `puzzle_check_solved()` succeeds. |
| 0x9466 | `f9466` | `puzzle_cell_backing_swap` | MEDIUM-HIGH | Computes a grid or tray cell's screen rect from `(a,b)` and `gfx_wipe_rect`s it to/from one of two fixed off-screen buffers selected by `c`, direction selected by `d` — a save/restore of a cell's background image, called in matched pairs around piece pick-up/placement and the solved-flash loop. |
| 0x950C | `f950c` | `puzzle_cell_highlight_draw` | HIGH | Draws a `rect_border_draw` frame around a grid/tray cell (thicker when `c` is set) plus `gfx_vline`/`gfx_bar` marks when `d` is set — the selection/flash highlight drawn around the cursor cell throughout the piece pick-up, hold and solved-flash logic. |

## src/SCORE.C (module C_9962_99E2)

| Address | Current | Proposed name | Conf. | Evidence |
|---|---|---|---|---|
| 0x9962 | `f9962` | `score_panel_frame_draw_flipped` | MEDIUM | `gfx_copy_rect_split_flip_v(4,360,32,28,80,360)` (the bottom-to-top row variant per `VIDEO.H`'s comment on `F_03C3`) then a shifted wipe near `score_panel_x/y`. No call site found in the current tree (likely reached through a table); exact animation phase (open vs. close of the score reel) not established. |
| 0x99A2 | `f99a2` | `score_panel_frame_draw` | MEDIUM | Same shape as `f9962` but with the plain (top-to-bottom) `gfx_copy_rect_split` and a 1px-different wipe offset — its non-flipped sibling. |

## asm/F_9EC3.ASM (standalone)

| Address | Current | Proposed name | Conf. | Evidence |
|---|---|---|---|---|
| 0x9EC3 | `_f9ec3` | `anim_step_row_copy` | HIGH | Own banner: "copy a clipped row through indexed source and destination tables." Both src/dst rows are looked up in `g3924[]` (the per-row far-pointer table `video_alloc_framebuffer` fills); its only caller is `ANIMSTEP.C`'s `anim_step_loop` (`F_9F40`, "0x10-step animation loop"), passed the mode-dependent row width (0x140/0x50/0xa0, matching `VIDEO.C`'s `w`) — the row-blit primitive behind the wipe/reveal transition. |

## src/SLOTS.C (module C_A09D_A24E)

| Address | Current | Proposed name | Conf. | Evidence |
|---|---|---|---|---|
| 0xA19D | `fa19d` | `player_type_toggle_draw` | HIGH | `gfx_fill_rect(40,126,238,10)` x2 + `gfx_box(40,126,238,20)` is exactly the two-row box `player_type_select` (`SLOTMENU.C`) draws at start (`fill(0x28,0x7e,0xee,0xa); box(0x28,0x7e,0xee,0x14)`); called there as `sel^=0x30; fa19d();` — redraws the Human/Computer toggle highlight after flipping the selection. |
| 0xA1E0 | `fa1e0` | `quit_confirm_toggle_draw` | HIGH | `gfx_fill_rect(50,103,218,10)` x2 + `gfx_box(50,103,218,20)` matches `confirm_quit_dialog`'s Yes/No box (`gfx_fill_rect(50,95... /103,218,10)`); called there as `i^=1; fa1e0();` — redraws the Yes/No toggle highlight. |

## src/SLOTMENU.C (module C_A33F_AD0E)

| Address | Current | Proposed name | Conf. | Evidence |
|---|---|---|---|---|
| 0xAA1F | `faa1f` | `slot_list_select_loop` | HIGH | Navigates the save-slot list with arrow keys (`faf45()`), Enter to choose or (in delete mode) confirm-delete via `dialog_slot_delete_confirm`, Esc to cancel — the save-slot picker's input loop, called from `fab66`. |
| 0xAB66 | `fab66` | `slot_menu_run` | HIGH | Top-level save-slot menu driver: draws the header/backdrop, loops calling `faa1f`/`player_slot_add_run` until a slot is chosen, then applies `sound_enabled`/`music_enabled` from the chosen record and returns it as `current_slot`. |
| 0xACE7 | `face7` | `slot_is_new_game` | MEDIUM | `return (slot_table[current_slot].flags & 0x20) == 0x20;` — bit 0x20 is set exclusively by `fadcf` when a slot is reset for a fresh game (see SLOTCOPY.C below), and is read by `PUZZLE.C` to pick the tutorial-style hint/resource variant. `C470.H` itself only documents the bit mechanically ("bit 0x20 tested by F_ACE7 and set by F_ADCF"), so the "new game" reading is an inference from the set/test sites, not a recovered name. |
| 0xAD0E | `fad0e` | `slot_flags_get` | HIGH | `return slot_table[current_slot].flags;` — plain accessor for the field `C470.H` documents as `flags`. |

## src/SLOTCOPY.C (module RELOC_F_AD25_F_ADCF)

| Address | Current | Proposed name | Conf. | Evidence |
|---|---|---|---|---|
| 0xAD25 | `fad25` | `slot_archive_and_delete` | HIGH | Archives the current live slot into the first free (or oldest-evicted) `gc360` transfer-copy slot, then `slot_delete(current_slot); slot_table_save();` — matches `C470.H`'s "the ten transfer copies" comment on `gc360`. |
| 0xADCF | `fadcf` | `slot_reset_for_new_game` | HIGH | Same archive-into-`gc360` step as `fad25`, then resets the current slot's fields for a fresh game: `flags=32, state=4, value=1, option=0, resume_round=0`, clears `round_progress[0..3]` and `byte26`, then `slot_table_save()`. |

## src/LEVEL.C (module C_AF45_C15E)

| Address | Current | Proposed name | Conf. | Evidence |
|---|---|---|---|---|
| 0xAF45 | `faf45` | `menu_wait_key_animated` | HIGH | Draws a two-box, 3-frame spinning icon (mirrored via `gfx_copy_rect_flip_h`) and animates it every tick until a key is available, then reads and returns it via `f6b1a()`. Used throughout `SLOTMENU.C` (`faa1f`, `player_type_select`) as the generic "wait for a menu key" call. |
| 0xB09A | `fb09a` | `level_frame_reveal_draw` | MEDIUM | Loads record 0x47, blits it into `(6,200)` then wipes it upward to `(6,0x158)`; loads record 0x48 and copies a highlighted (`g96=400`) sub-image at `(0x72,0xd3)`; wipes the whole area up to `(6,16)`. Called once at the very start of `level_play()` (after freeing the prior menu resources) — the level's opening backdrop/frame draw-and-reveal. |
| 0xB3D7 | `fb3d7` | `chapter_map_backdrop_draw` | MEDIUM-HIGH | `resource_load_record(84); gfx_blit_bitmap(0,0,...); gfx_wipe_rect(0,0,320,200,0,200);` — a full-screen backdrop reveal, called only from `fb6cd` (the post-level chapter/map driver, below). |
| 0xB4FB | `fb4fb` | `level_tile_data_init` | MEDIUM | Zeroes the 672-byte `g40d4` table then stamps two 8-byte marker groups at fixed offsets (72, 416). `g40d4` is the same buffer `MENURES.C`'s `menu_resources_load` normally fills from resource record 25 (also 672 bytes) for the animated-tile system; this is the level-specific override of that data, called right after `board_resource_expand()` in `level_play()`. Exact meaning of the marker values not established. |
| 0xB55E | `fb55e` | `menu_resources_free` | HIGH | `farfree(g7352); farfree(g7356); farfree(g735a); farfree(gbfc8);` frees exactly the four far pointers `MENURES.C`'s `menu_resources_load` allocates (records 20/21/22/19) — its direct counterpart, called at the start of `level_play()`. |
| 0xB60F | `fb60f` | `chapter_map_sprites_wipe` | MEDIUM-HIGH | Walks the same 32-byte-stride `gb3ae` record table as `F_4E9F`/`F_4B0C`, and for every record matching `board_record_index` with flag `e==0`, wipes its rect using the `g9bfc`/`gbf66` width/height tables — the erase-before-redraw pass for the chapter/world-map driver `fb6cd`, paired there with `f4b0c()`. |
| 0xB6CD | `fb6cd` | `level_chapter_driver` | HIGH | Matches the file banner's own description ("...and chapter driver"): resets board/animation state, draws the chapter backdrop (`fb3d7`), expands descriptors, then loops `fb60f()`+`f4b0c()` (erase+draw) for up to 200 ticks or while `g1784` stays set — the post-level chapter/map screen. |
| 0xB772 | `fb772` | `level_actor_row_init` | LOW-MEDIUM | Recomputes the `x`/`y` fields (offset +14/+16) of a fixed subset of `gb3ae` records (indices 5,7,9,11) whose flags match a template, positioning them along a computed row (`(i-5)/2*3+j`). Called only when `gc588` is set, immediately after `movmem`-ing template records into `gb3af` — mechanically a "lay out a fresh row of actor records" step at a room/screen transition; the game-facing purpose (what these records represent) is not established. |
| 0xB967 | `fb967` | `level_exit_transition_run` | MEDIUM-HIGH | `*board_records=7; f4b0c(); f1ecd(); while(!gb6cf) { ...; f4e9f(); f4b0c(); f1ecd(); ...}` — called from the movement loop exactly when the player reaches the level's exit region (`return 1` follows), marking the first board record with the same "occupied/consumed" value (7) `BOARD.C` uses elsewhere, then looping the draw cycle until the `gb6cf` completion flag is set — the level-exit-reached animation. |

## asm/M_D386_D3CF.ASM (standalone)

| Address | Current | Proposed name | Conf. | Evidence |
|---|---|---|---|---|
| 0xD386 | `_fd386` | `icon_list_animate_draw` | HIGH | Own module banner: "decode a far record list and handle its negative selector path." Walks the far record list at `gbfc4`; each record carries a per-icon frame countdown byte, decremented per call; on expiry falls through to `F_D3CF` to rearm it; otherwise looks the current frame up in `a72b2` and blits it via `gfx_blit_bitmap`. |
| 0xD3CF | `_fd3cf` | `icon_frame_reset_and_draw` | HIGH | Own module banner: "F_D3CF is the direct branch continuation of F_D386, not an independent flow." Rearms the expired icon's frame counter to 1 and falls into the same lookup/blit as `F_D386`. |

## src/F_D45C.C, F_D471.C, F_D487.C, F_D49D.C (standalone)

| Address | Current | Proposed name | Conf. | Evidence |
|---|---|---|---|---|
| 0xD45C | `fd45c` | — | LOW | `return dialog_list_pick(0,2,g2302);` — confirmed mechanically as a fixed-parameter wrapper around the shared `dialog_list_pick(startIndex,count,textTable)` (itself loading choice strings out of the shared record-64 table via `DLGPICK.C`). No caller exists in the current source tree (reached only through a data/menu table not in scope here), so which settings screen this instance serves is not established. |
| 0xD471 | `fd471` | — | LOW | `return dialog_list_pick(2,2,g2315);` — same pattern as `fd45c`, next slice of the shared choice table. Same caller gap. |
| 0xD487 | `fd487` | — | LOW | `return dialog_list_pick(4,2,g2326);` — same pattern, next slice. Same caller gap. |
| 0xD49D | `fd49d` | — | LOW | `return dialog_list_pick(6,3,g233b);` — same pattern with a 3-way choice. Same caller gap. |

## asm/M_D61C_D79C.ASM (standalone)

| Address | Current | Proposed name | Conf. | Evidence |
|---|---|---|---|---|
| 0xD61C | `_f_d61c` | `draw_queue_render_highlighted` | HIGH | Own comment: "apply bright attributes, render all records, then restore the normal attributes" — sets the 24-cell board lattice to a bright attribute, renders the draw-command queue (see F_D818/F_D825 below), then restores. |
| 0xD79C | `_f_d79c` | `draw_queue_render` | HIGH | Own comment: "the same record renderer without lattice changes" — walks the same ES:DI command list `F_D61C` renders, without the lattice highlight. |

## asm/M_D818_D825.ASM (standalone)

| Address | Current | Proposed name | Conf. | Evidence |
|---|---|---|---|---|
| 0xD818 | `_fd818` | `draw_queue_reset` | HIGH | Own comment: "reset the DS:2F30h byte counter" — zeroes the record-count byte of the compact draw-command queue at DS:2F30h. |
| 0xD825 | `_fd825` | `draw_queue_append` | HIGH | Own comment: "append one packed five-byte record at DS:2F30h" — increments the count, writes the caller's byte plus two packed words, returns the new record's address; the append half of the queue `F_D61C`/`F_D79C` render. |

## src/MUSIC.C (module M_DDD9_DF98)

| Address | Current | Proposed name | Conf. | Evidence |
|---|---|---|---|---|
| 0xDF98 | `fdf98` | `music_voice_frequency_lookup` | HIGH | File's own top-of-unit banner names it directly: "...and the cached voice-frequency lookup (fdf98)." Body computes a pitch-bend-scaled key from `val`/`music_tempo_scaled`, reuses the cached `g3752`/`gc5e4`/`gc5e6` triple when the key repeats, else recomputes from the `gc6c3` tuning table built by `music_reset_tuning_tables`. |

---

## asm/SOUND.ASM (module M_C1A0_CB48) — the PC-speaker/OPL sound driver

Every one of these already carries a one-to-two line behavioural banner in
the file itself; names below follow those banners directly (HIGH unless
noted) and the shared naming already established for the module
(`sound_stop_reset`, `sound_start`, voice enable/disable, etc.).

| Address | Current | Proposed name | Conf. | Evidence (own banner, abridged) |
|---|---|---|---|---|
| 0xC1A0 | `_fc1a0` | `sound_tick_entry` | HIGH | "sound-tick entry point: save the caller's registers, drive one sound_tick_step, then pump ES:_snd_seg2 voices unless newly paused." |
| 0xC1F7 | `_fc1f7` | `sound_voice_pump_loop` | HIGH | "pump loop: rescan the voice table in mode 2, dispatch one command batch, then optionally freeze the just-read state into _dv." |
| 0xC232 | `_fc232` | `sound_voice_table_prime` | HIGH | "voice-table scanner: (re)prime every configured voice's SI-relative control slots from the ES:[si+_g178c] entry table." |
| 0xC27D | `_fc27d` | `sound_voice_service_loop` | HIGH | "per-voice service loop: advance each voice's counter, retrigger command dispatch and the voice-off entry as their timers expire, and drive the paused/idle renderer once every voice has been serviced." |
| 0xC2EA | `_f_c2ea` | `sound_command_stream_dispatch` | HIGH | "command-stream dispatcher: split the byte at [si+command_cursor] into a low-nibble opcode and act on it..., then loop while more command bytes remain." |
| 0xC359 | `_f_c359` | `sound_control_value_select` | HIGH | "select and submit one value from the sound control state." (Consumes AL/AH/SI from its caller.) |
| 0xC3DB | `_f_c3db` | `sound_command_value_derive` | HIGH | "derive a command value from ES:DI state and update its selector." |
| 0xC440 | `_f_c440` | `sound_control_block_advance` | HIGH | "advance three two-byte slots in the 177Ch control-state block." |
| 0xC501 | `_f_c501` | `sound_secondary_command_dispatch` | HIGH | "dispatch one secondary stream command by the AH selector." |
| 0xC549 | `_f_c549` | `sound_command_flags_update` | HIGH | "update the paired command-control flags from AL." |
| 0xC567 | `_fc567` | `voice_level_percent_scale` | MEDIUM-HIGH | "signed scale-and-store for the sound-data state pair." Body: `store = round(cl * g1788/100)`, sign per `bl`, into `g178a`; `g1788` is the value `F_C59A`/`sound_param_scale4` stamps from a raw 0..63 (x4) level, so this scales a percentage of that base voice level. Twin of F_CA51 on the secondary stream's state. |
| 0xC5A8 | `_f_c5a8` | — | LOW | "store AL (zero-extended) through SI at DS:_g17a4." No field name for `g17a4` is established anywhere in the driver (the instructions for this exact case call out not to guess a name like "voice_set_attack" here). |
| 0xC5B3 | `_f_c5b3` | `sound_table_word_select_store` | HIGH | "select a word from the DS:1832h table by AL, then store it through SI at DS:17C4h." |
| 0xC5C6 | `_f_c5c6` | — | LOW | "store AL (zero-extended) through SI at DS:17DCh." No established field name for that offset. |
| 0xC5D1 | `_f_c5d1` | `voice_command_decode_apply` | HIGH | "decode one command byte and update the selected SI-relative voice-control slot." Body also computes a clamped frequency and calls `sound_pit_divisor_program` (F_C706), then mirrors `g17a4` into `g179c`. |
| 0xC678 | `_f_c678` | `voice_enable` | HIGH | "enable a voice, either by opening the speaker gate or by queueing its selected value for the alternate sound backend." |
| 0xC6B9 | `_f_c6b9` | `voice_disable` | HIGH | "disable a voice or submit an alternate-backend voice update." — mirror of F_C678. |
| 0xC706 | `_f_c706` | `sound_pit_divisor_program` | HIGH | "write a PIT divisor, queue it, or emit its two packed nibbles for the alternate sound backend." |
| 0xC755 | `_f_c755` | `sound_voices_reset_and_service` | HIGH | "former src/F_C755.ASM -- reset per-voice state, then service each configured voice" (clears `g1784`/`g1786`, zeroes each voice's `g17ac` slot, calls `voice_disable` per voice). |
| 0xC77A | `_f_c77a` | `sound_backend_select_init` | HIGH | "select and initialise a sound backend." |
| 0xC7CB | `_f_c7cb` | `sound_voice_table_reload` | HIGH | "reload per-voice values from the current backend table into the two SI-relative sound-control arrays." |
| 0xC834 | `_fc834` | `sound_voices_reset` | HIGH | "reset each configured voice and retain the voice count across the reset loop." |
| 0xC877 | `_fc877` | `sound_voices_disable_all` | HIGH | "submit one disabled voice update for every voice." |
| 0xC898 | `_fc898` | `opl_register_write` | HIGH | "write OPL register and data bytes with the required input-port settling reads between them." |
| 0xC8D4 | `_fc8d4` | `opl_port_write_byte` | HIGH | "former src/F_C8D4.ASM -- write the live AL value to the I/O port selected by the DS global _g1830 (the same OPL port as _opl_port above)." |
| 0xC914 | `_fc914` | `sound_stream_command_step` | HIGH | "fetch the next command byte, split it into a low nibble (command code, AL) and high nibble (argument, AH), dispatch to the matching handler, and loop while _snd_delay stays zero." |
| 0xC9A4 | `_f_c9a4` | `sound_stream_delay_decode` | HIGH | "decode one ES:[DI+1] command byte into a scaled delta and store it through _snd_delay." |
| 0xCA03 | `_f_ca03` | `sound_control_block_command_dispatch` | HIGH | "dispatch one control-block command by the AH selector." |
| 0xCA35 | `_f_ca35` | `sound_control_block_flags_latch` | HIGH | "latch or clear the control block's one-shot/length pair from AL." |
| 0xCA51 | `_f_ca51` | `stream_level_percent_scale` | HIGH | "signed percentage scale-and-store, the 1E84/1E86 twin of F_C567." |
| 0xCA83 | `_fca83` | `stream_level_base_set` | HIGH | "scale a raw 0..63 value by 4 and store it through _g1e84." (Secondary-stream twin of `sound_param_scale4`/F_C59A.) |
| 0xCA91 | `_fca91` | `stream_note_delay_set` | MEDIUM-HIGH | "store AL (zero-extended) through _g1e92." `g1e92` is established two functions later (F_CA9B) as the value copied into `_snd_delay`, i.e. it stages the next note's delay/duration for the secondary stream. |
| 0xCA9B | `_fca9b` | `stream_note_program` | HIGH | "look up a divisor in _notetab for a shifted note delta, program the PIT and speaker gate, then copy _g1e92 into _snd_delay." |
| 0xCAF1 | `_fcaf1` | `stream_control_block_arm` | HIGH | "when enabled and the caller's index is new, latch it, look up its far-pointer entry via _g175e, and reset the 1E84..1E94 control block for that entry." Matches every call site's usage as the generic "play sound-effect index N" entry point used across BOARD.C/LEVEL.C/etc. |

---

# Address-named global variables referenced from 3+ files

Counted by distinct source/header files that reference the raw address name
(`src/*.C`, `src/*.ASM`, `asm/*.ASM`, `include/*.H`); top 20 by that count,
restricted to names whose role the surrounding code (or an existing
in-repo comment) actually establishes. A few higher-count hits were left
out because the code itself documents them as ambiguous/overloaded
(`gc0fe`, `gc5da` — each explicitly repurposed by unrelated subsystems per
`include/GC0FE.H` and `src/PLRLDPUB.C`/`RECTTAB.C`/`RESCLOAD.C`) or because
no single consistent role could be pinned down (`g94`/`g98`/`g9a`).

| Global | Files | Proposed name | Conf. | Evidence |
|---|---|---|---|---|
| `g9ade` | 6 (`C470.H`, `BOARD.C`, `GAME.C`, `LEVEL.C`, `NODEIDX.C`, `TICKDIV.C`) | `campaign_round_node_cursor` | HIGH | `campaign_chapter_advance` sets it to `round_progress[i]*2+i*8` and walks it `+=1`/`-=2` per node attempt; `campaign_node_index()` returns `(g9ade&7)/2` and `TICKDIV.C`'s helper returns `g9ade/8` — the combined round/node position the campaign driver advances through. |
| `gc360` | 4 (`C470.H`, `OPTIONS.C`, `SLOTCOPY.C`, `SLOTS.C`) | `slot_transfer_table` | HIGH | `C470.H`'s own comment: "`extern struct c470_record gc360[10]; /* DS:C360 transfer copies */`" — the ten-slot archive `fad25`/`fadcf` copy the current slot into before deleting/resetting it. |
| `g8bfe` | 4 (`BOARD.C`, `GAME.C`, `LEVEL.C`, `PLAYERSL.C`) | `game_abort_jmpbuf` | HIGH | Always used as `longjmp((char far*)g8bfe, code)` (e.g. `board_scan_wipe_effect`'s quit path, `F_B99F`'s restart-confirm path) — the shared `setjmp` buffer the top-level game loop uses to unwind out of nested play/menu state. |
| `g736`, `g738`, `g73a` | 4 each (`BOARD.C`, `GAME.C`, `HITTEST.C`, `LEVEL.C`) | `cursor_x`, `cursor_y`, `cursor_copy_width` | HIGH | Consistently used together as the on-screen actor/cursor position and a paired width/flip parameter: `LEVEL.C`'s movement loop reads/writes them every tick (`g736+=g734`, `shadow_bitmap_hit_test(g736+8,g738-1,9)`), `BOARD.C`'s `board_scroll_transition` uses `copy(g736,g738,pool+...,g73a)`, `HITTEST.C`'s raycast/hit-test routines take them as the probe origin. |
| `g237c` | 4 (`SNDRQCLR.C`, `SNDRQDEC.C`, `SNDSTART.C`, `TIMER.C`) | `sound_request_count` | HIGH | Backing storage for the already-named `sound_request_count_dec`/clear helpers: `SNDRQCLR.C`'s own comment is "clear the counter at DS:237C", `SNDRQDEC.C`'s is "count the counter at DS:237C down and clamp it at zero." |
| `g1776` | 4 (`GAME.C`, `INTRO.C`, `LEVEL.C`, `PLAYERSL.C`) | `ui_redraw_suppress` | MEDIUM | Set to 1 around screen-drawing sections and back to 0 afterward in each of these files (e.g. `player_select_run`, `intro_run_chapter`, `fb6cd`'s chapter loop clears it once its tick budget elapses) — consistent with a "suppress background redraw/tick while this UI is drawing" flag; exact consumer of the flag outside these files not traced. |
| `gbb2` | 3 (`MENULACT.C`, `MENULDIS.C`, `MENULEN.C`) | `menu_list_enabled` | HIGH | Backing word for the already-named accessors: `MENULDIS.C`'s own comment is "F_7925 -- clear the word at DS:0BB2", `MENULACT.C`'s is "F_792C -- read back the word at DS:0BB2" — i.e. the flag `menu_list_active()`/`menu_list_disable()`/`menu_list_enable()` test and set. |
| `gb83` | 3 (`HUD.C`, `HUDPMSG.C`, `PROMPTS.C`) | `hud_prompt_kind` | HIGH | Set to a distinct small constant by each prompt-drawing routine (`hud_prompt_continue_draw` sets 2, `hud_prompt_select_draw` sets 3, `hud_prompt_confirm_draw` sets 4) — records which prompt box style is currently on screen. |
| `gb76` | 3 (`INTRO.C`, `LEVEL.C`, `TIMER.C`) | `timer_tick_count` | HIGH | `TIMER.C`'s own comment on `F_6C57`: "gb76 is the free-running tick (unsigned...)" — the PIT-driven tick counter `timer_deadline_arm`/`timer_deadline_reached` compare against. |
| `gb68` | 3 (`GAME.C`, `KEYBOARD.C`, `LEVEL.C`) | `key_jump_held` | HIGH | `KEYBOARD.C`'s IRQ handler sets it from the Up/Home/PgUp scancodes (0x47/0x48/0x49); `LEVEL.C`'s movement loop tests `gb68&&gb70&&...` to trigger a jump. |
| `gb6e` | 3 (`GAME.C`, `KEYBOARD.C`, `LEVEL.C`) | `key_right_held` | HIGH | Set from the Right/PgUp diagonal scancodes in the keyboard IRQ handler; `LEVEL.C`'s movement loop's `if(gb6e){...g736+=g734...}` branch moves the actor right while it is set. |
| `gb6c` | 3 (`GAME.C`, `KEYBOARD.C`, `LEVEL.C`) | `key_left_held` | HIGH | Mirror of `gb6e` for the Left/Home diagonal scancodes; `LEVEL.C`'s `if(gb6c){...g736-=g734...}` branch. |
| `gb70` | 3 (`GAME.C`, `KEYBOARD.C`, `LEVEL.C`) | `key_jump_edge` | HIGH | Set only on the scancode's break/release event (`if(scan&0x80) gb70=1`) alongside `gb68`; consumed once by `LEVEL.C`'s movement loop to trigger a single jump, matching a "key was just released" edge trigger rather than a held state. |
| `gb3ae` | 3 (`F_4E9F.ASM`, `F_4EEB.ASM`/`.C`, `LEVEL.C`) | `actor_record_table` | HIGH | The 0x20-byte-stride record table `F_4B0C`, `F_4E9F` and `F_4EEB`'s own banners all describe directly ("walk the 20h-stride record table at gB3AE... one record per active sprite/actor"); `LEVEL.C`'s `fb60f`/`fb772` index the same table by the same stride. |
| `gb3af` | 3 (`GB3AF.H`, `BOARD.C`, `LEVEL.C`) | `actor_state_table` | MEDIUM-HIGH | Companion near-array to `gb3ae`, indexed the same way (`((unsigned char near*)gb3af)[136]=1` in `level_display_init`, `gb3af[12].flag`/`.rest[7]` in the movement loop, `movmem(&saved[i],&gb3af[i*2+4],32)` in `fb772`) — per-record runtime state for the actor table, distinct from but paired with `gb3ae`. |
| `gbfc8` | 3 (`F_60A9.ASM`, `LEVEL.C`, `MENURES.C`) | `sprite_tile_bank` | HIGH | `F_60A9`'s own banner: "recomputes the tile's 1E6h-byte slot in the far sprite bank at gBFC8"; allocated by `MENURES.C`'s `menu_resources_load` (record 19) and freed by `LEVEL.C`'s `fb55e`/`menu_resources_free`. |
| `gbfc4` | 3 (`M_D386_D3CF.ASM`, `BOARD.C`, `GAME.C`) | `icon_record_list_ptr` | HIGH | The far list pointer `F_D386`/`icon_list_animate_draw` reads (`lds si, dword ptr ds:_gbfc4`) at the top of its walk — the head of the animated-icon record list it and `F_D3CF` render. |
| `g9bfc` / `gbf66` | 3 each (`LEVEL.C`, `MENURES.C`, and `F_4E9F.ASM`/`F_4E9F` resp.) | `tile_width_table` / `tile_height_table` | HIGH | Always loaded as a matched pair from adjacent resource records (0x52/0x53 in `board_resource_expand`, 0x57/0x58 in `level_expand_descriptors`, 23/24 in `menu_resources_load`) and consumed together in `LEVEL.C`'s `fb60f` as `r=x+g9bfc[c]-1` (right edge) / `b=y+gbf66[c]-1` (bottom edge) — a per-tile-type width and height lookup. |
| `gc5cc` | 3 (`LAYOUT.H`, `DIALOG.C`, `RESOURCE.C`) | `dialog_backdrop_save_size` | HIGH | Always passed alongside `ui_gfx_blob` to `gfx_save_rect`/`gfx_restore_rect` in `DIALOG.C`'s box show/hide pair — the saved-area byte count for the dialog's background buffer. |
| `gc0fc` | 3 (`HUD.C`, `HUDPMSG.C`, `PROMPTS.C`) | `hud_prompt_box_color` | MEDIUM-HIGH | Set to the caller-supplied background colour parameter in both `hud_prompt_select_draw` (`gc0fc=0`) and `hud_prompt_confirm_draw` (`gc0fc=b`, the box's fill colour argument) immediately before the box is drawn/cleared. |
| `g8fe` | 4 (`F_60A9.ASM`, `GAME.C`, `HITTEST.C`, `LEVEL.C`) | `cursor_over_object` | MEDIUM | `HITTEST.C` sets it to 1 at the start of a hit-test and clears it once a companion counter (`gc0c0`) expires; `GAME.C` tests it to gate `board_unit_script_trigger`/`sprite_slots_redraw`/`board_raycast_step` and clears it on several key-release paths; `F_60A9` (the animated-tile ticker) checks it too — consistent with "the mouse cursor is currently over a hit-testable board object", though the exact consumer relationship across files is not fully traced. |
