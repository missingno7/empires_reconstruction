/* src/OPLREG.C: OPL register writes and chip detection.
   One translation unit; the sections below were the separate member
   sources of grouped module C_E095_E54D and keep their original ids. */

#include "GC91B.H"
#include "SOUND.H"

extern void voice_apply_field();
extern void voice_program_all();
extern void voice_load_instrument();
extern void opl_set_depth_flags();
extern void opl_set_note_select();
extern void voice_write_level();
extern void voice_write_connection();
extern void voice_write_op1_attack_decay();
extern void voice_write_op2_attack_decay();
extern void voice_write_envelope_flags();
extern void voice_write_waveform();
extern void opl_register_write();
extern char voice_level_table[];                    /* DS:CA50 */
extern char voice_byte_table[];                    /* DS:2FE4, byte per voice */
extern char opl_note_select;                      /* DS:C6B4 */
extern char g2ff6[];                    /* DS:2FF6, byte per voice */
extern char g3008[];                    /* DS:3008, byte per voice */
extern char opl_am_depth;                      /* DS:C6AA */
extern char opl_vib_depth;                      /* DS:C6B5 */
extern int opl_enabled;                       /* DS:CA22 */
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
extern int inport();

/* ---- F_E095 (original code at 0xE095) ---- */
/* F_E095 -- store one byte into the voice's 14-byte record at a runtime
   member index and re-emit that member through voice_apply_field. */
void voice_set_field(v, k, val)
int v;
int k;
char val;
{
    voice_param_record[v].f[k] = val;
    voice_apply_field(v, k);
}


/* ---- F_E0C0 (original code at 0xE0C0) ---- */
/* F_E0C0 -- copy 13 bytes out of a caller's stride-2 table into the voice's
   14-byte record, then store the masked mode in the record's last byte and
   re-arm the voice through voice_program_all. */
void voice_load_instrument(v, q, w)
int v;
char far *q;
int w;
{
    char far *p;
    register int i;

    for (i = 0, p = (char far *)&voice_param_record[v]; i < 13; i++) {
        *p = *q;
        q += 2;
        p++;
    }
    *p = w &= 3;
    voice_program_all(v);
}


/* ---- F_E114 (original code at 0xE114) ---- */
void voice_load_instrument_words(v, q, w)
int v;
char far *q;
int w;
{
    int buf[14];
    register int i;
    for (i = 0; i < 13; i++)
        buf[i] = *q++;
    voice_load_instrument(v, (char far *)buf, w);
}


/* ---- F_E151 (original code at 0xE151) ---- */
/* F_E151 -- unit-order dispatch.  Plain C switch: TC 2.01 emits the 18-entry
   word table into _TEXT right after the jmp word ptr cs:[bx+t], and lays the
   case bodies out in source order, which is what fixes the case order here. */
void voice_apply_field(u, k)
int u, k;
{
    switch (k) {
    case 14: case 15: case 17:
        opl_set_depth_flags(); break;
    case 16:
        opl_set_note_select(); break;
    case 0: case 8:
        voice_write_level(u); break;
    case 2: case 12:
        voice_write_connection(u); break;
    case 3: case 6:
        voice_write_op1_attack_decay(u); break;
    case 4: case 7:
        voice_write_op2_attack_decay(u); break;
    case 1: case 5: case 9: case 10: case 11:
        voice_write_envelope_flags(u); break;
    case 13:
        voice_write_waveform(u); break;
    }
}


/* ---- F_E1C4 (original code at 0xE1C4) ---- */
/* F_E1C4 -- re-arm one voice: reset the two shared registers, then push every
   one of the voice's six parameter groups to the OPL in a fixed order. */
void voice_program_all(v)
register int v;
{
    opl_set_depth_flags();
    opl_set_note_select();
    voice_write_level(v);
    voice_write_connection(v);
    voice_write_op1_attack_decay(v);
    voice_write_op2_attack_decay(v);
    voice_write_envelope_flags(v);
    voice_write_waveform(v);
}


