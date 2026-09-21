/* F_6B66 -- drain the BIOS keyboard buffer: while F_6B4A still reports a
   key, take one.  The BIOS call is C through the `_AX` pseudo-register and
   the `__int__` intrinsic (same bytes as the former asm `xor ax,ax` /
   `int 16h`).  There is no frame because there is no parameter and no local
   (rule 11).  The leading EB04 is the while's jump to its test. */
extern int f6b4a();
void __int__(int);

void keyboard_buffer_drain()
{
    while (f6b4a()) {
        _AX = 0;
        __int__(0x16);
    }
}
