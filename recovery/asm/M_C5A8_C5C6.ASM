; M_C5A8_C5C6 -- adjacent setters for the SI-relative command-control state.

_TEXT segment byte public 'CODE'

_DATA segment word public 'DATA'

_DATA ends

_BSS segment word public 'BSS'

_BSS ends

DGROUP group _DATA,_BSS

assume cs:_TEXT, ds:DGROUP

public _f_c5a8, _f_c5b3, _f_c5c6



; Original entry F_C5A8 -- store AL (zero-extended) through SI at DS:17A4h.


_f_c5a8 proc near
    push bp
    mov bp,sp
    xor ah,ah
    mov [si+17a4h],ax
    pop bp
    ret
_f_c5a8 endp

; Original entry F_C5B3 -- select a word from the DS:1832h table by AL,
; then store it through SI at DS:17C4h.



_f_c5b3 proc near
    push bp
    mov bp,sp
    mov bl,al
    xor bh,bh
    shl bx,1
    mov bx,[bx+1832h]
    mov [si+17c4h],bx
    pop bp
    ret
_f_c5b3 endp

; Original entry F_C5C6 -- store AL (zero-extended) through SI at DS:17DCh.


_f_c5c6 proc near
    push bp
    mov bp,sp
    xor ah,ah
    mov [si+17dch],ax
    pop bp
    ret
_f_c5c6 endp

_TEXT ends

end

