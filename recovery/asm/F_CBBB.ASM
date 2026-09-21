; F_CBBB -- submit the current table entry and update the active selection.
; The original contribution contains no OMF fixups.  Its helper calls remain
; same-segment expressions, while its state/table references are typed direct
; DGROUP operands.
_TEXT segment byte public 'CODE'
assume cs:_TEXT, ds:DGROUP
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA, _BSS

public _options_toggle_music
public _f_cbbb_end
_options_toggle_music:
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
    jl short done
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
    jnz short accepted
    call $+0fc2bh            ; local target at CBBB+FC77
    jmp short done
accepted:
    call $+09feh             ; local target at CBBB+0A4F
done:
    xor ax, ax
    jmp short _f_cbbb_end
_f_cbbb_end:
    pop si
    ret
_TEXT ends
end
