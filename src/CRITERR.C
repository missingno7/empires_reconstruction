/* src/CRITERR.C: DOS critical-error handler.
   One translation unit; the sections below were the separate member
   sources of grouped module C_622C_625D and keep their original ids. */

/* ---- F_622C (original code at 0x622C) ---- */
extern int gb3e,gb40;
extern char near current_drive;
extern void hardretn(),hardresume();
void dos_critical_error_handler(a,b)
int a,b;
{
    gb3e=1;
    current_drive=b&0xff;
    if(gb40)hardretn(3);else hardresume(2);
    return 2;
}


/* ---- F_625D (original code at 0x625D) ---- */
/* F_625D -- install F_622C as the DOS critical-error handler.  In the compact
   model code is NEAR, so the handler is one word (`mov ax,offset / push ax`).
   F_622C is entered only by DOS, never called, so the census names no function
   object at IP 0x622C and the convention has nothing to resolve `_dos_critical_error_handler`
   against -- the byte comparison is the evidence, the LINK is the question. */
extern void harderr();
extern void dos_critical_error_handler();

void dos_critical_error_install()
{
    harderr(dos_critical_error_handler);
}
