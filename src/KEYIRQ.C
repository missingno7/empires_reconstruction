/* src/KEYIRQ.C: IRQ1 keyboard vector install and restore.
   One translation unit; the sections below were the separate member
   sources of grouped module C_695E_697D and keep their original ids. */

extern void interrupt (*getvect())();
extern void setvect();
extern void keyboard_irq_handler();
extern void interrupt (*int9_saved_vector)(void);   /* saved INT 9 vector: DS:C0CC offset, DS:C0CE segment */

/* ---- F_695E (original code at 0x695E) ---- */
/* F_695E -- take over IRQ1: stash the old int 9 vector at DS:C0CC and install
   F_699E.  The handler is pushed as `push cs / mov ax,offset / push ax`, so
   the source widens a NEAR function pointer to far with a cast; declaring
   F_699E `interrupt` instead makes TC 2.0 emit a segment fixup
   (`mov ax,seg / push ax`) and three bytes more. */
void keyboard_irq_install()
{
    int9_saved_vector = getvect(9);
    setvect(9, (void (far *) ()) keyboard_irq_handler);
}


/* ---- F_697D (original code at 0x697D) ---- */
/* Restore the INT 9 vector saved by keyboard_irq_install. */
void keyboard_irq_restore(void) { setvect(9, int9_saved_vector); }
