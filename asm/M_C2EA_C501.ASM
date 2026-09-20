; M_C2EA_C501 -- contiguous command-stream and sound-control routines.
; The dispatcher owns the first, third, and fifth entries directly; all five
; routines use the same 1770h..1814h control-state area.
_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA, _BSS
assume cs:_TEXT, ds:DGROUP
public _f_c2ea, _f_c359, _f_c3db, _f_c440, _f_c501

; F_C2EA -- dispatch the low nibble of the current ES:DI command byte.
; AL, AH, SI, ES and DI are caller-owned state.  The original compiler emitted
; all helper calls as non-relocating near calls; retain those relationships as
; same-segment expressions rather than manufacturing OMF fixups.

command_cursor equ 178ch
command_value equ 179ch
command_guard equ 17ech
command_state equ 17ach

_f_c2ea:
    push bp
    mov bp, sp
C2EA_dispatch:
    mov di, [si+command_cursor]
    mov al, es:[di]
    mov ah, al
    shr ah, 1
    shr ah, 1
    shr ah, 1
    shr ah, 1
    and al, 0fh
    cmp al, 0
    jz short C2EA_command_zero
    cmp al, 0dh
    jz short C2EA_command_d
    cmp al, 0eh
    jz short C2EA_command_e
    cmp al, 0fh
    jz short C2EA_command_f
    call _f_c359
    call _f_c3db
    mov word ptr [si+command_guard], 0
    jmp short C2EA_advance
    nop

C2EA_command_zero:
    call $+39ah             ; F_C6B9
    call _f_c3db
    mov word ptr [si+command_guard], 1
    jmp short C2EA_advance
    nop

C2EA_command_d:
    call _f_c501
    cmp word ptr [si+command_value], 0
    jmp short C2EA_advance
    nop

C2EA_command_e:
    call $+298h             ; F_C5D1
    jmp short C2EA_advance
    nop

C2EA_command_f:
    mov word ptr [si+command_state], 0
    call $+374h             ; F_C6B9
    jmp short C2EA_done
    nop

C2EA_advance:
    add word ptr [si+command_cursor], 2
    cmp word ptr [si+command_value], 0
    jz short C2EA_dispatch
C2EA_done:
    pop bp
    ret

; F_C359 -- select and submit one value from the sound control state.
; This routine consumes AL/AH/SI from its caller.  Its direct near calls were
; emitted without OMF fixups by the original compiler, so the same-segment
; offsets are deliberately retained as symbolic relative expressions.

snd_paused equ 1778h
voice_table_a equ 17fch
voice_table_b equ 1814h
voice_state equ 177ch

_f_c359:
    push bp
    mov bp, sp
    cmp word ptr ds:[snd_paused], 1
    jz short C359_paused_mode
    cmp word ptr ds:[snd_paused], 2
    jz short C359_alternate_table

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
    jmp short C359_done
    nop

C359_alternate_table:
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
    jmp short C359_done
    nop

C359_paused_mode:
    cmp si, 6
    jz short C359_reset_voice
    cmp ah, 0
    jz short C359_select_table_b
    dec ah
C359_select_table_b:
    mov cl, ah
    mov bx, voice_table_b
    dec al
    shl al, 1
    xor ah, ah
    add bx, ax
    mov ax, [bx]
    shr ax, cl
    cmp ax, 3ffh
    jng short C359_submit_value
    shr ax, 1
C359_submit_value:
    call $+341h             ; F_C706
    jmp short C359_done
    nop

C359_reset_voice:
    cmp word ptr [si+voice_state], 0
    jnz short C359_done
    dec al
    add al, 0e0h
    call $+4feh             ; F_C8D4
C359_done:
    pop bp
    ret

; F_C3DB -- derive a command value from ES:DI state and update its selector.
; Register inputs (SI, ES:DI) are part of this dispatch cluster's internal
; calling convention.  This contribution has no OMF fixups.

; Both 1788h and 178Ah are word memory operands.  TASM 1.0 discards the
; memory type of an EQU in these contexts, so retain typed absolute operands.
command_index equ 17f4h
command_mode equ 17e4h

_f_c3db:
    push bp
    mov bp, sp
    mov al, es:[di+1]
    mov ah, al
    mov bx, word ptr ds:[1788h]
    add bx, word ptr ds:[178ah]
    test ah, 80h
    jz short C3DB_decode_flags
    and al, 7fh
    jmp short C3DB_store_value
C3DB_decode_flags:
    and al, 7
    mov cl, al
    shr bx, cl
    test ah, 8
    jz short C3DB_test_index_flag
    mov dx, bx
    shr dx, 1
    add bx, dx
