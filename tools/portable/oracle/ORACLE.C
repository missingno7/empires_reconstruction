/* ORACLE.C -- historical oracle harness driver.
 *
 * Turbo C 2.0, compact model (-c -mc -1- -f- -N-).  Reads a binary command
 * script ORACLE.IN from the current directory and writes ORACLE.OUT.  See
 * tools/portable/oracle/README.md for the exact record format.
 *
 * Deliberately includes NO standard headers, matching the house style of
 * src/*.C: the pinned toolchain checkout carries TCC.EXE/TASM.EXE/TLINK.EXE
 * and CC.LIB but no TC\INCLUDE headers, so every library routine this file
 * uses is declared the same unprototyped/K&R way src/RESOURCE.C declares
 * open/read/write/close/lseek/farmalloc/exit.  FILE* handles from the
 * standard library (fopen/fread/fwrite/fclose -- confirmed present in
 * CC.LIB as _fopen/_fread/_fwrite/_fclose) are carried as opaque
 * `void far *` since this file never looks inside the FILE struct.
 *
 * Two families of historical entry points are exercised:
 *
 *   - asm/DECODE.ASM (_rle_packbits_decode, _lz_decompress,
 *     _sprite_decode_4bpp_planar, _sprite_decode_4bpp_mode13h): plain near
 *     C-convention routines with only far-pointer/word arguments and NO
 *     absolute DS references (verified: asm/DECODE.ASM contains no `ds:`
 *     addressing).  Called directly.
 *
 *   - asm/RUNTIME_BLOCK.ASM (_gfx_*): near C-convention routines that
 *     address a fixed DGROUP layout through absolute ds:[imm] offsets.  This
 *     harness builds a FAKE DGROUP (a zero-filled far block) with the same
 *     layout, then calls each _gfx_* entry point through ds_call() (see
 *     DSCALL.ASM), which switches DS to the fake block only for the
 *     duration of the near call.
 *
 * RUNTIME_BLOCK.OBJ must be linked so _runtime_base sits at _TEXT+0x039C
 * (build_oracle.py verifies this from OUT.MAP); the module is
 * position-dependent (RT_CS equ 039Ch) and its CS-relative jump/dispatch
 * tables are wrong at any other placement.
 */

typedef unsigned char  u8;
typedef int             i16;
typedef unsigned int    u16;
typedef long             i32;
typedef unsigned long   u32;

/* ------------------------------------------------------------------ */
/* Runtime library routines, declared K&R-style (no headers present)   */
/* ------------------------------------------------------------------ */

extern void far *fopen();
extern int fread();
extern int fwrite();
extern int fclose();

extern int open();
extern int read();
extern int write();
extern int close();
extern long lseek();
extern char far *farmalloc();
extern void exit();

/* ------------------------------------------------------------------ */
/* Historical entry points                                             */
/* ------------------------------------------------------------------ */

/* asm/DECODE.ASM -- no DS dependency, called directly. */
extern u16  rle_packbits_decode(char far *src, char far *dst, u16 max_len);
extern u16  lz_decompress(char far *src, char far *dst, u16 src_len);
extern void sprite_decode_4bpp_planar(char far *q);
extern void sprite_decode_4bpp_mode13h(char far *q);

/* asm/RUNTIME_BLOCK.ASM -- address-of only; never called directly (DS
 * dependency), only through ds_call().  Declared as plain externs so TCC
 * emits ordinary EXTDEF records matching the ASM module's PUBLIC names
 * (_gfx_bar, _gfx_vline, ...). */
extern void gfx_bar(), gfx_vline(), gfx_clear_rect(), gfx_fill_rect();
extern void gfx_save_rect(), gfx_restore_rect(), gfx_wipe_rect();
extern void gfx_copy_rect_flip_v(), gfx_copy_rect_flip_h(), gfx_copy_rect_flip_hv();
extern void gfx_copy_rect_split(), gfx_copy_rect_split_flip_v();
extern void gfx_draw_char(), gfx_blit_bitmap(), gfx_copy_rect();
extern void gfx_blit_image(), gfx_set_pixel(), gfx_get_pixel();

