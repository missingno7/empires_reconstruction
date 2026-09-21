; Original entry F_C5A8 -- store AL (zero-extended) through SI at DS:17A4h.

_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP
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
