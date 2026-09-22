# Portable source port — architecture contract

Branch `portable-sdl3`, forked from tag `historical-exact-oracle-v1`
(commit 873d1df0f505601d760c6880cdea0c6ae3d81405, AEPROG.EXE SHA-256
1259348425483d8d97fd8821860b47cfdf58fc8029711eb0ed0e78ab33807a10).

The historical tree (`src/`, `asm/`, `include/`, `recipes/`, `layout/`,
`tools/` except `tools/portable/`) is the behavioral oracle and is **not
edited** by port work.  Everything portable lives under `portable/`,
`third_party/`, `tools/portable/`, `docs/portable/` and the top-level
`CMakeLists.txt`.

## Layout

```
CMakeLists.txt              top level: options, SDL3 / Nuked-OPL3 dependencies
portable/
  include/                  public headers, one per subsystem (see below)
  compat/                   dos_types helpers, any temporary compatibility arena
  resource/                 AE000/AE001 archive + DECODE.ASM semantic port
  gfx/                      packed-4bpp software framebuffer + RUNTIME_BLOCK primitives
  audio/                    SOUND.ASM state machine, OPL wrapper, speaker synth
  platform/sdl3/            main(), window/texture, event pump, audio device, clock
  generated/                game_data.[ch], game_state.[ch] from tools/portable/datagen.py
  game/                     ported historical C translation units (normalized)
  tests/                    ctest unit tests + small deterministic fixtures
third_party/nuked-opl3/     pinned Nuked-OPL3 (fetched by CMake, or vendored)
tools/portable/             python generators / fixture builders / oracle harness
docs/portable/              this contract, inventories, parity notes
```

## Layering

```
ported game logic (portable/game)
   |  calls only these interfaces:
   v
resource.h  gfx.h  timer.h  input.h  sound.h  dosio.h   (portable/include)
   |
   v
portable/resource  portable/gfx  portable/audio  portable/compat
   |
   v
portable/platform/sdl3  (the ONLY code that includes <SDL3/SDL.h>)
```

No file outside `portable/platform/sdl3/` may include SDL headers.  No file
outside `portable/audio/` may include Nuked-OPL3 headers.

## Integer discipline (Phase 1)

`portable/include/dos_types.h` defines `dos_char/dos_uchar/dos_int/dos_uint/
dos_long/dos_ulong` (8/8/16/16/32/32 bits) and `dos_i16/dos_u16/dos_add16/
dos_sub16/dos_mul16`.  Rules:

1. Every object the historical source declared `int`, `unsigned`, `long`,
   `char` keeps that width via the alias.  Never use bare `int` for a
   historical quantity.
2. Where the historical comments prove a 16-bit truncation or a signed/
   unsigned compare (docs/current/portability-boundaries.md §8), write it
   explicitly with a cast or helper and cite the historical file/function in
   a comment.
3. Everything else is written naturally; differential tests decide whether
   more sites need explicit wrapping.

## Shared game state

Every DGROUP object the ported C references becomes one real C object with
the historical name (rename later, never alias-by-copy).  Two historical
declarations of the same address with different types (e.g. `mode` /
`display_mode` at DS:BFCD, `gb40` near/plain) become **one** object plus a
`#define` alias, documented in `docs/portable/state-map.md`.

- `portable/generated/game_data.[ch]`: initialized DATA objects generated from
  `recipes/data/game-initialized.json` + `src/data/*.json`.  Pointer32
  records become real C pointers to the target object (+ addend).
- `portable/generated/game_state.[ch]`: BSS objects generated from
  `src/data/GAME_BSS.json`, typed from the historical extern declarations.
- Far pointers become plain pointers.  Row tables, blob pointers, etc. are
  ordinary `uint8_t *`.
- The 37,250-byte BSS is **not** modelled as a byte array; if a temporary
  arena is needed for bring-up it lives in `portable/compat/` behind one
  module with a tracked removal list.

The supervisor owns the canonical ownership of every shared object.  Agents
must not create alternative definitions of state that already exists in
`portable/generated` or `portable/include`.

Ownership rule for hand-written subsystems: a subsystem module may *define*
the DGROUP objects its public header declares (e.g. `portable/gfx` defines
`g3924`, `result`, `gbc`, `rect_queue_write_ptr`, `g94..g9a`, `gc0de..gc0e8`;
`portable/resource` defines `ui_gfx_blob/shadow_a/shadow_b`, `gc0cb`,
`display_mode`).  Those names are listed in
`tools/portable/state_ownership.json` and the generator skips them, so each
object has exactly one definition.  Historical names are kept for every
DGROUP object; new names are used only for objects that never existed
historically (e.g. `gfx_vram`).

