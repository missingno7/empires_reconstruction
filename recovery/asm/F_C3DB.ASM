; F_C3DB -- derive a command value from ES:DI state and update its selector.
; Register inputs (SI, ES:DI) are part of this dispatch cluster's internal
; calling convention.  This contribution has no OMF fixups.
_TEXT segment byte public 'CODE'
assume cs:_TEXT, ds:DGROUP
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA, _BSS

; Both 1788h and 178Ah are word memory operands.  TASM 1.0 discards the
; memory type of an EQU in these contexts, so retain typed absolute operands.
command_value equ 179ch
command_index equ 17f4h
command_mode equ 17e4h

public _sound_command_value_derive
_sound_command_value_derive:
    push bp
    mov bp, sp
    mov al, es:[di+1]
    mov ah, al
    mov bx, word ptr ds:[1788h]
    add bx, word ptr ds:[178ah]
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
    cmp word ptr [si+command_index], 3
    jz short store_value
    cmp word ptr [si+command_index], 1
    jz short force_index_three
    inc word ptr [si+command_index]
    jmp short store_value
    nop
force_index_three:
    mov word ptr [si+command_index], 3
    jmp short store_value
    nop
increment_index:
    cmp word ptr [si+command_index], 0
    jz short store_value
    inc word ptr [si+command_index]
store_value:
    mov [si+command_value], bx
    mov word ptr [si+command_mode], 1
    pop bp
    ret
_TEXT ends
end
