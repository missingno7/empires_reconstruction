/* F_E48A -- program one OPL voice from the note tables.  Plain C.  The high
   byte of the fetched word is read at [bp-1] with CBW, i.e. as a signed char
   member of a union, not as (t >> 8). */
extern void fc898();
extern char tab_flag[];                 /* DS:CA17, byte per voice */
/*@SYM _tab_flag=0xCA17 kind=g key=storage_objects/REGION_2C547.phys*/
extern char gc6b6[];                    /* DS:C6B6, byte per voice; convention -> phys 0x2C1E6 (storage_objects/M_2C1E6) */
extern int  tab_bias[];                 /* DS:CA6D, word per voice */
/*@SYM _tab_bias=0xCA6D kind=g key=storage_objects/REGION_2C59D.phys*/
extern char far *gca24[];               /* DS:CA24, far ptr per voice; convention -> phys 0x2C554 (storage_objects/M_2C554) */
extern char tab_ix[];                   /* DS:C64A, byte per note */
/*@SYM _tab_ix=0xC64A kind=g key=storage_objects/REGION_2C17A.phys*/
extern char tab_oct[];                  /* DS:C5EA, byte per note */
/*@SYM _tab_oct=0xC5EA kind=g key=storage_objects/REGION_2C11A.phys*/

void fe48a(v, note, flag)
register int v;
int note;
int flag;
{
    union { int w; char b[2]; } t;
    register int d;

    tab_flag[v] = flag;
    gc6b6[v] = note;
    note += tab_bias[v];
    if (note > 0x5f) note = 0x5f;
    if (note < 0) note = 0;
    t.w = *(int far *)(gca24[v] + tab_ix[note] * 2);
    fc898(v + 0xa0, t.w);
    d = flag ? 0x20 : 0;
    d += tab_oct[note] * 4 + (t.b[1] & 3);
    fc898(v + 0xb0, d);
}
