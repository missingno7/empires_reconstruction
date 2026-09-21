/* startup.c -- portable replacement for src/STARTUP.C (F_4F63..F_53BF).
 *
 * The historical unit probes BIOS equipment, ROM signatures, INT 10h/15h
 * and the far heap to choose a display selector and a sound backend.  The
 * port keeps only the game-visible decisions:
 *
 *   - cmdline_parse_args(): the -E/-C/-T/-M/-V and -I/-S? switches, exactly
 *     as F_4F96 spells them;
 *   - video_mode_select(): defaults to selector 5 (VGA, AE000_002 runtime)
 *     -- the value the historical gate itself picks for a VGA machine with
 *     enough memory (`case 4: if (n >= 0x57720L) display_mode = 5;`); the
 *     farcoreleft() thresholds 0x3ADA0/0x57720/0x44620 are DOS capability
 *     checks and are retired (docs/current/portability-boundaries.md sec 10);
 *   - sound_backend_probe(): snd_backend_mode = 1 for selector 3, else 2
 *     (OPL present: the port always provides an OPL through Nuked-OPL3), or
 *     0 when -I / -SI asked for the speaker; mode 3 (the VGA-port probe that
 *     collapses to 1) is never selected.
 *
 * dos_write_handle2() keeps the historical strlen(text)-1 length.
 */
#include "game.h"

#include <stdio.h>

static int    s_argc;
static char **s_argv;

void startup_set_args(int argc, char **argv)
{
    s_argc = argc;
    s_argv = argv;
}

/* F_4F63 -- write to DOS handle 2 (stderr); the historical length is
 * strlen(text) - 1, i.e. it never printed the last character. */
void dos_write_handle2(dos_char *text)
{
    size_t n = strlen((const char *)text);
    if (n > 0)
        fwrite(text, 1, n - 1, stderr);
}

/* F_4F96 -- literal port. */
void cmdline_parse_args(void)
{
    dos_int i;
    for (i = 1; i < s_argc; i++) {
        if (s_argv[i][0] == '-' || s_argv[i][0] == '/') {
            switch (s_argv[i][1]) {
            case 'E': case 'e': display_mode = 1; break;
            case 'C': case 'c': display_mode = 2; break;
            case 'T': case 't': display_mode = 3; break;
            case 'M': case 'm': display_mode = 4; break;
            case 'V': case 'v': display_mode = 5; break;
            case 'I': case 'i': snd_backend_mode = 0; break;
            case 'S': case 's':
                switch (s_argv[i][2]) {
                case 'I': case 'i': snd_backend_mode = 0; break;
                case 'A': case 'a': snd_backend_mode = 2; break;
                case 'T': case 't': snd_backend_mode = 1; break;
                }
                break;
            }
        }
    }
}

/* F_50C1 -- BIOS equipment word: (floppy count bits) + 1.  The port reports
 * one drive; gbfcc only gates the historical A:/B: retry in
 * resource_file_open, which the port does not perform. */
void bios_equipment_probe(void)
{
    gbfcc = 1;
}

/* F_50D2 -- adapter detection.  PORT: the host always has "VGA". */
dos_int video_adapter_detect(void)
{
    display_mode = 5;
    b856 = 0;   /* the Tandy keypad-alias gate stays off (set only for selector 3) */
    return 0;
}

/* F_53BF -- see the file comment. */
dos_int sound_backend_probe(void)
{
    snd_backend_mode = 0;
    if (display_mode == 3) {
        snd_backend_mode = 1;
        return 0;
    }
    if (opl_detect() != 0) {
        snd_backend_mode = 2;
        return 0;
    }
    return 0;
}

/* F_520A -- mode selection.  Returns 0 when the game must not start. */
dos_int video_mode_select(void)
{
    dos_int i;
    /* The historical code stamps the boot drive letter into the three
     * archive path rows (ba22 == ga22 viewed as char[3][16]); the portable
     * loader ignores the drive prefix, so the rows are left as generated. */
    (void)i;
    bios_equipment_probe();
    video_adapter_detect();
    if (display_mode == 0) {
        dos_write_handle2(s859);
        return 0;
    }
    sound_backend_probe();
    cmdline_parse_args();
    /* PORT: farcoreleft() memory gate retired; display_mode stays as chosen. */
    return 1;
}

/* F_034F (src/VIDEO.C) -- historically `_AX=3; __int__(0x10);`, a BIOS
 * INT 10h AX=0003h call switching the adapter back to 80x25 text mode
 * before DOS exit.  PORT: no-op -- there is no BIOS text mode on the SDL3
 * target (docs/portable/architecture.md's "SDL3 sits below" pipeline owns
 * the window for the whole process lifetime), so this call has nothing
 * portable to do. */
void video_set_text_mode(void)
{
}
