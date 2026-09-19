/* F_49F0 -- "THE GAME": the top-level sequencer that calls the intro driver
   f56c6 (157C6) and then runs the per-level record sequence.  Entry 14AF0,
   163 bytes, 16 activations per cold boot.

   Disassembly (assets/AEPROG.EXE, ndisasm -b16 -o 0x49f0):
       55 8BEC 83EC02 56 57   push bp; mov bp,sp; sub sp,2; push si; push di
       E8 CB0C                call f56c6
       3D FFFF                cmp ax,-1        (accumulator short form)
       75 03 / E9 8A00        jnz +3 / jmp 4A8D  -- "if (...) return;"
       E8 46D8                call f224c        (0 args, result unused)
       1E B8FE8B 50           push ds; mov ax,0x8BFE; push ax  -- (char far *)g8bfe
       E8 45AF                call setjmp
       59 59                  pop cx; pop cx    (cdecl cleanup, 4 bytes = 2 pops)
       8BF8                   mov di,ax         -- d = setjmp(g8bfe)
       BEFFFF                 mov si,0xFFFF     -- s = -1
       E8 E327 / E8 988B / E8 3B48   f71fb(); fd5b3(); f9259();
       83FF03 7502 EB68       cmp di,3; jnz +2; jmp 4A8D   -- if (d == 3) return;
       83FF02 7D4D            cmp di,2; jnl 4A77           -- if (d < 2) { ... }
       E8 3961 3DFFFF 7502 EB59   if (fab66() == -1) return;
       A1ED13 BA1B00 F7E2     mov ax,[13ED]; mov dx,0x1B; mul dx
       8BD8 81C370C4          mov bx,ax; add bx,0xC470
       1E 07                  push ds; pop es          -- far-pointer form:
                                                          (char far *)gc470 + i*0x1B
       268A4715 98 50         mov al,[es:bx+0x15]; cbw; push ax
       E8 F628 59             call f7343; pop cx
       ... the same address recomputed, field +0x0C this time ...
       268A470C 98 8946FE     mov al,[es:bx+0xC]; cbw; mov [bp-2],ax
       0BC0 740D              or ax,ax; jz 4A77        -- if (n = ...) {
       FF4EFE                 dec word [bp-2]          --   n--;
       8B76FE 8BC6 50         mov si,[bp-2]; mov ax,si; push ax
                                                       --   f4943(s = n)
                                  (the assignment's VALUE is what is pushed:
                                   "s = n" loads si straight from memory, then
                                   the expression result goes through ax.  Two
                                   separate statements would have pushed si --
                                   see the loop body below, which does.)
       E8 CDFE 59             call f4943; pop cx
       EB0C                   jmp short 4A85           -- while (s != 4) {
       56 E8 EF87 59          push si; call fd26c; pop cx
       8BF0                   mov si,ax                --   s = fd26c(s);
       56 E8 BFFE 59          push si; call f4943; pop cx   -- f4943(s);
       83FE04 75EF            cmp si,4; jnz 4A79       -- }
       E8 9862                call fad25
       5F 5E 8BE5 5D C3       pop di; pop si; mov sp,bp; pop bp; ret

   The 27-byte stride at 0xC470 and the byte fields at +0x15 and +0x0C are
   the per-level record indexed by g13ed; the multiply is `mul` (unsigned),
   which is what TC 2.0 emits for `int * const` -- the same shape the already
   matched F_5AC3 carries at 5B35 (`mov dx,0x18; mul dx; mov bx,ax;
   add bx,0x900; push ds; pop es`). */

extern int  f56c6(), setjmp(), fab66(), fd26c();
extern void f224c(), f71fb(), fd5b3(), f9259(), f7343(), f4943(), fad25();

struct L {                              /* the per-level record, 0x1B bytes */
    char p0[0xc];
    char fc;
    char p1[8];
    char f15;
    char p2[5];
};

extern char g8bfe[];                    /* DS:8BFE */
extern struct L gc470[];                /* DS:C470, stride 0x1B */
extern int  g13ed;                      /* DS:13ED */

f49f0()
{
    int n;                              /* bp-2 */
    register int s, d;                  /* si, di */

    if (f56c6() == -1) return;
    f224c();
    d = setjmp(g8bfe);
    s = -1;
    f71fb();
    fd5b3();
    f9259();
    if (d == 3) return;
    if (d < 2) {
        if (fab66() == -1) return;
        f7343(gc470[g13ed].f15);
        if (n = gc470[g13ed].fc) {
            n--;
            f4943(s = n);
        }
    }
    while (s != 4) {
        s = fd26c(s);
        f4943(s);
    }
    fad25();
}
