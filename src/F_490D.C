/* F_490D -- boot init.  Entry 14A0D, 54 bytes.  A straight-line sequence
   of twelve calls, no branches.  Disassembly (assets/AEPROG.EXE):
       33C0 50 50 33C0 50   xor ax,ax; push ax; push ax; xor ax,ax; push ax
       E8 F8B0              call biostime          (1FB0F)   -- biostime(0, 0L):
                                                  the long/far arg is rightmost
                                                  (pushed first, one xor then
                                                  two pushes of the same zero
                                                  for its two halves), the int
                                                  arg is leftmost (pushed
                                                  second, its own fresh xor)
       83C406               add sp,6            (cdecl cleanup, 3 words)
       50                   push ax             (biostime's own return value)
       E8 E0AF              call srand          (1F9FE)   -- 1 arg
       59                   pop cx              (cdecl cleanup, 1 word --
                                                  TC 2.0 uses "pop reg" rather
                                                  than "add sp,2" to discard
                                                  exactly one pushed word)
       E8 5822              call f6b7a          (16C7A)   -- 0 args
       E8 3819              call f625d          (1635D)
       E8 1C8A              call fd344          (1D444)
       E8 93FF              call f48be          (149BE)
       E8 53B9              call f0281          (10381)
       E8 248C              call fd555          (1D655)
       E8 2A20              call f695e          (16A5E)
       FB                   sti                 (enable())
       E8 A3D8              call f21db          (122DB)   -- 0 args
       E8 6ED8              call f21a9          (122A9)
       33C0 50              xor ax,ax; push ax
       E8 6523              call f6ca6          (16DA6)   -- 1 arg, 0
       59                   pop cx              (cdecl cleanup, 1 word)
       C3                   ret
   The two 0-arg calls immediately after srand's cleanup (f6b7a, f625d, ...)
   have no push before them and no pop/add after: void, no return value
   used. */
extern int  biostime();
extern int  srand();
extern void f6b7a();
extern void f625d();
extern void fd344();
extern void f48be();
extern void f0281();
extern void fd555();
extern void f695e();
extern void f21db();
extern void f21a9();
extern void f6ca6();

f490d()
{
    srand(biostime(0, 0L));
    f6b7a();
    f625d();
    fd344();
    f48be();
    f0281();
    fd555();
    f695e();
    asm sti;
    f21db();
    f21a9();
    f6ca6(0);
}
