/* src/RESOURCE.C: Resource files and indexed record I/O.
   One translation unit; the sections below were the separate member
   sources of grouped module C_6266_68AA and keep their original ids. */

/* ---- F_6266 (original code at 0x6266) ---- */
/* F_6266 -- open the data file named in directory row i, retry once with the
   name's first letter case-folded, and report through gb31/dialog_run if neither
   attempt matches the recorded stamp.  Entry 16366, 468 bytes.

   627F  893E3E0B            `ok = 0; gb3e = ok;` -- TC 2.0 stores the
              register variable STRAIGHT to memory.  Written as the chain
              `gb3e = ok = 0;` it inserts `mov ax,di` first (attempt 2's
              first difference, extent offset 25: original 89, candidate 8B).
   628D  05220A 8CDA 52 50   the name argument is the ROW of a 2D char array,
              `ga22[i]`, which is one complete far-pointer operand: offset
              computed in AX, base added into AX, segment last.  Written as
              `(char far *)ga22 + i * 0x10 + K` instead, TC reassociates the
              two int addends and defers the base to the very end (attempt 3's
              first difference, extent offset 39: original 05, candidate 50).
   62A0..62B4  the relational is multiplied: `(ga22[i][0] > 0x42) * 3` gives
              the 0/1 branch pair and then `mov dx,3; mul dx`; `cond ? 3 : 0`
              would have written 3 straight into AX with no mul.  The far
              pointer is PUSHED before that addend is computed and POPPED
              back to add it, which is TC's shape for a pointer argument
              whose int addend needs branches of its own.
   62D7..6303  A && B && C && D under a `!`: each failing test jumps INTO the
              block and the all-pass path jumps over it.
   6338  B042 2A46F7 0441   `c = 0x42 - c + 0x41` stays in AL: char
              arithmetic in a byte register when operands and destination are
              all chars, and the two constants are NOT folded.
   63DD / 6403  gb31 = (char far *)&ga5e[i] twice, both in the
              array-of-0x23-byte-struct shape (mov bx,ax; add bx,offset;
              push ds; pop es) that F_656C's negative test established.
   62F1  8B97540A 8B87520A   ga52[i] is a plain long array indexed by a
              register: near DS addressing, base folded into the
              displacement, no segment materialised. */

#include "DIALOG.H"
#include "GA5E.H"
#include "LAYOUT.H"

extern int open();
extern int read(int,void far *,unsigned);
extern int write(int,void far *,unsigned);
extern int close(int);
extern long lseek(int,long,int);
extern void far * memmove();

extern int  gb3e, gb40, slot_file_handle;          /* DS:0B3E, DS:0B40, DS:C0C9 */
extern char far *gb31;                  /* DS:0B31, segment at DS:0B33 */
extern struct dialog gb2a;              /* DS:0B2A */
extern char gbfcc;                      /* DS:BFCC */
extern char ga22[][16];                 /* DS:0A22 -- 16-byte name rows */
extern long ga52[];                     /* DS:0A52 */
extern char near current_drive;
extern void resource_file_open(int), disk_reset_retry(int);
extern int  lz_decompress(), rle_packbits_decode(), sprite_sheet_decode_sequential(), sprite_sheet_decode_indexed(), sprite_decode_4bpp_mode13h(), sprite_decode_4bpp_planar();
extern char gc0cb;                      /* DS:C0CB */
extern char display_mode;                      /* DS:BFCD */
extern char far *ui_gfx_shadow_b;                 /* DS:C5BE, segment at DS:C5C0 */
extern char far *ui_gfx_shadow_a;                 /* DS:C5C6, segment at DS:C5C8 */
extern char far *ui_gfx_blob;                 /* DS:C5CA, segment at DS:C5CC */
extern int resource_load_record();
extern char far *farmalloc();
extern void game_shutdown();
extern void video_set_text_mode();
extern void dos_write_handle2();
extern void exit();
extern void movmem();
extern char gb42[];                     /* DS:0B42, the message */

void resource_file_open(i)
register int i;
{
    char c;                             /* bp-09 */
    int sv;                             /* bp-08 */
    long stamp;                         /* bp-06 */
    int n;                              /* bp-02 */
    register int ok;                    /* di */

    sv = gb40;
    gb40 = 1;
    do {
        ok = 0;
        gb3e = ok;
        slot_file_handle = open(ga22[i] + (ga22[i][0] > 0x42) * 3, 0x8004);
        n = read(slot_file_handle, &stamp, 4);
        if (!(slot_file_handle >= 0 && gb3e == 0 && n >= 4 && ga52[i] == stamp)) {
            if (slot_file_handle >= 0)
                close(slot_file_handle);
            if (gbfcc > 1 && (c = ga22[i][0]) <= 0x42) {
                c = 0x42 - c + 0x41;
                ga22[i][0] = c;
                gb3e = 0;
                slot_file_handle = open(ga22[i], 0x8004);
                n = read(slot_file_handle, &stamp, 4);
                if (!(slot_file_handle >= 0 && gb3e == 0 && n >= 4
                      && ga52[i] == stamp)) {
                    if (slot_file_handle >= 0)
                        close(slot_file_handle);
                    ga22[i][0] = 0x42 - c + 0x41;
                    gb31 = (char far *)&ga5e[i];
                    dialog_run(&gb2a);
                    ok = 1;
                }
            } else {
                gb31 = (char far *)&ga5e[i];
                dialog_run(&gb2a);
                ok = 1;
            }
        }
    } while (ok);
    gb40 = sv;
}


