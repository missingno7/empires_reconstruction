/* oplreg.c -- ported from src/OPLREG.C (OPL register writes and chip
 * detection).
 *
 * PORT: src/OPLREG.C's own opl_register_write() (port-I/O based) and
 * opl_detect() (chip-presence probe: register writes, a 200-iteration
 * settling-read spin loop, register writes again) are HARDWARE fragments
 * per tu-porting-rules.md sec 5 ("opl_register_write, opl_detect, port I/O
 * -> sound.h (opl_write(reg,val), opl_detect() returns 1)").  Neither is
 * ported here: sound.h/opl_write.c supplies both under the new names, and
 * game_funcs.h's own OPLREG.C prototype list omits opl_detect() for exactly
 * this reason (see sound.h's header comment).  The 200-iteration spin loop
 * and the two bracketing register writes it needs are therefore not
 * reproduced anywhere in this file: PORT: removed -- the settling reads
 * were an OPL bus requirement (let the just-written register latch before
 * the chip's status port is read back), which is a real-hardware timing
 * concern the software backend (Nuked-OPL3 or similar) does not have.
 *
 * Every OTHER function in the file is ported as-is, with every historical
 * opl_register_write(reg, val) call rewritten to opl_write(reg, val) (the
 * new backend primitive; sound.h's comment is explicit that this is a
 * different thing from the retired *timed* asm/SOUND.ASM entry point of
 * the same almost-name, opl_register_write, which this project keeps
 * declared in sound.h only for asm/SOUND.ASM's own C callers elsewhere).
 */
#include "game.h"

/* ---- F_E095 (original code at 0xE095) ---- */
/* F_E095 -- store one byte into the voice's 14-byte record at a runtime
   member index and re-emit that member through voice_apply_field. */
void voice_set_field(dos_int v, dos_int k, dos_char val)
{
    voice_param_record[v].f[k] = val;
    voice_apply_field(v, k);
}


/* ---- F_E0C0 (original code at 0xE0C0) ---- */
/* F_E0C0 -- copy 13 bytes out of a caller's stride-2 table into the voice's
   14-byte record, then store the masked mode in the record's last byte and
   re-arm the voice through voice_program_all. */
void voice_load_instrument(dos_int v, dos_char *q, dos_int w)
{
    dos_char *p;
    dos_int i;

    for (i = 0, p = (dos_char *)&voice_param_record[v]; i < 13; i++) {
        *p = *q;
        q += 2;
        p++;
    }
    *p = w &= 3;
    voice_program_all(v);
}


/* ---- F_E114 (original code at 0xE114) ---- */
void voice_load_instrument_words(dos_int v, dos_char *q, dos_int w)
{
    /* PORT: local renamed from the historical `buf` to `word_buf` -- DS:BFEE
       already names an unrelated global `buf` (portable/generated/
       game_state.h), and the historical local would otherwise shadow it
       (MSVC /W4 C4459). */
    dos_int word_buf[14];
    dos_int i;

    for (i = 0; i < 13; i++)
        word_buf[i] = *q++;
    voice_load_instrument(v, (dos_char *)word_buf, w);
}


/* ---- F_E151 (original code at 0xE151) ---- */
/* F_E151 -- unit-order dispatch.  Plain C switch: TC 2.01 emits the 18-entry
   word table into _TEXT right after the jmp word ptr cs:[bx+t], and lays the
   case bodies out in source order, which is what fixes the case order here. */
void voice_apply_field(dos_int u, dos_int k)
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
void voice_program_all(dos_int v)
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
   first byte above it, and hand the pair to opl_write.  Entry 1E2F2, 112
   bytes. */
void voice_write_level(dos_int i)
{
    dos_uint v;

    v = 0x3f - (voice_param_record[i].f[8] & 0x3f);
    v = voice_level_table[i] * v;
    v += v + 0x7f;
    /* int-semantics-inventory.md fact 12 (src/OPLREG.C:161): the historical
       divide is UNSIGNED (`xor dx,dx; div bx`), so v stays dos_uint; the
       explicit dos_u16() truncation on the division result documents that
       fact rather than relying on assignment truncation alone. */
    v = 0x3f - dos_u16(v / 0xfeu);
    v |= voice_param_record[i].f[0] << 6;
    opl_write(voice_byte_table[i] + 0x40, v);
}


/* ---- F_E262 (original code at 0xE262) ---- */
/* F_E262 -- write OPL register 0x08 with bit 6 set from the flag at DS:C6B4. */
void opl_set_note_select(void)
{
    opl_write(8, opl_note_select ? 0x40 : 0);
}


/* ---- F_E27B (original code at 0xE27B) ---- */
/* F_E27B -- push one voice's pitch/flag word to the OPL through opl_write. */
void voice_write_connection(dos_int v)
{
    dos_int d;

    if (g2ff6[v]) return;
    d = voice_param_record[v].f[2] * 2;
    d |= voice_param_record[v].f[12] ? 0 : 1;
    opl_write(g3008[v] + 0xc0, d);
}


