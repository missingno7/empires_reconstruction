; Original entry F_C5C6 -- store AL (zero-extended) through SI at DS:17DCh.

_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP
public _f_c5c6

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
