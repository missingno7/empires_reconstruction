/* src/STARTUP.C: DOS/BIOS startup: command line, equipment and video detection, mode setup.
   One translation unit; the sections below were the separate member
   sources of grouped module C_4F63_520A and keep their original ids. */

void __int__(int);
extern char far * far *_argv;
#include "SOUND.H"
extern char display_mode;

/* ---- F_4F63 (original code at 0x4F63) ---- */
/* Exact 51-byte DOS handle-2 writer using Turbo C register pseudo-variables and __int__; no inline ASM or byte emission.
   Preserve the historical strlen(text)-1 length and DS load exactly. */
extern unsigned strlen(const char far *);
void dos_write_handle2(char far *text)
{
 unsigned segment;
 register unsigned length,offset;
 length=strlen(text)-1;
 offset=(unsigned)text;
 segment=(unsigned)((unsigned long)text>>16);
 _BX=2;_CX=length;_DX=offset;_AH=0x40;_DS=segment;
 __int__(0x21);
}


/* ---- F_4F96 (original code at 0x4F96) ---- */
/* Exact command-line parser. Turbo C -B passes generated assembly to TASM;
 * this naturally reproduces the original switch and branch encodings. */
extern int _argc;
void cmdline_parse_args(void)
{
 register int i;
 for(i=1;i<_argc;i++) {
  if(_argv[i][0]=='-' || _argv[i][0]=='/') {
   switch(_argv[i][1]) {
    case 'E':case 'e':display_mode=1;break;
    case 'C':case 'c':display_mode=2;break;
    case 'T':case 't':display_mode=3;break;
    case 'M':case 'm':display_mode=4;break;
    case 'V':case 'v':display_mode=5;break;
    case 'I':case 'i':snd_backend_mode=0;break;
    case 'S':case 's':
     switch(_argv[i][2]) {
      case 'I':case 'i':snd_backend_mode=0;break;
      case 'A':case 'a':snd_backend_mode=2;break;
      case 'T':case 't':snd_backend_mode=1;break;
     }
     break;
   }
  }
 }
}


/* ---- F_50C1 (original code at 0x50C1) ---- */
/* F_50C1 -- read the BIOS equipment word and store (bits 6-5) + 1 at DS:BFCC.
   The int 11h is inline asm and the arithmetic is C through the `_AX`
   pseudo-variable; no frame, because there is no parameter and no local
   (rule 11), and the body names neither SI nor DI (rule 14). */
extern unsigned char gbfcc;             /* DS:BFCC */

void bios_equipment_probe()
{
    asm xor ax,ax
    asm int 11h
    gbfcc = ((_AX & 0xc0) >> 6) + 1;
}


/* ---- F_50D2 (original code at 0x50D2) ---- */
/* F_50D2 video adapter detection and F_53BF sound-hardware probe (formerly
   src/VIDDET.C, historically asm/M_50D2_53BF.ASM), recovered as C with Turbo C 2.0 pseudo-registers and the
   __int__/__inportb__/__outportb__ intrinsics; the remaining inline asm is
   the irreducible core:
     - `asm mov display_mode,N` (byte store without `byte ptr`): the original
       bytes are C6 06 xxxx imm 90 -- TASM sizes the forward-referenced EXTRN
       as a word on pass 1 and pads the corrected byte store with a NOP.  A C
       assignment (see VIDMODE.C) compiles to the clean 5-byte store, so only
       an inline-asm store inside a TCC-generated unit (whose EXTRNs come last)
       reproduces the NOP -- positive evidence of inline asm in a .C file;
     - `cmp byte ptr es:[si],..` ROM-signature probes (no C far-pointer idiom
       addresses ES:SI without a displacement);
     - `cmp bl,0ah / jl`: _BL is unsigned, C emits jb;
     - `xchg ah,al`, `loop` and the flag tests straight after INT 15h;
     - the forward `jmp l_done`/`jmp l_fin` sites, which the original left as
       short jumps padded with NOP (the same two-pass artifact as the stores),
       where a C goto compiles to an unpadded short jump;
     - the `cmp word ptr display_mode,N / jne` ladder at l_done: `if (x != N)
       goto L` compiles to an inverted `je $+3 / jmp L` pair (+2 bytes each),
       while the sound probe's single `== 3` test is C (`*(int *)&display_mode`
       reads the same word compare).
   Probed byte-exact alone and inside the STARTUP unit (tools/probe_tu.py). */
