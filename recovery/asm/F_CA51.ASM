; Original entry F_CA51 -- scale the AX input by the current control value.
; AH selects the sign; the zero-input branch stores zero directly.

_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

public _stream_level_percent_scale
extrn _g1e84:word
extrn _g1e86:word

_stream_level_percent_scale proc near
    push bp
    mov bp,sp
    mov bl,ah
    mov cl,al
    xor ch,ch
    cmp cx,0
    je store_zero
    mov ax,ds:_g1e84
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
    mov ds:_g1e86,ax
    pop bp
    ret
store_zero:
    mov word ptr ds:_g1e86,0
    pop bp
    ret
_stream_level_percent_scale endp

_TEXT ends
end