/* DSCALL.ASM thunk. */
extern int ds_call(unsigned seg, void (near *fn)(), int nwords, int near *words);
typedef void (near *fnptr)();
#define FN(x) ((fnptr)(x))

/* ------------------------------------------------------------------ */
/* Far pointer split/build without <dos.h> (FP_SEG/FP_OFF/MK_FP)       */
/*                                                                     */
/* A Turbo C `far *` occupies 4 bytes: offset word at the lower        */
/* address, segment word at the higher address (the same layout the   */
/* ASM's lds/les instructions read/write for far-pointer stack args,  */
/* confirmed against asm/DECODE.ASM and asm/RUNTIME_BLOCK.ASM).       */
/* ------------------------------------------------------------------ */

typedef union { char far *p; unsigned w[2]; } farsplit;

static unsigned fp_off(char far *p) { farsplit u; u.p = p; return u.w[0]; }
static unsigned fp_seg(char far *p) { farsplit u; u.p = p; return u.w[1]; }
static char far *mk_fp(unsigned seg, unsigned off)
{
    farsplit u;
    u.w[0] = off;
    u.w[1] = seg;
    return u.p;
}

/* ------------------------------------------------------------------ */
/* Fake DGROUP layout (asm/RUNTIME_BLOCK.ASM absolute offsets)         */
/* ------------------------------------------------------------------ */

#define FAKE_DGROUP_BYTES 0xC800u      /* 51200: comfortably covers every offset below */
#define ROW_TABLE_OFF     0x3924u      /* g3924[488] far row pointers */
#define GBC_OFF            0x00BCu     /* gbc: dirty-queue gate */
#define CLIP94_OFF          0x0094u
#define CLIP96_OFF          0x0096u
#define CLIP98_OFF          0x0098u
#define CLIP9A_OFF          0x009Au
#define QUEUE_PTR_OFF      0x40C4u     /* rect_queue_write_ptr (far; only the offset word is updated by the ASM) */
#define RESULT_OFF         0x40C8u     /* result: current color word */
#define FONT_C0DE_OFF      0xC0DEu
#define FONT_C0E0_OFF      0xC0E0u     /* font base SEGMENT (word, not a far pointer) */
#define FONT_C0E2_OFF      0xC0E2u
#define FONT_C0E4_OFF      0xC0E4u
#define FONT_C0E6_OFF      0xC0E6u
#define FONT_C0E8_OFF      0xC0E8u

#define FB_STRIDE 0xA0u                /* 160 bytes/row, display_mode 4 */
#define FB_ROWS   488u
#define FB_BYTES  78080UL              /* FB_STRIDE * FB_ROWS */

#define QUEUE_BUF_BYTES 8208UL
#define BLOB_BUF_BYTES  8192UL
#define DST_BUF_BYTES   16400UL
#define SAVE_BUF_BYTES  4096UL
#define FONT_BUF_BYTES  8192UL
/* src/PLAYERSL.C ui_gfx_alloc (the historical allocator for these three
 * buffers) does NOT lay blob/shadow_a/shadow_b out via flat offset
 * arithmetic within one segment:
 *
 *   ui_gfx_blob     = farmalloc(0xFA80L); s = FP_SEG(ui_gfx_blob) + 1;
 *   ui_gfx_blob     = MK_FP(s, 0x0E);
 *   ui_gfx_shadow_a = MK_FP(s + 1, 0);
 *   ui_gfx_shadow_b = MK_FP(s + 0x7D4, 0);
 *
 * shadow_a and shadow_b each get their OWN paragraph-normalized segment
 * with offset 0 -- physically identical to blob+2 / blob+0x7D32 (segment
 * s+1 is exactly 16 bytes past segment s; segment s+0x7D4 is exactly
 * 0x7D40 bytes past it), but the FAR POINTER's offset component starts
 * at 0 rather than 0x10/0x7D40.  That is load-bearing:
 * asm/DECODE.ASM's _lz_decompress stores its back-reference checkpoint
 * table as plain 16-bit offsets into the destination (`mov [bx],di`) and
 * later re-derives DS:SI from them assuming they are small values
 * relative to the destination segment's own start.  DI is a 16-bit
 * register that wraps at 64K of growth *from wherever it starts*; giving
 * shadow_a/shadow_b flat offsets of 0x10/0x7D40 (as an earlier version of
 * this harness did) leaves them only ~2 and ~33KB of headroom instead of
 * a full 64K, so any record whose LZ-stage output (before the RLE stage,
 * which can further shrink it) runs past that leaves DI wrapping back to
 * a small offset mid-decode, corrupting shadow_a while it is still being
 * read as the source and truncating the result (observed: AE000 record 2
 * decoded to 127 bytes instead of the correct 1886).
 *
 * RES_BLOCK_BYTES must cover the *entire* 64K that shadow_b's segment can
 * address (it starts 0x7D40 bytes into the block): 0x7D40 + 0x10000 =
 * 0x17D40 at minimum; allocated with generous headroom below. */
