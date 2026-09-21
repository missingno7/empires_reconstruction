/*@FLAGS -B */
/* F_520A -- video-mode selection and the memory gate.  Plain C; the module
   is on the TASM path (runD: the 5039..52AB region), which is what shortens
   the two forward jmps at 529E and 52AB and pads them with NOP. */
/*@PUB _video_mode_select*/
extern void dos_write_handle2(char far *);
extern void cmdline_parse_args(void), bios_equipment_probe(void);
extern int f50d2(), f53bf();
extern char getdisk();
extern long farcoreleft();
extern unsigned char _osmajor;
extern char far * far *_argv;
extern char display_mode;
extern char ba22[3][16];
extern char s859[], s8a8[];

int video_mode_select()
{
    char c;
    long n;
    int i;

    if (_osmajor >= 3 && _argv[0][1] == ':')
        c = _argv[0][0];
    else
        c = getdisk() + 0x41;
    for (i = 0; i < 3; i++)
        ba22[i][0] = c;
    bios_equipment_probe();
    f50d2();
    if (display_mode == 0) {
        dos_write_handle2(s859);
        return 0;
    }
    f53bf();
    cmdline_parse_args();
    if ((n = farcoreleft()) < 0x3ada0L) {
        dos_write_handle2(s8a8);
        return 0;
    }
    switch (display_mode) {
    case 5:
        if (n < 0x57720L) display_mode = 1;
    case 1:
    case 3:
        if (n < 0x44620L) display_mode = 2;
        break;
    case 4:
        if (n >= 0x57720L) display_mode = 5;
        else if (n < 0x44620L) display_mode = 2;
        break;
    }
    return 1;
}
