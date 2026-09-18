/* F_55C7 -- build the 22-entry far pointer table at DS:BFEE.  The first nine
   entries index the record loaded at DS:BFDE through its own leading word
   table; the next nine and the last four are fixed 674-byte strides into the
   block at DS:99D2.  The `push cx / push bx ... pop ax / pop dx` around each
   index computation is the ORIGINAL far-pointer shape of rule 3.  The two
   later loops increment the TABLE index before the loop counter, so the
   counter is the `for` variable and the table index rides in the comma. */
extern void f684a();
extern unsigned far *gbfde;             /* DS:BFDE offset, DS:BFE0 segment */
extern char far *gbfee[];               /* DS:BFEE */
extern char far *g99d2;                 /* DS:99D2 offset, DS:99D4 segment */

void f55c7()
{
    register int i;
    register int j;

    f684a(0x34, &gbfde);
    for (i = 0; i < 9; i++)
        gbfee[i] = (char far *) gbfde + gbfde[i] + 2;
    i = 9;
    for (j = 0; j < 9; i++, j++)
        gbfee[i] = g99d2 + j * 674;
    i = 0x12;
    for (j = 0x0c; j < 0x10; i++, j++)
        gbfee[i] = g99d2 + j * 674;
}
