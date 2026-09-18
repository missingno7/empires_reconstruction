/* F_5593 -- wait for a key while the deadline at F_6C87 has not passed.
   The first test is `cmp ax,0Dh`, three bytes against the CALL RESULT, not
   the five-byte `cmp [_g8fc],0Dh` two statements would give: the assignment
   is the tested expression (rule 4 in its value-of-assignment form), and the
   SECOND test reloads from memory.  The dead `jmp` after `return 0x0D` is
   the `if` statement's own end jump, which TC 2.0 emits only when the `if`
   HAS an `else`. */
extern int f6c87();
extern int f6b4a();
extern int f6b1a();
extern int g8fc;                        /* DS:08FC */

int f5593()
{
    while (f6c87() == 0) {
        if (f6b4a()) {
            if ((g8fc = f6b1a()) == 0x0d)
                return (0x0d);
            else if (g8fc == 0x1b)
                return (g8fc);
        }
    }
    return (-1);
}
