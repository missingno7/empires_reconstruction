# Portability boundaries

Inventory of every DOS/8086/Turbo-C-specific surface the historical tree
touches, as evidence for the coming portable-C + SDL3 normalization. This is
a documentation-only snapshot (2026-09-21) built from README.md,
docs/current/tu-structure.md, docs/current/closure-frontier.md,
docs/current/asm-provenance.md, docs/current/sound-state.md, include/*.H,
src/*.C section banners, asm/*.ASM header comments, and direct greps for the
markers listed in the task (`__int__`, register pseudo-variables,
`interrupt`, `asm`, `getvect`/`setvect`, DOS calls, `farmalloc` family,
`MK_FP`/`FP_SEG`, `setjmp`/`longjmp`, `biostime`, `outportb`/`inportb`,
resource-record calls, `AE00*`). It does not speculate about the game
design behind any of this — every claim below cites a file and function
that is already in the tree.

The historical build stays the oracle throughout the port: nothing here
proposes changing `src/`, `asm/`, `include/` or the production plan; the
proposed abstractions are for a *new*, separate portable tree that the
historical build keeps being diffed against.

Reminder on scale: 71 modules total (60 C translation units, 10 hand-written
assembler modules, 1 runtime block); `docs/current/source-quality.json`
puts 261 members / 44257 bytes in plain C, 16 members / 1924 bytes in C with
inline-asm fragments, 61 members / 5758 bytes in symbolic hand-written ASM,
and 6571 bytes in the runtime block.

---

## 1. Video runtime / EGA / VGA

**Current source.** `asm/RUNTIME_BLOCK.ASM` (`_runtime_base`, the EGA/VGA
driver+library runtime, 6571 bytes, classified `HISTORICAL_LIBRARY_OR_RUNTIME`
in docs/current/asm-provenance.md) holds mode set and every blit primitive.
`runtime_dispatch` (asm/RUNTIME_BLOCK.ASM:150-155) reads the mode byte at
`ds:[0BFCDh]` (`display_mode`/`mode`, DS:BFCD) and jumps through
`runtime_mode_dispatch`, a table of five absolute CS words
(docs/current/runtime-boundary-recovery.md, "003C-0046"). The three mode
bodies found: `rt_01a1` (EGA/VGA planar-mode initializer: `int 10h` AX=0Eh
then AH=5/AL=1, three `int 10h` AX=1000h palette-register calls, then direct
port I/O on 03CEh/03C4h to program the Graphics/Sequencer registers and
clear the 4-plane, 0x1000-word screen), `rt_0258` (EGA palette-register
restore via ports 03DAh/03DEh, then falls into mode 9 = 320x200 16-colour),
`rt_0294` (`int 10h` AX=13h, mode 13h/VGA). `runtime_dispatch_3e2` is the
parallel dispatch for the blit-primitive family (also keyed on
`display_mode`). `src/VIDEO.C` is the C side: `video_mode_select` (called
from `src/STARTUP.C:video_mode_select`, wired through `bios_equipment_probe`
+ `video_adapter_detect` + `sound_backend_probe`) decides which of 1..5
`display_mode` gets set, and `video_alloc_framebuffer` (VIDEO.C, F_0281)
builds the row table.

**Deliberately unresolved wrinkle** (docs/current/runtime-boundary-recovery.md,
"Deliberately unresolved"): selectors 2 and 5 do not run the built-in
`runtime_base` at all — `F_490D` (`boot_init_seed_rand`, GAME.C) calls
`F_48BE` (ui_gfx_alloc) before `video_alloc_framebuffer`, and for those two
modes the built-in runtime at CS:039C is replaced wholesale by a
resource-loaded runtime (`AE000_003` for selector 2, `AE000_002` for
selector 5 — `tools/report_runtime_platform.py`). The normal built-in
dispatch only ever sees selectors 1, 3 and 4 in practice.

**Public API game logic calls.**
- `include/VIDEO.H`: `gfx_box()`, `gfx_bar()`, `gfx_clear_rect()`,
  `gfx_fill_rect()`, `gfx_wipe_rect()`, `gfx_copy_rect()`,
  `gfx_blit_bitmap()`, `gfx_vline()`, `gfx_save_rect()`,
  `gfx_restore_rect()`, `gfx_copy_rect_flip_v/h/hv()`,
  `gfx_copy_rect_split[_flip_v]()`, `int gfx_draw_char()`,
  `gfx_blit_image()`, `gfx_set_pixel()`, `int gfx_get_pixel()`. All
  declared old-style K&R (`extern void gfx_box();`) — the header's own
  comment explains why: a typed prototype changes int/near/far promotion at
  call sites and would move reconstructed bytes off the exact original.
  A handful of TUs keep a locally typed prototype instead (listed in
  VIDEO.H's own "Exceptions" comment: HUD.C, BOARDDRW.C, INTRO.C,
  SLOTMENU.C, ROUNDEND.C, HITTEST.C, VIDEO.C).
- `src/VIDEO.C`: `video_mode_select()`, `video_alloc_framebuffer()`,
  `video_set_text_mode()`, `video_load_palette(char far *pal)`,
  `gfx_color_select(int i)`, `color_lookup_tables_init()`,
  `rect_border_draw(int,int,int,int)`.
- `src/STARTUP.C`: `bios_equipment_probe()`, `video_adapter_detect()`
  (sets `display_mode`).

**Implementation form.** RUNTIME_BLOCK.ASM: pure hand-written TASM (word-
public segment, `JMP_NEAR`/`CALL_NEAR` macros, direct `int 10h` and port
I/O). VIDEO.C: plain C plus two inline-asm fragments recorded as
irreducible in docs/current/closure-frontier.md §1 — `F_01BC
video_load_palette` (`asm les dx,pal`, 3 bytes vs. the 8-byte
`_ES=`/`_DX=` pseudo-register form) and the `-B`-anchoring inline asm in
STARTUP.C's `video_adapter_detect`/`sound_backend_probe` (F_50D2/F_53BF,
recovered as C with `__int__`/`__inportb__`/`__outportb__`, with a residual
`asm mov display_mode,N` byte store whose NOP padding is positive evidence
of TCC-generated inline asm — see closure-frontier.md §1 and
asm-provenance.md's "Promoted to C this wave").

**Proposed portable abstraction.** A `video_backend` API carrying: mode
select (`video_set_mode(int display_mode)` — but note `display_mode` is a
*driver selector* 0..5 tied to 1980s adapter classes, not a resolution/depth
pair; a portable layer should translate it once into an explicit
`{width,height,bpp,planar|chunky}` descriptor rather than propagating the
enum), a framebuffer-row-pointer table equivalent to `g3924[]` (488 rows,
one far pointer per row, built by `video_alloc_framebuffer`/
`video_normalize_far_ptr`), palette load (`video_load_palette(pal[256])`),
and the primitive set 1:1 with VIDEO.H (`box/bar/clear_rect/fill_rect/
wipe_rect/copy_rect[_flip_v|_flip_h|_flip_hv|_split[_flip_v]]/blit_bitmap/
vline/save_rect/restore_rect/draw_char/blit_image/set_pixel/get_pixel`).
State the historical code keeps in DGROUP that the abstraction must expose:
`cur_idx`(DS:3902), `mode`/`display_mode`(DS:BFCD, two views of the same
byte — VIDEO.C keeps both, see the file's own comment), the three per-mode
palette tables `g3904`/`gbe`/`gfe`, `result`(DS:40C8, "current color"),
`g3924[]` (row-pointer table) and `g40ca` (framebuffer base far pointer).
Note VIDEO.C's own comment on `ui_gfx_blob`/DS:C5CA (LAYOUT.H): several
routines depend on it being pushed as a bare 2-word far pointer, not a typed
`char far *` local — that ABI sensitivity disappears once the port owns its
own calling convention, but the *data* (which two things are adjacent, what
the caller expects back) must carry over.

**Does SDL3 replace it directly?** Only the outermost layer (get a
resizable RGBA/8-bit-indexed surface on screen, present it, handle palette
as an `SDL_Palette`). The planar EGA/VGA primitive semantics (nibble
clipping in `gfx_bar`/`gfx_vline`, the four flip/split copy variants, the
32-byte bitmap header format `gfx_copy_rect`/`gfx_blit_bitmap` read) are
game-owned drawing semantics that must be reimplemented against a linear
buffer and only *then* blitted to an `SDL_Texture`; SDL3 does not have an
EGA-planar equivalent to fall back on.

**Risks.** `gfx_copy_rect` clips against "the board viewport at
ds:[94h]/ds:[96h]" (VIDEO.H comment) — a global clip rectangle read by the
runtime block, not passed as an argument; the port must find and thread
that state explicitly or every clipped blit will misbehave. The selector-2/5
runtime-replacement path (AE000_002/003) means a portable "video backend"
is not simply "the code in RUNTIME_BLOCK.ASM" — for two of five display
modes the *replacement* runtime resource is the actual implementation and
must be decoded and analyzed on its own (see §11 and
docs/current/runtime-boundary-recovery.md's "Deliberately unresolved").

---

## 2. Palette operations

**Current source.** `src/VIDEO.C`: `video_load_palette(char far *pal)`
(F_01BC, `int 10h` AX=1012h, BX=0, CX=0x100 — load a 256-entry EGA/VGA DAC
block), `gfx_color_select(int i)` (F_01CE, selects `result` from one of
three per-mode tables `g3904`/`gbe`/`gfe` keyed on `mode`),
`color_table_entry_set(int,int,int)` (F_0215), `color_lookup_tables_init()`
(F_0232, `movmem` seeds `g3904`/`gbe` from two 0x20-byte DGROUP blobs
`g9c`/`gde`). `asm/RUNTIME_BLOCK.ASM`'s `rt_0258` additionally reprograms
the EGA Attribute Controller (ports 03DAh index / 03DEh data) directly —
this is a second, lower-level palette path the runtime block owns outright,
separate from the `int 10h AX=1012h` DAC path VIDEO.C uses.

**Public API.** `video_load_palette()` is called from
`video_alloc_framebuffer()` for modes 5 (`g11e`) and 4 (`g41e`) with
mode-specific 256-entry tables; `gfx_color_select`/`color_table_entry_set`/
`color_lookup_tables_init` are the index-to-RGB(-equivalent) plumbing other
drawing code (`result`, DS:40C8) reads before every `gfx_set_pixel` call
(VIDEO.H comment on `gfx_set_pixel`: "merging into whichever nibble the x
parity selects", using `ds:[40C8h]`).

**Implementation form.** C with one inline-asm fragment (`asm les dx,pal`)
for the DAC path; pure hand-written TASM (direct port I/O) for the EGA
Attribute Controller path in RUNTIME_BLOCK.ASM.

**Proposed portable abstraction.** `video_palette_load(const uint8_t rgb[256*3])`
mapping 1:1 onto `SDL_SetPaletteColors`/`SDL_SetSurfacePalette` for indexed
modes, plus a `video_color_select(int logical_index)` that resolves through
the *same* three-table indirection (`g3904`/`gbe`/`gfe`) so the per-mode
palette remap (mode 5 vs 2 vs other) is preserved as game logic, not
silently baked into a single fixed-size RGB LUT.

**SDL3 relationship.** Direct replacement for the "load 256 DAC entries"
half; the EGA Attribute Controller path (`rt_0258`) has no SDL3 analogue —
it is 1980s-hardware register poking whose *effect* (a specific palette-
register set for the EGA 16-colour path) must be reproduced by explicit
color-table logic in the port, not by any hardware call.

**Risks.** None involving exact timing; the risk is purely "silently
wrong colors" if the three-table indirection in `gfx_color_select` is
collapsed into one table during the port.

---

## 3. Keyboard IRQ + BIOS input

**Current source.** `src/KEYIRQ.C` (translation unit `C_695E_697D`):
`keyboard_irq_install()` (F_695E) — `int9_saved_vector = getvect(9);
setvect(9, (void (far *)()) keyboard_irq_handler);` — and
`keyboard_irq_restore()` (F_697D) — `setvect(9, int9_saved_vector)`.
`src/KEYBOARD.C` (TU `C_6990_6B74`): `keyboard_irq_handler()` (F_699E, the
actual `interrupt`-qualified INT 9 handler), `keyboard_chain_enable/disable/
active()` (F_6990/F_6997/F_6B74, the `keyboard_state[0]` chain gate),
`keyboard_read_blocking_hotkeys()` (F_6B1A, inline-asm `int 16h` plus an
F1..F10 hot-key dispatch into `menu_list_active`/`menu_loop_run`),
`keyboard_poll_nonblocking()` (F_6B4A, inline-asm `int 16h` AH=1 poll),
`keyboard_buffer_drain()` (F_6B66, C via `_AX`/`__int__(0x16)`).

**Public API game logic calls.** `keyboard_irq_install/restore()` (called
from `boot_init_seed_rand`/`game_shutdown` in GAME.C),
`keyboard_chain_enable/disable/active()`, `keyboard_read_blocking_hotkeys()`,
`keyboard_poll_nonblocking()`, `keyboard_buffer_drain()`. Game-visible state:
`key_up_held`, `key_up_left_held`, `key_up_right_held`, `key_up_released`,
`keyboard_state[]` (DS:0B72: `[0]` chain gate, `[1]`/`gb74` modifier state),
`gb6a` (a fifth key state), `b856`/`g856` (a display-mode-derived scancode
remap gate). 13 callers across DIALOG.C, GAME.C, HELPMENU.C, INTRO.C,
KEYBOARD.C, KEYIRQ.C, LEVEL.C, MENULOOP.C, OPTIONS.C, PLAYERSL.C, PROMPTS.C,
PUZZLE.C, SLOTMENU.C.

**Implementation form.** `keyboard_irq_handler` (KEYBOARD.C:699E) is C using
Turbo C's `interrupt` keyword and the hardware intrinsics `__sti__()`,
`__inportb__(int)`, `__outportb__(int,unsigned char)` — no inline `asm`
statements. The two BIOS-poll helpers (F_6B1A/F_6B4A) are C functions whose
*entire body* is inline asm (recorded as irreducible in
docs/current/closure-frontier.md §1: "branch on ZF straight after `int 16h`").
KEYIRQ.C's `getvect`/`setvect` calls are plain C, but the header comment
explains why F_699E cannot join KEYBOARD.C's TU: its `push cs` widening of a
NEAR handler to FAR needs the *non-interrupt* declared view of the symbol,
while the `interrupt`-qualified definition needs the interrupt view — one
declaration cannot serve both, which is why KEYIRQ.C is its own TU
(docs/current/tu-structure.md, "Runs rejected by the oracle").

**Proposed portable abstraction.** `input_poll_events()` that produces
scan-code make/break edges matching the handler's own dispatch
(`down = scan < 0x80`, `released = (scan & 0x80) != 0`, plus the E0/E1
prefix-byte ack-and-discard path at the top of `keyboard_irq_handler`) —
i.e. the abstraction is scancode-shaped, not ASCII-shaped, because game
logic reads `key_up_held`/`key_up_left_held`/etc. as level state, not text
input. A *separate* `input_read_hotkey_blocking()`/`input_poll_hotkey()`
pair covers the BIOS `int 16h` path used for menu hotkeys (F1..F10) and
text entry, which is edge/ASCII-shaped and must stay a distinct API from
the level-state one.

**SDL3 relationship.** SDL3 keyboard events (`SDL_EVENT_KEY_DOWN/UP` with
`SDL_Scancode`) replace both the IRQ9 handler *and* the `int 16h` BIOS calls
directly — there is no game-visible IRQ-ordering dependency here (contrast
§4/§5): the handler only maintains level/edge flags and dispatches a
handful of immediate actions (sound-effects toggle on scancode 0x1F,
Ctrl-state tracking on 0x1D), none of which depend on being serviced
between specific timer ticks.

**Risks.** Low. The one behavior worth verifying against the historical
build: the E0/E1 prefix bytes are acknowledged (PIC EOI at port 0x20 plus
the 0x61 keyboard-controller ack toggle) and then the handler *returns
without processing the following byte as a normal scancode* — an SDL3-based
replacement does not need to replicate this (SDL already gives semantic
scancodes), but any test harness that replays raw scancodes at the old
handler for parity checking must reproduce it.

---

## 4. Timer IRQ + timing

**Current source.** `src/TIMER.C` (TU `C_6B7A_6C87`, entirely inline-asm or
asm-adjacent): `timer_irq_install()` (F_6B7A), `timer_irq_restore()`
(F_6BAC), `timer_irq_handler()` (F_6BCF, `interrupt`-qualified), then plain-C
helpers `timer_wait_ticks(int n)` (F_6C26), `timer_deadline_arm(int n)`
(F_6C57), `timer_deadline_wait()` (F_6C6F), `timer_deadline_reached()`
(F_6C87).

**PIT reprogramming (`timer_irq_install`, all inline asm):**
```
mov al,36h ; out 43h,al        ; channel 0, mode 3, lobyte/hibyte, binary
mov ax,13b1h ; out 40h,al      ; divisor low byte  -> channel 0
mov al,ah    ; out 40h,al      ; divisor high byte -> channel 0
mov al,0b6h  ; out 43h,al      ; channel 2, mode 3 -- ARMS channel 2 for the
                                 sound driver (asm/SOUND.ASM writes its own
                                 divisor to port 42h later; no divisor is
                                 written here)
```
Divisor 0x13B1 = 5041 decimal; PIT base 1,193,182 Hz / 5041 ≈ **236.7 Hz**
IRQ0 rate while the game runs (vs. the BIOS-default 18.2 Hz / divisor
0x10000). `timer_irq_restore()` reverses only channel 0 (`mov al,36h; out
43h,al` then a zero divisor = default 65536 count, i.e. back to ~18.2 Hz);
it does not touch channel 2's mode.

**Handler (`timer_irq_handler`, F_6BCF — the historical INT 8 chain):**
```c
void interrupt timer_irq_handler(void)
{
 asm pushf;
 __sti__();
 ++timer_tick_phase; if(timer_tick_phase>=13) {timer_tick_phase=0;int8_saved_vector();}
 ++timer_ticks;
 if(!sound_request_count && (sound_enabled || music_enabled)) sound_tick_entry();
 __outportb__(0x20,0x20);
 asm popf;
}
```
This is the **13:1 chaining** cited in the task: every 13th fast tick (at
236.7 Hz) calls the *original* BIOS INT 8 vector (`int8_saved_vector`,
saved by `timer_irq_install`'s `int 21h` AX=3508h `getvect`-equivalent),
so BIOS-dependent code elsewhere (the 18.2 Hz `biostime`/midnight-rollover
counter at 0040:006C) keeps advancing at its expected rate — 236.7/13 ≈
18.2 Hz, matching the BIOS default exactly. `timer_ticks` (unsigned long,
DS:0B76/0B78) is the free-running fast-tick counter; `sound_tick_entry()`
runs at the full 236.7 Hz whenever `sound_request_count==0` and either sound
or music is enabled. The bare `pushf`/`popf` is recorded as irreducible
in docs/current/closure-frontier.md §1 ("F_6BCF timer_irq_handler ... bare
pushf/popf around the body; __emit__ would only hide it").

**Public API.** `timer_irq_install()`/`timer_irq_restore()` (from
`boot_init_seed_rand`/`game_shutdown`, GAME.C); `timer_wait_ticks(int n)`,
`timer_deadline_arm(int n)`, `timer_deadline_wait()`,
`timer_deadline_reached()` — used across ANIMSTEP.C, BOARD.C, DIALOG.C,
GAME.C, INTRO.C, LEVEL.C, PUZZLE.C, ROUNDEND.C, SLOTMENU.C, SNDREQ.C for
animation pacing and UI delays. `gc0d0` (DS:C0D0, the armed deadline) is the
DGROUP state the deadline pair shares.

**Implementation form.** `timer_irq_install`/`timer_irq_restore` are C
functions whose entire body is inline asm (irreducible per
closure-frontier.md §1: "explicit push ax/dx/ds/es around DOS calls and
`push cs / pop ds`: no C expression saves caller registers"). The handler
is C with the `interrupt` keyword, one `asm pushf`/`asm popf` pair, and the
hardware intrinsics `__sti__()`/`__outportb__()`. The four tick-arithmetic
helpers are plain C (rule evidence in TIMER.C's own comments: the `while`
loops compile to a leading `EB 00`/`EB 04` zero-body jump-to-test, and the
`<`/`<=` comparisons on `timer_ticks`/`gc0d0` are all `jb`/`ja`/`jae`,
proving both sides are `unsigned long` — tc20-codegen "rule 5").

**Proposed portable abstraction.** A `timer_service` with: (1) a
free-running monotonic tick counter at a *declared* fixed rate (the port
should keep 236.7 Hz, or whatever rate game-visible pacing turns out to
require — see risks), replacing `timer_ticks`; (2) an arm/wait deadline
pair (`timer_deadline_arm(n)`/`timer_deadline_wait()`/
`timer_deadline_reached()`) that keeps the exact "ticks elapsed" semantics
so `timer_wait_ticks` callers don't need to change; (3) a tick-driven hook
that invokes the sound engine's per-tick service (`sound_tick_entry`)
exactly once per abstracted tick, gated the same way
(`!sound_request_count && (sound_enabled||music_enabled)`). The 13:1 BIOS
chain itself is DOS-specific plumbing with no portable-code purpose (no
other subsystem in this tree reads the BIOS tick counter directly — see
`biostime` in §9, called once at boot) and should NOT be reproduced; it
existed only to keep the *DOS environment's own* consumers of INT 8 (BIOS
tick count, possibly the DOS scheduler on some TSRs) alive while the game
had the vector.

**SDL3 relationship.** Only sits below a game-owned layer. SDL3 gives
wall-clock time (`SDL_GetTicks`/`SDL_GetPerformanceCounter`) and a way to
run a fixed-step loop, but the *game-visible tick rate* (236.7 Hz, and
everything paced in units of it via `timer_wait_ticks`/`timer_deadline_*`)
is a piece of historical game behavior that the port must reproduce as
explicit fixed-step logic, not delegate to whatever frame rate SDL renders
at.

**Risks.** This is the sharpest timing-observability boundary in the tree.
`sound_tick_entry` runs synchronously *inside* the timer ISR in the
historical build — sound state changes happen at IRQ time, not at the
game's main-loop cadence. `timer_wait_ticks`/`timer_deadline_wait` are
busy-wait spins on `timer_ticks`, so any UI pacing that read "N ticks
passed" is exactly N/236.7 seconds historically; a port that free-runs the
main loop and separately drives a game-visible clock at a *different* rate
will desync animation/dialog timing from the original. The 13:1 constant
and the divisor 0x13B1 are both required-verification facts (see §(c)).

---

## 5. Sound

**Current source.** Two clusters, per docs/current/sound-state.md:

1. **asm/SOUND.ASM** (`M_C1A0_CB48`, 2492 bytes, one hand-written TASM
   module — see asm-provenance.md for the frame/SI-DI/REPT-macro evidence
   that closed all 18 former "reconstruction artifact" C wrappers). Public
   routines: `_sound_tick_entry`, `_sound_voice_pump_loop`,
   `_sound_voice_table_prime`, `_sound_voice_service_loop`,
   `_sound_command_stream_dispatch`, `_sound_control_value_select`,
   `_sound_command_value_derive`, `_sound_control_block_advance`,
   `_sound_secondary_cmd_dispatch`, `_sound_command_flags_update`,
   `_voice_percent_scale_store`, `_sound_param_scale4`, `_f_c5a8`,
   `_sound_table_word_select_store`, `_f_c5c6`,
   `_voice_command_decode_apply`, `_voice_enable`, `_voice_disable`,
   `_sound_pit_divisor_program`, `_sound_voices_reset_and_service`,
   `_sound_backend_select_init`, `_sound_voice_table_reload`,
   `_sound_voices_reset`, `_sound_voices_disable_all`,
   `_opl_register_write`, `_opl_port_write_byte`, `_sound_tick_step`,
   `_sound_stream_command_step`, `_sound_note_dispatch`, and more (41
   register-ABI routines total per asm-provenance.md's ranked actions).
2. **OPL music bank**: `src/OPLREG.C` (register writes:
   `voice_write_level/connection/op1_attack_decay/op2_attack_decay/
   envelope_flags/waveform`, `opl_set_depth_flags`, `opl_set_note_select`,
   `opl_register_write`, `int opl_detect()`), `src/OPLVOICE.C`
   (`voice_bank_retune_on(bank,base)`, `voice_bank_update(bank)`,
   `voice_update_tone(v)`, `voice_level_table_reset()`), `src/OPLINIT.C`
   (`opl_init()` — silences all 9 voices then enables), `src/MUSIC.C`
   (pure arithmetic: `music_note_to_divisor`, `music_build_octave_table`,
   `music_reset_tuning_tables`, `music_voice_frequency_lookup` — no I/O,
   feeds the tables `voice_bank_retune_on`/`voice_update_tone` read).

**Backend selection (`snd_backend_mode`, DS:1778, 0..3):** 0 = PC speaker
gate (port 0x61 bits, direct `in/out 61h` in `_voice_enable`/`_voice_disable`
in SOUND.ASM), 1 = Tandy/PCjr-style gate (same port, different bit pattern),
2 = "queued OPL bank" (the 4-voice effects driver pushes into
`voice_bank_retune_on(bank,base)`/`voice_bank_update(bank)`, i.e. 3 real OPL
voices per queued bank — SOUND.ASM's `C678_queue_value`/`C6B9_queue_stop`),
3 = a one-shot VGA-port probe that immediately collapses back to mode 1
(`sound_backend_probe`, STARTUP.C F_53BF, probes port 0x203 read-back before
falling through). `_opl_register_write` (SOUND.ASM) is the actual OPL bus
protocol: `out dx,al` (register-select port `_opl_port`, 0xC0 or 0x205),
6x `in al,dx` settling reads, `out dx,al` on `opl_port+1` (data), then a
`rept 35 / in al,dx / endm` settling delay — this exact instruction-count
delay is a real OPL2/OPL3 timing requirement, not incidental.

**PIT channel 2 (speaker tone):** SOUND.ASM writes divisor bytes straight to
port 0x42 (`_sound_pit_divisor_program`, mode-0 branch: `out 42h,al; mov
al,ah; out 42h,al`) — channel 2's *mode* was armed once by
`timer_irq_install` (see §4, `mov al,0b6h; out 43h,al`), so the sound driver
never needs to touch port 0x43 itself.

**Public API (include/SOUND.H, docs/current/sound-state.md).** The header
declares the DGROUP state every C consumer types identically: two loaded
far-pointer pairs (`snd_seg`/`snd_base`, `snd_seg2`/`snd_base2`), master
enables `sound_enabled`(DS:176E)/`music_enabled`(DS:1772), the "a stream is
armed" flag `snd_on`(DS:1770), `mus_flag`(DS:1774), `snd_flag2`(DS:1776),
`snd_backend_mode`(DS:1778), `snd_nvoices`(DS:177A, 1 or 4), per-voice
4-word tables `v_b/v_ctr/v_a/v_hold/v_len` and
`voice_stream_cursor_table/voice_stream_base_table/voice_rest_table`,
`notetab[12]` (note-to-PIT-divisor), `opl_port`, and the single-stream
"music stream" scalar twins `mus_ptr/mus_arg/snd_len/snd_delay/
stream_note_delay/snd_one`. Six fields are recorded LOW-confidence
(mechanism understood, semantic meaning not established): DS:17C4, 17CC,
17D4, 17DC, 17F4, 1E8C, plus two internal tables at 182C/1832 with no C-side
name at all (sound-state.md "Open items").

**Implementation form.** SOUND.ASM: pure hand-written TASM (confirmed by
frame-fingerprint, missing SI/DI saves, fixup-free internal calls, REPT
macro — asm-provenance.md). OPLREG.C/OPLVOICE.C/OPLINIT.C/MUSIC.C: pure C
(MUSIC.C explicitly documented as having "No inline asm"). `src/TIMER.C`'s
handler calls `sound_tick_entry()` (extern into SOUND.ASM) directly from
INT 8 context (see §4) — this is the one place the timing boundary and the
sound boundary are the same event.

**Proposed portable abstraction.** Two-layer split matching the historical
two-cluster design: (1) a low-level `audio_backend` that SDL3's audio
device replaces directly — a PCM callback fed at whatever host sample
rate, with a *synthesized* OPL2 (or a real chip emulator) driving three
possible waveform sources (square-wave speaker emulation for modes 0/1,
real OPL2 register writes for mode 2/3); (2) a game-owned `sound_driver`
layer that reimplements the SOUND.ASM state machine (voice command-stream
cursor/dispatch, the note tables, the "armed stream" protocol) exactly as
data-driven logic, producing the same register-write/PIT-divisor stream
SOUND.ASM does today — that stream is then fed to whichever backend
(PC-speaker square wave synth, or a real/emulated OPL2) is active. The
sound-state.md field map is the direct source for this struct's fields;
several are still LOW-confidence and should be named from `asm/SOUND.ASM`'s
own comments before or during the port rather than guessed.

**SDL3 relationship.** SDL3's audio device sits *below* the game-owned
sound driver, not in place of it — SDL3 has no OPL2/PC-speaker/PIT-divisor
concept; the port must own the entire note/voice/backend-mode state machine
and only hand SDL3 a rendered PCM stream (or drive a bundled OPL2 emulator
whose output feeds SDL3 audio). The `int 21h`/register-ABI parts of the
driver (calling convention, SI-as-voice-index, ES:DI table walks) disappear
entirely once reimplemented as portable C — they are pure 8086-ABI
artifacts with no game-visible semantics of their own.

**Risks.** `sound_tick_entry` executes inside the timer ISR (see §4) at
236.7 Hz — the sound engine's per-tick pacing (`v_ctr`/`snd_delay`
countdowns, note-off detection at `ctr==len-1`) is defined in units of that
exact tick rate; a port that services sound from a different clock (e.g.
once per audio callback, or once per render frame) will change note timing
unless it explicitly reproduces the 236.7 Hz service cadence. The
DS:1778 "backend mode" word is confirmed (via
docs/current/interface-conflicts.json's shared evidence id) to also be read
as `snd_paused` in one branch (`_sound_control_value_select`'s
`C359_paused_mode` label) — sound-state.md flags this as needing a human
call before any rename, and the port must preserve whatever that dual
meaning turns out to be. The `_opl_register_write` 35-iteration `in al,dx`
delay is a real hardware settling requirement for genuine OPL2 chips; a
port targeting a software OPL2 emulator (e.g. Nuked-OPL3) needs to confirm
the emulator's own timing model doesn't need this delay reproduced (most
emulators model settling internally), and a port targeting real
hardware-through-passthrough would need to keep an equivalent delay.

---

## 6. DOS filesystem calls

**Current source.** `src/RESOURCE.C` (TU `C_6266_68AA`): declares
`extern int open()`, `extern int read(int,void far*,unsigned)`,
`extern int write(int,void far*,unsigned)`, `extern int close(int)`,
`extern long lseek(int,long,int)` (all CC.LIB wrappers around DOS handle
I/O) and implements `resource_file_open(int i)` (F_6266 — opens the
directory-indexed data file, retries with a case-folded first letter,
raises a `dialog_run` prompt on stamp mismatch), `resource_file_write_record
(unsigned index, void far *data)` (F_643A), `disk_reset_retry(int drive)`
(F_652A — `int 13h` AH=0 reset then AH=2 CHS read retry, then `int 21h`
AH=0x0D DOS disk-reset), `resource_load_record(unsigned p)` (F_656C — opens,
`lseek`s to two index-table offsets, computes a record length, `lseek`s to
the record and `read`s it, then dispatches to one of four decoders based on
a header flag byte), `resource_load_record_alloc(int a, char far * far *pp)`
(F_684A — allocates via `farmalloc`, tears down and calls `exit()` on
allocation failure), `resource_load_record_into(unsigned a, char far *q)`
(F_68AA). `src/STARTUP.C`'s `dos_write_handle2(char far *text)` (F_4F63,
`int 21h` AH=0x40, handle 2 = stderr) is the DOS-console-write half; also
used from RESOURCE.C's `resource_load_record_alloc` and from
`video_mode_select`'s early-exit error paths.

**Public API game logic calls.** `resource_load_record(p)` and its two
wrappers are called from 16 units: ANIMFRAM.C, BOARD.C, GAME.C, HELPMENU.C,
HINTDLG.C, HUD.C, INTRO.C, LEVEL.C, MENURES.C, PLAYERSL.C, PLRLDPUB.C,
PUZZLE.C, RESCACHE.C, RESOURCE.C, ROUNDEND.C, SLOTS.C — this is the game's
one and only resource-loading entry point (see §11). `resource_file_open`/
`resource_file_write_record` are the save-slot record I/O
(`c470_record`/`slot_table`, include/C470.H).

**Implementation form.** Plain C for `open`/`read`/`write`/`close`/`lseek`
call sites (CC.LIB DOS wrappers, not inline asm). `disk_reset_retry` is C
using Turbo C register pseudo-variables and `__int__` (documented in the
file as "Exact 66-byte BIOS retry helper ... no inline ASM or byte
emission"). `dos_write_handle2` is the same pattern (register
pseudo-variables + `__int__(0x21)`, "no inline ASM or byte emission").

**Proposed portable abstraction.** `resource_io` with: `resource_open(int
directory_index)` (case-fold retry semantics kept as an explicit fallback
list, not OS-specific case sensitivity — the historical retry exists
because DOS FAT is case-insensitive but the directory table stores a
specific case, so on a case-sensitive host filesystem this logic should be
resolved once at asset-packaging time rather than reproduced as a runtime
FS quirk), `resource_load_record(unsigned packed_index)` returning
a byte buffer (index encodes directory-in-high-nibble +
record-in-low-12-bits — keep this encoding as the on-disk/API contract
since it is also the AE000/AE001 layout, §11), and
`resource_load_record_alloc`/`_into` as thin wrappers. `disk_reset_retry`
and the DOS critical-error interaction (§9) have no portable meaning at all
(no host filesystem raises "drive not ready, retry?" through a game-visible
callback) and should be dropped, not translated — a failed host file read
becomes an ordinary I/O error.

**SDL3 relationship.** SDL3 replaces none of this directly (SDL_IO is a
generic stream abstraction the port could use for the *mechanics* of
reading a resource-archive file, but the DOS-handle semantics, the
`AH=0x40` console write, and the CHS-retry dance are pure DOS API surface
with a 1:1 portable substitute being plain C `fopen`/`fread`/`fseek` or an
SDL3 IO stream — not an SDL3-specific concept).

**Risks.** Low for correctness once the archive format (§11) is understood;
the only historical-timing-observable piece is that `resource_load_record`'s
retry loop calls `dialog_run()` (a modal UI loop) on failure — i.e. a
resource load failure historically blocks on user input via the normal
dialog/keyboard/timer machinery, not a silent error path. A port must keep
that either as a real modal or as an explicit decision to fail differently.

---

## 7. Far-pointer / segment assumptions

**Current source.** `src/VIDEO.C` defines `MK_FP`/`FP_SEG` locally
(`#define MK_FP(seg,ofs) ((void far *)(((unsigned long)(seg)<<16)|(unsigned)(ofs)))`)
and uses them in `video_normalize_far_ptr(unsigned o, unsigned s)` (F_025B —
"normalise a far pointer: carry the top 12 bits of the offset into the
segment and keep the low nibble") and `video_alloc_framebuffer()` (builds
`g3924[]`, the 488-entry row-pointer table, by repeatedly normalizing
`q + w` and storing it). `src/PLAYERSL.C` defines the same macros locally
and uses them at lines 259-262 to carve three regions
(`ui_gfx_blob`/`ui_gfx_shadow_a`/`ui_gfx_shadow_b`) out of one `farmalloc`
block by paragraph arithmetic (`s = FP_SEG(p)+1; p = MK_FP(s,0x0e); ... =
MK_FP(s+1,0); ... = MK_FP(s+0x7d4,0)`). `asm/SOUND.ASM` and the sprite/blit
ASM modules use ES:DI/ES:SI as an implicit calling convention instead of
explicit far-pointer parameters — e.g. `asm/DRAWQ.ASM`'s
`draw_queue_render[_highlighted]` render "an ES:DI command list" (file's own
header comment), and SOUND.ASM's per-voice routines take the voice index in
SI and read/write `[si+_offset]` throughout (documented exhaustively in
docs/current/sound-state.md's field map, e.g. `_v_ctr[4]` at DS:179C
accessed as `[si+_g179c]`).

**Public API.** No dedicated header for this — it's a convention embedded
in dozens of function signatures (`char far *`, `void far *`, `int far *`)
across VIDEO.H (`gfx_blit_bitmap(char near *,int,char far *)` in HUD.C's
local exception), DIALOG.H (`struct dialog far *`), C470.H
(`int slot_row_draw(struct c470_record far *p,int,int)` — header comment:
"The far prototype is byte-significant at the call sites"), GC0FE.H
(`int (** far callbacks)()`).

**Implementation form.** Plain C macros/casts (VIDEO.C, PLAYERSL.C); ASM
register-convention (SOUND.ASM, the draw-queue and sprite modules) for
ES:SI/ES:DI.

**Proposed portable abstraction.** Replace every `far`/`near`/segment:offset
pointer with an ordinary flat pointer in the portable tree; the *paragraph-
rounding* allocation pattern (`FP_SEG(p)+1` to round up to the next 16-byte
paragraph, `MK_FP(s+N,0)` to carve fixed-size regions out of one block)
should become an explicit `struct { uint8_t *blob, *shadow_a, *shadow_b; }`
allocated with ordinary alignment (or three separate allocations) — the
paragraph math itself carries no game semantics, only "these three buffers
must not overlap and shadow_b must be big enough for the largest decode
staging use," which the port should re-derive from the actual buffer sizes
(0x7d4 paragraphs = 32000 bytes between `ui_gfx_shadow_a` and
`ui_gfx_shadow_b`, PLAYERSL.C) rather than keep as paragraph arithmetic.
The ES:SI/ES:DI ABI conventions in SOUND.ASM and the draw-queue/sprite
modules become ordinary struct-pointer-plus-index parameters once those
modules are reimplemented as portable C — see §11's "decoders" and §5's
sound-driver layer, which are the natural homes for that translation.

**SDL3 relationship.** Not applicable — this is a C-level/ABI concern SDL3
has no bearing on.

**Risks.** `video_normalize_far_ptr`'s "carry high offset bits into segment"
step exists only because 8086 far pointers are not unique (the same linear
address has many segment:offset encodings) and downstream code compares/
adds these pointers expecting a canonical form; a flat-pointer port has no
such non-uniqueness and can drop this step entirely, but must verify no
other code path relies on the *specific* canonical form (e.g. offset always
in 0..15) for a bit-packing trick. None found in the files read for this
inventory, but the sound-state.md report explicitly flags similar
"structural finding" risk for adjacent-table math (§5), so the port should
re-verify this per call site during implementation, not assume it from this
document alone.

---

## 8. 16-bit integer semantics

**Current source.** Documented pervasively as codegen-significant, not
incidental. Direct examples already in the tree's own comments:
- `src/TIMER.C`, `timer_deadline_reached()`: "`ja`/`jb`/`jae` on the two
  halves makes both sides UNSIGNED long (rule 5)"; `timer_wait_ticks`:
  "the comparison is `jb` twice, so both sides are UNSIGNED long ... the
  parameter is widened with `cwd`, so IT is a signed int."
- `src/RESOURCE.C`, `resource_load_record`: "`shr` (UNSIGNED) with the
  count in CL, so p is unsigned and the shift is not an arithmetic one"
  (`d = p >> 12`); "`(int)o2 - (int)o1` [is] a SIXTEEN-bit subtraction of
  the two longs' low words ... Writing `(int)(o2 - o1)` would have emitted
  the 32-bit sub/sbb pair first and then truncated" — i.e. the record
  length computation is deliberately truncating, and reproducing it with a
  wider intermediate changes behavior for any record whose length crosses
  a 16-bit-truncation boundary the original relied on.
- `src/VIDEO.C`, `video_alloc_framebuffer`: "the size is `(long)w*488+16`
  and the `cwd` before the long multiply makes `w` a SIGNED int widened to
  long (rule 16's shape)".
- `src/RESOURCE.C`, `resource_file_open`: `c = 0x42 - c + 0x41` "stays in
  AL: char arithmetic in a byte register when operands and destination are
  all chars" — an explicit signed-char-view dependency (the case-fold byte
  math).
- `include/GB3AF.H`/`GC316.H`: explicit `unsigned char flag`/`char kind`
  split documented per-field because different consumers read the *same*
  byte through different signedness (GC316.H: "F_8AA2 casts to unsigned for
  its 0xff sentinel compare").

**Public API / where it matters.** Every arithmetic boundary above sits on
a function already listed in another section (`timer_deadline_reached`,
`resource_load_record`, `video_alloc_framebuffer`, `resource_file_open`) —
this is not a separate subsystem but a property that cuts across all of
them.

**Implementation form.** Plain C, with the *codegen shape* (which
comparison instruction, which register width, whether a widening `cwd`
appears) used as the evidence for the intended C type — i.e. these are
facts about what the original programmer wrote, recovered by reading the
compiler's own translation rules backward.

**Proposed portable abstraction.** None needed as an API — this is a
constraint on the *reimplementation*, not a subsystem to wrap. The
concrete rule for the port: use `uint16_t`/`int16_t` (not `int`/`unsigned`,
which are 32-bit on virtually every modern target) everywhere the
historical type was Turbo C's 16-bit `int`/`unsigned`, and preserve
truncating casts exactly where the original source shows one (e.g.
`(int16_t)(o2 - o1)` computed from `long`/`int32_t` values, matching
RESOURCE.C's documented truncation) rather than "helpfully" widening the
computation.

**SDL3 relationship.** Not applicable.

**Risks.** This is the single easiest correctness bug class in a naive
"port the C to 64-bit" pass: every `int`/`unsigned` in `src/*.C` is 16 bits
on the original target and often 32 bits (or actually a different type
entirely, e.g. Rust's `i32`) on a modern one. Any arithmetic whose result
was truncated back into an `int` on Turbo C (wraparound-dependent counters,
the `resource_load_record` length subtraction) will silently produce
different results if ported to a wider type without an explicit truncation.
`timer_ticks` itself (`unsigned long`, 32-bit even under Turbo C) also
wraps roughly every 2^32/236.7 Hz ≈ 6.3 days of continuous uptime — the
`timer_deadline_*` comparisons (`timer_ticks < gc0d0`) are unsigned and
therefore already wraparound-correct for a single wrap, and should stay
that way in the port.

---

## 9. setjmp/longjmp and other runtime-specific control

**Current source.** `game_abort_jmpbuf` (DS:8BFE, `char[]` — Turbo C's
`jmp_buf` is an opaque byte array, not a struct the port can assume a
layout for) is armed once in `src/GAME.C`'s `game_run()` (F_49F0):
`d = setjmp(game_abort_jmpbuf); s = -1; ui_overlay_reset();
sound_request_count_clear(); puzzle_free_resources(); if (d==3) return; ...`
— the `setjmp` return value `d` selects which of several game-loop
re-entry points to resume at (0/1 = fresh game paths, 2 = resume from an
in-progress slot, 3 = abort back out of `game_run` entirely). Three
`longjmp(game_abort_jmpbuf, N)` sites unwind back to that point:
`src/BOARD.C:845` (`longjmp(...,2)`), `src/LEVEL.C:259`
(`dialog_run(&g1670); longjmp(game_abort_jmpbuf,1)`), `src/PLAYERSL.C:49/61/78`
(`longjmp(...,2)`, `longjmp(...,1)`, `longjmp(...,3)`). This is the game's
*save-slot/level-abort* control-flow mechanism, not a crash handler.

Separately, `src/CRITERR.C` installs a **DOS critical-error handler**
(`dos_critical_error_install()`, F_625D: `harderr(dos_critical_error_handler)`
— Turbo C's `harderr`/`hardresume`/`hardretn` library wrappers around
`int 24h`) whose handler `dos_critical_error_handler(int a,int b)` (F_622C)
sets `gb3e=1` (the "disk error happened" flag `resource_file_open`'s retry
loop checks), stashes the failing drive, and either `hardretn(3)` (DOS
"fail the call") or `hardresume(2)` (DOS "retry") depending on `gb40`
(whether we're inside a save-slot write, which must not silently fail).
This is called once from `boot_init_seed_rand` (GAME.C) alongside
`timer_irq_install()`.

`src/TIMER.C`/`KEYBOARD.C`/`KEYIRQ.C`'s `interrupt`-qualified functions
(§3, §4) are the third "runtime-specific control" surface: Turbo C's
`interrupt` keyword changes the function's prologue/epilogue (`pushf`
implicitly, `iret` instead of `ret`, all registers saved) and is only
meaningful as a raw IRQ vector target.

**Public API.** `game_abort_jmpbuf` + the four `setjmp`/`longjmp` call
sites listed above (only BOARD.C, GAME.C, LEVEL.C, PLAYERSL.C touch it —
this is a narrow, well-scoped mechanism). `dos_critical_error_install()`
(called once, from GAME.C).

**Implementation form.** Plain C (`setjmp`/`longjmp` and `harderr`/
`hardresume`/`hardretn` are all CC.LIB calls, no inline asm in CRITERR.C or
the setjmp/longjmp call sites themselves). The `interrupt` keyword is a
Turbo C language extension, not a library call.

**Proposed portable abstraction.** Replace `game_abort_jmpbuf`'s
`setjmp`/`longjmp` pair with ordinary structured control flow (a
`GameRunResult` enum returned up through `game_run`'s internal loop, or a
single early-return state machine) — a portable C target has no reason to
keep `setjmp`/`longjmp` for what is, on inspection, a 4-way "which
re-entry point" selector confined to 4 files; C++ or Rust equivalents would
use exceptions/`Result` respectively, but even in C this reads as ordinary
loop/`return` restructuring once ported. The DOS critical-error handler has
no portable target at all (§6): on a modern OS, a failed file read is
already reported through the return value of the read call, so
`dos_critical_error_handler`'s job (set an error flag, choose retry-vs-fail)
collapses into ordinary I/O error handling in the port's `resource_io`
layer. `interrupt`-qualified functions disappear entirely once §3/§4 are
reimplemented as SDL3 event/timer callbacks — there is no vector to
install them on.

**SDL3 relationship.** Not applicable to `setjmp`/`longjmp` (pure C control
flow). Not applicable to the critical-error handler (no host analogue
needed — see above). Not applicable to `interrupt` (disappears with §3/§4).

**Risks.** Low for `setjmp`/`longjmp` (scope is small and the 4 call sites
above are exhaustive per this inventory's greps). Medium for the critical-
error handler: `gb40` (whether we're mid-write) changing the DOS response
from "retry" to "fail" is a real behavioral distinction (a resource *read*
failure retries silently up to disk-reset, a save-slot *write* failure
fails the call outright) that the port's I/O error handling should
preserve as an explicit read-vs-write policy, not drop as "just DOS
plumbing."

---

## 10. Historical memory allocation

**Current source.** `farmalloc()`/`farcoreleft()` calls (CC.LIB, far heap)
appear in `src/BOARD.C`, `HINTDLG.C`, `INTRO.C`, `LEVEL.C`, `PLAYERSL.C`,
`PLRLDPUB.C`, `PUZZLE.C`, `RESOURCE.C`, `STARTUP.C`, `VIDEO.C`. Two
distinct uses:
1. **Startup memory gate**: `src/STARTUP.C`'s `video_mode_select()` (F_520A)
   calls `farcoreleft()` once and rejects modes that need more far heap
   than is available (`if ((n=farcoreleft()) < 0x3ada0L) { dos_write_handle2(s8a8); return 0; }`,
   then per-mode thresholds `0x57720L`/`0x44620L` demote `display_mode` 5→1
   or 1/3/4→2 when memory is short) — this is a *feature-detection* gate
   that changes which display mode the game selects, not merely an
   allocation-failure path.
2. **Paragraph-rounded region carving**: `src/VIDEO.C`'s
   `video_alloc_framebuffer()` (`g40ca = farmalloc((long)w*488+16); s =
   FP_SEG(g40ca)+1; g40ca = MK_FP(s,0);` — round the allocation up to the
   next paragraph boundary, discard the sub-paragraph remainder) and
   `src/PLAYERSL.C`'s three-region carve (§7) are both "one big farmalloc,
   then paragraph-align and sub-divide by pointer arithmetic," not three/four
   independent allocations.

`src/RESOURCE.C`'s `resource_load_record_alloc()` (F_684A) is the ordinary
allocate-or-die path: `*pp = farmalloc((long)n); if (*pp==0) { game_shutdown();
video_set_text_mode(); dos_write_handle2(gb42); exit(a); }`.

**Public API.** `farmalloc`/`farcoreleft` themselves are CC.LIB, not
game-owned; the game-owned entry points are `video_mode_select()` (gates
mode choice on `farcoreleft()`), `video_alloc_framebuffer()`,
`resource_load_record_alloc()`, and PLAYERSL.C's three-region carve
(unnamed in this excerpt — the allocating function around VIDEO.C:259-262).

**Implementation form.** Plain C throughout (no inline asm at any
`farmalloc`/`farcoreleft` call site found).

**Proposed portable abstraction.** Drop the far-heap/paragraph-rounding
mechanics entirely (§7) and replace with ordinary `malloc`/arena
allocation; keep the *policy* pieces as explicit, testable functions: a
`platform_available_memory()`-style capability check only if the port
intends to keep the automatic-mode-downgrade behavior (§ "Startup memory
gate") as a deliberate compatibility feature — otherwise this logic can be
retired outright, since a modern host has no comparable 640KB-conventional-
memory ceiling and the mode selection can instead be driven by explicit
user/config choice. If kept, it must reproduce the *specific* thresholds
(0x3ada0, 0x57720, 0x44620 bytes) and the per-mode demotion table exactly,
since these are original game behavior, not incidental to memory
management.

**SDL3 relationship.** Not applicable — general-purpose allocation, no
platform video/audio/input surface involved.

**Risks.** If the "startup memory gate" behavior is kept for parity testing
against the historical build (e.g. to reproduce which mode a given
`AEPROG.EXE`-equivalent run selects under constrained memory), the port
needs a way to *simulate* a specific `farcoreleft()` value, since the host
will never actually be memory-constrained the way DOS conventional memory
was — this makes it a test-harness concern rather than a runtime one.

---

## 11. Resource / archive access

**Current source.** `resource_load_record(unsigned p)` (src/RESOURCE.C,
F_656C) is the sole record-read entry point (§6); `p`'s top 4 bits select a
directory index `d = p>>12` (`resource_file_open(d)` opens
`ga22[d]`/`ga22[d]+case-fold`, a 16-byte name row — the AE000/AE001
archive files are the two entries this directory ultimately resolves to,
per `tools/pack_archives.py`'s `NAMES = ('AE000','AE001')` and
`tools/reconstruct_archives.py`), the low 12 bits are a record number
looked up in a per-directory long-offset table (`ga52[]`) to find the
record's byte range, which is then `read()` into `ui_gfx_blob` and
post-processed by a **header flag byte** (`fl = ui_gfx_blob[1]`) that
selects among four decoders:
```
(fl & 2) && (fl & 1): lz_decompress() then rle_packbits_decode()
fl & 2 only:          lz_decompress() then memmove()
fl & 1 only:          rle_packbits_decode() then memmove()
```
then, if `display_mode != 5`, a *second* dispatch on `gc0cb =
ui_gfx_blob[0]` selects a sprite decoder: `0x47` →
`sprite_decode_4bpp_mode13h`/`sprite_decode_4bpp_planar` (mode-dependent),
`0` → `sprite_sheet_decode_sequential`, `1` → `sprite_sheet_decode_indexed`.
All four low-level decoders (`lz_decompress`, `rle_packbits_decode`,
`sprite_decode_4bpp_mode13h`, `sprite_decode_4bpp_planar`) live in
`asm/DECODE.ASM` (module `M_6D86_6F4B`, 573 bytes — PackBits expansion,
an LZ-style decompressor with an inline CS-addressed bit-reader state
block, and the two 4bpp sprite unpackers), classified
`PROBABLE_ORIGINAL_ASM` HIGH confidence in asm-provenance.md.
`sprite_sheet_decode_sequential`/`sprite_sheet_decode_indexed` (RESOURCE.C,
plain C: walk a sequential or indexed sub-record list and re-dispatch per
sub-entry, same 0x47 test) are the "resource contains N sub-sprites" layer
on top of the single-record decode.

The **runtime-replacement mechanism** (§1): `AE000_002`/`AE000_003` are not
ordinary game-asset records — for `display_mode` 5/2 they are a
*replacement copy of the EGA/VGA runtime block itself*, loaded and installed
at CS:039C before `video_alloc_framebuffer` runs
(docs/current/runtime-boundary-recovery.md, `tools/report_runtime_platform.py`:
`{'value':2,...,'driver':'AE000_003',...}`, `{'value':5,...,'driver':'AE000_002',...}`).

**Public API.** `resource_load_record(p)`, `resource_load_record_alloc(a,pp)`,
`resource_load_record_into(a,q)` (§6's callers list — 16 TUs). Internally:
`sprite_sheet_decode_sequential(char *p,unsigned n)`,
`sprite_sheet_decode_indexed(char *p)` (RESOURCE.C); `lz_decompress()`,
`rle_packbits_decode()`, `sprite_decode_4bpp_mode13h()`,
`sprite_decode_4bpp_planar()` (asm/DECODE.ASM, extern'd into RESOURCE.C).

**Implementation form.** RESOURCE.C's dispatch logic: plain C. DECODE.ASM:
pure hand-written TASM.

**Proposed portable abstraction.** A `resource_archive` module that: (1)
owns the AE000/AE001 file format directly (directory table, per-directory
offset table, per-record header byte) as an explicit documented format
rather than "whatever `ga22`/`ga52`/`resource_load_record` currently
compute" — this is exactly the kind of thing `docs/current/` and
`tools/resource_formats.py`/`tools/reconstruct_archives.py` already
document/exercise and the port should treat as its format spec; (2) a
`resource_decode(fmt_flags, data) -> bytes` matching the two-bit
compression dispatch above, reimplementing `lz_decompress`/
`rle_packbits_decode` as portable C (they have no game-visible timing
dependency — pure data transforms); (3) a `sprite_decode` layer for the
0x47/0/1 header-byte dispatch and the two 4bpp unpackers, parameterized on
`display_mode` exactly as RESOURCE.C's `gc0cb` switch is (mode13h vs
planar) so the two pixel formats stay explicit rather than merged. The
runtime-replacement mechanism (AE000_002/003 as alternate runtime blocks)
is a *separate*, harder problem: it means "the video backend for two of
five display modes" is itself game data, not fixed code — the port should
either decode and permanently fold those two replacement runtimes into the
portable video backend (§1) as just two more supported modes, or
explicitly scope them out if those display modes are not required for the
initial port target.

**SDL3 relationship.** Not applicable — archive/resource decoding is
game-owned data-format logic with no platform-library equivalent; SDL3
only enters once decoded pixel data is blitted to a texture (§1) or decoded
audio is queued (§5).

**Risks.** None involving exact timing (this is pure data decode); the
practical risk is scope — the runtime-replacement mechanism means "finish
reverse-engineering the EGA/VGA video backend" is not complete until
AE000_002 and AE000_003's replacement runtime code is also understood,
per docs/current/runtime-boundary-recovery.md's own "Deliberately
unresolved" section (the built-in dispatch table's zero/out-of-range mode
slots are explicitly *not* evidence of a bug, because real execution for
modes 2/5 never reaches them).

---

## (a) Dependency table: which game units call which boundary

| Boundary | Called from (src/*.C, non-exhaustive but grep-verified) |
|---|---|
| Video/EGA/VGA primitives (§1, VIDEO.H) | 29 of 60 C units call at least one `gfx_*` primitive, incl. BOARDDRW.C, BOARD.C, DIALOG.C, HUD.C, HITTEST.C, INTRO.C, LEVEL.C, MENULIST.C, MRKCELL.C, PUZZLE.C, ROUNDEND.C, SCORE.C, SCOREPNL.C, SLOTMENU.C, TXTWRAP.C, VIDEO.C |
| Palette / mode select (§1-2) | STARTUP.C (mode select), VIDEO.C (palette + framebuffer), GAME.C (`boot_init_seed_rand` sequencing) |
| Keyboard IRQ + BIOS input (§3) | DIALOG.C, GAME.C, HELPMENU.C, INTRO.C, KEYBOARD.C, KEYIRQ.C, LEVEL.C, MENULOOP.C, OPTIONS.C, PLAYERSL.C, PROMPTS.C, PUZZLE.C, SLOTMENU.C |
| Timer IRQ + timing (§4) | ANIMSTEP.C, BOARD.C, DIALOG.C, GAME.C, INTRO.C, LEVEL.C, PUZZLE.C, ROUNDEND.C, SLOTMENU.C, SNDREQ.C, TIMER.C |
| Sound / OPL (§5) | 18 units touch `sound_enabled`/`music_enabled`/`snd_*`/`sound_start`/`sound_stop`, incl. GAME.C, INTRO.C, LEVEL.C, OPTIONS.C, OPLINIT.C, OPLREG.C, OPLVOICE.C, PLAYERSL.C, PLRLDPUB.C, PUZZLE.C, RESCACHE.C, ROUNDEND.C, SLOTMENU.C, SNDFXTGL.C, SNDREQ.C, TIMER.C |
| DOS filesystem / resource load (§6, §11) | ANIMFRAM.C, BOARD.C, GAME.C, HELPMENU.C, HINTDLG.C, HUD.C, INTRO.C, LEVEL.C, MENURES.C, PLAYERSL.C, PLRLDPUB.C, PUZZLE.C, RESCACHE.C, RESOURCE.C, ROUNDEND.C, SLOTS.C |
| Far-pointer/segment (§7) | Concentrated in VIDEO.C, PLAYERSL.C (explicit `MK_FP`/`FP_SEG`); implicit ES:SI/ES:DI ABI pervasive in asm/SOUND.ASM, asm/SPRITES.ASM, asm/SPRDRAW.ASM, asm/ICONANIM.ASM, asm/DRAWQ.ASM, asm/DRAWQBUF.ASM |
| Historical memory allocation (§10) | BOARD.C, HINTDLG.C, INTRO.C, LEVEL.C, PLAYERSL.C, PLRLDPUB.C, PUZZLE.C, RESOURCE.C, STARTUP.C, VIDEO.C |
| setjmp/longjmp / critical error (§9) | BOARD.C, GAME.C, LEVEL.C, PLAYERSL.C (jmp_buf); CRITERR.C + GAME.C (critical-error install) |

---

## (b) Ordered suggestion for the port

Abstract in this order, so the historical tree keeps being the oracle at
every step (each layer's portable reimplementation can be diffed against
the historical build's *observable output*, not its internals, once the
layer below it is in place):

1. **16-bit integer discipline first** (§8) — not a runtime layer but a
   coding rule that must be settled (fixed-width types, explicit truncating
   casts) before any of the arithmetic-heavy modules below are ported,
   since getting this wrong silently corrupts everything downstream.
2. **Resource/archive decode** (§11) — pure data transforms with no timing
   dependency and no other boundary depends on it being "live" (it can be
   unit-tested by decoding real AE000/AE001 records and diffing against
   `resource_load_record`'s output byte-for-byte). Establishes the asset
   pipeline the rest of the port consumes.
3. **Video primitives against a software framebuffer** (§1-2) — implement
   `gfx_*` against a plain linear buffer first (no SDL3 yet beyond
   presenting a texture), verified by rendering known frames and diffing
   pixels against the historical build under an emulator/DOSBox capture.
   This unblocks visual verification of every later layer.
4. **Timer service + fixed-step loop** (§4) — the 236.7 Hz tick and the
   deadline/wait API, driven by SDL3 timing but exposing the historical
   tick-counting semantics; needed before sound (which is tick-driven) and
   before any animation pacing can be verified.
5. **Keyboard input** (§3) — lowest-risk boundary (SDL3 replaces it almost
   entirely); do this once the timer service exists so hotkey/menu input
   can be tested against real frame pacing.
6. **Sound driver state machine** (§5) — highest-complexity boundary
   (LOW-confidence fields still exist per sound-state.md); implement the
   game-owned voice/command-stream logic against a stub backend first
   (e.g. logging register writes), verify the register-write *sequence*
   matches the historical driver bit-for-bit on known inputs, and only then
   attach a real OPL2 emulator + SDL3 audio device.
7. **setjmp/longjmp control-flow restructuring + memory-allocation policy
   retirement** (§9-10) — lowest risk, can be done any time after the
   layers above compile, since neither has an external I/O surface of its
   own.
8. **Runtime-replacement modes (AE000_002/003) last** — only after the
   above is solid, since this requires first fully reverse-engineering a
   second EGA/VGA implementation hidden inside game data
   (docs/current/runtime-boundary-recovery.md's own "Deliberately
   unresolved" section), and is not required for the primary display modes
   (1, 3, 4) to work correctly.

---

## (c) Behaviors that must be verified against the historical build

1. **Tick rate and chaining.** IRQ0 reprogrammed to PIT divisor **0x13B1**
   (5041 decimal, ≈236.7 Hz); every **13th** tick chains to the original
   BIOS INT 8 vector (`timer_tick_phase >= 13` in `timer_irq_handler`,
   src/TIMER.C) so BIOS-side 18.2 Hz consumers stay correct
   (236.7/13 ≈ 18.2 Hz). `timer_irq_restore` resets channel 0 to the
   default divisor (0, i.e. 65536) but does not reprogram channel 2.
2. **Sound tick cadence.** `sound_tick_entry()` runs once per *fast* tick
   (236.7 Hz), gated by `!sound_request_count && (sound_enabled ||
   music_enabled)` — not once per BIOS tick and not once per frame.
3. **Keyboard scancode edge flags.** `down = scan < 0x80`; break code sets
   `key_up_released = 1` only on the specific keys the switch in
   `keyboard_irq_handler` names (0x47/0x49/0x48/0x4B/0x4D with their
   `g856`-gated aliases 0x58/0x29/0x2B/0x4E/0x4A); E0/E1 prefix bytes are
   acknowledged and swallowed without being treated as scancodes
   themselves.
4. **PIT/speaker register sequencing.** Channel 0 control byte 0x36 then
   LSB/MSB to port 0x40; channel 2 control byte 0xB6 to port 0x43 (mode
   only, no divisor written at install time); sound driver later writes
   divisor LSB/MSB to port 0x42 only when `snd_backend_mode==0`; PC-speaker
   gate bits via port 0x61 (`or al,3`/`and al,0fch` for enable/disable in
   mode 0; `or al,60h` for the Tandy-mode gate).
5. **OPL bus protocol.** Register-select write to `opl_port`, 6x settling
   read, data write to `opl_port+1`, then a 35-iteration settling read
   loop (`_opl_register_write`, asm/SOUND.ASM) — verify against whatever
   OPL2/OPL3 target the port uses (real chip vs emulator) that the
   effective register-write ordering and any required settling delay is
   preserved.
6. **Startup memory-gate thresholds.** `farcoreleft()` cutoffs 0x3ada0
   (hard fail), 0x57720 (demote mode 5→1, or allow 4→5), 0x44620 (demote to
   mode 2) in `video_mode_select` (src/STARTUP.C) — if the automatic
   mode-downgrade behavior is kept in the port, these exact thresholds and
   the per-mode demotion table must be reproduced.
7. **`resource_load_record` length truncation.** `s = (int)o2 - (int)o1`
   is a 16-bit truncating subtraction of two `long` offsets — must not be
   "fixed" to 32-bit arithmetic in the port; any historical record whose
   true length exceeds 16 bits relies on this exact truncation.
8. **`game_abort_jmpbuf` re-entry codes.** The four `longjmp` call sites use
   codes 1/2/3 with distinct meanings selected by `game_run`'s `if
   (d==3) return; if (d<2) {...}` branch (src/GAME.C) — the portable
   control-flow replacement must preserve which of BOARD.C/LEVEL.C/
   PLAYERSL.C's three abort codes routes to which of `game_run`'s
   resume/exit paths.
9. **DOS critical-error retry-vs-fail policy.** `gb40` (mid-write flag)
   selects `hardretn(3)` (fail) during a save-slot write vs.
   `hardresume(2)` (retry) otherwise (src/CRITERR.C) — the port's I/O
   error handling should preserve this read-vs-write asymmetry even though
   the DOS-specific mechanism itself is retired.
10. **Sprite/resource header-byte dispatch.** The two-bit compression flag
    (`fl & 2`, `fl & 1`) and the header type byte (`gc0cb`: 0x47/0/1) in
    `resource_load_record` (src/RESOURCE.C) — every AE000/AE001 record's
    decode path must be re-derived exactly from these two bytes, not
    inferred from context, since the same record data can route through
    up to two decompression stages before the sprite-specific unpacker.
