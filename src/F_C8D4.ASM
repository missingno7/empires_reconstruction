; Original entry F_C8D4.
; Write the live AL value to the I/O port selected by the DS global g1830.

_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
DGROUP group _DATA
assume cs:_TEXT, ds:DGROUP

public _fc8d4
extrn _g1830:word

_fc8d4 proc near
    push bp
    mov bp,sp
    push dx
    mov dx,word ptr ds:_g1830
    out dx,al
    jmp short $+2
    pop dx
    pop bp
    ret
_fc8d4 endp

_TEXT ends
end
