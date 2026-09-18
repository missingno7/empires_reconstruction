/* F_21A9 -- load the two boot sprite sheets and publish their far addresses
   into the DS:C0D6 table F_6CA6 indexes.  An array NAME assigned to a far
   pointer stores `ds` into the segment half and the offset CONSTANT into the
   other; the constant index folds to a direct displacement (rule 2). */
extern void f68aa();
extern unsigned char g9cf2[];           /* DS:9CF2 */
extern unsigned char ga6b6[];           /* DS:A6B6 */
extern unsigned char far *gc0d6[];      /* DS:C0D6 */

void f21a9()
{
    f68aa(0, g9cf2);
    f68aa(1, ga6b6);
    gc0d6[0] = g9cf2;
    gc0d6[1] = ga6b6;
}
