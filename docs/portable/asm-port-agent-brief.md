# Brief for ASM-module porting agents (Wave 2)

Repository D:\Prog\empires_reconstruction, branch portable-sdl3, Windows.  Read first:
docs/portable/architecture.md, docs/portable/asm-module-inventory.md (your modules'
sections), docs/portable/tu-port-agent-brief.md (the compile command, the rules about
generated objects and prototypes), portable/include/game.h and everything it includes
(game_funcs.h has the C-callable prototypes of your routines; game_data.h/game_state.h
define every DGROUP object by its historical name; gfx.h has the primitives).

You translate the MEANING of each asm/*.ASM routine into ordinary typed C in
`portable/game/asm_<module>.c` (one file per module, lowercase, e.g. asm_rectq.c).
Not instruction-for-instruction: no register variables, no segment arithmetic.  But
every observable effect must be identical: the bytes written to DGROUP tables and to
the framebuffer, the order of gfx_* calls and their arguments, the return values,
16-bit wraparound where the ASM relied on it, signed vs unsigned compares.  Model
16-bit quantities with dos_int/dos_uint; far pointers become plain pointers; tables
indexed through DS become the generated objects; ES:DI/ES:SI register-ABI routines
become functions taking explicit pointer/index parameters — if a C caller (grep
src/*.C) calls a register-ABI routine through a wrapper or inline asm, port that
calling shape too and report it.

Read the ASM file completely; the header comments and per-label comments are the
specification.  Where a table's element layout is described (3-byte records, 5-byte
records, packed x/y words...), define a struct or explicit accessors in a small
header `portable/include/asm_<module>.h` (declare your functions there too when
they are not already in game_funcs.h; do not duplicate prototypes that are).
Where the ASM writes to a DGROUP address that has no generated symbol, report it
(state-map.md lists every symbol) -- do not invent storage.

Framebuffer access: only through gfx_* primitives or, when the ASM addresses rows
directly via _g3924, through `g3924[y] + offset` with the stride the ASM uses
(0xA0 planar, 0x140 VGA); check whether the routine is mode-generic (its C caller
passes the stride/divisor, e.g. ANIMSTEP.C) or hard-codes a mode, and report.

Compile-check each file standalone with the cl command from the TU brief; zero
warnings.  Append your files to portable/game/CMakeLists.txt as ONE new
target_sources line at the end (re-read before editing).  Write unit tests
`portable/tests/test_asm_<module>.c` + `portable/tests/asm_<module>.cmake` exercising
the routine on synthetic tables (queue append/consume, collision spans, draw-queue
execution against a framebuffer initialised with gfx_framebuffer_init in mode 5,
etc.) -- tests may link only what empires_core provides; if your routine calls
not-yet-ported C functions, declare and stub them in the TEST file only.
Never commit.  Report per routine: the C signature, the tables/structs defined, any
semantic ambiguity (quote the instructions), test results.