/* ---- F_E1F2 (original code at 0xE1F2) ---- */
/* F_E1F2 -- scale one record's bar into the 0..0x3F range, pack the record's
   first byte above it, and hand the pair to opl_register_write.  Entry 1E2F2, 112 bytes.

   E22B  33D2 F7F3           `xor dx,dx; div bx` is the UNSIGNED divide, so
              the running value is an unsigned int; a signed int would have
              been `cwd; idiv` and a long a runtime helper.  The same reading
              covers `mul si` at E21B.
   E21F  8BC6 057F00 03F0    `v += v + 0x7f`: the right side is computed whole
              into AX and then added back, which is why v appears twice.
   E232  BA3F00 2BD0 8BF2    `0x3f - v / 0xfe` has to borrow DX because the
              quotient already occupies AX; the same subtraction at E211,
              where AX is free, goes straight into SI.
   E216 / E250  voice_level_table[i] and voice_byte_table[i] are plain char arrays indexed by the
              register parameter, so both fold to a single near DS
              displacement (0xCA50 written as di-0x35b0), while voice_param_record[i].f8
              at E203 keeps the far form -- the scalar-element vs
              struct-member boundary F_E372 and F_D8F0 also carry. */

void voice_write_level(i)
register int i;
{
    register unsigned v;

    v = 0x3f - (voice_param_record[i].f[8] & 0x3f);
    v = voice_level_table[i] * v;
    v += v + 0x7f;
    v = 0x3f - v / 0xfe;
    v |= voice_param_record[i].f[0] << 6;
    opl_register_write(voice_byte_table[i] + 0x40, v);
}


/* ---- F_E262 (original code at 0xE262) ---- */
/* F_E262 -- write OPL register 0x08 with bit 6 set from the flag at DS:C6B4.
   `v = cond ? K : 0` writes the destination register straight (rule 9); here
   the destination is the argument slot. */
void opl_set_note_select()
{
    opl_register_write(8, opl_note_select ? 0x40 : 0);
}


/* ---- F_E27B (original code at 0xE27B) ---- */
/* F_E27B -- push one voice's pitch/flag word to the OPL through opl_register_write.
   The 14-byte record array at DS:C91B is indexed with a `mul`, which is how
   TC 2.0 reaches a non-power-of-two stride; the two member reads recompute
   the address independently (TC 2.0 does no CSE, tc20-codegen rule 6). */

void voice_write_connection(v)
register int v;
{
    register int d;

    if (g2ff6[v]) return;
    d = voice_param_record[v].f[2] * 2;
    d |= voice_param_record[v].f[12] ? 0 : 1;
    opl_register_write(g3008[v] + 0xc0, d);
}


/* ---- F_E2D6 (original code at 0xE2D6) ---- */
/* F_E2D6 -- twin of F_E324: members 3 and 6, OPL base 0x60. */

void voice_write_op1_attack_decay(v)
int v;
{
    register int d;

    d = voice_param_record[v].f[3] << 4;
    d |= voice_param_record[v].f[6] & 0xf;
    opl_register_write(voice_byte_table[v] + 0x60, d);
}


/* ---- F_E324 (original code at 0xE324) ---- */
/* F_E324 -- program the voice's carrier level/multiplier pair (OPL base 0x80)
   from members 4 and 7 of the 14-byte record.  Twin of F_E2D6, which does the
   same with members 3 and 6 and OPL base 0x60. */

void voice_write_op2_attack_decay(v)
int v;
{
    register int d;

    d = voice_param_record[v].f[4] << 4;
    d |= voice_param_record[v].f[7] & 0xf;
    opl_register_write(voice_byte_table[v] + 0x80, d);
}


