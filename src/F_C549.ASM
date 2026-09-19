; Original entry F_C549 -- update SI-relative state words from live AL.

_TEXT segment byte public 'CODE'
assume cs:_TEXT

public _f_c549

_f_c549 proc near
    push bp
    mov bp,sp
    cmp al,0
    je short set_one+1
    mov word ptr ds:[si+17b4h],0
    xor ah,ah
    mov ds:[si+17bch],ax
    jmp short done
set_one:
    nop
    mov word ptr ds:[si+17b4h],1
done:
    pop bp
    ret
_f_c549 endp

_TEXT ends
end
