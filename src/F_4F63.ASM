; Original entry F_4F63.
; Writes the far NUL-terminated buffer passed at BP+4 to DOS handle 2.

_TEXT segment byte public 'CODE'
assume cs:_TEXT

public _f4f63
extrn _strlen:near

_f4f63 proc near
    push bp
    mov bp,sp
    sub sp,2
    push si
    push di
    push [bp+6]
    push [bp+4]
    call _strlen
    pop cx
    pop cx
    mov si,ax
    dec si
    mov di,[bp+4]
    mov ax,[bp+6]
    mov [bp-2],ax
    mov bx,2
    mov cx,si
    mov dx,di
    mov ah,40h
    mov ds,[bp-2]
    int 21h
    pop di
    pop si
    mov sp,bp
    pop bp
    ret
_f4f63 endp

_TEXT ends
end
