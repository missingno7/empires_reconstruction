# Brief for TU-porting agents (read fully before touching a file)

Repository D:\Prog\empires_reconstruction, branch portable-sdl3, Windows.
Read, in this order: docs/portable/architecture.md, docs/portable/tu-porting-rules.md,
docs/portable/int-semantics-inventory.md (your files' entries), docs/portable/state-map.md
(the "Porting notes", "Interior aliases", "Storage aliases" sections),
portable/include/game.h and the headers it includes (game_funcs.h has every prototype;
game_data.h/game_state.h have every DGROUP object; game_structs.h the structs;
gfx.h/resource.h/timer.h/input.h/sound.h/cclib.h/dosio.h the services).

Historical tree (src/, asm/, include/) is READ-ONLY reference.  Never commit.
Write only `portable/game/<name>.c` for the files assigned to you, and append
`target_sources(empires_core PRIVATE <name>.c ...)` as ONE new line at the end of
portable/game/CMakeLists.txt (re-read it right before editing; other agents append too).

Compile-check every file you write with the VS 18 toolchain, standalone:
  cmd /c "call \"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat\" >nul && cl /nologo /c /W4 /std:c17 /wd4244 /wd4245 /Iportable\include /Iportable\generated /Fo:build\<name>.obj portable\game\<name>.c"
(a .bat in build/ is the easiest way to run that).  Zero warnings.  Do not try to link the game.

Rules of thumb beyond tu-porting-rules.md:
- Types: every historical int/unsigned/char/long becomes dos_int/dos_uint/dos_char/dos_long.
  Locals too.  `register` dropped.  Pointer params: `char far *p` -> `dos_char *p`.
- If a generated object's type differs from the historical extern you are replacing
  (e.g. generated `dos_char x[3][16]` vs local `extern char x[][16]`), use the generated
  one and adapt indexing without changing semantics; if the generated type is WRONG for
  the code (indexing arithmetic would differ), do not hack around it -- write it in your
  report as "generator type issue: symbol, historical type, generated type".
- If game_funcs.h's prototype disagrees with how your file calls a function, follow the
  DEFINITION (the callee's file) and report the discrepancy.
- Function pointers / callback tables: use the typedefs in game_structs.h; if none fits,
  add a local typedef in your .c and report it.
- Inline asm fragments in your files: port them to C from the surrounding comments and
  the instructions themselves (they are small, documented bodies); mark with /* PORT: */.
  If the semantics are not fully determinable, STOP on that function, leave a
  `#error PORT: ...` placeholder, and report.
- Keep string literals and numeric constants exactly.  Keep the historical function
  names (except `main` in GAME.C, which becomes `game_main`).
- `setjmp(game_abort_jmpbuf)` / `longjmp(game_abort_jmpbuf, n)`: keep (jmp_buf type).
- `farmalloc(n)` -> `malloc((size_t)(n))`, `farfree` -> `free`, `movmem` etc. per cclib.h.
- Report per file: functions ported, PORT: replacements made, open questions.