C3DB_test_index_flag:
    test ah, 10h
    jz short C3DB_increment_index
    cmp word ptr [si+command_index], 3
    jz short C3DB_store_value
    cmp word ptr [si+command_index], 1
    jz short C3DB_force_index_three
    inc word ptr [si+command_index]
    jmp short C3DB_store_value
    nop
C3DB_force_index_three:
    mov word ptr [si+command_index], 3
    jmp short C3DB_store_value
    nop
C3DB_increment_index:
    cmp word ptr [si+command_index], 0
    jz short C3DB_store_value
    inc word ptr [si+command_index]
C3DB_store_value:
    mov [si+command_value], bx
    mov word ptr [si+command_mode], 1
    pop bp
    ret

; F_C440 -- advance three two-byte slots in the 177Ch control-state block.
; This owner has no OMF relocations; the local renderer call remains a same-
; segment relative expression to preserve that historical object topology.

state_active equ 177ch
state_ready equ 17ach
state_mode equ 17e4h
state_index equ 17f4h
state_cursor_mode equ 17d4h
state_cursor equ 17c4h
state_cursor_next equ 17cch
state_guard equ 17ech
state_allow equ 17b4h
state_limit equ 17bch
state_value equ 179ch
state_text equ 17dch
renderer_delta equ 3e6h

_f_c440:
    push bp
    mov bp, sp
    xor si, si
    mov bl, 90h
C440_slot_loop:
    cmp word ptr [si+state_active], 0
    jz short C440_slot_ready
    jmp C440_slot_done
C440_slot_ready:
    cmp word ptr [si+state_ready], 0
    jnz short C440_slot_mode
    jmp C440_slot_done
C440_slot_mode:
    cmp word ptr [si+state_mode], 1
    jnz short C440_alternate_mode
    mov word ptr [si+state_mode], 0
    cmp word ptr [si+state_index], 2
    jl short C440_reset_cursor
    cmp word ptr [si+state_index], 3
    jz short C440_select_char
    mov word ptr [si+state_index], 0
    jmp short C440_select_char
    nop
C440_reset_cursor:
    mov word ptr [si+state_cursor_mode], 0
    mov cx, [si+state_cursor]
    mov [si+state_cursor_next], cx
    jmp short C440_select_char
    nop
C440_alternate_mode:
    cmp word ptr [si+state_guard], 1
    jnz short C440_alternate_index
    mov al, 0fh
    jmp short C440_render
    nop
C440_alternate_index:
    cmp word ptr [si+state_index], 1
    jz short C440_select_char
    cmp word ptr [si+state_index], 3
    jz short C440_select_char
    cmp word ptr [si+state_allow], 1
    jz short C440_select_char
    mov cx, [si+state_limit]
    cmp [si+state_value], cx
    jng short C440_render_blank
C440_select_char:
    cmp word ptr [si+state_cursor_mode], 0
    jz short C440_read_char
    cmp word ptr [si+state_cursor_mode], 1
    jg short C440_render_blank
    mov ax, [si+state_text]
    jmp short C440_render
    nop
C440_render_blank:
    mov al, 0fh
    jmp short C440_render
    nop
C440_read_char:
    mov di, [si+state_cursor_next]
    inc word ptr [si+state_cursor_next]
    mov al, [di]
    cmp al, 0ffh
    jnz short C440_render
    mov word ptr [si+state_cursor_mode], 1
    jmp short C440_slot_done
    nop
C440_render:
    add al, bl
    call $+renderer_delta
C440_slot_done:
    add bl, 20h
    add si, 2
    cmp si, 6
    jg short C440_done
    jmp C440_slot_loop
C440_done:
    pop bp
    ret

; F_C501 -- dispatch one secondary stream command by the AH selector.
; The six helper calls are direct same-segment relationships in the original
; no-fixup object, retained here as symbolic relative expressions.
_f_c501:
 push bp
 mov bp,sp
 mov al,es:[di+1]
 cmp ah,0
 jnz short C501_test_two
 call $+3ch
 jmp short C501_done
 nop
C501_test_two:
 cmp ah,2
 jg short C501_test_three
 call $+4fh
 jmp short C501_done
 nop
C501_test_three:
 cmp ah,3
 jnz short C501_test_four
 call $+85h
 jmp short C501_done
 nop
C501_test_four:
 cmp ah,4
 jnz short C501_test_five
 call $+6ch
 jmp short C501_done
 nop
C501_test_five:
 cmp ah,5
 jnz short C501_test_six
 call $+7ah
 jmp short C501_done
 nop
C501_test_six:
 cmp ah,6
 jnz short C501_done
 call $+82h
C501_done:
 pop bp
 ret

_TEXT ends
end
