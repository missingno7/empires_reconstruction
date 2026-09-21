; M_DAD7_DB35 -- three adjacent state-table command helpers.

; Each reads the signed high byte of a two-byte DS:2FD2h table entry.

_TEXT segment byte public 'CODE'

_DATA segment word public 'DATA'

_DATA ends

_BSS segment word public 'BSS'

    extrn _voice_write_level:near

    extrn _opl_register_write:near

_BSS ends

DGROUP group _DATA,_BSS

assume cs:_TEXT, ds:DGROUP

public _voice_channel_set_gate, _f_dad7_end, _voice_channel_level_refresh, _voice_channel_apply_freq



; F_DAD7 -- clamp a byte selection and submit its associated table entry.
_voice_channel_set_gate:
 push bp
 mov bp,sp
 push si
 cmp word ptr [bp+6],0
 jz short zero_value
 mov ax,7fh
 jmp short store_value
zero_value:
 xor ax,ax
store_value:
 mov [bp+6],ax
 mov bx,[bp+4]
 shl bx,1
 add bx,2fd2h
 push ds
 pop es
 mov al,es:[bx+1]
 cbw
 mov si,ax
 cmp word ptr [bp+6],7fh
 jna short bounded
 mov word ptr [bp+6],7fh
bounded:
 mov al,[bp+6]
 mov [si-35b0h],al
 push si
 call $+6e2h
 pop cx
_f_dad7_end:
 pop si
 pop bp
 ret

; Original entry F_DB17 -- fetch a signed state-table byte and dispatch it.
;
; The caller supplies the table index on the stack.  The table starts at
; DS:2FD2h and consists of two-byte entries; its high byte is sign-extended
; and passed to F_E1F2.



_voice_channel_level_refresh proc near
    push bp
    mov bp,sp
    push si
    mov bx,[bp+4]
    shl bx,1
    add bx,2fd2h
    push ds
    pop es
    mov al,es:[bx+1]
    cbw
    mov si,ax
    push si
    call _voice_write_level
    pop cx
    pop si
    pop bp
    ret
_voice_channel_level_refresh endp

; Original entry F_DB35 -- convert a signed state-table entry into a bar call.
;
; The table at DS:2FD2h has two-byte entries.  The high byte selects the
; source byte at SI+2FE4h; F_C898 receives the fixed 3Fh limit and that byte
; shifted into the 40h-based display range.



_voice_channel_apply_freq proc near
    push bp
    mov bp,sp
    push si
    mov bx,[bp+4]
    shl bx,1
    add bx,2fd2h
    push ds
    pop es
    mov al,es:[bx+1]
    cbw
    mov si,ax
    mov ax,3fh
    push ax
    mov al,[si+2fe4h]
    cbw
    add ax,40h
    push ax
    call _opl_register_write
    pop cx
    pop cx
    pop si
    pop bp
    ret
_voice_channel_apply_freq endp

_TEXT ends

end

