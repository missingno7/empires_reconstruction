/* src/RECTTAB.C: Rectangle table.
   One translation unit; the sections below were the separate member
   sources of grouped module C_D85F_D8F0 and keep their original ids. */

extern unsigned char g2f30[];
extern int memset(), voice_slot_load_pair();extern void movmem();

struct U {                              /* 56 bytes, `mov dx,0x38; mul dx` */
    int a;
    char rest[0x36];
};

extern char far *gc5da;                 /* DS:C5DA, segment at DS:C5DC */
extern char gc6ab[];                    /* DS:C6AB */
extern char gca62[];                    /* DS:CA62 */
extern char g301a[];                    /* DS:301A, stride 0x38 */
extern struct U g3044[];                /* DS:3044, stride 0x38 */

/* ---- F_D85F (original code at 0xD85F) ---- */
/* F_D85F -- delete the matching five-byte record and compact the table. */
void record_table_delete_compact(key)
int key;
{
    asm mov ax,ds
    asm mov es,ax
    asm mov si,offset DGROUP:g2f30
    asm lodsb
    asm mov cl,al
    asm sub ch,ch
    asm jcxz L_out
L_scan: asm mov al,[si]
    asm cmp al,[bp+4]
    asm je L_hit
    asm add si,5
    asm loop L_scan
    asm jcxz L_out
L_hit: asm mov di,si
    asm add si,5
    asm dec cx
    asm mov ax,cx
    asm shl ax,1
    asm shl ax,1
    asm add cx,ax
    asm rep movsb
    asm mov di,offset DGROUP:g2f30
    asm dec byte ptr es:[di]
L_out: ;
}


/* ---- F_D89A (original code at 0xD89A) ---- */
/* F_D89A -- return the first rectangle-table id overlapping the query. */
int rect_table_hit_id(x,y,w,h)
int x,y,w,h;
{
    asm        cld
    asm mov ax,[bp+4]
    asm mov bl,al
    asm mov ax,[bp+6]
    asm mov bh,al
    asm mov ax,[bp+8]
    asm mov dl,al
    asm mov ax,[bp+0Ah]
    asm mov dh,al
    asm add dx,bx
    asm dec dl
    asm dec dh
    asm mov si,offset DGROUP:g2f30
    asm lodsb
    asm sub ch,ch
    asm mov cl,al
    asm or cx,cx
    asm je L_zero
L_top: asm sub ah,ah
    asm lodsb
    asm mov di,ax
    asm lodsw
    asm cmp dl,al
    asm jb L_skip
    asm cmp dh,ah
    asm jb L_skip
    asm mov bp,ax
    asm lodsw
    asm add ax,bp
    asm cmp al,bl
    asm jbe L_next
    asm cmp ah,bh
    asm jbe L_next
    asm mov ax,di
    asm jmp short L_done
L_skip: asm add si,2
L_next: asm loop L_top
L_zero: asm sub ax,ax
L_done: ;
}


/* ---- F_D8F0 (original code at 0xD8F0) ---- */
/* F_D8F0 -- rebuild the nine-slot panel from the record at gc5da+8.  Entry
   1D9F0, 171 bytes.  One far-pointer local (bp-04, segment at bp-02), one
   register variable (SI = i), no parameters.

   D8F7  C41EDAC5 83C308 8C46FE 895EFC
                             p = gc5da + 8: `les` loads the far pointer whole,
                              the constant lands on the OFFSET, and the store
                              writes the SEGMENT word first.
   D91F  8B16DCC5 A1DAC5 051100 52 50
                             the same pointer + 0x11 as an ARGUMENT is built
                              the other way: two loads, add on the offset,
                              push segment then offset.
   D95A  BB3800 52 F7E3 8BD8 81C34430 1E 07 58 268907
                             `g3044[*p].a = <rhs>` -- the RHS is computed
                              first, PUSHED across the address computation and
                              popped back, which is TC 2.0's order for a store
                              whose left side needs a multiply.
   D962  81C34430 1E 07 268907     vs
   D978  BB1A30 8CD9 03D8 51 53
                             the two far-pointer shapes side by side in ONE
                              function: indexing an array of 0x38-byte
                              structs builds the OFFSET first and loads the
                              segment last, while a far CAST of an array base
                              plus an index materialises the pointer first and
                              adds the index after.  F_656C's negative test
                              named this distinction; here the original itself
                              carries both, 22 bytes apart.
   D988  C684ABC601          gc6ab[i] = 1 -- a plain char array indexed by the
                              register folds to one near DS displacement
                              (0xC6AB written as si-0x3955). */

record_panel_rebuild()
{
    char far *p;                        /* bp-04 */
    register int i;                     /* si */

    p = gc5da + 8;
    memset(gc6ab, 0, 9);
    movmem(gc5da + 0x11, gca62, 9);
    for (i = 0; i < 9; i++, p++) {
        if (*p != -1) {
            g3044[*p].a = 0x3f - gc5da[i + 0x1a] * 9;
            voice_slot_load_pair(i, (char far *)g301a + *p * 0x38);
            gc6ab[i] = 1;
        }
    }
}
