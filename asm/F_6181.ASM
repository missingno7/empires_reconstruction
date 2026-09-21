; F_6181 -- adjust the selected sprite record and issue two draw passes.
;
; TASM emits FIXUPP records in source-contribution order. Turbo C placed the
; DGROUP load before the two far-pointer records, even though its instruction
; is later in memory. The three ORG contributions preserve both that record
; order and the original contiguous _TEXT layout.
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA, _BSS

_TEXT segment byte public 'CODE'
assume cs:_TEXT, ds:DGROUP
extrn _sprbase:dword, _objtab:dword
extrn _gfx_wipe_rect:near, _gfx_copy_rect:near
public _sprite_record_adjust_draw

; Offset 57h: emit the DGROUP FIXUPP record before the low-address externals.
org 57h
    mov ax, DGROUP
    mov ds, ax

; Offset 0: sprite-record selection and state update.
org 0
_sprite_record_adjust_draw:
    push bp
    mov bp, sp
    push si
    push di
    push ds
    cld
    les di, dword ptr ds:[_sprbase]
    lds si, dword ptr ds:[_objtab]
    inc si
    mov ax, [bp+4]
    mov bx, ax
    shl ax, 1
    add si, ax
    add si, bx
    lodsw
    xor dx, dx
    mov dl, ah
    mov bh, dh
    mov bl, al
    shl bx, 1
    mov bp, dx
    mov al, [si]
    xor ah, ah
    test al, 40h
    jz short increment
    and al, 1fh
    dec al
    jnl short negative_done
    mov al, 17h
negative_done:
    jmp short store
increment:
    and al, 1fh
    inc al
    cmp al, 17h
    jng short store
    mov al, 0
store:
    and byte ptr [si], 0e0h
    or [si], al
    mov dx, 1e6h
    mul dx
    add ax, 2
    add di, ax
    mov dx, bp
    push es
    push dx

; Offset 5ch: draw calls follow the DGROUP reload in memory and in FIXUPP order.
org 5ch
    push dx
    mov bp, sp
    push bx
    mov ax, 1eh
    push ax
    mov ax, 1eh
    push ax
    mov ax, dx
    add ax, 148h
    push ax
    push bx
    call near ptr _gfx_wipe_rect
    add word ptr [bp], 0b8h
    call near ptr _gfx_wipe_rect
    pop bx
    add sp, 10
    pop dx
    pop es
    xor ax, ax
    push ax
    push es
    push di
    push dx
    push bx
    call near ptr _gfx_copy_rect
    pop bx
    pop dx
    add sp, 6
    mov ax, dx
    add ax, 0b8h
    push ax
    push bx
    mov ax, 1eh
    push ax
    mov ax, 1eh
    push ax
    push dx
    push bx
    call near ptr _gfx_wipe_rect
    add sp, 12
    pop ds
    pop di
    pop si
    pop bp
    ret
_TEXT ends
end
