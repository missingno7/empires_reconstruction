; Original entry F_C5A8 -- store AL (zero-extended) through SI at DS:17A4h.

_TEXT segment byte public 'CODE'
assume cs:_TEXT
public _f_c5a8

_f_c5a8 proc near
    push bp
    mov bp,sp
    xor ah,ah
    mov [si+17a4h],ax
    pop bp
    ret
_f_c5a8 endp

_TEXT ends
end
