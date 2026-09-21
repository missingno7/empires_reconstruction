; F_C9A4 -- decode a table command byte into the secondary selection state.
; The contribution has no OMF fixups; all state accesses are typed direct
; DGROUP words, preserving the original compact object topology.
_TEXT segment byte public 'CODE'
assume cs:_TEXT, ds:DGROUP
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA, _BSS

public _sound_stream_delay_decode
_sound_stream_delay_decode:
    push bp
    mov bp, sp
    mov al, es:[di+1]
    mov ah, al
    mov bx, word ptr ds:[1e84h]
    add bx, word ptr ds:[1e86h]
    test ah, 80h
    jz short decode_flags
    and al, 7fh
    jmp short store_value
decode_flags:
    and al, 7
    mov cl, al
    shr bx, cl
    test ah, 8
    jz short test_index_flag
    mov dx, bx
    shr dx, 1
    add bx, dx
test_index_flag:
    test ah, 10h
    jz short increment_index
    cmp word ptr ds:[1e8ch], 3
    jz short store_value
    cmp word ptr ds:[1e8ch], 1
    jz short force_index_three
    inc word ptr ds:[1e8ch]
    jmp short store_value
    nop
force_index_three:
    mov word ptr ds:[1e8ch], 3
    jmp short store_value
    nop
increment_index:
    cmp word ptr ds:[1e8ch], 0
    jz short store_value
    inc word ptr ds:[1e8ch]
store_value:
    mov word ptr ds:[1e90h], bx
    pop bp
    ret
_TEXT ends
end
