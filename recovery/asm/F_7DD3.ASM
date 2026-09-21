; Original entry F_7DD3 -- cleanup and return path for the F_7964 state loop.
; The preceding owner establishes the BP frame and preserves SI/DI.

_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

public _f_7dd3
extrn _menu_list_draw:near
extrn _menu_list_enable:near
extrn _sound_request_count_dec:near
extrn _ui_overlay_hide:near
extrn _keyboard_chain_disable:near
extrn _keyboard_buffer_drain:near

_f_7dd3 proc near
    mov ax,0ffffh
    push ax
    call _menu_list_draw
    pop cx
    cmp word ptr [bp-12h],0
    je skip_state_flag
    call _menu_list_enable
skip_state_flag:
    call _sound_request_count_dec
    call _ui_overlay_hide
    cmp word ptr [bp-14h],0
    jne skip_clear_flag
    call _keyboard_chain_disable
skip_clear_flag:
    call _keyboard_buffer_drain
    pop di
    pop si
    mov sp,bp
    pop bp
    ret
_f_7dd3 endp

_TEXT ends
end
