extern int cur_idx;             /* DS:3902 */
/*@SYM _cur_idx=0x3902 kind=g key=storage_objects/G_P23432.phys*/
extern unsigned char mode;      /* DS:BFCD */
/*@SYM _mode=0xBFCD kind=g key=storage_objects/M_2BAFD.phys*/
extern int g3904[];             /* DS:3904; convention -> DGROUP+0x3904 = phys 0x23434 */
extern int gbe[];               /* DS:00BE; convention -> DGROUP+0x00BE = phys 0x1FBEE */
extern int gfe[];               /* DS:00FE; convention -> DGROUP+0x00FE = phys 0x1FC2E */
extern int result;              /* DS:40C8 */
/*@SYM _result=0x40C8 kind=g key=storage_objects/M_23BF8.phys*/
void f01ce(register int i)
{
    cur_idx = i;
    if (mode == 5)      result = g3904[i];
    else if (mode == 2) result = gbe[i];
    else                result = gfe[i];
}
