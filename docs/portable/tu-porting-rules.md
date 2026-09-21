# Porting a historical C translation unit to `portable/game/`

These rules turn `src/NAME.C` (Turbo C 2.0, compact model) into
`portable/game/name.c` (C17).  They are mechanical for PURE units
(`docs/portable/tu-inventory.md`); HW/MIXED units additionally replace their
hardware fragments with calls into the portable services.  When a rule does
not cover something, stop and report it — do not invent semantics.

## 1. File shape

- One portable file per historical file, lowercase name (`GAME.C` →
  `game.c`).  Keep the `/* ---- F_xxxx (original code at 0x....) ---- */`
  section markers and the first descriptive paragraph of each member
  comment; drop disassembly listings and byte-level codegen evidence (they
  stay in the historical tree).
- The first non-comment line is `#include "game.h"`.  Delete every local
  `extern` object/function declaration and every local `#include` of a
  historical header (`DIALOG.H`, `C470.H`, ...): `game.h` provides the
  generated state (`game_state.h`, `game_data.h`), the ported structs
  (`game_structs.h`), every function prototype (`game_funcs.h`) and the
  service headers (`gfx.h`, `resource.h`, `timer.h`, `input.h`, `sound.h`,
  `cclib.h`).
- Static (file-local) data keeps its initializer in the ported file, with
  the same name and dos_* types (these are the `compiled-data` components
  the generator leaves to ported C).

## 2. Declarations and types

- K&R definitions become prototypes.  Historical `int` → `dos_int`,
  `unsigned` → `dos_uint`, `char` → `dos_char`, `unsigned char` →
  `dos_uchar`, `long` → `dos_long`, `unsigned long` → `dos_ulong`.
  K&R parameters declared `char` were promoted to `int` on the stack; write
  them as `dos_int` and note it when the body relies on the char range.
- Drop `register`, `near`, `far`, `interrupt`, `huge`.  `T far *` → `T *`
  with the element mapping above (`char far *` → `dos_char *`; keep
  `const` out unless the historical code never writes through it and the
  callee signature in `game_funcs.h` says so).
- `void far *` / bare `char far * far *` out-params → `void *` / `dos_char **`.
- Function pointers `int (**far callbacks)()` → the typedef in
  `game_structs.h`.
- Struct tags come from `game_structs.h` only; never re-declare a struct.

## 3. Arithmetic

- Keep every expression as written.  Apply the required spellings from
  `docs/portable/int-semantics-inventory.md` at the sites it lists (they
  are commented with the historical file/function).  Anything on that
  document's "review" list keeps its natural C form; differential tests
  decide later.
- Implicit truncation on store is preserved automatically by the dos_*
  object widths; do not add casts elsewhere.

## 4. CC.LIB and DOS calls

- `movmem`, `setmem`, `memcpy`, `memmove`, `strlen`, `strcpy`, `strcat`,
  `strcmp`, `itoa`, `ultoa`, `ltoa`, `sprintf`, `rand`, `srand`, `atoi`:
  use the portable `cclib.h` (Turbo C semantics: `rand()` is the Turbo C
  LCG, `itoa`/`ultoa` produce the same digits/radix behaviour, `movmem`
  handles overlap).  Standard functions with identical semantics map to
  `<string.h>`/`<stdlib.h>` directly.
- `farmalloc(n)` → `malloc(n)`; `farfree` → `free`; `farcoreleft()` →
  retire (see §6).
- `open/read/write/close/lseek` on the resource files are already inside
  `portable/resource`.  Save-slot file I/O → `dosio.h` (`dosio_open_read`,
  `dosio_read`, `dosio_write`, `dosio_seek`, `dosio_close`) with explicit
  error returns; `disk_reset_retry`, `harderr`, `hardresume`, `hardretn`,
  `dos_critical_error_*` → delete the call and route the read-vs-write
  policy through `dosio.h` (write failures fail, read failures report).
- `dos_write_handle2(text)` → `fputs(text, stderr)` semantics (historical
  length was strlen-1: it dropped the trailing character; reproduce that).
- `exit(n)` stays `exit(n)`.
- `biostime(0,0)` (only in `srand(biostime(...))`) → `dosio_bios_ticks()`.

## 5. Hardware fragments

| historical                                   | portable                                    |
|----------------------------------------------|---------------------------------------------|
| `video_load_palette`, `gfx_*`, `rect_border_draw`, `gfx_color_select`, `color_*` | already in `gfx.h`; delete VIDEO.C's copies |
| `video_alloc_framebuffer`, `ui_gfx_alloc`, `runtime_base`, `blitter_patch_variant` | `gfx_framebuffer_init()`, `resource_staging_init()`; see §6 |
| `video_set_text_mode`, `bios_equipment_probe`, `video_adapter_detect`, `sound_backend_probe`, `cmdline_parse_args`, `video_mode_select` | replaced by `portable/game/startup.c` (supervisor-written) |
| `timer_irq_install/restore/handler`, `timer_*` helpers | `timer.h` (already ported)             |
| `keyboard_irq_*`, `keyboard_*`               | `input.h` (already ported)                  |
| `sprite_sheet_select`, `text_line_width` (FONT.C asm) | C reimplementation of the documented walk (see FONT.C comments) |
| `opl_register_write`, `opl_detect`, port I/O | `sound.h` (`opl_write(reg,val)`, `opl_detect()` returns 1) |
| `sound_*`, `voice_*` ASM publics             | `sound.h` (portable driver, Wave 4; stubs until then) |
| `setjmp/longjmp(game_abort_jmpbuf, n)`       | keep, using `jmp_buf game_abort_jmpbuf` from `game_state.h` (Phase 11 restructures it later) |
| `__sti__()`, `enable()`, `asm sti`           | delete                                      |

## 6. Memory / startup policy

- Paragraph arithmetic (`MK_FP`, `FP_SEG`) never survives; the three
  staging pointers come from `resource_staging_init()`.
- The `farcoreleft()` mode-downgrade in `video_mode_select` is retired: the
  port selects `display_mode = 4` explicitly in `startup.c`.

## 7. Verification for each ported unit

- Compiles warning-free with `/W4` under the umbrella header.
- Every function in the historical file is present with the same name
  (the prototype list in `game_funcs.h` is the checklist).
- Any semantic replacement (§4/§5) is marked with a `/* PORT: ... */`
  comment citing the rule.
