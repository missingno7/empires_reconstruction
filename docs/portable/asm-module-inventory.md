# Hand-written ASM module inventory (Wave 2 spec index)

Scope: the 9 production hand-written assembler modules in `asm/*.ASM`
**other than** `RUNTIME_BLOCK.ASM` and `DECODE.ASM` (those two are
out-of-scope per the task; `RUNTIME_BLOCK.ASM` is covered by
`docs/current/portability-boundaries.md` §1, `DECODE.ASM` by §11).
Classifications and byte sizes below are taken from
`docs/current/asm-provenance.md` (all 9 are `PROBABLE_ORIGINAL_ASM`,
production, confidence HIGH except ICONANIM/DRAWQ = MEDIUM). This is the
specification index the Wave 2 ASM-to-C translation agents work from: for
each module, its public routines and their calling convention, every
DGROUP address it touches, its C callers, and the data structures its own
comments describe.

Two calling conventions appear across these modules:
- **Stack args, compact-model C convention** (caller pushes right-to-left,
  callee near, no register-ABI beyond that) — used by every routine a
  `src/*.C` file calls directly.
- **Register ABI** — used for ASM-internal helpers never called from C.
  The dominant shape is **SI = a table index already scaled to the
  table's stride** (voice index ×2 in `asm/SOUND.ASM`'s 4-word tables) or
  **ES:DI / DS:SI = a far pointer into a record table**, matching
  `docs/current/portability-boundaries.md` §7's description of the
  ES:SI/ES:DI convention.

---

## 1. `asm/RECTQ.ASM` — `F_1ECD` (74 bytes, HIGH)

### Public routine

| Routine | Original addr | Args | Returns |
|---|---|---|---|
| `_rect_queue_flush` | F_1ECD | none (reads module-level DGROUP state) | none |

**Convention:** near proc, no stack args at all — every operand is a
DGROUP global. Internally hand-written (not a Turbo C unit): DS is saved/
reloaded via `lds`, and AX/BX/CX/DI are four simultaneously live scratch
values across the drain loop with a `loop` instruction (the file's own
header comment, "runK idiom (a)").

### DGROUP addresses read/written

