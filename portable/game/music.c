/* music.c -- ported from src/MUSIC.C (music arithmetic module: note-to-
 * divisor, octave table, tuning-table reset, cached voice-frequency
 * lookup).  Pure arithmetic, no hardware fragments.
 *
 * gc6c3 is generated as `dos_uchar gc6c3[25][24]` (portable/generated/
 * game_state.h), matching the historical `unsigned char gc6c3[][24]`
 * (src/MUSIC.C:22): gc6c3[n] indexes naturally, same as the original.
 */
#include "game.h"

dos_long music_note_to_divisor(dos_int a, dos_int b)
{
    dos_long t;
    dos_long u;
    dos_long l;

    l = (dos_long)(b * 100);
    t = ((dos_long)(a * 6) + l) * 52088L;
    t = t / (l * 25);
    u = t << 14;
    u = u * 9;
    u = u / 111875L;
    return (u);
}

void music_build_octave_table(dos_uint *p, dos_int a, dos_int b)
{
    dos_long l;
    dos_int i;

    *p = ((dos_uint)(l = music_note_to_divisor(a, b)) + 4) >> 3;
    p++;
    for (i = 1; i < 12; i++) {
        l = l * 106;
        *p = ((dos_uint)(l = l / 100) + 4) >> 3;
        p++;
    }
}

void music_reset_tuning_tables(void)
{
    dos_uint r;
    dos_uint c;
    dos_uint a;
    dos_uint w;
    dos_uint p;
    dos_uint i;

    w = 4;
    for (a = i = 0; i < 25; i++, a += w)
        /* PORT: gc6c3[i] is dos_uchar[24] (unsigned); cast to dos_uint *
           for the word-stride table build, same as the historical
           unsigned-char-array-passed-as-unsigned-far-pointer call. */
        music_build_octave_table((dos_uint *)gc6c3[i], a, 100);
    for (r = 0; r < 11; r++) {
        gca24[r] = (dos_char *)gc6c3[0];
        gca6d[r] = 0;
    }
    for (p = 0, r = 0; r < 8; r++)
        for (c = 0; c < 12; c++, p++) {
            gc5ea[p] = r;
            gc64a[p] = c;
        }
}

/* F_DF98 -- complete far-call and state update routine. */
void music_voice_frequency_lookup(dos_int i, dos_int val)
{
    dos_int t;
    dos_long l;
    dos_int r;
    dos_int d;

    l = (dos_long)(val - 8192) * music_tempo_scaled;
    if (g3752 == l) {
        gca24[i] = gc5e6;
        gca6d[i] = gc5e4;
        return;
    }
    d = (dos_int)(l / 8192L);
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
    gc5e6 = gca24[i] = (dos_char *)gc6c3[r];
    g3752 = l;
}
