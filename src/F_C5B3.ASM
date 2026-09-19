; Original entry F_C5B3 -- select a word from the DS:1832h table by AL,
; then store it through SI at DS:17C4h.

_TEXT segment byte public 'CODE'
assume cs:_TEXT

public _f_c5b3

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

_TEXT ends
end
