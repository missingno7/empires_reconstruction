/* F_695E -- take over IRQ1: stash the old int 9 vector at DS:C0CC and install
   F_699E.  The handler is pushed as `push cs / mov ax,offset / push ax`, so
   the source widens a NEAR function pointer to far with a cast; declaring
   F_699E `interrupt` instead makes TC 2.0 emit a segment fixup
   (`mov ax,seg / push ax`) and three bytes more. */
extern void interrupt (*getvect())();
extern void setvect();
extern void f699e();
extern void interrupt (*gc0cc)();       /* DS:C0CC offset, DS:C0CE segment */

void f695e()
{
    gc0cc = getvect(9);
    setvect(9, (void (far *) ()) f699e);
}
