; Word alignment lets TLINK supply the preceding inter-module pad.
; M_D61C_D79C -- render an ES:DI command list, optionally highlighting the
; 24-cell board lattice while it is processed.

_TEXT segment word public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

extrn _gfx_copy_rect:near
extrn _gfx_blit_bitmap:near

public _f_d61c, _f_d79c

; The original hand-written code addresses the first cell with an 8-bit
; displacement and the following cells with 16-bit displacements.  TASM makes
; that distinction naturally from the offsets below.
set_cell_attribute macro cell, value
    mov byte ptr [bx+cell],value
endm

; Original entry F_D61C -- apply bright attributes, render all records, then
; restore the normal attributes.
_f_d61c proc near
    push bp
    mov bp,sp
    push di
    push si
    push ds
    cld
    mov word ptr ds:[96h],190h
    mov bx,2380h
    set_cell_attribute 21h,10h
    set_cell_attribute 0a3h,10h
    set_cell_attribute 105h,10h
    set_cell_attribute 197h,10h
    set_cell_attribute 219h,10h
    set_cell_attribute 27bh,10h
    set_cell_attribute 30dh,10h
    set_cell_attribute 38fh,10h
    set_cell_attribute 3f1h,10h
    set_cell_attribute 483h,10h
    set_cell_attribute 505h,10h
    set_cell_attribute 567h,10h
    set_cell_attribute 5f9h,10h
    set_cell_attribute 67bh,10h
    set_cell_attribute 6ddh,10h
    set_cell_attribute 76fh,10h
    set_cell_attribute 7f1h,10h
    set_cell_attribute 853h,10h
    set_cell_attribute 8e5h,10h
    set_cell_attribute 967h,10h
    set_cell_attribute 9c9h,10h
    set_cell_attribute 0a5bh,10h
    set_cell_attribute 0addh,10h
    set_cell_attribute 0b3fh,10h

    les di,dword ptr ds:[0bfc0h]
    sub ch,ch
    mov cl,es:[di]
    inc di
D61_record_loop:
    push cx
    push es
    mov bx,es:[di]
    inc di
    inc di
    mov cx,es:[di]
    inc di
    mov al,ch
    cbw
    sub ch,ch
    mov si,cx
    test ax,3
    jz short D61_record_adjusted
    jpo short D61_record_adjusted
    sub byte ptr es:[di],3
    jmp short D61_attribute_ready
D61_record_adjusted:
    inc byte ptr es:[di]
D61_attribute_ready:
    inc di
    mov dx,176h
    mul dx
    add ax,2380h
    sub dx,dx
    push dx
    push ds
    push ax
    mov ah,dl
    mov al,bh
    add ax,0b8h
    push ax
    mov ah,dl
    mov al,bl
    shl ax,1
    push ax
    call _gfx_copy_rect
    mov bp,sp
    add word ptr [bp],0ch
    add word ptr [bp+4],82h
    cmp si,0
    jng short D61_strip_loop_done
D61_strip_loop:
    call _gfx_blit_bitmap
    add word ptr [bp],8
    dec si
    jnz short D61_strip_loop
D61_strip_loop_done:
    add word ptr [bp+4],62h
    call _gfx_copy_rect
    add sp,0ah
    pop es
    pop cx
    loop D61_record_loop

    mov bx,2380h
    set_cell_attribute 21h,0bh
    set_cell_attribute 0a3h,0bh
    set_cell_attribute 105h,0bh
    set_cell_attribute 197h,0bh
    set_cell_attribute 219h,0bh
    set_cell_attribute 27bh,0bh
    set_cell_attribute 30dh,0bh
    set_cell_attribute 38fh,0bh
    set_cell_attribute 3f1h,0bh
    set_cell_attribute 483h,0bh
    set_cell_attribute 505h,0bh
    set_cell_attribute 567h,0bh
    set_cell_attribute 5f9h,0bh
    set_cell_attribute 67bh,0bh
    set_cell_attribute 6ddh,0bh
    set_cell_attribute 76fh,0bh
    set_cell_attribute 7f1h,0bh
    set_cell_attribute 853h,0bh
    set_cell_attribute 8e5h,0bh
    set_cell_attribute 967h,0bh
    set_cell_attribute 9c9h,0bh
    set_cell_attribute 0a5bh,0bh
    set_cell_attribute 0addh,0bh
    set_cell_attribute 0b3fh,0bh
    mov word ptr ds:[96h],9fh
    pop ds
    pop si
    pop di
    pop bp
    ret
_f_d61c endp

; Original entry F_D79C -- the same record renderer without lattice changes.
_f_d79c proc near
    push bp
    mov bp,sp
    push di
    push si
    push ds
    cld
    les di,dword ptr ds:[0bfc0h]
    sub ch,ch
    mov cl,es:[di]
    inc di
D79_record_loop:
    push cx
    push es
    mov bx,es:[di]
    inc di
    inc di
    mov cx,es:[di]
    inc di
    mov al,ch
    cbw
    sub ch,ch
    mov si,cx
    test ax,3
    jz short D79_record_adjusted
    jpo short D79_record_adjusted
    sub byte ptr es:[di],3
    jmp short D79_attribute_ready
D79_record_adjusted:
    inc byte ptr es:[di]
D79_attribute_ready:
    inc di
    mov dx,176h
    mul dx
    add ax,2380h
    sub dx,dx
    push dx
    push ds
    push ax
    mov ah,dl
    mov al,bh
    push ax
    mov al,bl
    shl ax,1
    push ax
    call _gfx_copy_rect
    mov bp,sp
    add word ptr [bp],0ch
    add word ptr [bp+4],82h
    cmp si,0
    jng short D79_strip_loop_done
D79_strip_loop:
    call _gfx_blit_bitmap
    add word ptr [bp],8
    dec si
    jnz short D79_strip_loop
D79_strip_loop_done:
    add word ptr [bp+4],62h
    call _gfx_copy_rect
    add sp,0ah
    pop es
    pop cx
    loop D79_record_loop
    pop ds
    pop si
    pop di
    pop bp
    ret
_f_d79c endp

_TEXT ends
end
