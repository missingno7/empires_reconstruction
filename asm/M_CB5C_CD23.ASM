; M_CB5C_CD23 -- contiguous row-editing and ten-row menu controls.
; These five routines share the selected-row record table and menu state.
_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA, _BSS
assume cs:_TEXT, ds:DGROUP
public _f_cb5c, _f_cb5c_end, _f_cbbb, _f_cbbb_end, _f_cc17, _f_cc17_end, _f_cc6b, _f_cd23

; F_CB5C -- update the selected table row and clear its pending marker.
; This is the companion to F_CBBB.  It has no OMF fixups: helper and state
; relationships are encoded as the original direct same-segment/DGROUP forms.

_f_cb5c:
    push si
    mov ax, word ptr ds:[13edh]
    mov dx, 1bh
    mul dx
    mov bx, ax
    add bx, 0c470h
    push ds
    pop es
    mov ax, es:[bx+11h]
    neg ax
    sbb ax, ax
    inc ax
    mov byte ptr ds:[2193h], al
    push ds
    mov ax, 2188h
    push ax
    call $+0bb4bh            ; local target at CB5C+BB6D
    pop cx
    pop cx
    mov si, ax
    or si, si
    jl short CB5C_done
    mov ax, word ptr ds:[13edh]
    mov dx, 1bh
    mul dx
    mov bx, ax
    add bx, 0c470h
    push ds
    pop es
    mov es:[bx+11h], si
    or si, si
    jz short CB5C_done
    mov ax, word ptr ds:[13edh]
    mov dx, 1bh
    mul dx
    mov bx, ax
    add bx, 0c470h
    push ds
    pop es
    mov word ptr es:[bx+13h], 0ffffh
CB5C_done:
    xor ax, ax
    jmp short _f_cb5c_end
_f_cb5c_end:
    pop si
    ret

; F_CBBB -- submit the current table entry and update the active selection.
; The original contribution contains no OMF fixups.  Its helper calls remain
; same-segment expressions, while its state/table references are typed direct
; DGROUP operands.

_f_cbbb:
    push si
    mov ax, word ptr ds:[13edh]
    mov dx, 1bh
    mul dx
    mov bx, ax
    add bx, 0c470h
    push ds
    pop es
    mov ax, es:[bx+0fh]
    neg ax
    sbb ax, ax
    inc ax
    mov byte ptr ds:[216bh], al
    push ds
    mov ax, 2160h
    push ax
    call $+0baeah            ; local target at CBBB+BB0C
    pop cx
    pop cx
    mov si, ax
    or si, si
    jl short CBBB_done
    mov ax, si
    push ax
    mov ax, word ptr ds:[13edh]
    mov dx, 1bh
    mul dx
    mov bx, ax
    add bx, 0c470h
    push ds
    pop es
    pop ax
    mov es:[bx+0fh], ax
    mov word ptr ds:[1772h], ax
    or si, si
    jnz short CBBB_accepted
    call $+0fc2bh            ; local target at CBBB+FC77
    jmp short CBBB_done
CBBB_accepted:
    call $+09feh             ; local target at CBBB+0A4F
CBBB_done:
    xor ax, ax
    jmp short _f_cbbb_end
_f_cbbb_end:
    pop si
    ret

; F_CC17 -- submit the selected row's 0Dh table field.
_f_cc17:
 push si
 mov ax,word ptr ds:[13edh]
 mov dx,1bh
 mul dx
 mov bx,ax
 add bx,0c470h
 push ds
 pop es
 mov ax,es:[bx+0dh]
 neg ax
 sbb ax,ax
 inc ax
 mov byte ptr ds:[217fh],al
 push ds
 mov ax,2174h
 push ax
 call $+0ba90h
 pop cx
 pop cx
 mov si,ax
 or si,si
 jl short CC17_done
 mov ax,si
 push ax
 mov ax,word ptr ds:[13edh]
 mov dx,1bh
 mul dx
 mov bx,ax
 add bx,0c470h
 push ds
 pop es
 pop ax
 mov es:[bx+0dh],ax
 mov word ptr ds:[176eh],ax
 or si,si
 jnz short CC17_done
 call $+0fee5h
CC17_done:
 xor ax,ax
 jmp short _f_cc17_end
_f_cc17_end:
 pop si
 ret

; F_CC6B -- display a ten-row menu, then await an CC6B_accepted input code.
; The original object contains no OMF call fixups.  Code-address constants
; therefore intentionally preserve its non-relocating near-call topology.

f8480 equ 8680h
record_table equ 103ceh
menu_draw equ 105a2h
f6cf0 equ 6ef0h
fa28d equ 0a48dh
f6b1a equ 6d1ah
f8453 equ 8653h

_f_cc6b:
    push bp
    mov bp, sp
    sub sp, 2
    push si
    push di
    mov ax, 1
    push ax
    push ds
    mov ax, 21c4h
    push ax
    call $+0b803h
    add sp, 6
    xor ax, ax
    push ax
    call $+3548h
    pop cx
    mov ax, 27h
    push ax
    mov ax, 32h
    push ax
    mov ax, 2bh
    push ax
    call $+370ch
    add sp, 6
    mov ax, 26h
    push ax
    mov ax, 32h
    push ax
    mov ax, 9dh
    push ax
    call $+36fah
    add sp, 6
    mov ax, 23h
    push ax
    mov ax, 32h
    push ax
    mov ax, 0dfh
    push ax
    call $+36e8h
    add sp, 6
    call $+0a030h
    mov [bp-2], ax
    xor si, si
    xor di, di
    jmp short CC6B_menu_test
CC6B_menu_row:
    xor ax, ax
    push ax
    mov ax, di
    add ax, 34h
    push ax
    push ds
    mov ax, si
    mov dx, 1bh
    mul dx
    add ax, 0c360h
    push ax
    call $+0d5ach
    add sp, 8
    inc si
    mov ax, [bp-2]
    inc ax
    add di, ax
CC6B_menu_test:
    cmp si, 10
    jl short CC6B_menu_row
    mov ax, 0c8h
    push ax
    mov ax, 130h
    push ax
    xor ax, ax
    push ax
    mov ax, 8
    push ax
    call $+369dh
    add sp, 8
CC6B_input_wait:
    call $+09e12h
    mov si, ax
    cmp si, 1bh
    jz short CC6B_accepted
    cmp si, 0dh
    jnz short CC6B_input_wait
CC6B_accepted:
    call $+0b73ch
    xor ax, ax
    jmp short CC6B_done
CC6B_done:
    pop di
    pop si
    mov sp, bp
    pop bp
    ret

; F_CD23 -- display a ten-row menu, then await an CD23_accepted input code.
; The original object contains no OMF call fixups.  Code-address constants
; therefore intentionally preserve its non-relocating near-call topology.


_f_cd23:
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
    jmp short CD23_menu_test
CD23_menu_row:
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
CD23_menu_test:
    cmp si, 10
    jl short CD23_menu_row
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
CD23_input_wait:
    call $+09d59h
    mov si, ax
    cmp si, 1bh
    jz short CD23_accepted
    cmp si, 0dh
    jnz short CD23_input_wait
CD23_accepted:
    call $+0b683h
    xor ax, ax
    jmp short CD23_done
CD23_done:
    pop di
    pop si
    mov sp, bp
    pop bp
    ret

_TEXT ends
end
