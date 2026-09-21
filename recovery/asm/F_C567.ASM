; Original entry F_C567 -- signed scale-and-store for the sound-data state pair.

_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

public _voice_level_percent_scale
extrn _g1788:word
extrn _g178a:word

_voice_level_percent_scale proc near
    push bp
    mov bp,sp
    mov bl,ah
    mov cl,al
    xor ch,ch
    cmp cx,0
    je store_zero
    mov ax,ds:_g1788
    mul cx
    mov cx,100
    div cx
    cmp dx,31h
    jle no_round_up
    inc ax
no_round_up:
    cmp bl,1
    je store_result
    neg ax
store_result:
    mov ds:_g178a,ax
    jmp short done
    nop
store_zero:
    mov word ptr ds:_g178a,0
done:
    pop bp
    ret
_voice_level_percent_scale endp

_TEXT ends
end
