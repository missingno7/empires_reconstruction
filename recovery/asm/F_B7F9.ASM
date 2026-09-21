; F_B7F9 -- initialise the board/control state, service the six display
; columns, then populate two fixed workspace ranges.

_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

public _level_display_init

_level_display_init proc near
    push si
    push di
    mov ax,48h
    push ax
    call $+0ad6dh
    pop cx
    mov word ptr ds:[0bch],0
    mov byte ptr ds:[0b437h],1
    mov byte ptr ds:[0b477h],1
    mov byte ptr ds:[0b4b7h],1
    mov byte ptr ds:[0b4f7h],1
    call $+132bh
    mov word ptr ds:[1774h],1
    mov ax,19h
    push ax
    call $+12c7h
    pop cx
    mov si,76h
    jmp B7_column_test

B7_column_body:
    mov ax,18h
    push ax
    call $+0b41fh
    pop cx
    mov ax,1bh
    push ax
    mov ax,si
    add ax,0fffch
    push ax
    mov ax,75h
    push ax
    mov ax,5ch
    push ax
    mov ax,163h
    push ax
    mov ax,si
    add ax,0fffch
    push ax
    call $+4b5ch
    add sp,0ch
    xor ax,ax
    push ax
    push word ptr ds:[0c5c8h]
    push word ptr ds:[0c5c6h]
    mov ax,1bh
    push ax
    push si
    call $+4b5eh
    add sp,0ah
    add word ptr ds:[0b571h],4
    add word ptr ds:[0b591h],4
    add word ptr ds:[0b5b1h],4
    add word ptr ds:[0b5f1h],4
    add word ptr ds:[0b631h],4
    add word ptr ds:[0b671h],4
    call $+927ah
    call $+69d4h
    mov ax,75h
    push ax
    mov ax,60h
    push ax
    mov ax,1bh
    push ax
    mov ax,si
    add ax,0fffch
    push ax
    call $+4af5h
    add sp,8
    call $+0b3bfh
    add si,4
B7_column_test:
    cmp si,0c6h
    jg short B7_columns_done
    jmp B7_column_body
B7_columns_done:
    mov word ptr ds:[1774h],0
    call $+1283h
    call $+69a1h
    mov word ptr ds:[96h],1e8h
    xor ax,ax
    push ax
    push word ptr ds:[0c5c8h]
    push word ptr ds:[0c5c6h]
    mov ax,163h
    push ax
    mov ax,si
    add ax,0fffch
    push ax
    call $+4ae6h
    add sp,0ah
    mov ax,0c8h
    push ax
    mov ax,6
    push ax
    mov ax,90h
    push ax
    mov ax,134h
    push ax
    mov ax,158h
    push ax
    mov ax,6
    push ax
    call $+4ab0h
    add sp,0ch
    mov word ptr ds:[96h],9fh
    mov word ptr ds:[0bch],1
    mov byte ptr ds:[0b6cfh],0
    mov ax,45h
    push ax
    call $+1cdah
    pop cx
    xor di,di
    jmp short B7_first_fill_test

B7_first_fill_body:
    mov ax,18h
    push ax
    call $+0b32ch
    pop cx
    les bx,dword ptr ds:[0c5cah]
    mov word ptr ds:[40c6h],es
    mov word ptr ds:[40c4h],bx
    call $+9564h
    call $+91ceh
    call $+6928h
    call $+6589h
    call $+0b328h
    inc di
B7_first_fill_test:
    cmp di,14h
    jl short B7_first_fill_body

    mov di,1b1h
    jmp short B7_second_fill_test
B7_second_fill_body:
    les bx,dword ptr ds:[0bfbch]
    mov byte ptr es:[bx+di],7
    inc di
B7_second_fill_test:
    cmp di,1b8h
    jng short B7_second_fill_body
    pop di
    pop si
    ret
_level_display_init endp

_TEXT ends
end
