/* src/VIDEO.C: Video and BIOS initialization: palette, mode and frame buffer.
   One translation unit; the sections below were the separate member
   sources of grouped module C_01BC_0355 and keep their original ids. */

#define MK_FP(seg, ofs) ((void far *) (((unsigned long) (seg) << 16) | (unsigned) (ofs)))
#define FP_SEG(fp) ((unsigned) ((unsigned long) (void far *) (fp) >> 16))

extern int cur_idx;             /* DS:3902 */
/*@SYM _cur_idx=0x3902 kind=g key=storage_objects/G_P23432.phys*/
extern unsigned char mode;      /* DS:BFCD -- unsigned-char view; F_0281 needs the plain char
                                    view `display_mode` at the same address (rule: byte view
                                    kept as a local-exception exception, see LEVEL.C). */
/*@SYM _mode=0xBFCD kind=g key=storage_objects/M_2BAFD.phys*/
extern int g3904[];             /* DS:3904; convention -> DGROUP+0x3904 = phys 0x23434 */
extern int gbe[];               /* DS:00BE; convention -> DGROUP+0x00BE = phys 0x1FBEE */
extern int gfe[];               /* DS:00FE; convention -> DGROUP+0x00FE = phys 0x1FC2E */
extern int result;              /* DS:40C8 */
/*@SYM _result=0x40C8 kind=g key=storage_objects/M_23BF8.phys*/
extern int g3902;
extern int g00be[];
extern void movmem(char far *s, char far *d, unsigned n);  /* CC.LIB MOVMEM, _TEXT+0xF348 */
extern char g9c[], gde[];  /* DGROUP+0x009C/0x00DE */
extern char far *farmalloc();
extern char far *video_normalize_far_ptr();
extern void runtime_base();
extern void f0232();
extern void video_load_palette();
extern char display_mode;                      /* DS:BFCD, the display mode -- plain char view;
                                                    see `mode` above for the unsigned-char view. */
extern char far *g40ca;                 /* DS:40CA offset, DS:40CC segment */
extern char far *g3924[];               /* DS:3924 */
extern unsigned char g11e[];            /* DS:011E */
extern unsigned char g41e[];            /* DS:041E */
extern void gfx_bar(int x, int y, int n);
extern void gfx_vline(int x, int y, int n);

/* ---- F_01BC (original code at 0x01BC) ---- */
/* F_01BC -- load a 256-entry DAC block through int 10h AX=1012h.  The BIOS
   register setup is C through Turbo C's pseudo-registers and __int__ (each
   assignment compiles to the original mov/xor, `_BX = 0` to `xor bx,bx`);
   only the far-pointer load stays asm: `les dx,pal` is one 3-byte
   instruction, while `_ES = FP_SEG(pal); _DX = FP_OFF(pal)` compiles to two
   loads and a segment move (5 bytes longer, probed 2026-09-21). */
void __int__(int);

void video_load_palette(pal)
char far *pal;
{
    asm les dx,pal
    _BX = 0;
    _CX = 0x100;
    _AX = 0x1012;
    __int__(0x10);
}


/* ---- F_01CE (original code at 0x01CE) ---- */
void gfx_color_select(register int i)
{
    cur_idx = i;
    if (mode == 5)      result = g3904[i];
    else if (mode == 2) result = gbe[i];
    else                result = gfe[i];
}


/* ---- F_020F (original code at 0x020F) ---- */
f020f(){return g3902;}


/* ---- F_0215 (original code at 0x0215) ---- */
void color_table_entry_set(int index, int value1, int value2)
{
    g3904[index] = value1;
    g00be[index] = value2;
}


/* ---- F_0232 (original code at 0x0232) ---- */
/* g3904/gbe declared int[] to agree with other members of this module. */
void f0232(void)
{
    movmem(g9c, (char *)g3904, 0x20);
    movmem(gde, (char *)gbe, 0x20);
}


/* ---- F_025B (original code at 0x025B) ---- */
/* F_025B -- normalise a far pointer: carry the top 12 bits of the offset into
   the segment and keep the low nibble.  Both parameters are promoted into
   register variables (rule 12).  Both assignments are PLAIN, not compound:
   `s = (o >> 4) + s` computes into AX and moves it to DI, where `s += o >> 4`
   would have added straight into DI (rule 9's spelling rule, in the direction
   opposite to the one F_E372 witnesses). */
char far *video_normalize_far_ptr(o, s)
register unsigned o;
register unsigned s;
{
    s = (o >> 4) + s;
    o = o & 15;
    return ((char far *) MK_FP(s, o));
}


/* ---- F_0281 (original code at 0x0281) ---- */
/* F_0281 -- allocate the 488-row screen buffer for the current mode, normalise
   its base to a paragraph and fill the DS:3924 row table with one normalised
   far pointer per row, then load the mode's palette.  The row width is a
   register variable (SI) declared before the loop counter (DI), rule 12; the
   size is `(long)w * 488 + 16` and the `cwd` before the long multiply makes
   `w` a SIGNED int widened to long (rule 16's shape). */
void video_alloc_framebuffer()
{
    char far *q;
    unsigned s;
    register int w;
    register int i;

    if (display_mode == 5)
        w = 0x140;
    else if (display_mode == 2)
        w = 0x50;
    else
        w = 0xa0;
    g40ca = farmalloc((long) w * 488 + 16);
    s = FP_SEG(g40ca) + 1;
    g40ca = MK_FP(s, 0);
    q = video_normalize_far_ptr(g40ca);
    for (i = 0; i < 488; i++) {
        g3924[i] = q;
        q = video_normalize_far_ptr(q + w);
    }
    runtime_base();
    f0232();
    if (display_mode == 5)
        video_load_palette(g11e);
    else if (display_mode == 4)
        video_load_palette(g41e);
}


/* ---- F_034F (original code at 0x034F) ---- */
/* F_034F -- switch to the text video mode through BIOS INT 10h (AX=0003h),
   plain C through the _AX pseudo-register and the __int__ intrinsic. */
void video_set_text_mode()
{
    _AX = 3;
    __int__(0x10);
}


/* ---- F_0355 (original code at 0x0355) ---- */
void rect_border_draw(int x, int y, int w, int h)
{
    gfx_bar(x, y, w);
    gfx_vline(x, y, h);
    gfx_bar(x, y + h - 1, w);
    gfx_vline(x + w - 1, y, h);
}
