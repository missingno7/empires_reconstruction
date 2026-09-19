; Original entry F_8C04 -- cleanup and boolean return for the F_880A loop.
; The preceding owner establishes the BP frame and preserves SI/DI.

_TEXT segment byte public 'CODE'
assume cs:_TEXT

public _f_8c04
extrn _f8453:near
extrn _f791e:near
extrn _fd5a6:near
extrn _f7162:near
extrn _f6997:near
extrn _f6b66:near

_f_8c04 proc near
    call _f8453
    cmp word ptr [bp-8],0
    je skip_state_flag
    call _f791e
skip_state_flag:
    call _fd5a6
    call _f7162
    cmp word ptr [bp-0ah],0
    jne skip_clear_flag
    call _f6997
skip_clear_flag:
    call _f6b66
    or si,si
    jge nonnegative_result
    xor ax,ax
    jmp short result_epilogue
    jmp short result_epilogue
nonnegative_result:
    mov ax,1
    jmp short result_epilogue
result_epilogue:
    pop di
    pop si
    mov sp,bp
    pop bp
    ret
_f_8c04 endp

_TEXT ends
end
