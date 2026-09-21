; Original entry F_C8D4.
; Write the live AL value to the I/O port selected by the DS global opl_port.

_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

public _opl_port_write_byte
extrn _opl_port:word

_opl_port_write_byte proc near
    push bp
    mov bp,sp
    push dx
    mov dx,word ptr ds:_opl_port
    out dx,al
    jmp short $+2
    pop dx
    pop bp
    ret
_opl_port_write_byte endp

_TEXT ends
end
