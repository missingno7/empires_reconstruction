/* F_6B66 -- drain the BIOS keyboard buffer: while F_6B4A still reports a
   key, take one.  The `int 16h` is an asm body (rule 14's positive case --
   it names neither SI nor DI), and there is no frame because there is no
   parameter and no local (rule 11).  The leading EB04 is the while's jump to
   its test. */
extern int f6b4a();

void keyboard_buffer_drain()
{
    while (f6b4a()) {
        asm xor ax,ax
        asm int 16h
    }
}
