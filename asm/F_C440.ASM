; F_C440 -- advance three two-byte slots in the 177Ch control-state block.
; This owner has no OMF relocations; the local renderer call remains a same-
; segment relative expression to preserve that historical object topology.
_TEXT segment byte public 'CODE'
assume cs:_TEXT, ds:DGROUP
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA, _BSS

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

public _f_c440
_f_c440:
    push bp
    mov bp, sp
    xor si, si
    mov bl, 90h
slot_loop:
    cmp word ptr [si+state_active], 0
    jz short slot_ready
    jmp slot_done
slot_ready:
    cmp word ptr [si+state_ready], 0
    jnz short slot_mode
    jmp slot_done
slot_mode:
    cmp word ptr [si+state_mode], 1
    jnz short alternate_mode
    mov word ptr [si+state_mode], 0
    cmp word ptr [si+state_index], 2
    jl short reset_cursor
    cmp word ptr [si+state_index], 3
    jz short select_char
    mov word ptr [si+state_index], 0
    jmp short select_char
    nop
reset_cursor:
    mov word ptr [si+state_cursor_mode], 0
    mov cx, [si+state_cursor]
    mov [si+state_cursor_next], cx
    jmp short select_char
    nop
alternate_mode:
    cmp word ptr [si+state_guard], 1
    jnz short alternate_index
    mov al, 0fh
    jmp short render
    nop
alternate_index:
    cmp word ptr [si+state_index], 1
    jz short select_char
    cmp word ptr [si+state_index], 3
    jz short select_char
    cmp word ptr [si+state_allow], 1
    jz short select_char
    mov cx, [si+state_limit]
    cmp [si+state_value], cx
    jng short render_blank
select_char:
    cmp word ptr [si+state_cursor_mode], 0
    jz short read_char
    cmp word ptr [si+state_cursor_mode], 1
    jg short render_blank
    mov ax, [si+state_text]
    jmp short render
    nop
render_blank:
    mov al, 0fh
    jmp short render
    nop
read_char:
    mov di, [si+state_cursor_next]
    inc word ptr [si+state_cursor_next]
    mov al, [di]
    cmp al, 0ffh
    jnz short render
    mov word ptr [si+state_cursor_mode], 1
    jmp short slot_done
    nop
render:
    add al, bl
    call $+renderer_delta
slot_done:
    add bl, 20h
    add si, 2
    cmp si, 6
    jg short done
    jmp slot_loop
done:
    pop bp
    ret
_TEXT ends
end
