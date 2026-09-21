; F_28AC -- draw ten three-byte records into the fixed ES:BFBCh board region.

_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

public _board_mark_record_cells

_board_mark_record_cells proc near
    push bp
    mov bp,sp
    sub sp,0eh
    push si
    push di
    les bx,dword ptr ds:[0bfbch]
    add bx,2ach
    mov [bp-2],es
    mov [bp-4],bx
    mov word ptr [bp-0eh],0
    jmp F28_outer_test

F28_outer_body:
    les bx,[bp-4]
    mov al,es:[bx]
    mov [bp-5],al
    or al,al
    jnz short F28_have_record
    jmp F28_outer_advance

F28_have_record:
    les bx,[bp-4]
    mov al,es:[bx+1]
    xor ah,ah
    xor dx,dx
    mov si,ax
    shl si,1
    les bx,[bp-4]
    mov al,es:[bx+2]
    xor ah,ah
    xor dx,dx
    mov di,ax
    mov ax,si
    shr ax,1
    shr ax,1
    shr ax,1
    push ax
    mov ax,di
    shr ax,1
    shr ax,1
    shr ax,1
    add ax,0fffeh
    mov dx,26h
    mul dx
    pop dx
    add dx,ax
    dec dx
    mov [bp-0ch],dx
    add di,0b4h
    sub si,4
    test byte ptr [bp-5],80h
    jz short F28_normal_stride

    xor ax,ax
    push ax
    push ds
    mov ax,6ca6h
    push ax
    push di
    push si
    call $+0da9fh            ; historical draw helper
    add sp,0ah
    mov word ptr [bp-0ah],26h
    jmp short F28_inner_begin

F28_normal_stride:
    xor ax,ax
    push ax
    push ds
    mov ax,6ac4h
    push ax
    push di
    push si
    call $+0da88h            ; historical draw helper
    add sp,0ah
    mov word ptr [bp-0ah],1

F28_inner_begin:
    mov word ptr [bp-8],0
    jmp short F28_inner_test
F28_inner_body:
    les bx,dword ptr ds:[0bfbch]
    add bx,[bp-0ch]
    mov byte ptr es:[bx],7
    inc word ptr [bp-8]
    mov ax,[bp-0ah]
    add [bp-0ch],ax
F28_inner_test:
    cmp word ptr [bp-8],6
    jl short F28_inner_body

F28_outer_advance:
    inc word ptr [bp-0eh]
    add word ptr [bp-4],3
F28_outer_test:
    cmp word ptr [bp-0eh],0ah
    jnl short F28_done
    jmp F28_outer_body
F28_done:
    pop di
    pop si
    mov sp,bp
    pop bp
    ret
_board_mark_record_cells endp

_TEXT ends
end
