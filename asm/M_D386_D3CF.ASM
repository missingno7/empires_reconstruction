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
extrn _gfx_blit_bitmap:near
public _icon_list_animate_draw, _icon_frame_reset_and_draw

_icon_list_animate_draw proc near
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
    js short _icon_frame_reset_and_draw
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
    call _gfx_blit_bitmap
    add sp, 8
    mov si, di
    add si, 0ch
    pop cx
    loop fd386_next_record
    pop si
    pop di
    pop bp
    ret
_icon_list_animate_draw endp

_icon_frame_reset_and_draw proc near
    mov byte ptr [di], 1
    mov si, di
    add si, 3
    lodsb
    jmp short fd386_lookup
_icon_frame_reset_and_draw endp

_TEXT ends
end
