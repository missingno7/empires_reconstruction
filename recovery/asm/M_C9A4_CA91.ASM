; M_C9A4_CA91 -- table-command decoder and four-way operation dispatcher.
; The command byte at ES:DI+1 selects shared DGROUP state in the 1E84h..1E94h
; control block.  The dispatcher and all four operations are contiguous.
_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
    extrn _g1e84:word, _g1e86:word, _g1e92:word
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP
public _f_c9a4, _f_ca03, _f_ca35, _f_ca51, _fca83, _fca91

_f_c9a4:
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



_f_ca51 proc near
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
_f_ca51 endp



_fca83 proc near
    push bp
    mov bp,sp
    xor ah,ah
    shl ax,1
    shl ax,1
    mov word ptr ds:_g1e84,ax
    pop bp
    ret
_fca83 endp

_fca91 proc near
    push bp
    mov bp,sp
    xor ah,ah
    mov ds:_g1e92,ax
    pop bp
    ret
_fca91 endp

_TEXT ends
end
