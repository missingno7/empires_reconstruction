; Original entry F_DB35 -- convert a signed state-table entry into a bar call.
;
; The table at DS:2FD2h has two-byte entries.  The high byte selects the
; source byte at SI+2FE4h; F_C898 receives the fixed 3Fh limit and that byte
; shifted into the 40h-based display range.

_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

public _voice_channel_apply_freq
extrn _opl_register_write:near

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