/* ---- F_643A (original code at 0x643A) ---- */
/* Exact 240-byte indexed record writer with native __sti__ intrinsic; no inline ASM or byte emission. */
/* This section needs the NEAR-qualified views of gb40/gb3e/slot_file_handle,
   gb31 and gb2a; F_6266 and F_656C use the plain (non-near) forms, kept
   above -- both views name the same storage, kept as a byte-significant
   local exception (compare RESOURCE.C's gb31 far*-near split). */
extern int near gb40,gb3e,slot_file_handle;
extern char far * near gb31;
extern struct dialog near gb2a;
void __sti__(void);
void resource_file_write_record(unsigned index,void far *data)
{
 int length;
 unsigned long first,second;
 int saved;
 register unsigned slot,group;
 slot=index;group=slot>>12;saved=gb40;slot&=0xfff;gb40=1;
 do {
 gb3e=0;resource_file_open(group);
 lseek(slot_file_handle,(unsigned long)(slot*4),0);
 read(slot_file_handle,&first,4);read(slot_file_handle,&second,4);
 length=second-first;
 lseek(slot_file_handle,first+2,0);
 write(slot_file_handle,data,length-2);close(slot_file_handle);
 if(gb3e) {disk_reset_retry(current_drive);gb31=(char near *)ga5e+0x69; /* Existing DGROUP string at 0x0ac7. */dialog_run(&gb2a);disk_reset_retry(current_drive);}
 }while(gb3e);
 gb40=saved;__sti__();
}


/* ---- F_652A (original code at 0x652A) ---- */
/* Exact 66-byte BIOS retry helper using Turbo C register pseudo-variables and __int__; no inline ASM or byte emission. */
void __int__(int);
void disk_reset_retry(int drive)
{
 char buffer[512];
 register int i;
 for(i=0;i<3;i++) {
 _AH=0;_DL=current_drive;__int__(0x13);
 _ES=(unsigned)((unsigned long)(char far *)buffer>>16);
 _BX=(unsigned)(char far *)buffer;
 _AH=2;_AL=1;_CH=1;_CL=1;_DH=0;_DL=drive;__int__(0x13);
 }
 _AH=0x0d;__int__(0x21);
}


/* ---- F_656C (original code at 0x656C) ---- */
/* F_656C -- load one indexed record from the open data file and hand it to
   whichever unpacker its header byte names.  Entry 1666C, 517 bytes, 950
   activations.  One unsigned parameter that packs a directory index in its
   top four bits and a record number in the low twelve.

   Read off assets/AEPROG.EXE (ndisasm -b16 -o 0x656c):

   6574  8B7E04 B10C D3EF     d = p >> 12: `shr` (UNSIGNED) with the count in
                               CL, so p is unsigned and the shift is not an
                               arithmetic one.
   6581  816604FF0F           p &= 0xFFF -- the parameter itself is the
                               destination, so the source assigns to it.
   658C  the do-while top. gb3e is cleared here, inside the loop, and tested
         twice: once at 6609 to guard the retry block and once at 6631 as the
         loop condition.  The back edge is 175 bytes, past a short jump's
         reach, so TC 2.0 writes it as `jz over / jmp top` (7403 E951FF).
   659A  D1E0 D1E0 33D2 52 50 p*4 zero-extended into DX:AX and pushed high
                               word first: ONE long argument built from an
                               unsigned int, which is what `(long)(p * 4)`
                               compiles to.  A signed int would have been
                               widened with CWD, and a long-typed p would not
                               have needed the widening at all.
   65B3  16 8D46F4 50         push ss / lea / push -- &o1 as a far pointer to
                               a local, the compact model's default.
   65D5  8B76F8 2B76F4        the size is a SIXTEEN-bit subtraction of the two
                               longs' low words: `(int)o2 - (int)o1`.  Writing
                               `(int)(o2 - o1)` would have emitted the 32-bit
                               sub/sbb pair first and then truncated.
   65EF  FF36CCC5 FF36CAC5    a far POINTER global pushed as segment word then
                               offset word (gc5cc:ui_gfx_blob), against the
                               `push ds / mov ax,offset / push ax` an array
                               name would have produced.
   6656  4E 4E                s -= 2 as two DECs on the register variable.
   locals            TC 2.0 lays locals out in REVERSE declaration order
                              upward from bp, each aligned to even: sv (int) at
                              bp-02, fl (char) at bp-03, o2 (long) at bp-08
                              (bp-07 rounded down), o1 at bp-0C, frame 0x0C.
   6658  F646FD02 743A F646FD01 7434
                              `if ((fl & 2) && (fl & 1))` -- both tests jump
                               to the same else arm, the shape && produces.
   670D  A0CBC0 98 0BC0 7431 3D0100 743D 3D4700 7402 EB45
                              a SWITCH on a char: one load, sign-extended,
                               then `or ax,ax` for the zero case and `cmp ax`
                               for the others, dispatched in ASCENDING case
                               order (0, 1, 0x47) while the bodies stay in
                               source order (0x47 at 6721, 0 at 6746, 1 at
                               6757) -- the same ordering F_5382 carries.
   6764  EB00                 the last arm's break, displacement zero.
   6769  EB00                 `return s;` jumps to the epilogue that already
                               follows it. */