#define RES_BLOCK_BYTES 0x30000UL

static unsigned    dgseg;
static char far    *dgbase;
static char far    *fbbase;
static char far    *queuebase;
static char far    *blob_buf;
static char far    *dst_buf;
static char far    *save_buf;
static char far    *font_buf;
static char far    *res_blob;
static char far    *res_shadow_a;
static char far    *res_shadow_b;

static i16   last_ret;
static u16   last_save_len;

static int   words[16];
static int   nwords;

static void far *inf, *outf;

/* ------------------------------------------------------------------ */
/* Small helpers                                                       */
/* ------------------------------------------------------------------ */

static void die(char far *msg, unsigned len)
{
    write(2, msg, len);
    exit(1);
}
#define DIE(s) die(s, sizeof(s) - 1)

static char far *fb_normalize(char far *p)
{
    u32 lin = ((u32)fp_seg(p) << 4) + (u32)fp_off(p);
    return mk_fp((unsigned)(lin >> 4), (unsigned)(lin & 0xFu));
}

static char far *falloc_norm(u32 bytes)
{
    char far *raw = farmalloc(bytes + 16UL);
    unsigned s;
    if (raw == (char far *)0)
        DIE("farmalloc failed\n");
    s = fp_seg(raw) + 1;
    return mk_fp(s, 0);
}

#define PEEKW(off)      (*(int far *)(dgbase + (off)))
#define POKEW(off, v)   (*(int far *)(dgbase + (off)) = (v))
#define POKEFP(off, p)  (*(char far * far *)(dgbase + (off)) = (p))
#define PEEKFP(off)     (*(char far * far *)(dgbase + (off)))

static void wi(int v) { words[nwords++] = v; }
static void wf(char far *p) { wi((int)fp_off(p)); wi((int)fp_seg(p)); }

static void fcopy(char far *dst, char far *src, unsigned n)
{
    unsigned i;
    for (i = 0; i < n; i++)
        dst[i] = src[i];
}

/* ------------------------------------------------------------------ */
/* Setup                                                               */
/* ------------------------------------------------------------------ */

static void setup(void)
{
    unsigned i;
    char far *q;
    unsigned r;

    dgbase = falloc_norm((u32)FAKE_DGROUP_BYTES);
    dgseg = fp_seg(dgbase);
    for (i = 0; i < (unsigned)FAKE_DGROUP_BYTES; i++)
        dgbase[i] = 0;

    fbbase = falloc_norm(FB_BYTES);
    q = fbbase;
    for (r = 0; r < FB_ROWS; r++) {
        POKEFP(ROW_TABLE_OFF + (u32)r * 4, q);
        q = fb_normalize(q + FB_STRIDE);
    }

    queuebase = falloc_norm(QUEUE_BUF_BYTES);
    POKEFP(QUEUE_PTR_OFF, queuebase);

    blob_buf  = falloc_norm(BLOB_BUF_BYTES);
    dst_buf   = falloc_norm(DST_BUF_BYTES);
    save_buf  = falloc_norm(SAVE_BUF_BYTES);
    font_buf  = falloc_norm(FONT_BUF_BYTES);
    /* Exact transcription of src/PLAYERSL.C ui_gfx_alloc's pointer
     * construction (see the RES_BLOCK_BYTES comment above): each of the
     * three pointers gets its own paragraph-normalized segment. */
    {
        char far *raw = farmalloc((u32)RES_BLOCK_BYTES);
        unsigned rs;
        if (raw == (char far *)0)
            DIE("farmalloc failed\n");
        rs = fp_seg(raw) + 1;
        res_blob    = mk_fp(rs, 0x0E);
        res_shadow_a = mk_fp(rs + 1, 0);
        res_shadow_b = mk_fp(rs + 0x7D4, 0);
    }

    last_ret = 0;
    last_save_len = 0;
}