/* DS:BFCD, the display mode (VIDMODE.C writes it) -- declared at top of file */
extern char b856;
extern int opl_detect();
unsigned char __inportb__(int);
void __outportb__(int, unsigned char);

video_adapter_detect()
{
    asm mov display_mode,0
    _AH = 0x0f;
    __int__(0x10);
    if (_AL != 7) {
        _ES = 0xf000;
        _SI = 0xfffe;
        asm cmp byte ptr es:[si],0ffh
        asm jne l_pc
        _SI = 0xc000;
        asm cmp byte ptr es:[si],21h
        asm jne l_pc
        asm mov display_mode,3
        b856 = 1;
        goto l_done;
l_pc:
        _AX = 0x1a00;
        __int__(0x10);
        if (_AL == 0x1a) {
            asm cmp bl,0ah
            asm jl l_check_ega_class
            asm mov display_mode,4
            asm jmp l_done
l_check_ega_class:
            if (_BL == 4 || _BL == 5) { asm mov display_mode,1; asm jmp l_done ; }
            if (_BL == 7 || _BL == 8) { asm mov display_mode,5; asm jmp l_done ; }
            if (_BL == 2) { asm mov display_mode,2; asm jmp l_done ; }
        }
        _BH = 0xff;
        _CL = 0xff;
        _AH = 0x12;
        _BL = 0x10;
        __int__(0x10);
        if (_BH <= 1 && _CL <= 0x0f) { asm mov display_mode,1; asm jmp l_done ; }
        _DX = 0x3d4;
        _AL = 0x0f;
        __outportb__(_DX, _AL);
        _DX++;
        _AL = __inportb__(_DX);
        _AH = _AL;
        _AL = 0x66;
        __outportb__(_DX, _AL);
        _CX = 0x100;
l_spin:
        asm loop l_spin
        _AL = __inportb__(_DX);
        asm xchg ah,al
        __outportb__(_DX, _AL);
        asm cmp ah,66h
        asm jne l_done
        asm mov display_mode,2
    }
l_done:
    asm cmp word ptr display_mode,1
    asm jne l_check_pcjr
    _AH = 0x12;
    _BL = 0x10;
    __int__(0x10);
    asm or bl,bl
    asm jne l_fin
    asm mov display_mode,2
    asm jmp l_fin
l_check_pcjr:
    asm cmp word ptr display_mode,4
    asm jne l_check_mcga
    asm jmp l_fin
l_check_mcga:
    asm cmp word ptr display_mode,5
    asm jne l_fin
l_fin:
    ;
}

/* ---- F_53BF (original code at 0x53BF) ---- */
sound_backend_probe()
{
    snd_backend_mode = 0;
    if (*(int *)&display_mode == 3) {
        snd_backend_mode = 1;
        goto l_selected;
    }
    if (opl_detect() != 0) {
        snd_backend_mode = 2;
        goto l_selected;
    }
    _AH = 0xc0;
    __int__(0x15);
    asm or ah,ah
    asm jnz l_selected
    asm cmp word ptr es:[bx+2],0bfch
    asm jne l_selected
    _CX = 10;
    _DX = 0x203;
l_wait_vga_port:
    _AL = 0xa5;
    __outportb__(_DX, _AL);
    _AL = __inportb__(_DX);
    asm cmp al,0a5h
    asm jne l_selected
    asm loop l_wait_vga_port
    snd_backend_mode = 3;
l_selected:
    ;
}


/* ---- F_520A (original code at 0x520A) ---- */
/* F_520A -- video-mode selection and the memory gate.  Plain C; the module
   is on the TASM path (runD: the 5039..52AB region), which is what shortens
   the two forward jmps at 529E and 52AB and pads them with NOP. */
/*@PUB _video_mode_select*/
extern void dos_write_handle2(char far *);
extern void cmdline_parse_args(void), bios_equipment_probe(void);
extern int video_adapter_detect(), sound_backend_probe();
extern char getdisk();
extern long farcoreleft();
extern unsigned char _osmajor;
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
    video_adapter_detect();
    if (display_mode == 0) {
        dos_write_handle2(s859);
        return 0;
    }
    sound_backend_probe();
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
