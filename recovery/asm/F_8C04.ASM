; Original entry F_8C04 -- cleanup and boolean return for the F_880A loop.
; The preceding owner establishes the BP frame and preserves SI/DI.

_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

public _f_8c04
extrn _dialog_restore_screen:near
extrn _menu_list_enable:near
extrn _sound_request_count_dec:near
extrn _ui_overlay_hide:near
extrn _keyboard_chain_disable:near
extrn _keyboard_buffer_drain:near

_f_8c04 proc near
    call _dialog_restore_screen
    cmp word ptr [bp-8],0
    je skip_state_flag
    call _menu_list_enable
skip_state_flag:
    call _sound_request_count_dec
    call _ui_overlay_hide
    cmp word ptr [bp-0ah],0
    jne skip_clear_flag
    call _keyboard_chain_disable
skip_clear_flag:
    call _keyboard_buffer_drain
    or si,si
    jge nonnegative_result
    xor ax,ax
    jmp short result_epilogue
    jmp short result_epilogue
nonnegative_result:
    mov ax,1
    jmp short result_epilogue
result_epilogue:
    pop di
    pop si
    mov sp,bp
    pop bp
    ret
_f_8c04 endp

_TEXT ends
end
