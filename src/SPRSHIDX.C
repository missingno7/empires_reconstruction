/* F_6CEA -- read back the word at DS:C0EA (the index F_6CA6 latched there).
   The trailing EB00 is the `return`'s jump to the epilogue at zero
   displacement (tc20-codegen rule 7's shape, here for a return). */
extern unsigned gc0ea;                  /* DS:C0EA */

int sprite_sheet_index_get()
{
    return (gc0ea);
}
