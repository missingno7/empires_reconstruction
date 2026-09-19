/* F_D5BA -- load one of two adjacent records into the shared staging buffer,
   but only when the cached index at DS:237E says it is not already there. */
extern void f68aa();
extern char far *gc5da;                 /* DS:C5DA offset, DS:C5DC segment */
/* The compiler emits this two-byte initializer in this module's _DATA. */
int g237e = -1;                          /* DS:237E, the cached index */
extern int g1778;                       /* DS:1778, the mode */

void fd5ba(v)
register int v;
{
    if (g237e != v) {
        if (g1778 == 0) f68aa(v, gc5da);
        else f68aa(v + 1, gc5da);
        g237e = v;
    }
}
