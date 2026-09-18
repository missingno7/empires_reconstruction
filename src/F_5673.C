/* F_5673 -- blit the far bitmap at DS:BFEE to (0,0x20) with mode 0.  The far
   pointer GLOBAL pushes its segment word then its offset word (rule 5). */
extern void f03cc();
extern char far *gbfee;                 /* DS:BFEE offset, DS:BFF0 segment */

void f5673()
{
    f03cc(0, 0x20, gbfee, 0);
}
