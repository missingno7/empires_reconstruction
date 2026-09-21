; F_CD23 -- display a ten-row menu, then await an accepted input code.
; The original object contains no OMF call fixups.  Code-address constants
; therefore intentionally preserve its non-relocating near-call topology.
_TEXT segment byte public 'CODE'
assume cs:_TEXT, ds:DGROUP
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA, _BSS

dialog_draw equ 8680h
record_table equ 103ceh
menu_draw equ 105a2h
dialog_line_height_get equ 6ef0h
slot_row_draw equ 0a48dh
f6b1a equ 6d1ah
dialog_restore_screen equ 8653h

public _slot_list_show
_slot_list_show:
    push bp
    mov bp, sp
    sub sp, 2
    push si
    push di
    mov ax, 1
    push ax
    push ds
    mov ax, 21d8h
    push ax
    call $+0b74bh
    add sp, 6
    xor ax, ax
    push ax
    call $+3490h
    pop cx
    mov ax, 27h
    push ax
    mov ax, 32h
    push ax
    mov ax, 2bh
    push ax
    call $+3654h
    add sp, 6
    mov ax, 26h
    push ax
    mov ax, 32h
    push ax
    mov ax, 99h
    push ax
    call $+3642h
    add sp, 6
    mov ax, 37h
    push ax
    mov ax, 32h
    push ax
    mov ax, 0dch
    push ax
    call $+3630h
    add sp, 6
    call $+09f78h
    mov [bp-2], ax
    xor si, si
    xor di, di
    jmp short menu_test
menu_row:
    mov ax, 1
    push ax
    mov ax, di
    add ax, 34h
    push ax
    push ds
    mov ax, si
    mov dx, 1bh
    mul dx
    add ax, 0c470h
    push ax
    call $+0d4f3h
    add sp, 8
    inc si
    mov ax, [bp-2]
    inc ax
    add di, ax
menu_test:
    cmp si, 10
    jl short menu_row
    mov ax, 0c8h
    push ax
    mov ax, 130h
    push ax
    xor ax, ax
    push ax
    mov ax, 8
    push ax
    call $+35e4h
    add sp, 8
input_wait:
    call $+09d59h
    mov si, ax
    cmp si, 1bh
    jz short accepted
    cmp si, 0dh
    jnz short input_wait
accepted:
    call $+0b683h
    xor ax, ax
    jmp short done
done:
    pop di
    pop si
    mov sp, bp
    pop bp
    ret
_TEXT ends
end

