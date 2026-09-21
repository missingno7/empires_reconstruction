; Original entry F_CA03 -- dispatch a record operation selected in AH.

_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

public _sound_ctlblock_command_dispatch
extrn _sound_ctlblock_flags_latch:near
extrn _stream_level_percent_scale:near
extrn _stream_level_base_set:near
extrn _stream_note_delay_set:near

_sound_ctlblock_command_dispatch proc near
    push bp
    mov bp,sp
    mov al,es:[di+1]
    cmp ah,0
    jne check_two_or_less
    call _sound_ctlblock_flags_latch
    jmp short done
    nop
check_two_or_less:
    cmp ah,2
    jg check_three
    call _stream_level_percent_scale
    jmp short done
    nop
check_three:
    cmp ah,3
    jne check_four
    call _stream_note_delay_set
    jmp short done
    nop
check_four:
    cmp ah,4
    jne done
    call _stream_level_base_set
done:
    pop bp
    ret
_sound_ctlblock_command_dispatch endp

_TEXT ends
end
