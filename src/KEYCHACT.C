/* F_6B74 -- read back the flag at DS:0B72.  The trailing EB00 is the
   `return`'s jump to the epilogue at zero displacement (rule 7's shape). */
extern int keyboard_state;                        /* DS:0B72 */

int keyboard_chain_active()
{
    return (keyboard_state);
}
