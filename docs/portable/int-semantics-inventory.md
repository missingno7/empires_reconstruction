# 16-bit integer semantics inventory

Companion to `docs/portable/architecture.md`'s "Integer discipline (Phase 1)"
section and `docs/current/portability-boundaries.md` §8. That section lays
out the rule (`portable/include/dos_types.h`'s `dos_char/dos_uchar/dos_int/
dos_uint/dos_long/dos_ulong` aliases, plus the `dos_i16/dos_u16/dos_add16/
dos_sub16/dos_mul16/dos_uadd16/dos_usub16` helpers); this document is the
per-site evidence that later Wave 2 agents translate against.

Built 2026-09-21 by grepping `src/*.C` for the phrases the task specifies
(`16-bit`, `SIXTEEN`, `unsigned`, `signed`, `truncat`, `cwd`, the `jb`/`ja`/
`jae`/`jl`/`jg` family, `rule 5`, `rule 16`, `shr (UNSIGNED)`, `char
arithmetic`, `byte register`, `wrap`) and reading each hit's surrounding
function. Every entry below already carries a disassembly-backed claim in
the historical source's own comments — this document does not add new
claims, it re-expresses the existing ones as `dos_types.h` spellings.

## How to read this

- **Confirmed facts** (part 1): the historical comment cites concrete
  codegen evidence (an instruction mnemonic, an opcode byte sequence, or an
  explicit "rule N" cross-reference to `docs/current/tasm-reconstruction-rules.md`-style
  compiler rules). Treat these as facts, matching
  `docs/current/portability-boundaries.md`'s own confidence bar.
- **Review candidates** (part 2): a plain regex sweep for `long` arithmetic,
  `>>`/`/`/`%` on operands whose signedness is not proven here, and a note on
  mixed-type comparisons. These are **not** claimed facts — each needs a
  human (or a differential test) to confirm the operand types before any
  `dos_*` helper is applied. Marking a site "review" here means "found by
  grep, not yet verified," per the task's own instruction not to overclaim.

Facts that are purely about **far-pointer push order / calling convention**
(the many "rule 5: a far pointer GLOBAL pushes its segment word then its
offset word" comments, e.g. `src/INTRO.C:263`, `src/RESOURCE.C:314`) are
**out of scope for this document** — they are ABI facts belonging to
`docs/current/portability-boundaries.md` §7 (far/near pointers), not
width/signedness facts. They are omitted here unless the same comment also
carries a width/signedness claim (two do, and are included below with both
aspects noted).

---

## Part 1 — Confirmed width/signedness facts

### 1. `src/RESOURCE.C:225`, `resource_load_record` (F_656C)

```c
d = p >> 12;
```

Comment (`src/RESOURCE.C:172-175`, disassembly `6574 8B7E04 B10C D3EF`):
"`d = p >> 12`: `shr` (UNSIGNED) with the count in CL, so p is unsigned and
the shift is not an arithmetic one." `p` is the function's `unsigned p`
parameter (declared `unsigned` at `src/RESOURCE.C:217`).

**Portable spelling:** keep `p`/`d` as `dos_uint` (not `dos_int`); a plain
`d = p >> 12;` on `dos_uint` operands already reproduces a logical shift in
C (an unsigned 16-bit value promotes to a non-negative `int`, so `>>` never
sign-extends). No runtime helper needed — the whole fact is "don't let this
become a signed type."

**Confidence:** HIGH (disassembly-cited).

### 2. `src/RESOURCE.C:235`, `resource_load_record` (F_656C)

```c
s = (int)o2 - (int)o1;
```

Comment (`src/RESOURCE.C:189-192`, disassembly `65D5 8B76F8 2B76F4`): "the
size is a SIXTEEN-bit subtraction of the two longs' low words: `(int)o2 -
(int)o1`. Writing `(int)(o2 - o1)` would have emitted the 32-bit sub/sbb
pair first and then truncated." `o1`/`o2` are `long` locals
(`src/RESOURCE.C:219-220`). This is the record-length computation and is
also cited as a required-verification behavior in
`docs/current/portability-boundaries.md` §(c).7.

**Portable spelling** (this is the literal example
`docs/portable/architecture.md` gives):
```c
dos_int s = dos_sub16(dos_i16(o2), dos_i16(o1));
/* equivalently: dos_i16((int16_t)(uint16_t)o2 - (int16_t)(uint16_t)o1) */
```
Truncate **each** operand to 16 bits first, then subtract — never compute
`o2 - o1` as a 32-bit subtraction and truncate the result afterward; the two
give different answers whenever the true (32-bit) difference does not fit
in 16 bits, which `docs/current/portability-boundaries.md` §(c).7 says the
historical code relies on for some records.

**Confidence:** HIGH (disassembly-cited; also a required-verification item).

### 3. `src/RESOURCE.C:232`, `resource_load_record` (F_656C)

```c
lseek(slot_file_handle, (long)(p * 4), 0);
```

Comment (`src/RESOURCE.C:181-186`, disassembly `659A D1E0 D1E0 33D2 52 50`):
"`p*4` zero-extended into DX:AX and pushed high word first: ONE long
argument built from an unsigned int, which is what `(long)(p * 4)` compiles
to. A signed int would have been widened with CWD, and a long-typed p would
not have needed the widening at all."

**Portable spelling:** keep `p` as `dos_uint`; the multiply-then-widen must
stay zero-extending, not sign-extending:
```c
dos_long offset = (dos_long)(dos_uint)(p * 4u);
```
The inner `p * 4u` should itself be computed in `dos_uint` (or explicitly
`dos_u16(...)`) before the widening cast, since `p`'s promoted-to-`int`
value under C's integer-promotion rules could in principle differ from the
8086's native 16-bit unsigned multiply if `p` were ever large enough to
overflow 16 bits (it isn't here — `p` is masked to `0xfff` at line 226 — but
the spelling should not rely on that invisibly).

**Confidence:** HIGH (disassembly-cited).

### 4. `src/RESOURCE.C:92`, `resource_file_open` (F_6266)

```c
c = 0x42 - c + 0x41;
```

Comment (`src/RESOURCE.C:26-30`, disassembly `6338 B042 2A46F7 0441`):
"`c = 0x42 - c + 0x41` stays in AL: char arithmetic in a byte register when
operands and destination are all chars, and the two constants are NOT
folded." `c` is `char` (`src/RESOURCE.C:74`); this is the directory-name
case-fold byte math.

**Portable spelling:** declare `c` as `dos_char` (signed 8-bit, matching
Turbo C's default-signed `char`) and let the assignment truncate:
```c
dos_char c;
...
c = (dos_char)(0x42 - c + 0x41);
```
The right-hand side promotes to `int` under modern C (Turbo C's byte-register
codegen is an implementation detail, not an observable difference here),
but storing back into `dos_char c` truncates to the low 8 bits exactly as
the 8086's AL-only computation did, since both are mod-256 arithmetic on the
same byte values. The explicit cast documents the truncation even though
the plain assignment alone would already be correct.

**Confidence:** MEDIUM (disassembly-cited for the codegen shape; the
"stays in AL" claim is about register allocation, not about a value the
plain-assignment truncation could get wrong — verified by re-deriving the
arithmetic, not by a differential test).

### 5. `src/VIDEO.C:110-125`, `video_alloc_framebuffer` (F_0281)

```c
register int w;
...
g40ca = farmalloc((long) w * 488 + 16);
```

Comment (`src/VIDEO.C:109-111`): "the size is `(long)w * 488 + 16` and the
`cwd` before the long multiply makes `w` a SIGNED int widened to long
(rule 16's shape)." `w` is declared `register int w;` (signed).

**Portable spelling:**
```c
dos_int w;
...
dos_long size = (dos_long)w * 488 + 16;
```
The explicit `(dos_long)` cast on `w` before the multiply is required — if
the multiply were done in `dos_int` first and only the *result* cast to
`dos_long`, an intermediate overflow (impossible here given the row-table
size, but the point is codegen fidelity, not just this call site's safe
range) would round-trip differently. This is rule 16's "cast-before-widen"
shape, the mirror image of fact 2's "truncate-then-subtract."

**Confidence:** HIGH (disassembly-cited, `rule 16`).

### 6. `src/TIMER.C:100-106`, `timer_wait_ticks` (F_6C26)

```c
void timer_wait_ticks(n)
int n;
{
    unsigned long t;
    t = n + timer_ticks;
    while (timer_ticks < t) ;
}
```

Comment: "The comparison is `jb` twice, so both sides are UNSIGNED long
(tc20-codegen rule 5); the parameter is widened with `cwd`, so IT is a
signed int." This is also required-verification item (c).1 in
`docs/current/portability-boundaries.md`.

**Portable spelling:**
```c
void timer_wait_ticks(dos_int n)
{
    dos_ulong t;
    t = (dos_ulong)(dos_long)n + timer_ticks;   /* cwd: sign-extend n first */
    while (timer_ticks < t)
        ;
}
```
`(dos_long)n` reproduces the `cwd` sign-extension (matters if `n` is ever
negative — no caller currently passes a negative tick count, but the
disassembly proves the compiler emitted the sign-extending path, so the
port should too); the subsequent `(dos_ulong)` reinterprets the bit pattern,
matching "both sides UNSIGNED long" for the comparison. `timer_ticks` and
`t` as `dos_ulong` make `timer_ticks < t` a native unsigned compare in C —
no helper needed once the types are right.

**Confidence:** HIGH (disassembly-cited, required-verification item).

### 7. `src/TIMER.C:143-146`, `timer_deadline_reached` (F_6C87)

```c
int timer_deadline_reached()
{
    if (timer_ticks < gc0d0)
        ...
}
```

Comment: "`ja`/`jb`/`jae` on the two halves makes both sides UNSIGNED long
(rule 5), and one convention name reaches each high word as `_sym+2`
(rule 18)."

**Portable spelling:** both `timer_ticks` and `gc0d0` declared `dos_ulong`;
`if (timer_ticks < gc0d0)` is then a native unsigned 32-bit compare with no
cast needed. (Also documents why `timer_ticks`/`gc0d0` must never become
`dos_long` — the wraparound-correctness note in
`docs/current/portability-boundaries.md` §8's risk paragraph depends on the
comparison staying unsigned.)

**Confidence:** HIGH (disassembly-cited).

### 8. `src/INTRO.C:161`, `intro_animate_step` (F_5382)

```c
if (*when > timer_ticks) return;
```

Comment (`src/INTRO.C:110-113`, disassembly at `538A`): "`*when > timer_ticks`
-- the 32-bit UNSIGNED compare (jc / ja / jna), so both sides are unsigned
long." `when` is `unsigned long far *`.

**Portable spelling:** `dos_ulong far *when` (or, once far pointers are
flattened per §7, a plain `dos_ulong *when`); `if (*when > timer_ticks)
return;` is then already correct as a native unsigned compare.

**Confidence:** HIGH (disassembly-cited).

### 9. `src/INTRO.C:182`, `intro_animate_step` (F_5382)

```c
*when = n * step + timer_ticks;
```

Comment (`src/INTRO.C:128-131`, disassembly `548B 8B46F6 F76608 99`): "n *
step then CWD: the `mul` is a 16x16 int multiply whose high word is thrown
away by the sign-extension that widens the int result to the long that is
added to timer_ticks." `n` and `step` are both `int`.

**Portable spelling:**
```c
dos_int n, step;              /* both already int locals/params */
...
dos_ulong when_next = (dos_ulong)(dos_long)dos_mul16(n, step) + timer_ticks;
```
The product `n * step` must be computed and truncated to 16 bits **first**
(`dos_mul16`), matching the "16x16 int multiply, high word thrown away"
claim, and only then sign-extended (`cwd`) to `dos_long` before the addition
against the unsigned `timer_ticks`. Computing the product directly in a
32-bit-or-wider type would silently keep bits the original code discarded.

**Confidence:** HIGH (disassembly-cited; same function as fact 8, so both
sides of this deadline-arm/deadline-check pair are now pinned down).

### 10. `src/SNDREQ.C:26-30`, `sound_request_count_dec` (F_D5A6)

```c
extern int sound_request_count;   /* DS:237C */

void sound_request_count_dec()
{
    if (--sound_request_count < 0)
        sound_request_count = 0;
}
```

Comment: "The `jnl` on the DECREMENT's own flags is what makes this ONE
statement ... `jnl` is signed, so the counter is a signed int (rule 5)."

**Portable spelling:** `dos_int sound_request_count;` — a plain `dos_int`
decrement-and-compare-against-zero needs no helper (signed comparison
against the literal `0` behaves identically at 16 or 32 bits); the fact to
preserve is purely the **type** (must stay `dos_int`, i.e. signed, not
`dos_uint`) — this is directly load-bearing for §4's timer/sound gate
(`!sound_request_count && ...`), since a `dos_uint` version of this counter
would never legitimately go negative-then-clamp the way the signed one does
mid-decrement (it would instead wrap to 0xFFFF).

**Confidence:** HIGH (disassembly-cited).

### 11. `src/OPLINIT.C:71-79`, `music_set_tempo` (F_DA20)

```c
void music_set_tempo(v)
register unsigned v;
{
    if (v > 12) v = 12;
    if (v < 1) v = 1;
    ...
}
```

Comment (`src/OPLINIT.C:66-69`): "clamp the tempo to 1..12, latch it, and
recompute the derived word. `jbe`/`jae` make the clamp UNSIGNED and the
`mul` is an unsigned int multiply (tc20-codegen rule 5)."

**Portable spelling:** `dos_uint v` (already declared `unsigned`, keep it
`dos_uint` not `dos_int`); the clamp comparisons (`v > 12`, `v < 1`) are
correct unsigned compares in C regardless of promotion. The **derived-word
multiply** that follows the clamp (not shown in the excerpt above — the
recomputed global read) should use an explicit unsigned truncation if its
result is stored back into a `dos_uint`:
```c
dos_uint derived = dos_u16((uint32_t)v * SOME_CONST);
```
because a `dos_uint` operand promotes to plain (signed) `int` under modern
C's integer-promotion rules (16-bit unsigned values all fit in a 32-bit
`int`), which is a real behavioral difference from the 8086, where
`unsigned int` was already the native 16-bit type and needed no promotion at
all. This promotion-to-signed-int gap does not change the *result* for this
specific multiply (operands stay small, no overflow), but the spelling
should not rely on that invisibly — hence `dos_u16(...)`.

**Confidence:** HIGH for the clamp; MEDIUM for the promotion-gap note on the
multiply (informational, not disassembly-cited beyond "unsigned int
multiply").

### 12. `src/OPLREG.C:161`, `voice_write_level` (F_E1F2)

```c
v = 0x3f - v / 0xfe;
```

Comment (`src/OPLREG.C:133-140`, disassembly `E22B 33D2 F7F3`): "`xor dx,dx;
div bx` is the UNSIGNED divide, so the running value is an unsigned int; a
signed int would have been `cwd; idiv` and a long a runtime helper. The same
reading covers `mul si` at E21B."

**Portable spelling:** `dos_uint v;` (not `dos_int`); `v / 0xfe` on a
`dos_uint` promotes to signed `int` under modern promotion rules (same gap
as fact 11) but again does not change the result here since `v` stays in
0..0x3F range — still, prefer `dos_u16(v / 0xfeu)` at any site feeding a
value back into a `dos_uint` storage location, to make the truncation
explicit rather than relying on assignment truncation alone.

**Confidence:** HIGH (disassembly-cited).

### 13. `src/PLAYERSL.C:258`, `ui_gfx_alloc` (F_D344)

```c
ui_gfx_blob = farmalloc(0xfa80L);
```

Comment (`src/PLAYERSL.C:247-251`): "carve three far blocks out of one
64,128-byte allocation ... The size is an unsigned long CONSTANT (`xor
dx,dx`, not `cwd` -- the negative of rule 16's signed-int case)."

**Portable spelling:** keep the literal's width explicit when this call
site becomes an ordinary allocation per §10:
```c
dos_ulong size = 0xfa80UL;   /* 64128 bytes, exact -- do not recompute */
```
This is the *negative* case of fact 5/9: because the constant is already a
`long` literal (the `L` suffix), the compiler emits a zero-extending
`xor dx,dx` rather than a sign-extending `cwd`. There is no arithmetic here
that a modern compiler could get wrong (it's a literal), but the exact value
0xfa80 (64128) must survive the farmalloc→malloc migration unchanged since
it is the source for the three-region carve documented in
`docs/current/portability-boundaries.md` §7.

**Confidence:** HIGH (disassembly-cited); LOW *risk* (informational — no
computed arithmetic to get wrong, just don't typo the constant).

### 14. `src/TICKDIV.C:1-6`, `tick_div8` (F_1EB4)

```c
extern int campaign_round_node_cursor;

int tick_div8(void)
{
    return campaign_round_node_cursor / 8;
}
```

Comment: "the tick divided by eight. Signed: cwd/idiv, not shr."

**Portable spelling:** `dos_int campaign_round_node_cursor;` and `dos_int
tick_div8(void) { return campaign_round_node_cursor / 8; }` — **no helper
needed**. This is a case where the historical fact ("must be signed
division, not an unsigned shift") is automatically satisfied by modern C: a
plain `/` on signed operands rounds toward zero in C99 and later (matching
8086 `idiv`), so the only requirement is that `campaign_round_node_cursor`
stay a **signed** `dos_int` (never get "optimized" to `dos_uint` or to a
`>> 3` shift, which would round differently for negative values).

**Confidence:** HIGH (disassembly-cited: "Signed: cwd/idiv, not shr" is an
explicit statement of the risk this entry documents).

### 15. `src/OPLVOICE.C:84-90`, `voice_level_table_reset` (F_DDC7)

```c
void voice_level_table_reset()
{
    register int i;
    for (i = 0; i < 18; i++)
        voice_level_table[i] = 0x7f;
}
```

Comment: "The counter is the one register variable (SI), the index is
signed (`jl`), and the pre-test loop opens with the jump to the test that
TC 2.0 emits for `for`."

**Portable spelling:** `dos_int i;` — no helper needed (loop bound 18 is
far below any 16/32-bit-int behavioral difference); included for
completeness since it is disassembly-cited, and to record that `i` must not
become `dos_uint` (irrelevant for this specific bound, but the fact as
stated is about the type, not the bound).

**Confidence:** HIGH (disassembly-cited); LOW risk.

---

## Part 2 — Review candidates (regex sweep, unverified)

Per the task: enumerate every `long` arithmetic site and every `>>`/`/`/`%`
on a possibly-negative int found by a regex pass, and mark them "review"
rather than claiming a fact. These were **not** individually read against
the disassembly the way Part 1 was — they are grep hits only. A blank-ish
entry (e.g. a `long`-typed `extern` declaration with no arithmetic) is
included because the task asked for "every long arithmetic," and telling
declarations apart from arithmetic expressions by regex alone is not
reliable; skim before acting on any row here.

### 2.1 `long`-typed declarations/arithmetic (34 grep hits)

| File:Line | Snippet |
|---|---|
| INTRO.C:19 | `extern unsigned long timer_ticks;` |
| INTRO.C:80 | `unsigned long when;` (local in `intro_play_script`, see fact 8/9's `intro_animate_step` for the pointee) |
| INTRO.C:144 | `unsigned long far *when;` (parameter, see facts 8-9) |
| LEVEL.C:219 | `extern unsigned long timer_ticks;` |
| LEVEL.C:236 | `unsigned long deadline;` -- review: same deadline-arm shape as TIMER.C/INTRO.C, not individually disassembly-checked here |
| LIB_RAND.C:2 | `static long state = 1L;` -- review: the RNG state word; see `rng` interface note in tu-inventory.md |
| MUSIC.C:28-30,46,92 | `long t; long u; long l;` (repeated, several functions) -- review |
| MUSIC.C:32-33 | `l = (long)(b * 100); t = ((long)(a * 6) + l) * 52088L;` -- review: cast-before-multiply shape resembles fact 5/9, not individually confirmed |
| MUSIC.C:82 | `extern long g3752;` (cached key) |
| MUSIC.C:96 | `l = (long)(val - 8192) * music_tempo_scaled;` -- review |
| MUSIC.C:102 | `d = (int)(l / 8192L);` -- review: truncating a `long` back to `int` division result, resembles fact 2's shape but not confirmed |
| MUSIC.C:105,111 | `gc5e4 = gca6d[i] = -(t / 25); ... = d / 25;` -- review: sign of `t`/`d` not established here |
| RESOURCE.C:46 | `extern long lseek(int,long,int);` (declaration only) |
| RESOURCE.C:54 | `extern long ga52[];` (declaration only) |
| RESOURCE.C:77 | `long stamp;` (local, `resource_file_open`) |
| RESOURCE.C:130 | `unsigned long first,second;` -- review (in a function not covered by facts 1-4) |
| RESOURCE.C:136 | `lseek(slot_file_handle,(unsigned long)(slot*4),0);` -- review: same shape as fact 3 but a *different* call site (slot-record path, not the resource-record path); not individually disassembly-checked |
| RESOURCE.C:156 | `_ES=(unsigned)((unsigned long)(char far *)buffer>>16);` -- far-pointer segment extraction, §7 territory, not an int-width fact |
| RESOURCE.C:219-220 | `long o1; long o2;` (facts 2-3's locals, declarations) |
| RESOURCE.C:232 | `lseek(slot_file_handle, (long)(p * 4), 0);` (= fact 3) |
| SLOTMENU.C:333 | `extern long str_concat_far_list(...);` (declaration only) |
| SLOTROW.C:4 | `ultoa((long)p->value,buf,10);` -- review: `p->value`'s declared width not checked here |
| STARTUP.C:20 | `segment=(unsigned)((unsigned long)text>>16);` -- far-pointer segment extraction, §7 territory |
| STARTUP.C:217,225 | `extern long farcoreleft(); long n;` -- the memory-gate thresholds, already covered as exact required-verification facts in `docs/current/portability-boundaries.md` §(c).6, not re-litigated here |
| TIMER.C:82,97-98,109 | `timer_ticks`/`gc0d0` declarations (facts 6-7's operands) |
| VIDEO.C:125 | `g40ca = farmalloc((long) w * 488 + 16);` (= fact 5) |

### 2.2 `>>` (shift) sites (17 grep hits)

| File:Line | Snippet | Note |
|---|---|---|
| BOARD.C:279,344 | `index=(x>>3)+((y>>3)-2)*38-1;` | review -- `x`/`y` signedness not established here; board-coordinate math, worth checking against fact 1's "keep unsigned" pattern before porting |
| BOARD.C:469 | `c = (c & 0x70) >> 4;` | review -- `c & 0x70` is non-negative regardless of `c`'s signedness (mask clears the sign bit for a `char`), likely low-risk but not confirmed |
| GAME.C:95 | `(cursor_x >> 1) + 1` | review |
| GAME.C:507 | `cursor_x = ((cursor_x + 3) >> 2) << 2;` | review -- shift-round-shift idiom, sensitive if `cursor_x` can be negative |
| GAME.C:520 | `campaign_round_node_cursor >> 1` | review -- compare against fact 14's sibling `tick_div8` (`/8`, confirmed signed); this `>>1` sibling not individually checked |
| HITTEST.C:99,130 | `((y>>3)-2)*0x26+(x>>3)-1`; `x>>1` | review |
| HITTEST.C:231 | `c=gc0c4[(y<<4)-y+(x>>1)];if(x&1)c&=15;else c=(c>>4)&15;` | review -- nibble unpack, `c` likely `char`/`unsigned char`, not confirmed |
| LIB_RAND.C:12 | `return (int)(state >> 16) & 0x7fff;` | review -- `state` is `long` (signed, fact 2.1); `state >> 16` on a signed negative `state` is *implementation-defined* in C89/historically arithmetic-shift on Turbo C, and is well-defined arithmetic-shift-equivalent in practice on all modern targets too, but this is exactly the kind of RNG bit-exactness site that needs a differential test against the historical PRNG, not a guess -- see `rng` note in tu-inventory.md |
| RESOURCE.C:133,156,225,284 | `slot>>12`; segment extract; `p>>12` (=fact 1); `(*t>>1)-1` | RESOURCE.C:225 = fact 1 (confirmed); others review |
| STARTUP.C:20,66 | segment extract; `((_AX & 0xc0) >> 6) + 1` | review (STARTUP.C:66 masks first, likely non-negative, but not confirmed) |
| VIDEO.C:99 | `s = (o >> 4) + s;` (`video_normalize_far_ptr`, `o` is `unsigned`) | review -- likely safe (same shape as fact 1) but not individually disassembly-checked |

### 2.3 `%` (modulo) sites (10 grep hits)

| File:Line | Snippet | Note |
|---|---|---|
| GAME.C:192 | `if (cursor_x % 8 == 0)` | review -- `%` on a possibly-negative `cursor_x` rounds toward zero in C (matching 8086 `idiv` remainder sign), but `cursor_x`'s value range at this call site isn't re-derived here |
| MUSIC.C:106,112 | `r = (t - 24) % 25; r = d % 25;` | review |
| PUZZLE.C:91,102 | `rand() % (0xc - slot); rand() % 4;` | review -- `rand()`'s historical Turbo C algorithm vs. a modern `rand()` is the real risk here, not the `%`; see `rng` note |
| PUZZLE.C:220 | `(r.a%3)*32` | review |
| ROUNDEND.C:87,106,110 | `rand()%8`, `rand()%3`, `rand()%2` | review -- same RNG-algorithm caveat as PUZZLE.C |
| SLOTMENU.C:346 | `selected = (selected + count) % count;` | review -- `selected`/`count` both look non-negative by construction (menu index/count), low risk, not confirmed |

### 2.4 `/` (division) sites (33 grep hits, non-comment)

Concentrated in `BOARD.C` (board-coordinate-to-cell math, 5 sites: lines
551, 648, 742, 788, 801), `DIALOG.C` (dialog centering math, 7 sites: lines
196-275), `GAME.C` (3 sites: 241, 457), `MUSIC.C` (tuning-table math, 5
sites: 34, 37, 102, 105, 111), `LEVEL.C` (2 sites: 179, 299), `HUD.C:149`,
`NODEIDX.C:2`, `OPLREG.C:161` (= fact 12), `PUZZLE.C:220`, `TICKDIV.C:6`
(= fact 14). All of the board/dialog/coordinate-math sites divide small
positive-looking pixel/cell quantities by small constants (2, 4, 8) and are
**plausibly** safe under C's round-toward-zero signed division matching
8086 `idiv` the way fact 14 is -- but none of these are individually
disassembly-confirmed the way fact 14 is, so treat each as review, not fact,
until a Wave 2 agent checks the operand's declared range at that call site
(some, like `DIALOG.C`'s `(0x140 - dialog_box_w) / 2`, look definitely
non-negative by construction; others, like board-coordinate deltas, are
worth a second look for camera-scroll/negative-offset cases).

### 2.5 Mixed-type comparisons

A reliable regex-only detector for "comparison between differently-typed
operands" does not exist without a symbol table (the same identifier's
declared type has to be looked up at each use). Two examples surfaced
incidentally while reading Part 1's context and are flagged here rather
than guessed at scale:

- `src/HITTEST.C:130`: `if ((obj = rect_table_hit_id(x >> 1, y, 1, 1)) >= 8
  && obj <= 0x1f)` -- `rect_table_hit_id`'s return type vs. `obj`'s
  declared type should be cross-checked (review).
- `src/RESOURCE.C:281` (`resource_load_record_alloc`, F_684A): `n =
  resource_load_record(a);` then `*pp = farmalloc((long)n);` -- the header
  comment for this function already says "The far pointer OUT parameter is
  compared against zero the way TC 2.0 compares a far pointer: OR the two
  halves," which is a §7 (far-pointer) fact already covered elsewhere, not
  an int-width one; listed here only to note it was checked and excluded.

For anything beyond these two, a Wave 2 agent should cross-reference each
comparison's operands against their `extern`/local declarations (or run a
differential test) rather than trust a regex pass -- this is exactly the
kind of claim `docs/current/portability-boundaries.md` insists be cited to
a concrete evidence source, and grep alone cannot provide that citation.

---

## Summary

- **15 confirmed width/signedness facts** (Part 1), each disassembly-cited
  in the historical source's own comments, spanning RESOURCE.C (4),
  VIDEO.C (1), TIMER.C (2), INTRO.C (2), SNDREQ.C (1), OPLINIT.C (1),
  OPLREG.C (1), PLAYERSL.C (1), TICKDIV.C (1), OPLVOICE.C (1).
- **~94 review candidates** from the regex sweep (34 `long` sites, 17 `>>`
  sites, 10 `%` sites, 33 `/` sites; some overlap with the confirmed facts
  and are cross-referenced above), plus 2 flagged mixed-type-comparison
  examples. None of these are claims — they are the starting list for Wave
  2's per-site verification pass.
- The two RNG-touching clusters (`LIB_RAND.C`, and `rand()` call sites in
  PUZZLE.C/ROUNDEND.C) carry a distinct risk this document flags but does
  not resolve: Turbo C's `rand()`/`srand()` algorithm is not the same as
  any modern libc's, so bit-exact determinism (if required) needs a
  reimplementation of the historical PRNG, not just a `dos_uint`/`dos_int`
  typing fix.
