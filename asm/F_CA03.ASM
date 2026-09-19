; Original entry F_CA03 -- dispatch a record operation selected in AH.

_TEXT segment byte public 'CODE'
assume cs:_TEXT

public _f_ca03
extrn _f_ca35:near
extrn _f_ca51:near
extrn _fca83:near
extrn _fca91:near

_f_ca03 proc near
    push bp
    mov bp,sp
    mov al,es:[di+1]
    cmp ah,0
    jne check_two_or_less
    call _f_ca35
    jmp short done
    nop
check_two_or_less:
    cmp ah,2
    jg check_three
    call _f_ca51
    jmp short done
    nop
check_three:
    cmp ah,3
    jne check_four
    call _fca91
    jmp short done
    nop
check_four:
    cmp ah,4
    jne done
    call _fca83
done:
    pop bp
    ret
_f_ca03 endp

_TEXT ends
end
