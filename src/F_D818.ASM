; Original entry F_D818 -- reset the DS:2F30h byte counter.

_TEXT segment byte public 'CODE'
assume cs:_TEXT

public _fd818

_fd818 proc near
    push bp
    mov bp,sp
    push di
    mov di,2f30h
    mov byte ptr [di],0
    pop di
    pop bp
    ret
_fd818 endp

_TEXT ends
end