/* ------------------------------------------------------------------ */
/* src/RESOURCE.C sprite-sheet dispatch, transcribed verbatim          */
/* (display_mode is fixed at 4 for this harness: the planar branch     */
/* always runs, matching src/RESOURCE.C's `display_mode != 2` arm).    */
/* ------------------------------------------------------------------ */

static void sprite_sheet_decode_sequential(char far *p, unsigned n)
{
    unsigned i;
    char far *q;
    for (i = 0; i < n; ) {
        q = p + i;
        if ((u8)*q != 0x47)
            break;
        sprite_decode_4bpp_planar(q + 2);
        i += (unsigned)(u8)q[34] * (unsigned)(u8)q[35] + 36;
    }
}

static void sprite_sheet_decode_indexed(char far *p)
{
    int i, n;
    unsigned far *t;
    char far *q;
    t = (unsigned far *)p;
    n = (*t >> 1) - 1;
    for (i = 0; i < n; i++) {
        q = p + *t;
        t++;
        if ((u8)*q == 0x47)
            sprite_decode_4bpp_planar(q + 2);
    }
}

/* ------------------------------------------------------------------ */
/* Little-endian script I/O (x86 DOS is little-endian: a raw fread     */
/* into a correctly sized C object already has the right value).      */
/* ------------------------------------------------------------------ */

static int rd_u8(u8 *v)  { return fread(v, 1, 1, inf) == 1; }
static int rd_i16(i16 *v){ return fread(v, 2, 1, inf) == 1; }
static int rd_u16(u16 *v){ return fread(v, 2, 1, inf) == 1; }
static int rd_u32(u32 *v){ return fread(v, 4, 1, inf) == 1; }

static void wr_u8(u8 v)   { fwrite(&v, 1, 1, outf); }
static void wr_i16(i16 v) { fwrite(&v, 2, 1, outf); }
static void wr_u16(u16 v) { fwrite(&v, 2, 1, outf); }

/* ------------------------------------------------------------------ */
/* Command handlers                                                    */
/* ------------------------------------------------------------------ */

static void do_seed_fb(void)
{
    u32 seed, x;
    unsigned r, c;
    char far *row;

    rd_u32(&seed);
    x = seed;
    for (r = 0; r < FB_ROWS; r++) {
        row = PEEKFP(ROW_TABLE_OFF + (u32)r * 4);
        for (c = 0; c < FB_STRIDE; c++) {
            x = x * 1103515245UL + 12345UL;   /* unsigned long: wraps mod 2^32 */
            row[c] = (u8)((x >> 16) & 0xFFu);
        }
    }
    POKEFP(QUEUE_PTR_OFF, queuebase);
    last_save_len = 0;
    last_ret = 0;
}

static void do_set_state(void)
{
    i16 result, gbc, g94, g96, g98, g9a;
    rd_i16(&result); rd_i16(&gbc); rd_i16(&g94); rd_i16(&g96); rd_i16(&g98); rd_i16(&g9a);
    POKEW(RESULT_OFF, result);
    POKEW(GBC_OFF, gbc);
    POKEW(CLIP94_OFF, g94);
    POKEW(CLIP96_OFF, g96);
    POKEW(CLIP98_OFF, g98);
    POKEW(CLIP9A_OFF, g9a);
}

static void do_set_font(void)
{
    i16 c0de, c0e2, c0e4, c0e6, lineh;
    u16 blen;
    rd_i16(&c0de); rd_i16(&c0e2); rd_i16(&c0e4); rd_i16(&c0e6); rd_i16(&lineh);
    rd_u16(&blen);
    if (blen)
        fread(font_buf, 1, blen, inf);
    POKEW(FONT_C0DE_OFF, c0de);
    POKEW(FONT_C0E2_OFF, c0e2);
    POKEW(FONT_C0E4_OFF, c0e4);
    POKEW(FONT_C0E6_OFF, c0e6);
    POKEW(FONT_C0E8_OFF, lineh);
    POKEW(FONT_C0E0_OFF, (int)fp_seg(font_buf));
}

