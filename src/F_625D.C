/* F_625D -- install F_622C as the DOS critical-error handler.  In the compact
   model code is NEAR, so the handler is one word (`mov ax,offset / push ax`).
   F_622C is entered only by DOS, never called, so the census names no function
   object at IP 0x622C and the convention has nothing to resolve `_f622c`
   against -- the byte comparison is the evidence, the LINK is the question. */
extern void harderr();
extern void f622c();

void f625d()
{
    harderr(f622c);
}
