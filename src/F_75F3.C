/* The local array initializer emits the original SCOPY argument order and
   the module-owned 15-byte _DATA contribution. */

extern int f020f(), f01ce(), f039f(), f6ca6(), f6cea(), f6d3c();

/* Shared UI state immediately precedes the caption initializers.
   Consumers establish widths: F_7313 int, F_734E char, F_703E/F_7162 ints.
   The final zero byte is part of gb85, not alignment padding. */
int gb80 = 4;
char gb82 = 4;
int gb83 = 0;
int gb85 = 0;
extern char gbfcd;                      /* DS:BFCD */

f75f3()
{
    char cap[15] = "\027\030 to Continue";                     /* bp-10 */
    register int s, d;                  /* si, di */

    gb83 = 2;
    s = f020f();
    d = f6cea();
    if (gbfcd == 2)
        f01ce(5);
    else
        f01ce(0xf);
    f6ca6(0);
    f6d3c(0x18, 0xbc, &cap);
    f039f(0x18, 0xbc, 0x94, 0xa);
    f01ce(s);
    f6ca6(d);
}
