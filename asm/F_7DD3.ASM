; Original entry F_7DD3 -- cleanup and return path for the F_7964 state loop.
; The preceding owner establishes the BP frame and preserves SI/DI.

_TEXT segment byte public 'CODE'
assume cs:_TEXT

public _f_7dd3
extrn _f7bfc:near
extrn _f791e:near
extrn _fd5a6:near
extrn _f7162:near
extrn _f6997:near
extrn _f6b66:near

_f_7dd3 proc near
    mov ax,0ffffh
    push ax
    call _f7bfc
    pop cx
    cmp word ptr [bp-12h],0
    je skip_state_flag
    call _f791e
skip_state_flag:
    call _fd5a6
    call _f7162
    cmp word ptr [bp-14h],0
    jne skip_clear_flag
    call _f6997
skip_clear_flag:
    call _f6b66
    pop di
    pop si
    mov sp,bp
    pop bp
    ret
_f_7dd3 endp

_TEXT ends
end
