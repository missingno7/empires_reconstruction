; Original entry F_C549 -- update SI-relative state words from live AL.

_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

public _sound_command_flags_update

_sound_command_flags_update proc near
    push bp
    mov bp,sp
    cmp al,0
    je short set_one+1
    mov word ptr ds:[si+17b4h],0
    xor ah,ah
    mov ds:[si+17bch],ax
    jmp short done
set_one:
    nop
    mov word ptr ds:[si+17b4h],1
done:
    pop bp
    ret
_sound_command_flags_update endp

_TEXT ends
end