| Addr / name | Access | Role |
|---|---|---|
| `_ui_gfx_blob` (DS:C5CA, far) | R (`lds si,...`) | Read cursor — reused from the resource-decode staging blob (`src/RESOURCE.C`'s `ui_gfx_blob`); this queue's read pointer piggybacks on that same far-pointer cell. |
| `_rect_queue_write_ptr` (far) | R (`les cx,...`), W (reset to the read pointer at drain end) | Write cursor for the rect-invalidation queue. |
| `_g40c6` (word) | W (segment half of the write pointer, stored after drain) | Segment half companion to `_rect_queue_write_ptr`. |

### Data structure

4-byte queue record (per the file's header comment): word1 low byte =
`x/2`, high byte = `y`; word2 low byte = `w/2`, high byte = `h`. Each
record is popped and handed to `gfx_box(x,y,w,h)`.

### C callers

None — `_rect_queue_flush` is never called from any `src/*.C` file (grep
confirmed). It is presumably reached only from other ASM (or was reachable
from a since-removed/inlined call site); Wave 2 should re-confirm this via
a full-binary call-site search before assuming it is dead code, since the
header comment ties it to `F_233E`'s cast animation (`d()` in that
function), which is itself ASM/inline-asm territory this inventory did not
trace further.

---

## 2. `asm/BOARDCOL.ASM` — `F_1F91` (126 bytes, HIGH)

### Public routine

| Routine | Original addr | Args (stack, compact model) | Returns |
|---|---|---|---|
| `_board_collision_span_or` | F_1F91 | `x` [bp+4], `y` [bp+6], `h` [bp+8] (a row count) | AX = accumulated OR of the collision bytes |

**Convention:** near proc, standard stack args, C-callable. Explicitly
**not** a Turbo C unit per its own header: DS is saved/reloaded via `lds`,
AX/BX/CX/DI are four simultaneously live scratch values across the x/y
clamps, and CX is a `loop` counter (same "runK idiom (a)" as RECTQ.ASM).

### DGROUP addresses read/written

| Addr / name | Access | Role |
|---|---|---|
| `_board_records` (far, extern dword) | R (`lds si,...`, then indexed) | The far collision/attribute map this routine spans. |

### Data structure

A row-major byte map addressed as `si + row*0x26` (38 decimal) after `x`/`y`
are clamped to the play window (x: 8..0x137, y: 0x10..0x9F), scaled `/8`,
and the row span clamped to a max of 0x13 (19) rows; the routine ORs
together every byte the clipped x/y/h span covers and returns the
accumulated result. This is the same `board_records` far block
`asm/SPRITES.ASM`'s `_sprite_table_wipe_active`/`_board_actors_draw`
index by a *different* (20h-byte-record) scheme — `board_records` itself
is the collision/attribute grid, addressed here directly by row.

### C callers

`src/GAME.C`, `src/LEVEL.C` (grep-confirmed).

---

## 3. `asm/SPRITES.ASM` — `M_4AA8_4EEB` (1211 bytes, HIGH)

Four routines assembled as one word-aligned module (per
`docs/current/tu-structure.md` fact 5 — the pad before `F_4AA8` marks a
genuine module boundary): `F_4AA8`, `F_4B0C`, `F_4E9F`, `F_4EEB`.

### Public routines

| Routine | Original addr | Args | Returns / clobbers |
|---|---|---|---|
| `_play_window_wipe_clipped` | F_4AA8 | `x1` [bp+4], `y1` [bp+6], `w` [bp+8], `h` [bp+0Ah] (stack args) | none. Clips to the play window (8..0x137, 0x10..0x9F) then calls `gfx_wipe_rect` with `y+0xB8`. |
| `_sprite_script_frame_driver` | F_4B0C | none (no stack args) | none; clobbers ax/bx/cx/dx/si/di (all pushed/popped). Called once per interpreter tick. |
| `_sprite_table_wipe_active` | F_4E9F | none | none; clobbers ax/bx/cx (si/di preserved). |
| `_board_actors_draw` | F_4EEB | `y0` [bp+4] (one stack arg) | none; clobbers ax/bx/cx/dx (si/di/bp preserved). |

**Convention:** all four are near procs with the compact-model C stack
convention where they take arguments at all (`_play_window_wipe_clipped`,
`_board_actors_draw`); `_sprite_script_frame_driver` and
`_sprite_table_wipe_active` take no arguments and drive themselves purely
off DGROUP state + fixed table addresses. `_play_window_wipe_clipped` is
itself called from `_sprite_table_wipe_active` (ASM-to-ASM, not from C) —
its header comment notes it uses exactly the four registers (AX/BX/CX/DX)
Turbo C's convention lets a callee clobber, unlike a genuine Turbo C unit
which would keep a live variable only in SI/DI and save them.

### DGROUP addresses read/written

| Addr / name | Access | Role |
|---|---|---|
| `_actor_record_table` (near array, byte) | R | 0x20-byte-stride record table, count in byte 0; `_sprite_table_wipe_active`/`_board_actors_draw` both walk it. |
| `_board_record_index` (word) | R | Current board id; records whose `+1` word doesn't match are skipped. |
| `_tile_height_table`, `_tile_width_table` (byte arrays) | R | Indexed by record byte `+6`; feed the wipe-clip call's w/h. |
| `_resource_ptr_table` (far pointer array, dword) | R | Indexed by record byte `+6 * 4`; supplies the far sprite bitmap pointer `_board_actors_draw` blits. |
| `0B3AEh` / `0B3AFh` (raw displacement, actor/script table) | R/W | `_sprite_script_frame_driver`'s own 0x20-byte-stride bytecode-program record table (count byte, then records) — **not** the same table as `_actor_record_table` above (different stride/shape; this one carries a resumable bytecode program per record at `+0Dh`). No C-facing name found in this scan; likely `include/*.H` or `recipes/data` has one — flagged review for Wave 2. |
| `0BFBAh` (word, "current board id") | R | Read as `ds:[0bfbah]` inside the interpreter; almost certainly the same value `_board_record_index` names elsewhere (both gate "belongs to the current board") but this scan did not find a shared extern tying the two spellings together — **review**. |
| `74Ah` (code address, jump table) | — | The bytecode dispatcher's opcode-handler jump table (`jmp [bx]` with `bx = opcode*2 + 74Ah`); a code address, not DGROUP data. |

### Data structures

- **`_actor_record_table`** (0x20-byte records, count-prefixed): byte 0
  unused/flags, word `+1` = board id, word `+2`/`+4` = x/y, byte `+6` =
  sprite/tile index (into `_tile_height_table`/`_tile_width_table`/
  `_resource_ptr_table`), byte `+7` = a secondary index feeding the far
  pointer lookup's low byte, byte `+8` = "skip this record" flag, byte
  `+0x1A` = optional extra vline-strip length.
- **`0B3AEh` bytecode-program table** (0x20-byte records, count-prefixed):
  byte 0 = "already rendered" flag, byte `+1` = board id, byte `+0Ah` =
  countdown timer, word `+0Dh` = saved bytecode program offset (resumed
  through the `74Ah` jump table). The interpreter's opcode set (per the
  file's own F_4B0C header comment): relative/absolute jumps, delay
  countdowns, calls into `F_CAF1` (= `_stream_control_block_arm`,
  `asm/SOUND.ASM` — an actor/sprite opcode can arm a sound stream directly)
  and into `F_2A2D`/`F_338A`/`F_36F0` (board unit-script functions, per the
  `_board_run_unit_script`/`_board_advance_unit_moves` externs), flag set/
  test, movement-limit and velocity setup, clamped and absolute position
  moves (with optional redraw), enable/disable, bit-test-and-skip, and a
  random skip (`_rand` extern).

### C callers

- `_play_window_wipe_clipped`: none found (ASM-internal, called only from
  `_sprite_table_wipe_active`).
- `_sprite_script_frame_driver`: `src/GAME.C`, `src/LEVEL.C`.
- `_sprite_table_wipe_active`: `src/GAME.C`, `src/LEVEL.C`.
- `_board_actors_draw`: `src/BOARD.C`, `src/GAME.C`, `src/LEVEL.C`.

---

## 4. `asm/SPRDRAW.ASM` — `M_6036_6181` (502 bytes, HIGH)

Three contiguous routines in one module: `F_6036`, `F_60A9`, `F_6181`
(only `F_6036` starts at an even address; the other two continue from the
routine before them, per the same word-alignment evidence as SPRITES.ASM).

### Public routines

| Routine | Original addr | Args | Returns / clobbers |
|---|---|---|---|
| `_sprite_table_queue_draws` | F_6036 | none | none. Clobbers ax/bx/cx/dx/si/di/es (bp/ds preserved via push/pop). |
| `_animated_tile_tick` | F_60A9 | none | none. **Not** a Turbo C unit: BP is used as a data register (`mov bp,dx`/`mov dx,bp`) and then re-pointed at the stack mid-body to patch an already-pushed argument between two calls; SI/DI are LODS-driven with DS reloaded via `lds`; the loop count lives in CX across two calls (file's own header comment). |
| `_sprite_record_adjust_draw` | F_6181 | `id` [bp+4] (one stack arg) | none. Assembled with explicit `org` directives so its FIXUPP record order matches the original Turbo-C-adjacent layout (see the file's own comment on FIXUPP emission order). |

### DGROUP addresses read/written

| Addr / name | Access | Role |
|---|---|---|
| `_sprbase` (far, dword) | R | Sprite frame bank; frame offset = `(flags & 0x1F) * 0x1E6 + 2`. |
| `_objtab` (far, dword) | R | 3-byte-record sprite object table (word id/offset, byte flags), count-prefixed. |
| `_g0a20` (word) | W | Set to `10` once `_sprite_table_queue_draws` exhausts `_objtab` — "draw list ready" flag. |
| `_ga20` (word) | R/W | `_animated_tile_tick`'s own countdown — decremented every call, the tick body only runs every 10th call (`_ga20` reset to `0Ah`). **Distinct word from `_g0a20` above** (offsets 0x0A20 vs whatever `_ga20` resolves to — confirm exact addresses via `recipes/data` before Wave 2 renames either; the names are easy to conflate). |
| `_g40d0` (far, dword) | R | Animated-tile record table (length-prefixed, 3-byte records: word x/y-packed, byte flags/frame). |
| `_sprite_tile_bank` (far, dword) | R | Far sprite bank `_animated_tile_tick` indexes the same way `_sprbase` is indexed (`frame*0x1E6+2`). |
| `_raycast_trail_active` (word) | R | Gates the whole tick body (`cmp ...,0 / je armed`). |

### Data structures

- **`_objtab`**: count byte, then 3-byte records (word packed x/y or id,
  byte flags — low 5 bits select the frame within `_sprbase`).
- **`_g40d0`**: length-prefixed, 3-byte animation records (word x/y, byte
  flags/frame — bit 7 arms the record, bit 6 selects count-up vs
  count-down, low 5 bits are the current frame 0..0x17).
- **`draw_queue_append`** (external, `asm/DRAWQBUF.ASM`) is called from
  `_sprite_table_queue_draws` with `(id, x, y, color=0xF, height=0x1E)` —
  this ties SPRDRAW.ASM's output directly into DRAWQBUF's 5-byte queue
  record format (§6 below).

### C callers

`_sprite_table_queue_draws`: `src/BOARD.C`. `_animated_tile_tick`:
`src/GAME.C`. `_sprite_record_adjust_draw`: `src/BOARD.C`.

---

## 5. `asm/ANIMROW.ASM` — `F_9EC3` (125 bytes, HIGH)

### Public routine

| Routine | Original addr | Args (stack) | Returns |
|---|---|---|---|
| `_anim_step_row_copy` | F_9EC3 | `srcRowIdx` [bp+4], `dstOff` [bp+6], `srcOff` [bp+8], `count` [bp+0Ah], `dstRowIdx` [bp+0Ch], `?` [bp+0Eh], `rowStride` [bp+10h], `?` [bp+12h] | none |

**Convention:** near proc, standard C stack args (8 words — matches
`src/ANIMSTEP.C`'s call shape, `anim_step_row_copy(a/4, b, q/4, d, e/4, f,
i, 0x50)` / `(a/2, b, q/2, d, e/2, f, i, 0xA0)`). Copies a clipped byte
span through two independently-indexed rows of the `_g3924` row-pointer
table, byte-by-byte via a self-modifying-looking (but actually a fixed
XLAT-table-driven) copy loop — the `xlat` calls against `ds:[12B0h]`/
`ds:[12C0h]` are lookup tables this scan did not further decode (worth a
Wave 2 look: they gate/transform each row-segment's copy length).

### DGROUP addresses read/written

| Addr / name | Access | Role |
|---|---|---|
| `_g3924` (far pointer array, dword, `extrn` in this file) | R | The 488-entry row-pointer table `docs/current/portability-boundaries.md` §1 documents (`video_alloc_framebuffer`'s output) — indexed twice, once per row argument, each scaled `*4` (dword stride) before the `les`/`lds`. |
| `12B0h`, `12C0h` (raw DS offsets, byte tables) | R (`xlat`) | Two 256-entry lookup tables this scan did not identify by C-facing name — review for Wave 2 (likely small fixed constant tables baked into DGROUP init data; check `recipes/data`). |

### Data structure

Operates directly on the packed 4bpp framebuffer's row-pointer table
(`g3924`) described in `docs/portable/architecture.md`'s "Video model"
section — this routine is the row-copy primitive `ANIMSTEP.C`'s two
`anim_step_row_copy` call sites use for the intro/animation horizontal
scroll effect (stride `0x50`=80 or `0xA0`=160 bytes, i.e. quarter- and
half-width copies of the 320px-wide/160-byte-stride framebuffer).

### C callers

`src/ANIMSTEP.C` (2 call sites, lines 18 and 20).

---

## 6. `asm/ICONANIM.ASM` — `M_D386_D3CF` (84 bytes, MEDIUM)

### Public routines

| Routine | Original addr | Args | Returns |
|---|---|---|---|
| `_icon_list_animate_draw` | F_D386 | none (reads `_icon_record_list_ptr`) | none |
| `_icon_frame_reset_and_draw` | F_D3CF | none — **direct branch continuation of `F_D386`**, not an independently-entered flow (file's own header comment) | none |

**Convention:** near procs, no stack args; `_icon_frame_reset_and_draw`'s
own body (`mov byte ptr [di],1; mov si,di; add si,3; lodsb; jmp short
fd386_lookup`) only makes sense with DI/SI already positioned by
`_icon_list_animate_draw`'s in-progress record walk — it is reachable from
C only in the sense that it's `public`, but semantically it is a shared
tail, not a standalone entry point.

### DGROUP addresses read/written

| Addr / name | Access | Role |
|---|---|---|
| `_icon_record_list_ptr` (far, dword) | R | Far pointer to the count-prefixed icon-animation record list. |
| `_a72b2` (far pointer array, dword) | R | Indexed by frame id (`(frame&0xff)<<2`) to find the bitmap `gfx_blit_bitmap` draws. |

### Data structure

Variable-length, nested count-prefixed record list at
`_icon_record_list_ptr`: outer byte = record count; each record is
byte(len)+byte(state/frame-index)+word(x/y packed)+byte(frame id), where a
frame id with its sign bit set (`dec al; js ...`) routes to the "reset"
path (`_icon_frame_reset_and_draw`) instead of drawing directly. The frame
id (after masking) indexes `_a72b2` to find a far bitmap pointer, which is
handed to `gfx_blit_bitmap(es:si, x_hi, y*2)`.

### C callers

`_icon_list_animate_draw`: `src/GAME.C` (grep-confirmed).
`_icon_frame_reset_and_draw`: none found from C — confirms it is an
internal tail-continuation, not a real second entry point, despite being
`public`.

---

## 7. `asm/DRAWQ.ASM` — `M_D61C_D79C` (507 bytes, MEDIUM)

### Public routines

| Routine | Original addr | Args | Returns |
|---|---|---|---|
| `_draw_queue_render_highlighted` | F_D61C | none | none |
| `_draw_queue_render` | F_D79C | none | none |

**Convention:** near procs, no stack args — both read their command list
through a far pointer stored at a fixed DGROUP cell (see below), matching
the module's own header comment: "render an ES:DI command list" (the
ES:DI convention here means "the routine loads ES:DI itself from a known
DGROUP cell," not "the caller passes ES:DI in" — no caller in `src/*.C`
sets up ES:DI before calling these; they are ordinary 0-arg near calls).
`_draw_queue_render_highlighted` is the same renderer plus a "brighten the
24-cell board lattice, render, then restore" wrapper (`set_cell_attribute`
macro, `mov byte ptr [bx+cell],value` at 24 fixed offsets from `bx=2380h`).

### DGROUP addresses read/written

| Addr / name | Access | Role |
|---|---|---|
| `0BFC0h` (far pointer, raw displacement) | R (`les di,dword ptr ds:[0bfc0h]`) | Points at the command-list buffer both routines render. **No C-facing name or write site was found anywhere in `src/*.C` or `asm/*.ASM` in this scan** — flagged review; per the record shape below it is very plausibly the same buffer `asm/DRAWQBUF.ASM` (§8) produces at a fixed `DS:2F30h`, but the write site that would prove `0BFC0h`'s far pointer actually resolves to `DGROUP:2F30h` was not located here (may be set by `RUNTIME_BLOCK.ASM`, by initialized DATA in `recipes/data`, or by a routine this scan didn't cover). |
| `g2380` / `0x2380` (near byte array, `char g2380[][0x82]`, `src/BOARD.C:15`) | W (24 fixed cell offsets, via the `set_cell_attribute` macro) | The 24-cell board-lattice highlight grid — confirmed by name via `src/BOARD.C`'s own `extern char g2380[][0x82];` declaration; row stride 0x82 (130) matches the 24 offsets here exactly (21h, 0A3h=21h+82h, 105h=0A3h+82h, ... each +0x82). |
| `ds:[96h]` (word, viewport row-clip state) | W (toggled `0x190` on entry, restored to `0x9F` on exit, highlighted variant only) | The viewport-clip word `include/VIDEO.H`'s own comment names (`docs/current/portability-boundaries.md` §1: "`gfx_copy_rect` clips against the viewport words DS:94h/96h") — `_draw_queue_render_highlighted` temporarily widens the row clip while it paints the lattice-highlighted pass, then restores it. |

### Data structure

Both routines consume the **same record format** `asm/DRAWQBUF.ASM` (§8)
produces: a leading count byte, then 5-byte records (byte "attribute" +
word + word) read as: `bx = word1` (a packed x/y-ish value whose low byte
becomes a cell-lattice index via `*0x176 + 0x2380`), then a 3-value
row-strip count derived from `word2`'s low byte with a parity-adjust
(`test ax,3; jz/jpo ...`), then a `gfx_copy_rect` call followed by 0 or
more `gfx_blit_bitmap` strip calls and a closing `gfx_copy_rect`. This is
the draw-queue that `docs/portable/architecture.md`'s gfx layer needs to
reproduce as one more "flush this frame's queued draws" primitive,
alongside RECTQ.ASM's separate invalidation-rect queue (§1).

### C callers

`_draw_queue_render_highlighted`: `src/BOARD.C`. `_draw_queue_render`:
`src/GAME.C`.

---

## 8. `asm/DRAWQBUF.ASM` — `M_D818_D825` (71 bytes, split HIGH/MEDIUM)

### Public routines

| Routine | Original addr | Args | Returns |
|---|---|---|---|
| `_draw_queue_reset` | F_D818 | none | none |
| `_draw_queue_append` | F_D825 | `attr` [bp+4] (byte), `w1` [bp+6] (word), `w2` [bp+8]... — see below | AX = address of the record's final word |

**Convention:** near procs, standard compact-model stack args for
`_draw_queue_append`; `_draw_queue_reset` takes none. Both operate on a
fixed near DS offset (`0x2F30`), not a far pointer — this is a local,
DGROUP-resident queue, unlike RECTQ.ASM's far-pointer-cursor queue.

Looking at `_sprite_table_queue_draws`'s call shape (`asm/SPRDRAW.ASM`,
§4): `draw_queue_append(id, x, y, color=0xF, height=0x1E)` — 5 pushed
words — while `_draw_queue_append`'s own body only reads 3 stack words
([bp+4] byte, [bp+6]/[bp+8] or [bp+10]/[bp+12] depending on the exact
call-site arg count). Wave 2 should re-derive the exact parameter count
from a disassembly cross-check (`tools/probe_module.py`) before writing a
C prototype — this inventory did not fully resolve whether every caller
passes the same argument count.

### DGROUP addresses read/written

| Addr / name | Access | Role |
|---|---|---|
| `DS:2F30h` (raw fixed displacement, near, byte array) | R/W | The queue's own leading count byte (`_draw_queue_reset` zeroes it; `_draw_queue_append` reads it, increments it, and computes the new record's address as `count*5 + 1` bytes past `0x2F30`). |

### Data structure

Leading count byte at `DS:2F30h`, then 5-byte records: byte (the caller's
`attr`/`id` argument), word (`w1`, packed as low=caller-byte-arg high=
caller-word-arg per the `stosw`/`mov ah,bl` shape), word (`w2`, same
packing). This is the exact record shape `asm/DRAWQ.ASM` (§7) consumes.

### C callers

`_draw_queue_reset`, `_draw_queue_append`: `src/BOARD.C` (both). Also
called ASM-to-ASM from `asm/SPRDRAW.ASM`'s `_sprite_table_queue_draws`
(§4) — i.e. this queue has both a C producer (BOARD.C) and an ASM producer
(SPRDRAW.ASM) feeding the same `DS:2F30h` buffer that DRAWQ.ASM drains.

---

## 9. `asm/SOUND.ASM` — `M_C1A0_CB48` (2492 bytes, HIGH, 41 members)

By far the largest module. Full DGROUP field map (every address, every
reader/writer, confidence per field) already exists at
`docs/current/sound-state.md` — **not reproduced here**; this section adds
the calling-convention/C-caller angle that document doesn't cover, and
lists all 41 routines by role.

### Calling convention

Only **8 of the 41** public routines are ever called from `src/*.C`
(grep-verified below); those 8 use the ordinary compact-model C stack
convention. The remaining 33 are ASM-to-ASM internal helpers using the
register ABI `docs/current/sound-state.md`'s field map documents
throughout: **SI = voice index × 2** (the stride into the 4-word
per-voice tables at DS:178C..17FB, e.g. `_v_b`, `_v_ctr`, `_v_a`,
`_v_hold`, `_v_len`), **ES:DI = the loaded sound-resource far pointer**
(`_snd_seg`:`_snd_base`) for the command-stream walkers, and plain
register args (AL = command byte, etc.) for the small decode/store
helpers. `docs/current/portability-boundaries.md` §5/§7 already documents
this SI-as-voice-index convention at a summary level; this table is the
per-routine detail.

### C-facing entry points (8)

| Routine | Original addr | Role | C callers |
|---|---|---|---|
| `_sound_tick_entry` | F_C1A0 | Per-tick service entry: save caller regs, drive one pump/service pass, restore. Called from inside the timer ISR (`src/TIMER.C`'s `timer_irq_handler`, §4 of the boundaries doc) — **the one place the timing boundary and the sound boundary are the same event**. | `src/TIMER.C` |
| `_sound_backend_select_init` | F_C77A | Select and initialise a sound backend (0=PC speaker/1=Tandy gate/2=queued OPL/3=probe). | `src/PLRLDPUB.C` |
| `_sound_voice_table_reload` | F_C7CB | Reload per-voice values from the current backend table (gated on `music_enabled`). | `src/RESCACHE.C` |
| `_sound_voices_reset` | F_C834 | Reset each configured voice, retain voice count. | `src/GAME.C`, `src/LEVEL.C`, `src/OPTIONS.C`, `src/PLAYERSL.C`, `src/SLOTMENU.C`, `src/SNDFXTGL.C` |
| `_sound_voices_disable_all` | F_C877 | Submit a disabled-voice update for every voice. | `src/SNDREQ.C` |
| `_opl_register_write` | F_C898 | Write one OPL register+data pair with the required settling delay (register-select write, 6× settling read, data write, 35-iteration settling loop — the exact hardware-timing sequence `docs/current/portability-boundaries.md` §(c).5 requires verifying against the target OPL backend). | `src/OPLINIT.C`, `src/OPLREG.C`, `src/VOXCHAN.C` |
| `_stream_control_block_arm` | F_CAF1 | Arm the single music-stream cursor for a new cue index (self-referential priority gate: only a ≤-current index pre-empts). Also called from ASM (`asm/SPRITES.ASM`'s bytecode interpreter, §3 above, via `F_CAF1`). | `src/BOARD.C`, `src/GAME.C`, `src/HITTEST.C`, `src/INTRO.C`, `src/LEVEL.C`, `src/PUZZLE.C`, `src/ROUNDEND.C` |
| `_sound_stop_reset` | F_CB48 | Clear the "stream armed" flag and invalidate the cached stream index. | `src/BOARD.C`, `src/GAME.C`, `src/INTRO.C`, `src/LEVEL.C`, `src/OPTIONS.C`, `src/PLAYERSL.C`, `src/PUZZLE.C`, `src/ROUNDEND.C`, `src/SLOTMENU.C`, `src/SNDFXTGL.C`, `src/SNDREQ.C` |

### Internal-only routines (33, register-ABI, no C caller)

| Routine | Original addr | One-line role |
|---|---|---|
| `_sound_voice_pump_loop` | F_C1F7 | Rescan the voice table in mode 2, dispatch one command per voice. |
| `_sound_voice_table_prime` | F_C232 | (Re)prime every configured voice's per-voice table entries. |
| `_sound_voice_service_loop` | F_C27D | Advance each voice's counter, retrigger on note-off. |
| `_sound_command_stream_dispatch` | F_C2EA | Split/dispatch the command byte at `[si+voice_stream_cursor_table]`. |
| `_sound_control_value_select` | F_C359 | Select and submit one value from the sound control state (the `snd_backend_mode==1` "paused" branch label lives here — see sound-state.md's DS:1778 caveat). |
| `_sound_command_value_derive` | F_C3DB | Derive a command value from ES:DI state, update its selector. |
| `_sound_control_block_advance` | F_C440 | Advance three two-word slots in the DS:177Ch control-state block (the pause-overlay byte-stream renderer). |
| `_sound_secondary_cmd_dispatch` | F_C501 | Dispatch one secondary stream command by its AH selector. |
| `_sound_command_flags_update` | F_C549 | Update the paired command-control flags (`v_hold`/`v_len`) from AL. |
| `_voice_percent_scale_store` | F_C567 | Signed scale-and-store for the `1788`/`178A` state pair. |
| `_sound_param_scale4` | F_C59A | Scale a raw 0..63 value by 4, store through `g1788`. |
| `_f_c5a8` | F_C5A8 | Store AL (zero-extended) through SI at `voice_dur_table` (17A4). |
| `_sound_table_word_select_store` | F_C5B3 | Select a word from the `1832h` table by AL, store into `state_cursor` (17C4). |
| `_f_c5c6` | F_C5C6 | Store AL (zero-extended) through SI at `state_text` (17DC). |
| `_voice_command_decode_apply` | F_C5D1 | Decode one command byte, update the selected SI-relative note/frequency state. |
| `_voice_enable` | F_C678 | Enable a voice: open the speaker gate (mode 0/1) or queue a value for the OPL bank (mode 2). |
| `_voice_disable` | F_C6B9 | Disable a voice, or submit an alternate-backend voice update. |
| `_sound_pit_divisor_program` | F_C706 | Write a PIT divisor (mode 0), queue it (mode 2), or emit packed OPL nibbles. |
| `_sound_voices_reset_and_service` | F_C755 | Combined reset+immediate-service helper. |
| `_opl_port_write_byte` | F_C8D4 | Write one raw byte to the OPL data port (helper `_opl_register_write` calls). |
| `_sound_tick_step` | F_C8E2 | Run one music-stream command via `F_C914` when due, age the delay counter. |
| `_sound_stream_command_step` | F_C914 | Fetch the next command byte, split into nibble sub-dispatch. |
| `_sound_note_dispatch` | F_C988 | Look up a PIT divisor for (note, octave-shift) in `_notetab`, program it. |
| `_sound_stream_delay_decode` | F_C9A4 | Decode one ES:[DI+1] command byte into a scaled delay, store into `_snd_delay`. |
| `_sound_ctlblock_command_dispatch` | F_CA03 | Dispatch one control-block command by AH selector (music-stream cluster). |
| `_sound_ctlblock_flags_latch` | F_CA35 | Latch/clear the control block's one-shot/length pair from AL. |
| `_stream_percent_scale_store` | F_CA51 | Signed percentage scale-and-store — the `1E84`/`1E86` twin of `F_C567`. |
| `_stream_base_value_set` | F_CA83 | Scale a raw 0..63 value by 4, store through `g1e84`. |
| `_stream_note_delay_set` | F_CA91 | Store AL (zero-extended) through `stream_note_delay` (1E92). |
| `_stream_note_program` | F_CA9B | Look up a divisor in `_notetab` for a shifted note delta, program it. |
| `_speaker_gate_on` | F_CAD0 | Open the PC-speaker gate (timer-2 output + speaker enable bits, port 0x61). |
| `_speaker_gate_off` | F_CADB | Close the PC-speaker gate. |
| `_pit_channel2_set_divisor` | F_CAE6 | Write AX's low then high byte to PIT channel 2 (port 0x42) — see `docs/current/portability-boundaries.md` §(c).4 for the exact register-sequencing requirement. |

### DGROUP clusters (pointer to full map)

Two clusters plus two standalone constants — see
`docs/current/sound-state.md`'s "Field map" tables for every address,
width, reader/writer list, and confidence:

- **DS:175E..1834** — primary sound-effects-driver state (up to 4
  "voices": `snd_seg`/`snd_base` pair, `sound_enabled`/`music_enabled`,
  `snd_on`, `mus_flag`, `snd_flag2`, `snd_backend_mode`, `snd_nvoices`,
  the 14-table struct-of-arrays block `v_b`/`voice_stream_cursor_table`/
  `voice_stream_base_table`/`v_ctr`/`voice_dur_table`/`v_a`/`v_hold`/
  `v_len`/plus 6 LOW-confidence pause-overlay-renderer fields, `notetab`,
  `opl_port`).
- **DS:1E84..1E96** — secondary single-stream "music stream" cluster
  (`mus_ptr`, `mus_arg`, `snd_len`, `snd_delay`, `stream_note_delay`,
  `snd_one`), the scalar twin of the per-voice cluster above.
- Port 0x42 (PIT channel 2 divisor), port 0x61 (PC-speaker gate bits),
  `opl_port` (0xC0 or 0x205, register-select) / `opl_port+1` (data) — I/O
  ports, not DGROUP addresses; see
  `docs/current/portability-boundaries.md` §5 and §(c).4-5 for the exact
  sequencing each requires.

### Data structures

The 14-table struct-of-arrays block at DS:178C..17FB (stride 8 bytes,
voice 0..3, SI=voice*2 into each table) and the two 12-word note tables
(`notetab` at 17FC, its sibling at 1814) are described exhaustively in
`docs/current/sound-state.md`'s "Structural finding" section — not
repeated here.

### C callers

See the "C-facing entry points" table above; `docs/current/sound-state.md`
already cross-references every DGROUP field's C-side readers/writers
(`src/PLRLDPUB.C`, `src/TIMER.C`, `src/OPTIONS.C`, `src/SLOTMENU.C`,
`src/SNDFXTGL.C`, `src/RESCACHE.C`, `src/OPLREG.C`, `src/OPLVOICE.C`,
`src/INTRO.C`, and the 7-8 files that call `stream_control_block_arm`/
`sound_stop_reset` above).

---

## Summary for Wave 2

- **9 modules, ~50 public routines total** (RECTQ 1, BOARDCOL 1, SPRITES 4,
  SPRDRAW 3, ANIMROW 1, ICONANIM 2, DRAWQ 2, DRAWQBUF 2, SOUND 41 — 8
  C-facing + 33 internal).
- **Two draw-queue families** share record shapes across module pairs:
  DRAWQBUF.ASM (producer, `DS:2F30h`) / DRAWQ.ASM (consumer, far pointer at
  `DS:0BFC0h` — write site not located in this scan, flagged review) for
  the "board lattice command list," and RECTQ.ASM (self-contained
  producer+consumer via `ui_gfx_blob`/`rect_queue_write_ptr`) for the
  separate invalidation-rect queue.
- **Cross-module ASM-to-ASM calls**: `asm/SPRDRAW.ASM` calls
  `asm/DRAWQBUF.ASM`'s `_draw_queue_append`; `asm/SPRITES.ASM`'s bytecode
  interpreter calls `asm/SOUND.ASM`'s `_stream_control_block_arm`
  (`F_CAF1`) and `asm/GAME.C`-side board-script functions — Wave 2 must
  preserve these cross-module call edges even though the two modules will
  likely land in different `portable/` subsystem directories (gfx vs.
  audio).
- **Open review items for Wave 2** (not resolved by this pass): the exact
  C-facing name (if any) for `0B3AEh`'s bytecode-program table and
  `0BFBAh` in SPRITES.ASM; the `12B0h`/`12C0h` XLAT tables in ANIMROW.ASM;
  confirmation that `DS:0BFC0h` (DRAWQ.ASM) actually resolves to
  `DGROUP:2F30h` (DRAWQBUF.ASM); and `_draw_queue_append`'s exact
  parameter count/order (SPRDRAW.ASM's call site pushes 5 words, the
  routine's own body reads what looks like fewer).
