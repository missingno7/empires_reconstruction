# Startup / intro / menu call graph (Milestone D minimum TU set)

Depth-first trace from `main()` (`src/GAME.C:773`) through the boot and
menu chain the task specifies, by reading each function's body and
grepping the definition of every C/ASM callee, down to the first point a
level's turn loop actually runs. Every function is tagged **PURE** or
**HW** using the same classification `tools/portable/inventory/scan_tus.py`
computes per-file in `docs/portable/tu-inventory.md` (a function in a PURE
file is PURE; a function in an HW/MIXED file is marked HW here since the
question this document answers is "does reaching this function pull in a
DOS/hardware-facing translation unit," not "is this specific function's
own body hardware code").

Each function is listed once, at its first appearance in the depth-first
walk; later call sites reference back to that entry instead of repeating
its expansion.

---

## 0. Top-level chain

```
main()                                    src/GAME.C:773            HW (calls into HW immediately)
 └─ video_mode_select()                   src/STARTUP.C:222          HW
     └─ (gate passes) boot_init_seed_rand()   src/GAME.C:629          HW
         └─ game_run()                   src/GAME.C:717              HW
             └─ intro_run_chapter()      src/INTRO.C:280             MIXED (file), HW-heavy
             └─ (d<2) slot_menu_run()    src/SLOTMENU.C:381          PURE (file), HW-heavy
             └─ loop: player_select_run()    src/PLAYERSL.C:209      HW
             └─ loop: campaign_chapter_advance()  src/GAME.C:649     HW
                 └─ level_driver_run()   src/GAME.C:499               HW  <-- MILESTONE D/E BOUNDARY
                     └─ board_actors_draw(0)  asm/SPRITES.ASM (F_4EEB)  [first level-rendering call]
                     └─ turn_loop_run()  src/GAME.C's grouped TU (TURNLOOP member, C_3A75_4A93)  [level turn loop starts here -- NOT expanded further, out of scope]
         └─ game_shutdown()              src/GAME.C:661               HW
         └─ video_set_text_mode()        src/VIDEO.C:139              HW
```

`main()` itself has no branches beyond the `video_mode_select()` gate (see
its own header comment, `src/GAME.C:748-772`, for the disassembly-backed
"single guarded block with no early exit" shape).

---

## 1. `video_mode_select()` — `src/STARTUP.C:222` (file class: **HW**)

```
video_mode_select()
 ├─ getdisk()                    CC.LIB (not in this tree)
 ├─ bios_equipment_probe()       src/STARTUP.C:62    HW  (int 11h via inline asm)
 ├─ video_adapter_detect()       src/STARTUP.C:99    HW  (int 10h/15h, pseudo-regs + __int__/__inportb__/__outportb__; irreducible inline-asm core per docs/current/closure-frontier.md §1)
 ├─ dos_write_handle2()          src/STARTUP.C:14    HW  (int 21h AH=0x40 via pseudo-regs + __int__)
 ├─ sound_backend_probe()        src/STARTUP.C:177   HW
 │   ├─ opl_detect()             src/OPLREG.C:341    HW  (inport/outport-family calls this scan's marker sweep initially missed -- see tu-inventory.md's `inport` note)
 │   │   └─ opl_register_write() asm/SOUND.ASM (F_C898)  HW (see docs/portable/asm-module-inventory.md §9)
 │   ├─ __int__(0x15)            intrinsic
 │   └─ __inportb__/__outportb__(0x203)  intrinsics
 ├─ cmdline_parse_args()         src/STARTUP.C:30    HW (file), body is plain argv parsing
 └─ farcoreleft()                CC.LIB far-heap probe
```

**Files reached:** STARTUP.C, OPLREG.C, asm/SOUND.ASM.

---

## 2. `boot_init_seed_rand()` — `src/GAME.C:629` (file class: **HW**)

```
boot_init_seed_rand()
 ├─ biostime(0, 0L)              CC.LIB (BIOS clock read, int 1Ah)
 ├─ srand(...)                   CC.LIB
 ├─ timer_irq_install()          src/TIMER.C            HW (entire body inline asm; §4 of docs/current/portability-boundaries.md)
 ├─ dos_critical_error_install() src/CRITERR.C:26       HW
 │   └─ harderr(dos_critical_error_handler)   CC.LIB; installs src/CRITERR.C:9's handler (never called directly, only by DOS -- §9)
 ├─ ui_gfx_alloc()               src/PLAYERSL.C:254     HW (MK_FP/FP_SEG region carve; fact 13 in docs/portable/int-semantics-inventory.md)
 │   └─ farmalloc(0xfa80L)       CC.LIB far heap
 ├─ blitter_patch_variant()      src/GAME.C:567         HW
 │   ├─ resource_load_record()   src/RESOURCE.C:217     HW
 │   └─ memmove((char far*)runtime_base, ...)   asm/RUNTIME_BLOCK.ASM  (out of this inventory's scope; §1 of portability-boundaries.md)
 ├─ video_alloc_framebuffer()    src/VIDEO.C:111        HW
 │   ├─ farmalloc()              CC.LIB
 │   ├─ video_normalize_far_ptr()   src/VIDEO.C:95      HW
 │   ├─ runtime_base()           asm/RUNTIME_BLOCK.ASM  (out of scope)
 │   ├─ color_lookup_tables_init()  src/VIDEO.C         HW
 │   └─ video_load_palette()     src/VIDEO.C            HW (`asm les dx,pal` irreducible fragment)
 ├─ player_record_load_publish() src/PLRLDPUB.C:14      MIXED (file)
 │   ├─ farmalloc()              CC.LIB
 │   ├─ resource_load_record_alloc()   src/RESOURCE.C:281   HW
 │   └─ sound_backend_select_init()    asm/SOUND.ASM (F_C77A)  HW
 ├─ keyboard_irq_install()       src/KEYIRQ.C            HW (`getvect`/`setvect`; §3)
 ├─ asm sti                      inline asm (GAME.C itself, TU-anchoring fragment per docs/current/tu-structure.md)
 ├─ resource_stripe_table_load() src/BOARD.C:138         HW (file)
 │   └─ resource_load_record()   src/RESOURCE.C          HW
 ├─ sprite_load_boot_sheets()    src/BOARD.C:123         HW (file)
 │   └─ resource_load_record_into()   src/RESOURCE.C     HW
 └─ sprite_sheet_select(0)       src/FONT.C:17           HW (asm body, ES:SI far-table walk)
```

**Files reached:** GAME.C, TIMER.C, CRITERR.C, PLAYERSL.C, RESOURCE.C,
VIDEO.C, PLRLDPUB.C, KEYIRQ.C, BOARD.C, FONT.C, plus
`asm/RUNTIME_BLOCK.ASM` and `asm/SOUND.ASM` (both out of the Wave-2 ASM
inventory's scope but reached here).

---

## 3. `game_run()` own body — `src/GAME.C:717` (file class: **HW**)

Direct calls not already covered by the sub-sections below:

```
game_run()
 ├─ intro_run_chapter()          src/INTRO.C:280        -> §4
 ├─ hud_arena_init()             src/BOARD.C:149         HW (file)
 │   ├─ farmalloc(0x55f0L)       CC.LIB
 │   ├─ resource_icon_table_load()   src/BOARD.C:71      HW  (16x resource_load_record_into -> RESOURCE.C)
 │   ├─ resource_scoreboard_unpack() src/BOARD.C:96      HW  (resource_load_record + memmove loop)
 │   └─ hud_icons_load()         src/HUD.C:38            PURE (file) -> resource_load_record_alloc (RESOURCE.C, HW)
 ├─ setjmp(game_abort_jmpbuf)    CC.LIB (§9; game_run is the one setjmp site -- docs/current/portability-boundaries.md §9)
 ├─ ui_overlay_reset()           src/HUD.C:134           PURE (file); trivial DGROUP clear
 ├─ sound_request_count_clear()  src/SNDREQ.C:41         PURE (file); trivial DGROUP clear
 ├─ puzzle_free_resources()      src/PUZZLE.C:198        MIXED (file); frees two far blocks (`free`/farfree-shaped NULL test, §7)
 ├─ slot_menu_run()              src/SLOTMENU.C:381      -> §5   (only when d<2)
 ├─ energy_set()                 src/HUD.C:227           PURE (file); trivial store
 ├─ campaign_chapter_advance()   src/GAME.C:649           -> §7  (only when a resume round exists, or from the main loop below)
 ├─ player_select_run()          src/PLAYERSL.C:209      -> §6  (main `while (s != 4)` loop)
 └─ slot_archive_and_delete()    not expanded (reached only after the `while` loop exits with s==4, i.e. after Milestone D/E boundary territory begins)
```

---

## 4. `intro_run_chapter()` — `src/INTRO.C:280` (file class: **MIXED**, HW-heavy)

```
intro_run_chapter()
 ├─ gfx_color_select()           src/VIDEO.C             HW
 ├─ clear() / box() / copy()     src/VIDEO.C / asm/RUNTIME_BLOCK.ASM (fixed-address gfx-primitive aliases 0x039F/0x03A8/0x03CC)   HW
 ├─ resource_record_cache_load()/_reset()   src/RESCACHE.C   PURE (file) -> resource_load_record (RESOURCE.C, HW)
 ├─ splash_draw_and_clear()      src/INTRO.C:206         -> resource_load_record + gfx_blit_bitmap/gfx_wipe_rect (HW)
 ├─ resource_load_record_alloc() src/RESOURCE.C:281      HW
 ├─ resource_ptr_table_build()   src/INTRO.C:244         -> resource_load_record_alloc (HW)
 ├─ bitmap_blit_topleft()        src/INTRO.C:263         -> gfx_copy_rect (HW)
 ├─ intro_play_script()          src/INTRO.C:87          -> intro_animate_step() (src/INTRO.C:139, facts 8-9 in int-semantics-inventory.md) -> gfx_wipe_rect/gfx_box/stream_control_block_arm (SOUND.ASM, HW)
 ├─ hud_prompt_continue_draw()/_clear()   src/HUD.C       PURE (file) -> gfx primitives (HW)
 ├─ timer_deadline_arm()         src/TIMER.C              HW
 ├─ intro_wait_key()             src/INTRO.C:222         -> timer_deadline_reached() (TIMER.C, HW), keyboard_poll_nonblocking()/keyboard_read_blocking_hotkeys() (KEYBOARD.C, HW)
 ├─ dialog_run()                 src/DIALOG.C:379         PURE (file) -> keyboard_chain_active()/keyboard_read_blocking_hotkeys() (KEYBOARD.C, HW), gfx primitives (HW)
 ├─ anim_step_loop()             src/ANIMSTEP.C:8         PURE (file)
 │   └─ anim_step_row_copy()     asm/ANIMROW.ASM (F_9EC3)  -- see docs/portable/asm-module-inventory.md §5
 ├─ text_draw_wrapped()          src/FONT.C                HW (file)
 ├─ stream_control_block_arm() / sound_stop_reset()   asm/SOUND.ASM (F_CAF1 / F_CB48)   HW
 ├─ timer_wait_ticks()           src/TIMER.C              HW
 └─ farfree()                    CC.LIB far heap
```

**Files reached (new):** INTRO.C, RESCACHE.C, HUD.C, DIALOG.C,
ANIMSTEP.C, FONT.C (already reached in §2), plus `asm/ANIMROW.ASM` and
`asm/SOUND.ASM` (already reached).

---

## 5. `slot_menu_run()` — `src/SLOTMENU.C:381` (file class: **PURE**, HW-heavy)

```
slot_menu_run()
 ├─ keyboard_chain_active()/_enable()/_disable()/keyboard_buffer_drain()   src/KEYBOARD.C   HW
 ├─ menu_list_disable()          src/MENULDIS.C:4        PURE (file); trivial DGROUP clear
 ├─ sound_stop_reset()/sound_voices_reset()   asm/SOUND.ASM (F_CB48/F_C834)   HW
 ├─ sound_start()                src/SNDREQ.C:13         PURE (file) -> asm/SOUND.ASM
 ├─ slot_menu_draw_header()      src/SLOTS.C:15          PURE (file) -> resource_load_record[_into] (RESOURCE.C, HW), gfx_blit_bitmap (HW)
 ├─ gfx_blit_bitmap()/gfx_color_select()/gfx_clear_rect()/gfx_box()   src/VIDEO.C / asm/RUNTIME_BLOCK.ASM   HW
 ├─ slot_find_free()             src/SLOTS.C:65           PURE (file); pure DGROUP scan
 ├─ slot_row_draw()              src/SLOTROW.C:4          PURE (file) -> text_draw_wrapped (FONT.C, HW), sprite_pool_draw_masked (src/SPRPOOLD.C, PURE per tu-inventory -- not expanded further)
 ├─ anim_step_loop()             -> §4 (already expanded)
 ├─ slot_list_draw()             src/SLOTMENU.C:52       -> gfx primitives (HW), slot_row_draw (above)
 ├─ slot_list_select_loop()      src/SLOTMENU.C:335       -> menu_wait_key_animated() (src/LEVEL.C:57, **HW** file -- first LEVEL.C reach, menu-support helper only, not the level simulation), str_concat_far_list() (src/STRCATF.C, PURE file), dialog_run() (DIALOG.C, above), slot_delete() (src/SLOTS.C)
 ├─ player_slot_add_run()        src/SLOTMENU.C:284       -> keyboard_*/ui_overlay_show()/hide() (HUD.C, PURE file) / sound_start() / menu_list_active()/_enable() (?) / dialog_draw()/dialog_restore_screen() (DIALOG.C) / player_name_edit() (SLOTMENU.C, local) / player_type_select() (SLOTMENU.C, local) / setmem() (CC.LIB) / strcpy() (custom, src/STRCATF.C-adjacent F_F2DB per its own `@SYM` tag)
 ├─ dialog_run()                 -> already expanded above
 ├─ slot_table_save()            src/SLOTS.C:33           -> resource_file_write_record (src/RESOURCE.C, HW)
 └─ sound_request_count_dec()    src/SNDREQ.C:26          PURE (file); disassembly fact 10 in int-semantics-inventory.md
```

**Files reached (new):** SLOTMENU.C, MENULDIS.C, SNDREQ.C, SLOTS.C,
SLOTROW.C, LEVEL.C (menu-helper reach only — see note below), STRCATF.C.
`src/SPRPOOLD.C` is referenced but not expanded (leaf sprite-pool blit
helper, PURE per tu-inventory.md).

**Note on LEVEL.C:** `menu_wait_key_animated()` is grouped into LEVEL.C's
translation unit (`C_AF45_C15E` per `docs/current/tu-structure.md`) even
though it is pure menu-navigation-arrow-blink code with no level-simulation
content — reaching it during the menu chain does **not** mean level
simulation has started; it means Milestone D's minimum TU set must still
include (a slice of) LEVEL.C's *file*, since the historical build compiled
this helper into the same unit as the real turn-loop code. Wave 2 should
treat `menu_wait_key_animated` as extractable/portable independently of the
rest of LEVEL.C if the TU is split during porting.

---

## 6. `player_select_run()` — `src/PLAYERSL.C:209` (file class: **HW**)

```
player_select_run()
 ├─ keyboard_chain_active()/_enable()/_disable()   src/KEYBOARD.C   HW
 ├─ player_select_load_flags()   src/PLAYERSL.C (local)  -- not further expanded (small DGROUP-flag loader)
 ├─ resource_record_cache_reset()   src/RESCACHE.C        PURE (file) -> RESOURCE.C (HW)
 ├─ player_select_mark()         src/PLAYERSL.C:91        local, trivial
 ├─ player_select_draw_screen()  src/PLAYERSL.C (local)   -- gfx-heavy, not further expanded
 ├─ menu_list_source_set_players()/_default()   src/PLAYERSL.C / elsewhere (local)
 ├─ sound_request_count_dec()    -> already reached (§5)
 ├─ gfx_wipe_rect()/gfx_color_select()/gfx_clear_rect()   src/VIDEO.C   HW
 ├─ hud_prompt_select_draw()     src/PLAYERSL.C-adjacent (HUD-family) -- not further expanded
 ├─ player_select_choose_slot()  src/PLAYERSL.C:158
 │   ├─ player_select_draw_highlight()/_clear_highlight()   src/PLAYERSL.C:150-155  -> gfx_wipe_rect/gfx_box (HW)
 │   ├─ keyboard_buffer_drain()/keyboard_read_blocking_hotkeys()   src/KEYBOARD.C   HW
 │   └─ player_select_restart_confirm()   src/PLAYERSL.C:69
 │       ├─ sound_start()        -> already reached
 │       ├─ dialog_run()         -> already reached (DIALOG.C)
 │       ├─ slot_table_save()    -> already reached (SLOTS.C)
 │       ├─ sound_stop_reset()/sound_voices_reset()   -> already reached (SOUND.ASM)
 │       └─ longjmp(game_abort_jmpbuf, 3)   CC.LIB (§9's abort-code 3 site)
 ├─ hud_panel_clear()            src/HUD.C (PURE file)   -- not further expanded
 ├─ player_select_close_wipe()   src/PLAYERSL.C:196       -> gfx_wipe_rect/gfx_box (HW)
 └─ sound_voices_reset()         -> already reached
```

**Files reached (new):** none beyond PLAYERSL.C, KEYBOARD.C, RESCACHE.C,
VIDEO.C, DIALOG.C, SLOTS.C, SOUND.ASM — all already listed above.
`player_select_quit_confirm()`/`player_select_menu_confirm()` (same file,
`src/PLAYERSL.C:44/54`) are the other two `game_abort_jmpbuf` sites
(codes 2 and 1) reachable from the menu-loop keyboard handler, not from
`player_select_run` itself, and are noted here for completeness of §9's
four-site abort mechanism.

---

## 7. `campaign_chapter_advance()` — `src/GAME.C:649` (file class: **HW**) and the boundary

```
campaign_chapter_advance(i)
 └─ level_driver_run()           src/GAME.C:499           HW   <== MILESTONE D/E BOUNDARY
     ├─ menu_backdrop_paint()    (not expanded)
     ├─ menu_list_source_set_default()   (not expanded)
     ├─ board_redraw_paint()     (not expanded)
     ├─ roundend_round_setup()   src/ROUNDEND.C            (not expanded -- Milestone E territory)
     ├─ puzzle_clear_grid()      src/PUZZLE.C              (not expanded)
     ├─ hud_scroll_reset()       src/HUD.C                 (not expanded)
     ├─ anim_step_loop()         -> already reached (§4)
     ├─ board_actors_draw(0)     asm/SPRITES.ASM (F_4EEB)  <== first level-rendering ASM call reached; see docs/portable/asm-module-inventory.md §3
     ├─ sprite_draw_cursor()     (not expanded)
     ├─ hud_panel_open()         src/HUD.C:45              -> gfx_blit_bitmap + hud_draw_meter/hud_tab_draw/energy_draw (not expanded)
     ├─ tutorial_hint_dialog_show()   src/HINTDLG.C:14     MIXED (file) -> resource_load_record_alloc/farfree (HW) -- already-documented in tu-inventory.md
     ├─ resource_record_cache_reset()   -> already reached
     └─ turn_loop_run()          GAME.C's grouped TU (TURNLOOP member of C_3A75_4A93, per docs/current/tu-structure.md)  <== THE LEVEL TURN LOOP. Not expanded: this is where "a level is running" begins and is Milestone E's own starting point, out of this document's scope.
```

This is the point the task asks this document to stop at: `turn_loop_run()`
(and the fallback `level_play_chapter()` called if it returns nonzero at
`campaign_round_node_cursor==0x27`) is the first point actual per-turn
level simulation runs, reached via `board_actors_draw()` (the first
`asm/SPRITES.ASM` call in the whole boot->menu->level path) immediately
before it.

---

## Milestone D (intro/menu) minimum TU set

Every `src/*.C` file reached before `level_driver_run()`'s body starts
(i.e. everything in §0-§6 above), with its `tu-inventory.md` classification:

| File | Class | Reached via |
|---|---|---|
| GAME.C | HW | main/boot/game_run/campaign_chapter_advance (entry point file) |
| STARTUP.C | HW | video_mode_select |
| TIMER.C | HW | boot_init_seed_rand, intro_run_chapter |
| CRITERR.C | HW | boot_init_seed_rand |
| PLAYERSL.C | HW | boot_init_seed_rand (ui_gfx_alloc), player_select_run |
| VIDEO.C | HW | boot_init_seed_rand, throughout |
| PLRLDPUB.C | MIXED | boot_init_seed_rand |
| KEYIRQ.C | HW | boot_init_seed_rand |
| BOARD.C | HW | boot_init_seed_rand, game_run (hud_arena_init) |
| FONT.C | HW | boot_init_seed_rand, intro_run_chapter |
| RESOURCE.C | HW | pervasive (resource_load_record family) |
| OPLREG.C | HW (after this pass's `inport`/`outport` fix) | video_mode_select (opl_detect) |
| HUD.C | PURE | game_run, intro_run_chapter, player_select_run |
| PUZZLE.C | MIXED | game_run (puzzle_free_resources) |
| INTRO.C | MIXED | game_run |
| RESCACHE.C | PURE | intro_run_chapter, player_select_run |
| DIALOG.C | PURE | intro_run_chapter, slot_menu_run, player_select_run |
| ANIMSTEP.C | PURE | intro_run_chapter, slot_menu_run |
| SLOTMENU.C | PURE | game_run |
| MENULDIS.C | PURE | slot_menu_run |
| SNDREQ.C | PURE | game_run, slot_menu_run |
| SLOTS.C | PURE | slot_menu_run, player_select_run |
| SLOTROW.C | PURE | slot_menu_run |
| LEVEL.C | HW | slot_menu_run (menu_wait_key_animated helper only — see §5 note) |
| STRCATF.C | PURE | slot_menu_run |
| KEYBOARD.C | HW | pervasive |
| SPRPOOLD.C | PURE | slot_menu_run (slot_row_draw, referenced not expanded) |
| HINTDLG.C | MIXED | level_driver_run (tutorial_hint_dialog_show, at the boundary itself) |

**28 `src/*.C` files.** Plus these `asm/*.ASM` modules:
`RUNTIME_BLOCK.ASM` (blitter patch, framebuffer alloc, gfx primitives — out
of this inventory's Wave-2 scope but load-bearing), `SOUND.ASM` (pervasive,
§9 of `docs/portable/asm-module-inventory.md`), `ANIMROW.ASM`
(`anim_step_row_copy`, §5), and `SPRITES.ASM` (`board_actors_draw`, reached
exactly at the boundary in `level_driver_run` — arguably the first Milestone
E dependency rather than D, included here since it is called before
`turn_loop_run`).

## Milestone E starting point

`turn_loop_run()` (GAME.C's TU, TURNLOOP member — grouped per
`docs/current/tu-structure.md`'s `C_3A75_4A93` entry) and the fallback
`level_play_chapter()`. Not traced further per the task's scope; this is
the handoff point from Milestone D to Milestone E.