/* ---- F_E2D6 (original code at 0xE2D6) ---- */
/* F_E2D6 -- twin of F_E324: members 3 and 6, OPL base 0x60. */
void voice_write_op1_attack_decay(dos_int v)
{
    dos_int d;

    d = voice_param_record[v].f[3] << 4;
    d |= voice_param_record[v].f[6] & 0xf;
    opl_write(voice_byte_table[v] + 0x60, d);
}


/* ---- F_E324 (original code at 0xE324) ---- */
/* F_E324 -- program the voice's carrier level/multiplier pair (OPL base
   0x80) from members 4 and 7 of the 14-byte record.  Twin of F_E2D6, which
   does the same with members 3 and 6 and OPL base 0x60. */
void voice_write_op2_attack_decay(dos_int v)
{
    dos_int d;

    d = voice_param_record[v].f[4] << 4;
    d |= voice_param_record[v].f[7] & 0xf;
    opl_write(voice_byte_table[v] + 0x80, d);
}


/* ---- F_E372 (original code at 0xE372) ---- */
/* F_E372 -- fold five fields of a 14-byte record into one flag word and hand
   it, with the record's own glyph, to opl_write.  Entry 1E472, 174 bytes. */
void voice_write_envelope_flags(dos_int i)
{
    dos_int v;

    v = voice_param_record[i].f[9] ? 0x80 : 0;
    v += voice_param_record[i].f[10] ? 0x40 : 0;
    v += voice_param_record[i].f[5] ? 0x20 : 0;
    v += voice_param_record[i].f[11] ? 0x10 : 0;
    v += voice_param_record[i].f[1] & 0xf;
    opl_write(voice_byte_table[i] + 0x20, v);
}


/* ---- F_E420 (original code at 0xE420) ---- */
/* F_E420 -- write OPL register 0xBD, ORing the two flags at DS:C6AA and
   DS:C6B5 into it. */
void opl_set_depth_flags(void)
{
    dos_int v;

    v = opl_am_depth ? 0x80 : 0;
    v |= opl_vib_depth ? 0x40 : 0;
    opl_write(0xbd, v);
}


/* ---- F_E44B (original code at 0xE44B) ---- */
/* F_E44B -- the last of the per-voice OPL writes (base 0xE0): the record's
   member 13, masked, but only while the enable word at DS:CA22 is set. */
void voice_write_waveform(dos_int v)
{
    dos_int d;

    if (opl_enabled) d = voice_param_record[v].f[13] & 3;
    else d = 0;
    opl_write(voice_byte_table[v] + 0xe0, d);
}


/* ---- F_E48A (original code at 0xE48A) ---- */
/* F_E48A -- program one OPL voice from the note tables.  Plain C.  The high
   byte of the fetched word is read at [bp-1] with CBW, i.e. as a signed char
   member of a union, not as (t >> 8); ported here via dos_rd16/dos_hi8
   (portable/resource/archive.c's established idiom: "never reinterpret
   buffers through wider pointer types") instead of the historical
   union{int w; char b[2];} punning trick -- same little-endian byte split,
   no local typedef needed. */
void voice_set_frequency(dos_int v, dos_int note, dos_int flag)
{
    dos_uint w;
    dos_int d;

    tab_flag[v] = flag;
    gc6b6[v] = note;
    /* PORT / int-semantics gap (not currently in
       docs/portable/int-semantics-inventory.md): tab_bias (== gca6d) is
       generated as dos_uint in portable/generated/game_state.h, but
       src/MUSIC.C's music_voice_frequency_lookup stores NEGATIVE values
       into it (`gca6d[i] = -(t / 25)`) and this file's own historical
       extern declared it `int tab_bias[]` (signed) -- on the 8086 the
       write (as unsigned) and this read (as signed) agree bit-for-bit
       because both are exactly 16 bits.  A dos_uint promotes to a
       NONNEGATIVE (32-bit) int under modern C promotion rules, so a bare
       `note += tab_bias[v]` would add the wrong large positive value
       instead of reproducing the historical 16-bit wraparound ADD.  The
       explicit (dos_int) reinterpret-cast below recovers it; see this
       file's port report. */
    note += (dos_int)tab_bias[v];
    if (note > 0x5f) note = 0x5f;
    if (note < 0) note = 0;
    w = dos_rd16((const uint8_t *)(gca24[v] + tab_ix[note] * 2));
    opl_write(v + 0xa0, w);
    d = flag ? 0x20 : 0;
    d += tab_oct[note] * 4 + (dos_hi8(w) & 3);
    opl_write(v + 0xb0, d);
}


/* ---- F_E52A (original code at 0xE52A) ---- */
/* F_E52A -- silence one voice: zero its two OPL key/level registers. */
void voice_key_off(dos_int v)
{
    opl_write(v + 0xb0, 0);
    opl_write(v + 0xa0, 0);
}

/* F_E54D (opl_detect) intentionally not ported here -- see file header. */
