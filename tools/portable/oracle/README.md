# Historical oracle harness

Generates golden fixtures for `portable/gfx` and `portable/resource` by
running the *real* 8086 code of `asm/DECODE.ASM` and
`asm/RUNTIME_BLOCK.ASM` under the pinned Turbo C 2.0 / TASM 1.0 toolchain
and MS-DOS Player. End users and the portable build never need this;
it only *produces* `portable/tests/fixtures/*.json`.

## Pieces

- `ORACLE.C` -- Turbo C (compact model) driver. Reads a binary command
  script `ORACLE.IN` from the current directory, writes `ORACLE.OUT`.
  Written with **no standard headers** (`<stdio.h>`, `<dos.h>`, ...): the
  pinned toolchain checkout only carries `TCC.EXE`/`TASM.EXE`/`TLINK.EXE`
  and `CC.LIB`, no `TC\INCLUDE`, matching the historical tree's own house
  style (`src/*.C` never includes standard headers either -- see e.g.
  `src/RESOURCE.C`'s `extern int open(); extern char far *farmalloc();`).
  FILE* handles are carried as opaque `void far *` (confirmed present in
  `CC.LIB` as `_fopen`/`_fread`/`_fwrite`/`_fclose`/`_open`/`_read`/
  `_write`/`_close`/`_lseek`/`_farmalloc`/`_exit`); `FP_SEG`/`FP_OFF`/
  `MK_FP` are hand-rolled via a `union { char far *p; unsigned w[2]; }`
  (a Turbo C far pointer is 4 bytes, offset word at the lower address,
  segment word at the higher address -- confirmed against how
  `asm/DECODE.ASM`'s `lds`/`les` read far-pointer stack arguments).
- `DSCALL.ASM` -- `int ds_call(unsigned seg, void (near *fn)(), int nwords,
  int near *words)`. `asm/RUNTIME_BLOCK.ASM` primitives address a fixed
  DGROUP layout through absolute `ds:[imm]` offsets; this thunk pushes
  `words[nwords-1..0]` (so `words[0]` lands at the callee's `[bp+4]`,
  matching ordinary left-to-right C parameter order), swaps `DS` to the
  fake DGROUP block, does one near `CALL`, swaps `DS` back, and returns
  `AX`. No C statement runs while `DS` is switched.
- `build_oracle.py` -- assembles/compiles/links `ORACLE.EXE` and verifies
  `_runtime_base` (the entry point of `asm/RUNTIME_BLOCK.ASM`, which is
  position-dependent: `RT_CS equ 039Ch`) lands at exactly `_TEXT+0x039C`,
  inserting a padding `_TEXT` object (`tools/omf_scaffold.make_text_padding`)
  between `C0C.OBJ` and `RUNTIME_BLOCK.OBJ` and re-linking until it does.
  Output: `build/oracle/ORACLE.EXE` (+ `ORACLE.MAP`).
- `gen_fixtures.py` -- builds the ORACLE.IN scripts, runs them through
  MS-DOS Player (falls back to DOSBox via `tools/dos_runner.py`), and
  writes `portable/tests/fixtures/{gfx_cases,decode_cases,
  resource_golden_dos}.json`.

Rebuild everything with:

```
python tools/portable/oracle/build_oracle.py
python tools/portable/oracle/gen_fixtures.py
```

(`gen_fixtures.py` builds `ORACLE.EXE` itself if it's missing; pass
`--skip-build` to reuse an existing one, `--skip-resource` to skip
`resource_golden_dos.json`, `--dosbox` to force the DOSBox backend.)

## ORACLE.IN / ORACLE.OUT record format

All integers little-endian (x86 DOS is little-endian, so a raw
`fread`/`fwrite` of the right width already has the right value -- no
manual byte assembly anywhere in `ORACLE.C`).

`ORACLE.IN`: 4-byte magic `"ORC1"`, then commands until `0xFF`:

| op   | payload |
|------|---------|
| 0x01 SEED_FB    | `u32 seed` -- fills the 488x160 framebuffer row-major with `x = x*1103515245 + 12345` (mod 2^32, unsigned long), `byte = (x>>16)&0xFF`, **update-then-emit** (the formula runs once before the first byte). Also resets `rect_queue_write_ptr` to the base of the dirty-queue buffer and clears the last-save/last-return tracking. |
| 0x02 SET_STATE  | `i16 result, gbc, g94, g96, g98, g9a` |
| 0x03 SET_FONT   | `i16 c0de, c0e2, c0e4, c0e6, line_height; u16 blob_len; blob` -- copies `blob` into a dedicated far block and sets `gc0e0` (DS:C0E0, a plain *segment* word, not a far pointer) to that block's segment. Not reset by SEED_FB: set once, persists for the rest of the process. |
| 0x04 CALL       | `u8 op_id; i16 a[6]; u16 blob_len; blob` -- see op table below |
| 0x05 DUMP       | (no payload) -- appends a result record to `ORACLE.OUT` |
| 0x06 DECODE     | `u8 kind; u16 param; u16 blob_len; blob` -- `kind`: 0=`rle_packbits_decode` (param=max_len), 1=`lz_decompress` (param=src_len), 2=`sprite_decode_4bpp_planar` (in place, param ignored), 3=`sprite_decode_4bpp_mode13h` (in place, param ignored) |
| 0x07 RESOURCE   | `u8 dir (0=AE000.DAT, 1=AE001.DAT); u32 record_index` -- runs `src/RESOURCE.C resource_load_record`'s flag/type dispatch (transcribed into `do_resource()`) against the real archive files, which must sit next to `ORACLE.EXE` |

`CALL` op ids (argument order matches `portable/include/gfx.h` exactly;
unused `a[]` slots are `0`; word counts below are the actual argument
words `ds_call` pushes -- every far pointer, e.g. an image/bitmap/buffer
argument, is 2 words, offset then segment):
`0`=bar(x,y,n) [3w] `1`=vline(x,y,n) [3w]
`2`=clear_rect(x,y,w,h) [4w] `3`=fill_rect(x,y,w,h) [4w]
`4`=save_rect(x,y,w,h,off,seg) [6w] (destination is the harness's own
scratch buffer) `5`=restore_rect(x,y,off,seg) [4w] (blob is the buffer:
`u16 bytes_per_row, u16 rows`, then `bytes_per_row*rows` data bytes)
`6`=wipe_rect(sx,sy,w,h,dx,dy) [6w] `7`=copy_rect_flip_v(...) [6w]
`8`=copy_rect_flip_h(...) [6w] `9`=copy_rect_flip_hv(...) [6w]
`10`=copy_rect_split(...) [6w] `11`=copy_rect_split_flip_v(...) [6w]
`12`=draw_char(x,y,glyph) [3w] `13`=blit_bitmap(x,y,off,seg) [4w] (blob:
32-byte header + `u8 bytes_per_row, u8 rows` + data)
`14`=copy_rect(x,y,off,seg,flip) [5w] (same blob layout as blit_bitmap)
`15`=blit_image(x,y,off,seg) [4w] (blob: `u8 bytes_per_row, u8 rows` +
1bpp data -- **see "Known issue" below**) `16`=set_pixel(x,y) [2w]
`17`=get_pixel(x,y) [2w]`.

`ORACLE.OUT`: one record per `DUMP`/`DECODE`/`RESOURCE`, in script order:

- `DUMP` -> `0xD0`, the full 78080-byte framebuffer (row-major, 160
  bytes/row), `u16 dirty_len` + that many dirty-queue bytes (written since
  the last `SEED_FB`), `i16 last_ret` (the last `CALL`'s `AX`, meaningful
  for `draw_char`/`get_pixel`), `u16 save_len` + that many bytes (the most
  recent `save_rect` output: `u16 bytes_per_row, u16 rows` header + data;
  `0` if no `save_rect` has run yet).
- `DECODE` -> `0xD2`, `i16 ret`, `u16 outlen`, `outlen` bytes (the decoded
  buffer for kinds 0/1, the in-place-fixed-up blob for kinds 2/3).
- `RESOURCE` -> `0xD1`, `i16 s` (the historical 16-bit `int` exactly as
  `resource_load_record` computed it -- **may be negative**, a legitimate
  16-bit-truncation result, not an error), `u8 type` (`gc0cb`), `u16
  outlen` (`0` when `s <= 0` -- never the 16-bit-wrapped view of a
  negative `s`, which would falsely claim tens of KB of trailing data),
  `outlen` bytes.

## `gfx_cases.json` fixture fields

Each case is `{name, seed, state, calls, expect}` plus, for `draw_char`
cases only, a `font` object -- since `SET_FONT` is issued once up front
in `gen_fixtures.py` and font state is not part of `SEED_FB`/`SET_STATE`,
a `draw_char` case is not independently replayable without it. `font` is
`{blob_hex, gc0de, gc0e2, gc0e4, gc0e6, line_height}`: `blob_hex` is the
exact bytes of the synthetic font sheet as laid out in memory at the
`gc0e0` base (`gen_fixtures.build_font_blob`, 14 glyphs -- widths
1..12, 16, 20 -- `line_height` rows each, deterministic bit patterns);
`gc0de`/`gc0e2`/`gc0e4`/`gc0e6` are offsets **relative to the blob's own
start** (a consumer must set the fake-DGROUP word at `0xC0E0` to the
blob's segment and the words at `0xC0DE`/`0xC0E2`/`0xC0E4`/`0xC0E6` to
`blob_offset + <that field>`, exactly what `do_set_font()` does). The
case's own `expect.ret` is the glyph's advance (`gfx_draw_char`'s return
value).

## Invariants

- **`_runtime_base` at `_TEXT+0x039C`.** `asm/RUNTIME_BLOCK.ASM` computes
  its CS-relative jump/dispatch tables against `RT_CS equ 039Ch`; the link
  order is `C0C.OBJ + PAD.OBJ + RUNTIME_BLOCK.OBJ + ORACLE.OBJ + DECODE.OBJ
  + DSCALL.OBJ`, and `build_oracle.py` iterates the padding size
  (`tools/omf_scaffold.make_text_padding`) until `OUT.MAP` reports
  `_runtime_base` at exactly that offset. Observed: `pad_len = 0x1E0`,
  matching the task's own note that the historical module preceding
  `RUNTIME_BLOCK` (VIDEO.C's unit) is 0x1E0 bytes.
- **DS switching.** `asm/RUNTIME_BLOCK.ASM` primitives are the *only*
  code that ever runs with `DS` pointed at the fake DGROUP block; `DSCALL.ASM`
  switches `DS` immediately before the `CALL` and restores it immediately
  after, and `ORACLE.C` never touches a `gfx_*` global directly (only
  through the far-pointer `PEEKFP`/`POKEFP`/`PEEKW`/`POKEW` helpers, which
  address the fake DGROUP through its own far pointer variable and so work
  correctly regardless of the current `DS`).
- **`asm/DECODE.ASM` has no DS dependency.** Verified (`grep -c "ds:"
  asm/DECODE.ASM` => 1, and that one hit is not an absolute-offset
  reference): its four routines take only far-pointer/word arguments and
  are called directly, without `ds_call`.
- **Fake DGROUP layout** (all offsets absolute, matching
  `asm/RUNTIME_BLOCK.ASM`): row table at `0x3924` (488 far pointers,
  paragraph-normalized exactly like `src/VIDEO.C video_alloc_framebuffer`,
  stride `0xA0`), `gbc` at `0x00BC`, clip words at `0x94/0x96/0x98/0x9A`,
  `rect_queue_write_ptr` at `0x40C4` (only its *offset* word is updated by
  the ASM after the initial `SEED_FB`-time reset -- the segment word is
  never touched again, so the dirty-queue scratch buffer must never
  itself cross a 64K boundary), `result` at `0x40C8`, font state at
  `0xC0DE..0xC0E8`.

## Known issue: `gfx_blit_image` (widths not a multiple of 4 bytes)

Root cause found (not a harness calling-convention bug -- argument words
were already correct: 4 words, `x, y, offset, segment`, matching
`asm/RUNTIME_BLOCK.ASM`'s `runtime_f03cf` body exactly).

`runtime_f03cf` processes a source row 16 "half-nibble slots" at a time
(`bx = bytes_per_row*4`; the unrolled block at `rt_18b8..rt_1909` handles
one full group of 16; `sub bx,0x10; jg $-67h` loops back for the next
full group). Whenever **fewer than 16 remain** -- i.e. whenever
`bytes_per_row` is not itself a multiple of 4, or as the leftover
remainder after processing the full groups of a wider row -- it instead
does `jmp word ptr cs:[bx+runtime_even_pixel_dispatch-_runtime_base+RT_CS]`
(even x) or the `...odd_pixel_dispatch` equivalent. Both dispatch tables
are defined **once**, at `asm/RUNTIME_BLOCK.ASM:3084/3088`, and are also
the tables `runtime_f03c6` (`gfx_draw_char`, line 2956/3020) jumps
through -- their entries are `rt_14xx`/`rt_15xx` labels, i.e.
**draw_char's** code, not blit_image's. blit_image's own matching targets
(`rt_18xx`/`rt_15xx`-adjacent labels used by nothing else) sit a little
further down, in a table the file's own comment names
`runtime_unreferenced_pixel_dispatch` -- *"Unreferenced translated copy
of runtime_even_pixel_dispatch (+0424h)"* (line 3474) -- i.e. the
reconstruction already noticed this second, unused table exists at a
fixed +0x424 offset from the first, but `runtime_f03cf`'s jump was
transcribed to reference the wrong (draw_char's) one. Landing in
draw_char's code with blit_image's register state is what crashes
`ORACLE.EXE`.

Confirmed empirically by varying only `bytes_per_row` (1,1,x=0, isolated
in an otherwise-empty `ORACLE.EXE` run, so no cross-case state is
involved): 1,2,3,5,6,7,9 all crash (`Abnormal program termination`);
4,8,12,16 -- the widths whose 16-slot groups divide evenly, so the
dispatch table is never reached -- all render correctly. `gfx_cases.json`
therefore only exercises `blit_image` at `bytes_per_row` 4/8/12/16;
`gen_fixtures.py`'s comment above the case list has the same writeup.
This is a claim about `asm/RUNTIME_BLOCK.ASM` (read-only historical
tree), not something this harness can or should work around by e.g.
supplying a different table -- flagged for the supervisor to confirm and
decide how to record/fix upstream.

## Batch size (gfx / resource runs)

`Script` records each case/record as its own byte "segment"
(`tools/portable/oracle/gen_fixtures.py`), and `run_gfx`/`run_resource`
replay them across several separate `ORACLE.EXE` invocations
(`GFX_CHUNK_SIZE`, `RESOURCE_CHUNK_SIZE`) rather than one huge run. This
started as a workaround for what looked like a cumulative MS-DOS Player
issue, but turned out to have two distinct, since-fixed causes once
isolated case-by-case:

1. The `gfx_blit_image` crash above (now simply excluded for the widths
   that hit it).
2. A real bug in this harness's `res_block` layout (see next section):
   fixed, no longer a factor.

With both addressed, chunking is no longer load-bearing for correctness,
only for keeping any single MS-DOS Player invocation's log/output
manageable; it's kept as cheap insurance.

## Fixed bug: `resource_golden_dos.json` was wrong (`res_block` layout)

An earlier version of this harness laid `blob`/`shadow_a`/`shadow_b` out
with flat *offset* arithmetic within one segment (`blob = res_block +
0x0E`, `shadow_a = res_block + 0x10`, `shadow_b = res_block + 0x7D40`).
That is wrong: `src/PLAYERSL.C ui_gfx_alloc` (the historical allocator
for these three buffers) does not do that --

```c
ui_gfx_blob     = farmalloc(0xFA80L); s = FP_SEG(ui_gfx_blob) + 1;
ui_gfx_blob     = MK_FP(s, 0x0E);
ui_gfx_shadow_a = MK_FP(s + 1, 0);
ui_gfx_shadow_b = MK_FP(s + 0x7D4, 0);
```

-- it gives `shadow_a`/`shadow_b` their own paragraph-normalized segment
with offset 0 each. Physically that's the same byte as the flat-offset
version (segment `s+1` is exactly 16 bytes past segment `s`; `s+0x7D4` is
exactly `0x7D40` bytes past it), but the *far pointer's offset component*
differs, and that is load-bearing: `asm/DECODE.ASM`'s `_lz_decompress`
stores back-reference checkpoints as plain 16-bit offsets into the
destination (`mov [bx],di`) and later reconstructs a source address from
one assuming it is small relative to the destination segment's own
start. `DI` is a 16-bit register that wraps at 64K of growth *from
wherever it starts* -- giving `shadow_a`/`shadow_b` flat offsets of
`0x10`/`0x7D40` left them only ~2 and ~33KB of headroom instead of a full
64K, so any record whose LZ-stage output (before the RLE stage, which can
shrink it back down again) ran past that wrapped `DI` back to a small
offset mid-decode, corrupting `shadow_a` while it was still being read as
the source and truncating the result. Confirmed against
`docs/current/runtime-platform.json` / `portable/tests/fixtures/
resource_golden.json`: `AE000_002` (a display-selector-5 replacement
runtime blob, `flags=3` so both LZ and RLE stages run) decoded to 127
bytes through the broken layout instead of the correct 1886
(`sha256 763d28e6...`).

Fixed in `ORACLE.C` by constructing `res_blob`/`res_shadow_a`/
`res_shadow_b` with the exact same paragraph-normalizing arithmetic as
`ui_gfx_alloc` (`RES_BLOCK_BYTES` oversized to 0x30000 so `shadow_b`'s
segment, which starts 0x7D40 bytes into the block, has its entire
addressable 64K covered by the allocation -- the historical 0xFA80 total
only guarantees `shadow_b` headroom up to the end of that smaller block,
which the original game apparently never exceeded but a handful of real
`AE000.DAT`/`AE001.DAT` records do). `record_counts()` was also fixed to
report `table_entries - 1` records per archive (the offset table's last
entry is the end-of-data marker for the previous record, not the start of
a record of its own -- confirmed: 89 + 131 = 220 matches
`resource_golden.json` exactly). **All 220 records now match
`portable/tests/fixtures/resource_golden.json`'s `s`/`sha256` exactly.**
