; Original entry F_DB17 -- fetch a signed state-table byte and dispatch it.
;
; The caller supplies the table index on the stack.  The table starts at
; DS:2FD2h and consists of two-byte entries; its high byte is sign-extended
; and passed to F_E1F2.

_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

public _voice_channel_level_refresh
extrn _voice_write_level:near

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

_TEXT ends
end
