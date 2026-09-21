; F_CB5C -- update the selected table row and clear its pending marker.
; This is the companion to F_CBBB.  It has no OMF fixups: helper and state
; relationships are encoded as the original direct same-segment/DGROUP forms.
_TEXT segment byte public 'CODE'
assume cs:_TEXT, ds:DGROUP
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA, _BSS

public _options_toggle_option
public _f_cb5c_end
_options_toggle_option:
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
    jl short done
    mov ax, word ptr ds:[13edh]
    mov dx, 1bh
    mul dx
    mov bx, ax
    add bx, 0c470h
    push ds
    pop es
    mov es:[bx+11h], si
    or si, si
    jz short done
    mov ax, word ptr ds:[13edh]
    mov dx, 1bh
    mul dx
    mov bx, ax
    add bx, 0c470h
    push ds
    pop es
    mov word ptr es:[bx+13h], 0ffffh
done:
    xor ax, ax
    jmp short _f_cb5c_end
_f_cb5c_end:
    pop si
    ret
_TEXT ends
end
