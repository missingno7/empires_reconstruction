; Original entry F_6EFF -- normalize eight packed words, then expand a nibble stream.

_TEXT segment byte public 'CODE'
assume cs:_TEXT

public _f6eff

_f6eff proc near
    push bp
    mov bp,sp
    push si
    push di
    push ds
    cld
    les di,dword ptr [bp+4]
    mov ax,es
    mov ds,ax
    mov bx,di
    mov cx,8
clear_upper_nibbles:
    mov ax,es:[di]
    and ax,0f0fh
    stosw
    loop clear_upper_nibbles
    add di,10h
    mov si,di
    lodsw
    mul ah
    mov cx,ax
    mov di,si
expand_next_byte:
    lodsb
    mov ah,al
    and ax,0f00fh
    xlatb
    xchg ah,al
    shr al,1
    shr al,1
    shr al,1
    shr al,1
    xlatb
    shl al,1
    shl al,1
    shl al,1
    shl al,1
    or al,ah
    stosb
    loop expand_next_byte
    pop ds
    pop di
    pop si
    pop bp
    ret
_f6eff endp

_TEXT ends
end