/* ---- F_E372 (original code at 0xE372) ---- */
/* F_E372 -- fold five fields of a 14-byte record into one flag word and hand
   it, with the record's own glyph, to opl_register_write.  Entry 1E472, 174 bytes.

   E377  8B7E04              the parameter is copied into DI, so it is a
                              register variable; SI takes the running sum,
                              which means TC 2.0 gave SI to the BODY's
                              register local and DI to the register PARAMETER.
   E37A  BA0E00 F7E2 8BD8 81C31BC9 1E 07 26807F0900
                             voice_param_record[i].f9 -- a struct member through an array
                              with a runtime index, so the far pointer is
                              materialised OFFSET first (add bx,offset;
                              push ds; pop es), the shape F_656C's second
                              attempt established.
   E38E  7405 BE8000 EB02 33F6
                             `v = cond ? 0x80 : 0` writes the destination
                              REGISTER directly in both arms.  The four that
                              follow are `v += cond ? K : 0` and go through AX
                              (mov ax,K / xor ax,ax) and then `add si,ax`.
   E40E  8A85E42F            voice_byte_table[i] is a plain char array indexed by the
                              register: TC folds base+index into ONE near DS
                              displacement and materialises no segment at all.
                              That is the same folding that made F_49F0's
                              first attempt 12 bytes short -- it happens for a
                              scalar array element and not for a struct
                              member. */

void voice_write_envelope_flags(i)
register int i;
{
    register int v;

    v = voice_param_record[i].f[9] ? 0x80 : 0;
    v += voice_param_record[i].f[10] ? 0x40 : 0;
    v += voice_param_record[i].f[5] ? 0x20 : 0;
    v += voice_param_record[i].f[11] ? 0x10 : 0;
    v += voice_param_record[i].f[1] & 0xf;
    opl_register_write(voice_byte_table[i] + 0x20, v);
}


/* ---- F_E420 (original code at 0xE420) ---- */
/* F_E420 -- write OPL register 0xBD, ORing the two flags at DS:C6AA and
   DS:C6B5 into it.  Rule 9 both ways: the plain `= cond ? K : 0` writes the
   register variable straight, and the `|= cond ? K : 0` routes through AX
   first. */

void opl_set_depth_flags()
{
    register int v;

    v = opl_am_depth ? 0x80 : 0;
    v |= opl_vib_depth ? 0x40 : 0;
    opl_register_write(0xbd, v);
}


/* ---- F_E44B (original code at 0xE44B) ---- */
/* F_E44B -- the last of the per-voice OPL writes (base 0xE0): the record's
   member 13, masked, but only while the enable word at DS:CA22 is set. */

void voice_write_waveform(v)
int v;
{
    register int d;

    if (opl_enabled) d = voice_param_record[v].f[13] & 3;
    else d = 0;
    opl_register_write(voice_byte_table[v] + 0xe0, d);
}


/* ---- F_E48A (original code at 0xE48A) ---- */
/* F_E48A -- program one OPL voice from the note tables.  Plain C.  The high
   byte of the fetched word is read at [bp-1] with CBW, i.e. as a signed char
   member of a union, not as (t >> 8). */

void voice_set_frequency(v, note, flag)
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
    opl_register_write(v + 0xa0, t.w);
    d = flag ? 0x20 : 0;
    d += tab_oct[note] * 4 + (t.b[1] & 3);
    opl_register_write(v + 0xb0, d);
}


/* ---- F_E52A (original code at 0xE52A) ---- */
/* F_E52A -- silence one voice: zero its two OPL key/level registers. */

void voice_key_off(v)
int v;
{
    opl_register_write(v + 0xb0, 0);
    opl_register_write(v + 0xa0, 0);
}


/* ---- F_E54D (original code at 0xE54D) ---- */
/* F_E54D -- is an OPL chip there?  Reset both timers, read the status, arm
   timer 1, spin 200 port reads to let it expire, read the status again and
   reset.  The two register variables are declared counter-first (rule 12:
   SI then DI in DECLARATION order, not in order of first use).  The verdict
   is a plain `&&`, which is why both arms end at one EB00. */

int opl_detect()
{
    unsigned t;
    register unsigned i;
    register unsigned d;

    opl_register_write(4, 0x60);
    opl_register_write(4, 0x80);
    d = inport(opl_port);
    opl_register_write(2, 0xff);
    opl_register_write(4, 0x21);
    for (i = 0; i < 200; i++)
        inport(opl_port);
    t = inport(opl_port);
    opl_register_write(4, 0x60);
    opl_register_write(4, 0x80);
    return (!(d & 0xe0) && (t & 0xe0) == 0xc0);
}
