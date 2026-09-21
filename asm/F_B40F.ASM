; F_B40F -- expand two compact descriptor tables into the fixed B07C/B07E
; record stream used by the display/state subsystem.

_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

public _level_expand_descriptors

_level_expand_descriptors proc near
    push si
    push di
    xor si,si

    ; Initialise the first compact table, then install its two far pointers.
    mov ax,57h
    push ax
    call $+0b155h
    pop cx
    mov ax,54h
    push ax
    push word ptr ds:[0c5c8h]
    push word ptr ds:[0c5c6h]
    push ds
    mov ax,9bfch
    push ax
    call $+3f70h
    add sp,0ah

    ; Initialise the second compact table in the same way.
    mov ax,58h
    push ax
    call $+0b136h
    pop cx
    mov ax,54h
    push ax
    push word ptr ds:[0c5c8h]
    push word ptr ds:[0c5c6h]
    push ds
    mov ax,0bf66h
    push ax
    call $+3f51h
    add sp,0ah

    push ds
    mov ax,0c592h
    push ax
    mov ax,55h
    push ax
    call $+0b3f0h
    add sp,6

    xor di,di
    jmp short B40_first_test
B40_first_copy:
    mov ax,di
    shl ax,1
    les bx,dword ptr ds:[0c592h]
    add bx,ax
    mov ax,es:[bx]
    mov cx,word ptr ds:[0c594h]
    mov bx,word ptr ds:[0c592h]
    add bx,ax
    inc bx
    inc bx
    push cx
    push bx
    mov bx,si
    shl bx,1
    shl bx,1
    pop ax
    pop dx
    mov [bx-4f82h],dx
    mov [bx-4f84h],ax
    inc si
    inc di
B40_first_test:
    cmp di,2
    jl short B40_first_copy

    push ds
    mov ax,0c596h
    push ax
    mov ax,56h
    push ax
    call $+0b3abh
    add sp,6

    xor di,di
    jmp short B40_second_test
B40_second_copy:
    mov ax,di
    shl ax,1
    les bx,dword ptr ds:[0c596h]
    add bx,ax
    mov ax,es:[bx]
    mov cx,word ptr ds:[0c598h]
    mov bx,word ptr ds:[0c596h]
    add bx,ax
    inc bx
    inc bx
    push cx
    push bx
    mov bx,si
    shl bx,1
    shl bx,1
    pop ax
    pop dx
    mov [bx-4f82h],dx
    mov [bx-4f84h],ax
    inc si
    inc di
B40_second_test:
    cmp di,0ch
    jl short B40_second_copy
    jmp short B40_tail_test

B40_tail_copy:
    mov dx,word ptr ds:[0b07eh]
    mov ax,word ptr ds:[0b07ch]
    mov bx,si
    shl bx,1
    shl bx,1
    mov [bx-4f82h],dx
    mov [bx-4f84h],ax
    inc si
B40_tail_test:
    cmp si,54h
    jl short B40_tail_copy
    pop di
    pop si
    ret
_level_expand_descriptors endp

_TEXT ends
end