resource_load_record(p)
unsigned p;
{
    long o1;                            /* bp-0C */
    long o2;                            /* bp-08 */
    char fl;                            /* bp-03 */
    int sv;                             /* bp-02 */
    register int s, d;                  /* si, di */

    d = p >> 12;
    sv = gb40;
    p &= 0xfff;
    gb40 = 1;
    do {
        gb3e = 0;
        resource_file_open(d);
        lseek(slot_file_handle, (long)(p * 4), 0);
        read(slot_file_handle, &o1, 4);
        read(slot_file_handle, &o2, 4);
        s = (int)o2 - (int)o1;
        lseek(slot_file_handle, o1, 0);
        read(slot_file_handle, ui_gfx_blob, s);
        close(slot_file_handle);
        if (gb3e != 0) {
            gb31 = (char far *)&ga5e[d];
            dialog_run(&gb2a);
        }
    } while (gb3e != 0);
    gb40 = sv;
    gc0cb = ui_gfx_blob[0];
    fl = ui_gfx_blob[1];
    s -= 2;
    if ((fl & 2) && (fl & 1)) {
        s = lz_decompress(ui_gfx_shadow_a, ui_gfx_shadow_b, s);
        s = rle_packbits_decode(ui_gfx_shadow_b, ui_gfx_shadow_a, s);
    } else if (fl & 2) {
        s = lz_decompress(ui_gfx_shadow_a, ui_gfx_shadow_b, s);
        memmove(ui_gfx_shadow_a, ui_gfx_shadow_b, s);
    } else if (fl & 1) {
        s = rle_packbits_decode(ui_gfx_shadow_a, ui_gfx_shadow_b, s);
        memmove(ui_gfx_shadow_a, ui_gfx_shadow_b, s);
    }
    if (display_mode != 5) {
        switch (gc0cb) {
        case 0x47:
            if (display_mode == 2)
                sprite_decode_4bpp_mode13h(ui_gfx_shadow_a);
            else
                sprite_decode_4bpp_planar(ui_gfx_shadow_a);
            break;
        case 0:
            sprite_sheet_decode_sequential(ui_gfx_shadow_a, s);
            break;
        case 1:
            sprite_sheet_decode_indexed(ui_gfx_shadow_a);
            break;
        }
    }
    __sti__();
    return s;
}


/* ---- F_6771 (original code at 0x6771) ---- */
sprite_sheet_decode_sequential(p,n) char *p;unsigned n;{register unsigned i;unsigned char *q;for(i=0;i<n;){q=p+i;if(*q!=0x47)break;if(display_mode==2)sprite_decode_4bpp_mode13h(q+2);else sprite_decode_4bpp_planar(q+2);i+=q[34]*q[35]+36;}}


/* ---- F_67DC (original code at 0x67DC) ---- */
sprite_sheet_decode_indexed(p) char *p;{register int i,n;unsigned *t;char *q;t=(unsigned *)p;n=(*t>>1)-1;for(i=0;i<n;i++){q=p+*t;t++;if(*q==0x47){if(display_mode==2)sprite_decode_4bpp_mode13h(q+2);else sprite_decode_4bpp_planar(q+2);}}}


/* ---- F_684A (original code at 0x684A) ---- */
/* F_684A -- read one indexed record, allocate a far block for it and copy the
   staged buffer in; on an allocation failure, tear the screen down, print the
   message at DS:0B42 and exit.  The far pointer OUT parameter is compared
   against zero the way TC 2.0 compares a far pointer: OR the two halves. */
void resource_load_record_alloc(a, pp)
int a;
char far * far *pp;
{
    register int n;

    n = resource_load_record(a);
    *pp = farmalloc((long)n);
    if (*pp == 0) {
        game_shutdown();
        video_set_text_mode();
        dos_write_handle2(gb42);
        exit(a);
    }
    movmem(ui_gfx_shadow_a, *pp, n);
}


/* ---- F_68AA (original code at 0x68AA) ---- */
/* F_68AA -- read one indexed record through resource_load_record and copy the staged buffer
   the far pointer at DS:C5C6 names into the caller's buffer.  The pointer is
   a far POINTER global: it pushes segment word then offset word
   (tc20-codegen rule 5), which an array name would not have done. */
void resource_load_record_into(a, q)
unsigned a;
char far *q;
{
    register unsigned n;

    n = resource_load_record(a);
    movmem(ui_gfx_shadow_a, q, n);
}
