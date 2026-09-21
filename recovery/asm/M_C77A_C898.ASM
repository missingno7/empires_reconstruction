; M_C77A_C898 -- sound backend selection, voice-state reload, and OPL output.

_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
    extrn _snd_mode:word, _snd_hi:word, _snd_nvoices:word, _snd_backend_mode:word
    extrn _v_a:word, _v_b:word, _snd_backend_mode:word, _g177a:word, _voice_disable:near
    extrn _opl_port:word
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

public _sound_backend_select_init, _sound_voice_table_reload, _sound_voices_reset, _sound_voices_disable_all, _opl_register_write

; Original entry F_C77A -- select and initialise a sound backend.
_sound_backend_select_init proc near
    push bp
    mov bp,sp
    push ax
    mov ax,word ptr ds:[1778h]
    cmp ax,1
    jz short C77A_mode_one
    cmp ax,2
    jz short C77A_mode_two
    cmp ax,3
    jz short C77A_mode_three
    mov word ptr ds:[177ah],1
    jmp short C77A_done
    nop
C77A_mode_two:
    mov word ptr ds:[177ah],4
    call $+11fch            ; historical target F_DB9B
    jmp short C77A_done
    nop
C77A_mode_three:
    mov word ptr ds:[1830h],205h
    mov word ptr ds:[177ah],4
    mov word ptr ds:[1778h],1
    jmp short C77A_done
C77A_mode_one:
    mov word ptr ds:[1830h],0c0h
    mov word ptr ds:[177ah],4
    call $+0feb3h           ; historical target F_C878
C77A_done:
    pop ax
    pop bp
    ret
_sound_backend_select_init endp

; Original entry F_C7CB -- reload per-voice values from the current backend
; table into the two SI-relative sound-control arrays.
_sound_voice_table_reload proc near
    push bp
    mov bp,sp
    push ax
    push cx
    push si
    push di
    push es
    mov es,word ptr ds:[1764h]
    mov di,[bp+4]
    mov word ptr ds:[1786h],di
    cmp word ptr ds:[1772h],0
    jz short C7CB_done
    cmp word ptr ds:[1778h],2
    jnz short C7CB_select_width
    push es
    push di
    call $+1102h            ; historical target F_D8F0
    pop di
    pop es
    mov word ptr ds:[177ah],4
C7CB_select_width:
    mov cl,1
    cmp word ptr ds:[1778h],0
    jz short C7CB_width_ready
    mov cl,3
C7CB_width_ready:
    shl di,cl
    xor si,si
    mov cx,word ptr ds:[177ah]
    add di,word ptr ds:[1762h]
C7CB_copy_voice:
    mov ax,es:[di]
    add ax,word ptr ds:[1762h]
    mov [si+178ch],ax
    mov [si+1794h],ax
    add di,2
    add si,2
    loop C7CB_copy_voice
    mov word ptr ds:[1784h],1
C7CB_done:
    pop es
    pop di
    pop si
    pop cx
    pop ax
    pop bp
    ret
_sound_voice_table_reload endp

; Original entry F_C834 -- reset each configured voice and retain the voice
; count across the reset loop.
_sound_voices_reset proc near
    push bp
    mov bp,sp
    push ax
    push cx
    push si
    mov word ptr _snd_mode,0
    mov word ptr _snd_hi,-1
    xor si,si
    mov cx,_snd_nvoices
    push cx
    cmp word ptr _snd_backend_mode,2
    jnz short C834_reset_voice
    mov word ptr _snd_nvoices,4
C834_reset_voice:
    mov word ptr [si+_v_a],0
    mov word ptr [si+_v_b],0
    call _voice_disable
    add si,2
    loop C834_reset_voice
    pop _snd_nvoices
    pop si
    pop cx
    pop ax
    pop bp
    ret
_sound_voices_reset endp

; Original entry F_C877 -- submit one disabled voice update for every voice.
_sound_voices_disable_all proc near
    push bp
    mov bp,sp
    push cx
    push si
    xor si,si
    mov cx,_g177a
    cmp word ptr _snd_backend_mode,2
    jnz short C877_voice_count_ready
    mov cx,4
C877_voice_count_ready:
    call _voice_disable
    add si,2
    loop C877_voice_count_ready
    pop si
    pop cx
    pop bp
    ret
_sound_voices_disable_all endp

; Original entry F_C898 -- write OPL register and data bytes with the required
; input-port settling reads between them.
_opl_register_write proc near
    push bp
    mov bp,sp
    mov dx,_opl_port
    mov ax,[bp+4]
    out dx,al
    in al,dx
    in al,dx
    in al,dx
    in al,dx
    in al,dx
    in al,dx
    inc dx
    mov ax,[bp+6]
    out dx,al
    dec dx
    rept 35
        in al,dx
    endm
    pop bp
    ret
_opl_register_write endp

_TEXT ends
end
