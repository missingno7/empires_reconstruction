/* src/TIMER.C: Timer waits and deadlines.
   One translation unit; the sections below were the separate member
   sources of grouped module C_6C26_6C87 and keep their original ids. */

extern unsigned long gb76;              /* DS:0B76, high word at DS:0B78 */
extern unsigned long gc0d0;             /* DS:C0D0, high word at DS:C0D2 */

/* ---- F_6C26 (original code at 0x6C26) ---- */
/* F_6C26 -- spin until the 32-bit counter at DS:0B76 has advanced by n.
   The comparison is `jb` twice, so both sides are UNSIGNED long
   (tc20-codegen rule 5); the parameter is widened with `cwd`, so IT is a
   signed int.  The loop body is empty, which is why the jump to the test is
   the zero-displacement EB00. */
void timer_wait_ticks(n)
int n;
{
    unsigned long t;

    t = n + gb76;
    while (gb76 < t) ;
}


/* ---- F_6C57 (original code at 0x6C57) ---- */
/* F_6C57 -- arm a tick deadline.  gb76 is the free-running tick (unsigned
   long), gc0d0 the deadline.  Plain C. */
void timer_deadline_arm(int n)
{
    gc0d0 = n + gb76;
}


/* ---- F_6C6F (original code at 0x6C6F) ---- */
/* F_6C6F -- spin until the deadline armed by timer_deadline_arm has passed.  The EB 00 at
   the entry is the while-loop's jump-to-test with an empty body. */
void timer_deadline_wait(void)
{
    while (gb76 < gc0d0)
        ;
}


/* ---- F_6C87 (original code at 0x6C87) ---- */
/* F_6C87 -- has the 32-bit counter at DS:0B76 reached the deadline at
   DS:C0D0?  `ja`/`jb`/`jae` on the two halves makes both sides UNSIGNED long
   (rule 5), and one convention name reaches each high word as `_sym+2`
   (rule 18).  Both arms END in a jump to the epilogue -- the second one is
   the zero-displacement EB00 -- so both are `return` statements of an
   `if`; a single `return (a < b ? 0 : 1)` lets the second arm fall through
   and loses the EB00. */
int timer_deadline_reached()
{
    if (gb76 < gc0d0)
        return (0);
    return (1);
}