static void do_call(void)
{
    u8 opid;
    i16 a[6];
    u16 blen;
    int k;

    rd_u8(&opid);
    for (k = 0; k < 6; k++)
        rd_i16(&a[k]);
    rd_u16(&blen);
    if (blen)
        fread(blob_buf, 1, blen, inf);

    nwords = 0;
    last_ret = 0;

    switch (opid) {
    case 0:  /* bar(x,y,n) */
        wi(a[0]); wi(a[1]); wi(a[2]);
        last_ret = (i16)ds_call(dgseg, FN(gfx_bar), nwords, words);
        break;
    case 1:  /* vline(x,y,n) */
        wi(a[0]); wi(a[1]); wi(a[2]);
        last_ret = (i16)ds_call(dgseg, FN(gfx_vline), nwords, words);
        break;
    case 2:  /* clear_rect(x,y,w,h) */
        wi(a[0]); wi(a[1]); wi(a[2]); wi(a[3]);
        last_ret = (i16)ds_call(dgseg, FN(gfx_clear_rect), nwords, words);
        break;
    case 3:  /* fill_rect(x,y,w,h) */
        wi(a[0]); wi(a[1]); wi(a[2]); wi(a[3]);
        last_ret = (i16)ds_call(dgseg, FN(gfx_fill_rect), nwords, words);
        break;
    case 4:  /* save_rect(x,y,w,h,buf) -- buf is our scratch save_buf */
        wi(a[0]); wi(a[1]); wi(a[2]); wi(a[3]); wf(save_buf);
        last_ret = (i16)ds_call(dgseg, FN(gfx_save_rect), nwords, words);
        {
            unsigned bpr = *(unsigned far *)save_buf;
            unsigned rows = *(unsigned far *)(save_buf + 2);
            u32 len = 4UL + (u32)bpr * rows;
            if (len > SAVE_BUF_BYTES)
                len = SAVE_BUF_BYTES;
            last_save_len = (u16)len;
        }
        break;
    case 5:  /* restore_rect(x,y,buf) -- blob is the buffer content (header+rows) */
        if (blen)
            fcopy(save_buf, blob_buf, blen);
        wi(a[0]); wi(a[1]); wf(save_buf);
        last_ret = (i16)ds_call(dgseg, FN(gfx_restore_rect), nwords, words);
        break;
    case 6:  /* wipe_rect(sx,sy,w,h,dx,dy) */
        wi(a[0]); wi(a[1]); wi(a[2]); wi(a[3]); wi(a[4]); wi(a[5]);
        last_ret = (i16)ds_call(dgseg, FN(gfx_wipe_rect), nwords, words);
        break;
    case 7:  /* copy_rect_flip_v(sx,sy,w,h,dx,dy) */
        wi(a[0]); wi(a[1]); wi(a[2]); wi(a[3]); wi(a[4]); wi(a[5]);
        last_ret = (i16)ds_call(dgseg, FN(gfx_copy_rect_flip_v), nwords, words);
        break;
    case 8:  /* copy_rect_flip_h(sx,sy,w,h,dx,dy) */
        wi(a[0]); wi(a[1]); wi(a[2]); wi(a[3]); wi(a[4]); wi(a[5]);
        last_ret = (i16)ds_call(dgseg, FN(gfx_copy_rect_flip_h), nwords, words);
        break;
    case 9:  /* copy_rect_flip_hv(sx,sy,w,h,dx,dy) */
        wi(a[0]); wi(a[1]); wi(a[2]); wi(a[3]); wi(a[4]); wi(a[5]);
        last_ret = (i16)ds_call(dgseg, FN(gfx_copy_rect_flip_hv), nwords, words);
        break;
    case 10: /* copy_rect_split(sx,sy,w,h,dx,dy) */
        wi(a[0]); wi(a[1]); wi(a[2]); wi(a[3]); wi(a[4]); wi(a[5]);
        last_ret = (i16)ds_call(dgseg, FN(gfx_copy_rect_split), nwords, words);
        break;
    case 11: /* copy_rect_split_flip_v(sx,sy,w,h,dx,dy) */
        wi(a[0]); wi(a[1]); wi(a[2]); wi(a[3]); wi(a[4]); wi(a[5]);
        last_ret = (i16)ds_call(dgseg, FN(gfx_copy_rect_split_flip_v), nwords, words);
        break;
    case 12: /* draw_char(x,y,glyph) -> advance */
        wi(a[0]); wi(a[1]); wi(a[2]);
        last_ret = (i16)ds_call(dgseg, FN(gfx_draw_char), nwords, words);
        break;
    case 13: /* blit_bitmap(x,y,bitmap) -- blob = 32-byte hdr + word(bpr,rows) + rows */
        wi(a[0]); wi(a[1]); wf(blob_buf);
        last_ret = (i16)ds_call(dgseg, FN(gfx_blit_bitmap), nwords, words);
        break;
    case 14: /* copy_rect(x,y,bitmap,flip) -- same blob layout as blit_bitmap */
        wi(a[0]); wi(a[1]); wf(blob_buf); wi(a[2]);
        last_ret = (i16)ds_call(dgseg, FN(gfx_copy_rect), nwords, words);
        break;
    case 15: /* blit_image(x,y,image) -- blob = bytes_per_row,rows,1bpp bits */
        wi(a[0]); wi(a[1]); wf(blob_buf);
        last_ret = (i16)ds_call(dgseg, FN(gfx_blit_image), nwords, words);
        break;
    case 16: /* set_pixel(x,y) */
        wi(a[0]); wi(a[1]);
        last_ret = (i16)ds_call(dgseg, FN(gfx_set_pixel), nwords, words);
        break;
    case 17: /* get_pixel(x,y) -> nibble */
        wi(a[0]); wi(a[1]);
        last_ret = (i16)ds_call(dgseg, FN(gfx_get_pixel), nwords, words);
        break;
    default:
        DIE("unknown CALL op\n");
    }
}

