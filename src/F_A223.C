/* F_A223 -- the index of the first free slot of the 27-byte-stride table at
   DS:C470, or 10 if there is none.  The `mul` by 27 then `mov bx,ax / add
   bx,0C470h / push ds / pop es` is rule 2's struct-member-through-an-array
   shape (offset FIRST).  Both returns end in a jump to the epilogue and the
   second is the zero-displacement EB00 (rule 24). */
struct S { char f; char pad[26]; };
extern struct S gc470[];                /* DS:C470, stride 27 */

int fa223()
{
    register int i;

    for (i = 0; i < 10; i++)
        if (gc470[i].f == 0)
            return (i);
    return (10);
}
