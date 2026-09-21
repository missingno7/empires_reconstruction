; F_C359 -- select and submit one value from the sound control state.
; This routine consumes AL/AH/SI from its caller.  Its direct near calls were
; emitted without OMF fixups by the original compiler, so the same-segment
; offsets are deliberately retained as symbolic relative expressions.
_TEXT segment byte public 'CODE'
assume cs:_TEXT, ds:DGROUP
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA, _BSS

snd_paused equ 1778h
voice_table_a equ 17fch
voice_table_b equ 1814h
voice_state equ 177ch

public _sound_control_value_select
_sound_control_value_select:
    push bp
    mov bp, sp
    cmp word ptr ds:[snd_paused], 1
    jz short paused_mode
    cmp word ptr ds:[snd_paused], 2
    jz short alternate_table

    mov cl, ah
    mov bx, voice_table_a
    dec al
    shl al, 1
    xor ah, ah
    add bx, ax
    mov ax, [bx]
    shr ax, cl
    call $+38bh             ; F_C706
    call $+2fah             ; F_C678
    jmp short done
    nop

alternate_table:
    dec al
    xor cx, cx
    mov cl, ah
    shl cx, 1
    shl cx, 1
    mov bx, cx
    shl cx, 1
    add cx, bx
    add al, cl
    xor ah, ah
    call $+36eh             ; F_C706
    call $+2ddh             ; F_C678
    jmp short done
    nop

paused_mode:
    cmp si, 6
    jz short reset_voice
    cmp ah, 0
    jz short select_table_b
    dec ah
select_table_b:
    mov cl, ah
    mov bx, voice_table_b
    dec al
    shl al, 1
    xor ah, ah
    add bx, ax
    mov ax, [bx]
    shr ax, cl
    cmp ax, 3ffh
    jng short submit_value
    shr ax, 1
submit_value:
    call $+341h             ; F_C706
    jmp short done
    nop

reset_voice:
    cmp word ptr [si+voice_state], 0
    jnz short done
    dec al
    add al, 0e0h
    call $+4feh             ; F_C8D4
done:
    pop bp
    ret
_TEXT ends
end