## Video model (primary target = historical display selector 5, "V" = VGA)

The game ships five display selectors.  Selector 5 (`-V`) is the real VGA
path and is the port's primary target; selector 4 (`-M`, mode 13h showing
the 16 standard colours through the built-in runtime) was ported first and
stays as the validated secondary driver.  Facts from the historical tree:

- Selector 5 replaces the built-in `asm/RUNTIME_BLOCK.ASM` with the decoded
  `AE000_002` record (1886 bytes of 8086 code, annotated disassembly in
  `docs/portable/reference/AE000_002-vga-runtime.lst`), same 20-entry jump
  table order as `include/VIDEO.H`.
- Logical framebuffer: 320 x 488 pixels, **8 bpp, 320-byte rows** (0x140),
  one VGA DAC index per byte, addressed through the 488-entry row table
  `g3924`.  Palette `g11e` (256 x RGB6, 251 distinct colours) loaded once.
- Art records stay packed 4 bpp (`resource_load_record` skips the sprite
  fix-ups in mode 5).  Each 34-byte bitmap header carries an EGA/CGA table
  (+0x00) and a VGA table (+0x10); the VGA blitters (`gfx_blit_bitmap`,
  `gfx_copy_rect`) expand each nibble through the bitmap's VGA table at draw
  time (nibble 0 = transparent in `gfx_copy_rect`).
- `result` (DS:40C8) low byte is the DAC index; `gfx_color_select(i)` reads
  `g3904[i]` (seeded from `g9c` = identity 0..15, changed at runtime by
  `color_table_entry_set`).
- `gfx_box(x,y,w,h)` presents by plain row copies into the 320x200 VRAM.
- The dirty-rect queue records and the `gfx_copy_rect` column clip words
  (`g98`/`g9a`) stay in packed (x/2) units even in VGA mode.
- `gfx_draw_char`/`gfx_blit_image` paint 1-bpp MSB-first glyphs one byte
  per pixel.

Secondary driver (selectors 1/3/4, packed 4 bpp, 160-byte rows, nibble
semantics, mode-13h present transform through `g41e`): see
`portable/gfx/gfx_planar.c`; both drivers sit behind the `gfx_*` API and
are selected by `display_mode`.

SDL3 sits below: VRAM (8 bpp) -> DAC palette -> RGBA texture -> window,
nearest-neighbour, integer scaled.  Selector 2 (`AE000_003`, CGA-class) is
out of scope until Milestone G.

## Timing model

- `timer_ticks` advances at PIT 1193182 / 0x13B1 ≈ 236.7 Hz from a
  fixed-step service (`portable/platform/sdl3/clock.c` -> `timer.h`).
- A dedicated timer thread mirrors the historical INT 8 body once per tick:
  `++timer_ticks; if (!sound_request_count && (sound_enabled || music_enabled))
  sound_tick_entry();`  The 13:1 BIOS chain is dropped.
- Game logic runs on its own thread and blocks in `timer_wait_ticks()` /
  `timer_deadline_wait()` exactly as the historical busy-waits did; rendering
  (texture upload) runs on the SDL main thread at display rate and never
  drives game ticks.
- Tests may run the timer in manual-step mode.

## Input model

SDL key events (main thread) feed two game-visible surfaces exactly as the
historical INT 9 handler + BIOS INT 16h did:

1. Level/edge state: `key_up_held`, `key_up_left_held`, `key_up_right_held`,
   `key_up_released`, `gb6a`, `keyboard_state[1]` (Ctrl), with the same
   scancode switch as `keyboard_irq_handler` (src/KEYBOARD.C) including the
   `g856`-gated aliases.
2. A BIOS-style keystroke FIFO (ASCII, scancode) for
   `keyboard_read_blocking_hotkeys()` / `keyboard_poll_nonblocking()` /
   `keyboard_buffer_drain()`; keys the handler swallows when
   `keyboard_state[0]==0` never reach the FIFO.

## Audio model

Game-owned SOUND.ASM state machine reimplemented in C (`portable/audio/
sound_driver.c`), serviced once per 236.7 Hz tick, emitting timestamped
events (OPL register writes, PIT ch2 divisor, speaker gate).  Backends:
Nuked-OPL3 (pinned) for OPL, a phase-continuous square-wave synth for the
speaker.  Mixed PCM is pushed to one SDL3 audio stream.  Event logs are the
first parity artifact; PCM comes after.  Output level: fixed per-source
gains (OPL 0.25, speaker 0.06 of full scale -- both synths are far hotter
than the original hardware) under a user master volume (`--volume`,
`audio.music_volume` / `audio.sound_volume` in `empires.json`): the OPL
chip is only ever driven by the music player, the PC speaker is shared,
so the driver marks the cue-stream helpers' events (`speaker_gate_on/off`,
`pit_channel2_set_divisor` -- the sound effects) with
`SOUND_ORIGIN_EFFECTS` and the speaker synth takes the volume of whichever
cluster wrote to it last.  Gains never touch the event timeline.

