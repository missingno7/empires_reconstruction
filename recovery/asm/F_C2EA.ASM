; F_C2EA -- dispatch the low nibble of the current ES:DI command byte.
; AL, AH, SI, ES and DI are caller-owned state.  The original compiler emitted
; all helper calls as non-relocating near calls; retain those relationships as
; same-segment expressions rather than manufacturing OMF fixups.
_TEXT segment byte public 'CODE'
assume cs:_TEXT, ds:DGROUP
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA, _BSS

command_cursor equ 178ch
command_value equ 179ch
command_guard equ 17ech
command_state equ 17ach

public _sound_command_stream_dispatch
_sound_command_stream_dispatch:
    push bp
    mov bp, sp
dispatch:
    mov di, [si+command_cursor]
    mov al, es:[di]
    mov ah, al
    shr ah, 1
    shr ah, 1
    shr ah, 1
    shr ah, 1
    and al, 0fh
    cmp al, 0
    jz short command_zero
    cmp al, 0dh
    jz short command_d
    cmp al, 0eh
    jz short command_e
    cmp al, 0fh
    jz short command_f
    call $+49h              ; F_C359
    call $+0c8h             ; F_C3DB
    mov word ptr [si+command_guard], 0
    jmp short advance
    nop

command_zero:
    call $+39ah             ; F_C6B9
    call $+0b9h             ; F_C3DB
    mov word ptr [si+command_guard], 1
    jmp short advance
    nop

command_d:
    call $+1d3h             ; F_C501
    cmp word ptr [si+command_value], 0
    jmp short advance
    nop

command_e:
    call $+298h             ; F_C5D1
    jmp short advance
    nop

command_f:
    mov word ptr [si+command_state], 0
    call $+374h             ; F_C6B9
    jmp short done
    nop

advance:
    add word ptr [si+command_cursor], 2
    cmp word ptr [si+command_value], 0
    jz short dispatch
done:
    pop bp
    ret
_TEXT ends
end
