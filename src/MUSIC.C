/* src/MUSIC.C: music arithmetic module (M_DDD9_DF98), one pure Turbo C
   translation unit: note-to-divisor, octave table, tuning-table reset and the
   cached voice-frequency lookup (fdf98). No inline asm: the native compiler
   object emits its FIXUPP subrecords descending, which reproduces the
   historical relocation order (see docs/current/relocation-topology.md).

   Merged translation unit for M_DDD9_DF98 (music arithmetic module).
   Combines the four reference sources (src/F_DDD9.C, src/F_DE7E.C,
   src/F_DEFA.C, src/F_DF98.C) into one ordinary Turbo C unit, in the same
   top-to-bottom order the functions appear at ascending load addresses in
   asm/MUSIC.ASM: music_note_to_divisor, music_build_octave_table,
   music_reset_tuning_tables, fdf98. Public/extern names renamed to match the
   module's actual public symbols so this is a single self-contained TU
   instead of four separately-bound objects. */

#define EMPIRES_SHARED_ARITHMETIC_MODULE 1

extern char far *gca24[];               /* DS:CA24 */
extern unsigned gca6d[];                /* DS:CA6D */
extern unsigned char gc5ea[];           /* DS:C5EA */
extern unsigned char gc64a[];           /* DS:C64A */
extern unsigned char gc6c3[][24];       /* DS:C6C3 */

long music_note_to_divisor(a, b)
int a;
int b;
{
    long t;
    long u;
    long l;

    l = (long) (b * 100);
    t = ((long) (a * 6) + l) * 52088L;
    t = t / (l * 25);
    u = t << 14;
    u = u * 9;
    u = u / 111875L;
    return (u);
}

void music_build_octave_table(p, a, b)
unsigned far *p;
int a;
int b;
{
    long l;
    register int i;

    *p = ((unsigned) (l = music_note_to_divisor(a, b)) + 4) >> 3;
    p++;
    for (i = 1; i < 12; i++) {
        l = l * 106;
        *p = ((unsigned) (l = l / 100) + 4) >> 3;
        p++;
    }
}

void music_reset_tuning_tables()
{
    unsigned r;
    unsigned c;
    unsigned a;
    unsigned w;
    register unsigned p;
    register unsigned i;

    w = 4;
    for (a = i = 0; i < 25; i++, a += w)
        music_build_octave_table(gc6c3[i], a, 100);
    for (r = 0; r < 11; r++) {
        gca24[r] = gc6c3[0];
        gca6d[r] = 0;
    }
    for (p = 0, r = 0; r < 8; r++)
        for (c = 0; c < 12; c++, p++) {
            gc5ea[p] = r;
            gc64a[p] = c;
        }
}

extern int music_tempo_scaled;               /* DS:CA6B -- music_tempo_scaled */
extern long g3752;              /* DS:3752/3754 -- last computed key (cache) */
extern int gc5e4;               /* DS:C5E4 -- cached octave value */
extern char far *gc5e6;         /* DS:C5E6/C5E8 -- cached far pointer (offset/segment) */

/* F_DF98 -- complete far-call and state update routine. */
void fdf98(i, val)
int i;
int val;
{
    int t;
    long l;
    register int r;
    register int d;

    l = (long) (val - 8192) * music_tempo_scaled;
    if (g3752 == l) {
        gca24[i] = gc5e6;
        gca6d[i] = gc5e4;
        return;
    }
    d = (int) (l / 8192L);
    if (d < 0) {
        t = 24 - d;
        gc5e4 = gca6d[i] = -(t / 25);
        r = (t - 24) % 25;
        if (r)
            r = 25 - r;
        goto finish;
    }
    gc5e4 = gca6d[i] = d / 25;
    r = d % 25;
finish:
    gc5e6 = gca24[i] = gc6c3[r];
    g3752 = l;
}
