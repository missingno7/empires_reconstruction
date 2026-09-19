/* F_D85F -- delete the matching five-byte record and compact the table. */
extern unsigned char g2f30[];
void fd85f(key)
int key;
{
    asm mov ax,ds
    asm mov es,ax
    asm mov si,offset DGROUP:g2f30
    asm lodsb
    asm mov cl,al
    asm sub ch,ch
    asm jcxz L_out
L_scan: asm mov al,[si]
    asm cmp al,[bp+4]
    asm je L_hit
    asm add si,5
    asm loop L_scan
    asm jcxz L_out
L_hit: asm mov di,si
    asm add si,5
    asm dec cx
    asm mov ax,cx
    asm shl ax,1
    asm shl ax,1
    asm add cx,ax
    asm rep movsb
    asm mov di,offset DGROUP:g2f30
    asm dec byte ptr es:[di]
L_out: ;
}
