; F_DF98: arithmetic/state update.  The high-address contribution is emitted
; first so TASM 1.0 retains the historical descending FIXUPP record order.
_TEXT segment word public 'CODE'
assume cs:_TEXT, ds:_TEXT

extrn LXMUL@:far, LDIV@:far
public _fdf98

g3752 equ 3752h
g3754 equ 3754h
gc5e4 equ 0c5e4h
gc5e6 equ 0c5e6h
gc5e8 equ 0c5e8h
music_tempo_scaled equ 0ca6bh
gca24 equ 0ca24h
gca26 equ 0ca26h
gca6d equ 0ca6dh
gc6c3 equ 0c6c3h

; This contribution starts at 0065h.  Its fixup is deliberately emitted first.
org 65h
fdf98_divide:
    call far ptr LDIV@
    mov di, ax
    or di, di
    jnl short fdf98_nonnegative
    mov ax, 18h
    sub ax, di
    mov [bp-6], ax
    mov ax, [bp-6]
    mov bx, 19h
    cwd
    idiv bx
    neg ax
    mov bx, [bp+4]
    shl bx, 1
    mov word ptr [bx+gca6d], ax
    mov word ptr ds:[gc5e4], ax
    mov ax, [bp-6]
    add ax, -18h
    mov bx, 19h
    cwd
    idiv bx
    mov si, dx
    or si, si
    jz short fdf98_remainder_zero
    mov ax, 19h
    sub ax, si
    mov si, ax
fdf98_remainder_zero:
    jmp short fdf98_finish
fdf98_nonnegative:
    mov ax, di
    mov bx, 19h
    cwd
    idiv bx
    mov bx, [bp+4]
    shl bx, 1
    mov word ptr [bx+gca6d], ax
    mov word ptr ds:[gc5e4], ax
    mov ax, di
    mov bx, 19h
    cwd
    idiv bx
    mov si, dx
fdf98_finish:
    mov ax, si
    mov dx, 18h
    mul dx
    add ax, gc6c3
    mov dx, ds
    mov bx, [bp+4]
    shl bx, 1
    shl bx, 1
    mov word ptr [bx+gca26], dx
    mov word ptr [bx+gca24], ax
    mov word ptr ds:[gc5e8], dx
    mov word ptr ds:[gc5e6], ax
    mov dx, [bp-2]
    mov ax, [bp-4]
    mov word ptr ds:[g3754], dx
    mov word ptr ds:[g3752], ax
fdf98_return:
    pop di
    pop si
    mov sp, bp
    pop bp
    ret

; The entry contribution starts at zero and contains the LXMUL@ fixup.
org 0
_fdf98:
    push bp
    mov bp, sp
    sub sp, 6
    push si
    push di
    mov ax, [bp+6]
    add ax, 0e000h
    cwd
    push dx
    push ax
    mov ax, word ptr ds:[music_tempo_scaled]
    cwd
    pop bx
    pop cx
    call far ptr LXMUL@
    mov [bp-2], dx
    mov [bp-4], ax
    mov dx, word ptr ds:[g3754]
    mov ax, word ptr ds:[g3752]
    cmp dx, [bp-2]
    jnz short fdf98_slow
    cmp ax, [bp-4]
    jnz short fdf98_slow
    mov dx, word ptr ds:[gc5e8]
    mov ax, word ptr ds:[gc5e6]
    mov bx, [bp+4]
    shl bx, 1
    shl bx, 1
    mov word ptr [bx+gca26], dx
    mov word ptr [bx+gca24], ax
    mov ax, word ptr ds:[gc5e4]
    mov bx, [bp+4]
    shl bx, 1
    mov word ptr [bx+gca6d], ax
    jmp fdf98_return
fdf98_slow:
    xor dx, dx
    mov ax, 2000h
    push dx
    push ax
    push word ptr [bp-2]
    push word ptr [bp-4]
_TEXT ends
end