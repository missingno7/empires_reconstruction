; Original entry F_CA35 -- select the word state at DS:1E94h from live AL.

_TEXT segment byte public 'CODE'
assume cs:_TEXT

public _f_ca35

_f_ca35 proc near
    push bp
    mov bp,sp
    cmp al,0
    je short set_one
    mov word ptr [1e94h],0
    xor ah,ah
    mov [1e8eh],ax
    pop bp
    ret
set_one:
    mov word ptr [1e94h],1
    pop bp
    ret
_f_ca35 endp

_TEXT ends
end