## Frame interpolation

The game draws one frame per `timer_deadline_arm(24)` ..
`timer_deadline_wait()` window (~9.86 Hz) and knows nothing about host
refresh rates.  `portable/gfx/gfx_tween.c` (`gfx_tween.h`) lets the SDL
presenter show that window at the display's rate with the moving sprites
interpolated, without touching game logic, its tick ratio or any pixel
the game produces:

- Capture (game thread): game code sets `gfx_tween_tag` around the
  player draw (`turn_loop_run`, `level_run_loop`) and the actor-record
  draws (`sprite_script_frame_driver`, `board_actors_draw`) -- plain
  stores, no behavioural effect.  The VGA driver's `vga_copy_rect` /
  `vga_vline` record a tagged blit (arguments, clip words, a copy of the
  bitmap, the pixels it is about to cover) and then draw exactly as
  before.
- Publish (game thread): `timer_deadline_wait()` reports the frame
  boundary through `timer_set_frame_observer`; the front end snapshots
  VRAM, the op list and the frame period (deadline minus the previous
  deadline, i.e. the game's own 24 ticks) as frame n.
- Compose (presenter thread, every refresh): start from frame n's VRAM,
  put back the covered pixels of every op (newest first), redraw each op
  with the same driver clip/draw code at `lerp(frame n-1, frame n,
  alpha)`, alpha being the host time's progress through the frame period
  since the publish.  Ops match between frames by tag and order; a
  per-axis jump over 48 px (projectile spawn, room change) is a teleport
  and is not smoothed.  When the game presents something after the
  publish without a new frame following (menus, dialogs, transitions),
  the presenter falls back to the live VRAM.

Interpolation is between consecutive game frames only: an actor whose
bytecode moves it every N-th frame still steps every N frames.  Pinned
deterministic replays keep it off; `test_gfx_tween` covers capture,
matching, teleports and the fallback.

## Host configuration

`portable/compat/config.c` (`config.h`) is a flat dotted-key store that
round-trips `empires.json` (strict JSON subset: nested objects of numbers,
booleans, strings; no arrays).  It holds host-side preferences only --
volume, fullscreen, directories, later tweening and in-game options --
never game state, and nothing in `portable/game/` reads it: the front end
resolves every setting at start-up and pushes it into the subsystem it
belongs to (mixer gain, window flags, resource directories).  Precedence:
built-in defaults < file < environment < command line; env/CLI overrides
are per run and are not written back.  The file is created with the
defaults on first run; a malformed file is reported and ignored.

## Testing

- Pure subsystem unit tests under `portable/tests/` (ctest).
- Golden fixtures under `portable/tests/fixtures/` are small and
  deterministic; bulk game data is never committed.  Tests needing
  `assets/AE000.DAT`/`AE001.DAT` skip cleanly when the assets are absent.
- Historical oracle harness (`tools/portable/oracle/`) uses the pinned DOS
  toolchain + MS-DOS Player only to *generate* fixtures; end users never
  need it.

## Deterministic replay

`--deterministic` removes the tick thread: the game's own waits
(`timer_wait_ticks`, `timer_deadline_wait`), polls (`timer_deadline_reached`,
the two busy-poll sites through `timer_poll()`) and blocking key reads
advance the 236.7 Hz clock themselves, scripted input and frame dumps run
from the tick observer on that virtual time, and the RNG seed is fixed.  A
scripted run is therefore byte-reproducible; `tools/portable/replay_test.py`
pins the presented frames' hashes (`portable/tests/replay/*.json`, ctest
`replay_*`, SDL dummy drivers).  Real-time mode is unchanged.

## Milestones

A skeleton -> B resource parity -> C framebuffer renderer -> D intro/menu ->
E first playable level -> F audio -> G full parity.  Historical build
(`python tools/build_exe.py verify`) must stay green at every milestone.

Status (2026-09-22): A-F reached; G in progress (sign-in, map, caverns,
puzzles, save/resume, Help/File/Options menus exercised by pinned replays;
the sound state machine is certified bit-exact against the original 8086
code by the Unicorn oracle; round-end/score and later caverns still being
swept).  Known intentional deviations: `farcoreleft()` mode downgrade and
the DOS critical-error/disk-reset paths retired; save slots persist as
overlay files instead of rewriting the archives; degenerate 0xN sprite
records and `gfx_blit_image` widths not divisible by 4 (historically
undefined) are no-ops / closed-form.
