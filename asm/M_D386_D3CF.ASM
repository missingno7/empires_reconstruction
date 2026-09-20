; M_D386_D3CF -- decode a far record list and handle its negative selector path.
; F_D3CF is the direct branch continuation of F_D386, not an independent flow.
_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
    extrn _gbfc4:dword, _a72b2:dword
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP
extrn _f03c9:near
public _fd386, _fd3cf

_fd386 proc near
    push bp
    mov bp, sp
    push di
    push si
    cld
    lds si, dword ptr ds:_gbfc4
fd386_read_record:
    lodsb
    xor cx, cx
    mov cl, al
fd386_next_record:
    push cx
    xor bx, bx
    mov di, si
    lodsb
    mov bl, al
    lodsw
    mov cx, ax
    add si, bx
    lodsb
    dec al
    js short _fd3cf
    inc byte ptr [di]
fd386_lookup:
    mov bl, al
    shl bx, 1
    shl bx, 1
    les si, dword ptr ds:[bx+_a72b2]
    push es
    push si
    xor dx, dx
    xchg dl, ch
    push dx
    shl cx, 1
    push cx
    call _f03c9
    add sp, 8
    mov si, di
    add si, 0ch
    pop cx
    loop fd386_next_record
    pop si
    pop di
    pop bp
    ret
_fd386 endp

_fd3cf proc near
    mov byte ptr [di], 1
    mov si, di
    add si, 3
    lodsb
    jmp short fd386_lookup
_fd3cf endp

_TEXT ends
end
