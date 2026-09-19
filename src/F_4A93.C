/* F_4A93 -- main().  Entry 14B93, 20 bytes.  A straight-line sequence of
   five calls: f520a() gates whether the rest ever runs (init failure ->
   immediate return with no side effects on the four other calls), then
   f490d (boot init), f49f0 (the game), f49e3 (shutdown), f034f (exit).
   Disassembly (assets/AEPROG.EXE, physical 14B93):
       E8 74 07        call f520a          (1530A)
       0B C0            or  ax,ax
       74 0C            jz  14BA6           (the ret at the end)
       E8 70 FE        call f490d          (14A0D)
       E8 50 FF        call f49f0          (14AF0)
       E8 40 FF        call f49e3          (14AE3)
       E8 A9 B8        call f034f          (1044F)
       C3               ret
   No MOV sets ax before the single ret, so no "return <value>" appears in
   the source.  An "if (x == 0) return;" early-return shape does NOT
   reproduce these bytes: TC 2.0's unoptimized codegen for that shape is the
   "jnz +2 / jmp +N" idiom (two branches, 4 bytes) rather than a single
   inverted jcc, which overflows this 20-byte extent by 2 bytes (confirmed
   by a first compile attempt, T00.OBJ, kept as the negative test). The
   ORIGINAL shape is a single guarded block with no early exit: */
extern int  f520a();
extern void f490d();
extern void f49f0();
extern void f49e3();
extern void f034f();

main()
{
    if (f520a()) {
        f490d();
        f49f0();
        f49e3();
        f034f();
    }
}
