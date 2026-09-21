; M_C5D1_C706 -- sound command dispatch, speaker control, and PIT output.
;
; These four adjacent entries form one 388-byte no-fixup contribution.  The
; historical compiler encoded calls outside this contribution as direct
; same-segment displacements, retained below as symbolic relative expressions.

_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

public _voice_command_decode_apply, _voice_enable, _voice_disable, _sound_pit_divisor_program

; Original entry F_C5D1 -- decode one command byte and update the selected
; SI-relative voice-control slot.
_voice_command_decode_apply proc near
    push bp
    mov bp,sp
    mov dl,es:[di+1]
    xor dh,dh
    cmp dx,0
    jnz short C5D1_have_value
    call _voice_disable
    mov word ptr [si+17ech],1
    jmp C5D1_store_result
C5D1_have_value:
    cmp word ptr ds:[1778h],1
    jz short C5D1_mode_one
    cmp word ptr ds:[1778h],2
    jz short C5D1_mode_two
    mov cl,7
    shl dx,cl
    neg dx
    mov cl,ah
    mov bx,word ptr ds:[1778h]
    shl bx,1
    mov bx,[bx+182ch]
    mov ax,[bx]
    add ax,dx
    shr ax,cl
    call _sound_pit_divisor_program
    call _voice_enable
    jmp short C5D1_store_result
    nop
C5D1_mode_two:
    mov word ptr [si+17e4h],1
    jmp short C5D1_store_result
    nop
C5D1_mode_one:
    mov bx,dx
    shl bx,1
    add bx,dx
    shr bx,1
    shr bx,1
    inc bx
    mov cl,ah
    cmp cl,0
    jnz short C5D1_have_shift
    inc cl
C5D1_have_shift:
    mov dx,0d70h
    shr dx,cl
    cmp cl,4
    jz short C5D1_scaled
    jg short C5D1_shift_down
    mov al,4
    sub al,cl
    mov cl,al
    shl bx,cl
    jmp short C5D1_scaled
    nop
C5D1_shift_down:
    sub cl,4
    shr bx,cl
C5D1_scaled:
    sub dx,bx
    cmp dx,400h
    jng short C5D1_frequency_ready
    shr dx,1
C5D1_frequency_ready:
    mov ax,dx
    call _sound_pit_divisor_program
    mov word ptr [si+17e4h],1
    mov word ptr [si+17ech],0
C5D1_store_result:
    mov ax,[si+17a4h]
    mov [si+179ch],ax
    pop bp
    ret
_voice_command_decode_apply endp

; Original entry F_C678 -- enable a voice, either by opening the speaker gate
; or by queueing its selected value for the alternate sound backend.
_voice_enable proc near
    push bp
    mov bp,sp
    cmp word ptr [si+177ch],0
    jnz short C678_done
    cmp word ptr ds:[1778h],1
    jz short C678_open_alt_gate
    cmp word ptr ds:[1778h],2
    jz short C678_queue_value
    in al,61h
    or al,3
    out 61h,al
    jmp short C678_done
    nop
C678_queue_value:
    push si
    push di
    push es
    mov ax,[si+1766h]
    push ax
    mov ax,si
    shr ax,1
    push ax
    call $+14bah            ; historical target F_DA60
    pop ax
    pop ax
    pop es
    pop di
    pop si
    jmp short C678_done
    nop
C678_open_alt_gate:
    in al,61h
    or al,60h
    out 61h,al
C678_done:
    pop bp
    ret
_voice_enable endp

; Original entry F_C6B9 -- disable a voice or submit an alternate-backend
; voice update.
_voice_disable proc near
    push bp
    mov bp,sp
    cmp word ptr [si+177ch],0
    jnz short C6B9_done
    cmp word ptr ds:[1778h],1
    jz short C6B9_queue_reset
    cmp word ptr ds:[1778h],2
    jz short C6B9_queue_stop
    in al,61h
    and al,0fch
    out 61h,al
    jmp short C6B9_done
    nop
C6B9_queue_stop:
    push si
    push di
    push es
    push cx
    mov ax,si
    shr ax,1
    push ax
    call $+15e4h            ; historical target F_D6F6
    pop ax
    pop cx
    pop es
    pop di
    pop si
    jmp short C6B9_done
    nop
C6B9_queue_reset:
    mov ax,si
    shl al,1
    shl al,1
    shl al,1
    shl al,1
    add al,90h
    or al,0fh
    call $+1d8h             ; historical target F_C8D4
    xor ax,ax
    call _sound_pit_divisor_program
C6B9_done:
    pop bp
    ret
_voice_disable endp

; Original entry F_C706 -- write a PIT divisor, queue it, or emit its two
; packed nibbles for the alternate sound backend.
_sound_pit_divisor_program proc near
    push bp
    mov bp,sp
    cmp word ptr [si+177ch],0
    jnz short C706_done
    cmp word ptr ds:[1778h],1
    jz short C706_emit_nibbles
    cmp word ptr ds:[1778h],2
    jz short C706_store_queued
    out 42h,al
    mov al,ah
    out 42h,al
    jmp short C706_done
    nop
C706_store_queued:
    mov [si+1766h],ax
    jmp short C706_done
    nop
C706_emit_nibbles:
    mov bx,ax
    mov ax,si
    shl al,1
    shl al,1
    shl al,1
    shl al,1
    add al,80h
    mov ah,bl
    and ah,0fh
    or al,ah
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    call $+189h             ; historical target F_C8D4
    mov al,bl
    call $+184h             ; historical target F_C8D4
C706_done:
    pop bp
    ret
_sound_pit_divisor_program endp

_TEXT ends
end
