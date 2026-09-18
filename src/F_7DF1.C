/* F_7DF1 -- hand one DGROUP buffer to f7932.  Compact model: the array name
   becomes a far pointer, push ds / mov ax,OFFSET / push ax. */
extern char g0d36[];
extern int f7932();

void f7df1(void)
{
    f7932(g0d36);
}