static void do_dump(void)
{
    unsigned r;
    char far *row;
    unsigned qoff, base, dlen;

    wr_u8(0xD0);
    for (r = 0; r < FB_ROWS; r++) {
        row = PEEKFP(ROW_TABLE_OFF + (u32)r * 4);
        fwrite(row, 1, FB_STRIDE, outf);
    }

    qoff = fp_off(PEEKFP(QUEUE_PTR_OFF));
    base = fp_off(queuebase);
    dlen = qoff - base;
    wr_u16(dlen);
    if (dlen)
        fwrite(queuebase, 1, dlen, outf);

    wr_i16(last_ret);
    wr_u16(last_save_len);
    if (last_save_len)
        fwrite(save_buf, 1, last_save_len, outf);
}

static void do_decode(void)
{
    u8 kind;
    u16 param, blen;
    int ret = 0;
    unsigned outlen = 0;
    char far *outp = dst_buf;

    rd_u8(&kind);
    rd_u16(&param);
    rd_u16(&blen);
    if (blen)
        fread(blob_buf, 1, blen, inf);

    switch (kind) {
    case 0:  /* rle_packbits_decode */
        ret = (int)rle_packbits_decode(blob_buf, dst_buf, param);
        outlen = (unsigned)ret;
        outp = dst_buf;
        break;
    case 1:  /* lz_decompress -- overwrites the FRONT of its src, so blob_buf must be fresh (it is: reloaded above) */
        ret = (int)lz_decompress(blob_buf, dst_buf, param);
        outlen = (unsigned)ret;
        outp = dst_buf;
        break;
    case 2:  /* sprite_decode_4bpp_planar -- in place */
        sprite_decode_4bpp_planar(blob_buf);
        ret = (int)blen;
        outlen = blen;
        outp = blob_buf;
        break;
    case 3:  /* sprite_decode_4bpp_mode13h -- in place */
        sprite_decode_4bpp_mode13h(blob_buf);
        ret = (int)blen;
        outlen = blen;
        outp = blob_buf;
        break;
    default:
        DIE("unknown DECODE kind\n");
    }

    wr_u8(0xD2);
    wr_i16((i16)ret);
    wr_u16(outlen);
    if (outlen)
        fwrite(outp, 1, outlen, outf);
}

