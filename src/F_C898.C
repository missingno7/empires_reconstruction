/* F_C898 -- OPL2 register write: address, 6 in-al,dx settle reads, data,
   35 more.  TC 2.01 has no unroller and reloads DX per inportb(), so the
   body is asm; the two parameters are read from the frame by the asm. */
extern unsigned int opl_port;           /* DS:1830 */
/*@SYM _opl_port=0x1830 kind=g key=storage_objects/G_P21360.phys*/

void fc898(reg, val)
int reg, val;
{
    asm mov dx,word ptr opl_port
    asm mov ax,[bp+4]
    asm out dx,al
    asm in al,dx
    asm in al,dx
    asm in al,dx
    asm in al,dx
    asm in al,dx
    asm in al,dx
    asm inc dx
    asm mov ax,[bp+6]
    asm out dx,al
    asm dec dx
    asm in al,dx
    asm in al,dx
    asm in al,dx
    asm in al,dx
    asm in al,dx
    asm in al,dx
    asm in al,dx
    asm in al,dx
    asm in al,dx
    asm in al,dx
    asm in al,dx
    asm in al,dx
    asm in al,dx
    asm in al,dx
    asm in al,dx
    asm in al,dx
    asm in al,dx
    asm in al,dx
    asm in al,dx
    asm in al,dx
    asm in al,dx
    asm in al,dx
    asm in al,dx
    asm in al,dx
    asm in al,dx
    asm in al,dx
    asm in al,dx
    asm in al,dx
    asm in al,dx
    asm in al,dx
    asm in al,dx
    asm in al,dx
    asm in al,dx
    asm in al,dx
    asm in al,dx
}
