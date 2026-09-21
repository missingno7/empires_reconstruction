; Original entry F_D818 -- reset the DS:2F30h byte counter.

_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

public _draw_queue_reset

_draw_queue_reset proc near
    push bp
    mov bp,sp
    push di
    mov di,2f30h
    mov byte ptr [di],0
    pop di
    pop bp
    ret
_draw_queue_reset endp

_TEXT ends
end