/* src/RESOURCE.C resource_load_record, transcribed (file I/O only; the
 * gb40/gb3e retry-on-failure and dialog_run paths are omitted -- this
 * harness only ever reads the assets already known to be present).
 * display_mode is fixed at 4 (!= 5, != 2), matching the port's first
 * target mode. */
static void do_resource(void)
{
    u8 dir;
    u32 idx;
    int fd;
    i32 o1, o2;
    int s;
    u8 type, fl;
    char far *blob     = res_blob;
    char far *shadow_a = res_shadow_a;
    char far *shadow_b = res_shadow_b;

    rd_u8(&dir);
    rd_u32(&idx);

    fd = open(dir ? "AE001.DAT" : "AE000.DAT", 0x8004);
    if (fd < 0)
        DIE("cannot open archive\n");
    lseek(fd, (i32)idx * 4L, 0);
    read(fd, &o1, 4);
    read(fd, &o2, 4);
    s = (int)o2 - (int)o1;
    lseek(fd, o1, 0);
    read(fd, blob, s);
    close(fd);

    type = (u8)blob[0];
    fl = (u8)blob[1];
    s -= 2;

    if ((fl & 2) && (fl & 1)) {
        s = (int)lz_decompress(shadow_a, shadow_b, (u16)s);
        s = (int)rle_packbits_decode(shadow_b, shadow_a, (u16)s);
    } else if (fl & 2) {
        s = (int)lz_decompress(shadow_a, shadow_b, (u16)s);
        fcopy(shadow_a, shadow_b, (unsigned)s);
    } else if (fl & 1) {
        s = (int)rle_packbits_decode(shadow_a, shadow_b, (u16)s);
        fcopy(shadow_a, shadow_b, (unsigned)s);
    }

    switch (type) {
    case 0x47:
        sprite_decode_4bpp_planar(shadow_a);
        break;
    case 0:
        sprite_sheet_decode_sequential(shadow_a, (unsigned)s);
        break;
    case 1:
        sprite_sheet_decode_indexed(shadow_a);
        break;
    }

    /* `s` is the historical 16-bit `int` exactly as resource_load_record
     * computed it (including its (int)o2-(int)o1 truncation), and is
     * reported signed/as-is in the `s` field -- a negative value is a
     * legitimate golden result, not an error.  outlen/the data bytes that
     * follow describe only what was actually copied to shadow_a: 0 bytes
     * when s <= 0, never the 16-bit-wrapped view of a negative s (which
     * would falsely claim tens of KB of trailing data). */
    wr_u8(0xD1);
    wr_i16((i16)s);
    wr_u8(type);
    wr_u16(s > 0 ? (u16)s : 0);
    if (s > 0)
        fwrite(shadow_a, 1, (unsigned)s, outf);
}

/* ------------------------------------------------------------------ */
/* main                                                                 */
/* ------------------------------------------------------------------ */

int main(void)
{
    u8 magic[4];
    u8 op;

    setup();

    inf = fopen("ORACLE.IN", "rb");
    if (!inf)
        DIE("cannot open ORACLE.IN\n");
    outf = fopen("ORACLE.OUT", "wb");
    if (!outf)
        DIE("cannot open ORACLE.OUT\n");

    if (fread(magic, 1, 4, inf) != 4 ||
        magic[0] != 'O' || magic[1] != 'R' || magic[2] != 'C' || magic[3] != '1')
        DIE("bad ORACLE.IN magic\n");

    for (;;) {
        if (!rd_u8(&op))
            break;
        if (op == 0xFF)
            break;
        switch (op) {
        case 0x01: do_seed_fb();  break;
        case 0x02: do_set_state(); break;
        case 0x03: do_set_font(); break;
        case 0x04: do_call();     break;
        case 0x05: do_dump();     break;
        case 0x06: do_decode();   break;
        case 0x07: do_resource(); break;
        default:
            DIE("unknown opcode\n");
        }
    }

    fclose(inf);
    fclose(outf);
    return 0;
}
