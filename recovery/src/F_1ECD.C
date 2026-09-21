/* F_1ECD -- drain the far rectangle queue and reset its write pointer. */
extern void gfx_box();
extern unsigned long rect_queue_write_ptr;
extern unsigned int g40c6;
extern unsigned long ui_gfx_blob;

void rect_queue_flush()
{
    asm cld
    asm mov di,ds
    asm les cx,dword ptr rect_queue_write_ptr
    asm lds si,dword ptr ui_gfx_blob
    asm sub cx,si
    asm shr cx,1
    asm shr cx,1
L_loop: asm push cx
    asm push ds
    asm lodsw
    asm mov dx,ax
    asm lodsw
    asm xor bx,bx
    asm mov bl,ah
    asm push bx
    asm mov bl,al
    asm shl bx,1
    asm push bx
    asm xor bx,bx
    asm mov bl,dh
    asm push bx
    asm mov bl,dl
    asm shl bx,1
    asm push bx
    asm mov ds,di
    asm call near ptr gfx_box
    asm add sp,8
    asm pop ds
    asm pop cx
    asm loop L_loop
    asm mov ds,di
    asm les bx,dword ptr ui_gfx_blob
    asm mov word ptr g40c6,es
    asm mov word ptr rect_queue_write_ptr,bx
}
