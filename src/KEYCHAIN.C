/* src/KEYCHAIN.C: Keyboard chaining flags.
   One translation unit; the sections below were the separate member
   sources of grouped module C_6990_6997 and keep their original ids. */

extern int keyboard_state;                        /* DS:0B72 */

/* ---- F_6990 (original code at 0x6990) ---- */
/* F_6990 -- set the flag at DS:0B72.  No frame (rule 11). */
void keyboard_chain_enable()
{
    keyboard_state = 1;
}


/* ---- F_6997 (original code at 0x6997) ---- */
void keyboard_chain_disable(void)
{
    keyboard_state = 0;
}
