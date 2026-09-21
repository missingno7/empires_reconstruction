/* tbl[]: 27-byte records at DS:C4F0 (DGROUP offset 50288,
   storage_objects/M_2BFA0.phys).  F_A85E names every offset it touches;
   F_68CF only touches offsets 13 and 15, which F_A85E's d13/f15 already
   name and which agree in type (both int). */
struct tbl_entry {
    char pad0[9];
    int  a9;        /* +9  */
    char b11;       /* +11 */
    char pad1;
    int  d13;       /* +13: F_A85E .d13 / F_68CF .a */
    int  f15;       /* +15: F_A85E .f15 / F_68CF .b */
    int  h17;       /* +17 */
    int  j19;       /* +19 */
    char l21;       /* +21 */
    char pad2[5];
};                  /* sizeof == 27 */

extern struct tbl_entry tbl[];
