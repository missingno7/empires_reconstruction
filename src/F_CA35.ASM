; Original entry F_CA35 -- select the word state at DS:1E94h from live AL.

_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

public _f_ca35

_f_ca35 proc near
    push bp
    mov bp,sp
    cmp al,0
    je short set_one
    mov word ptr ds:[1e94h],0
    xor ah,ah
    mov ds:[1e8eh],ax
    pop bp
    ret
set_one:
    mov word ptr ds:[1e94h],1
    pop bp
    ret
_f_ca35 endp

_TEXT ends
end
