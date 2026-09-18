/* F_734E -- step the signed char gb82 by n, clamp it at 4, notify f738A,
   and return it.  The reload `mov al,[gb82]` before the clamp test is the
   VALUE of the assignment expression: `add [gb82],al` leaves the result in
   memory, not in a register, so writing the += inside the condition forces
   TC to read it back.  A separate `gb82 += n; if (gb82 > 4)` compares memory
   in place and loses the reload. */
extern char gb82;
extern void f738a();
extern void fd4b3();

int f734e(n)
int n;
{
    if (n != 0) {
        if (gb82 == 2 && n < 0)
            fd4b3(4);
        if ((gb82 += n) > 4)
            gb82 = 4;
        f738a();
    }
    return gb82;
}
