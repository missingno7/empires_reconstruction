; M_D818_D825 -- reset and append operations for the DS:2F30h packed record table.

_TEXT segment byte public 'CODE'

_DATA segment word public 'DATA'

_DATA ends

_BSS segment word public 'BSS'

_BSS ends

DGROUP group _DATA,_BSS

assume cs:_TEXT, ds:DGROUP

public _fd818, _fd825



; Original entry F_D818 -- reset the DS:2F30h byte counter.



_fd818 proc near
    push bp
    mov bp,sp
    push di
    mov di,2f30h
    mov byte ptr [di],0
    pop di
    pop bp
    ret
_fd818 endp

; Original entry F_D825 -- append one packed five-byte record at DS:2F30h.
;
; The leading byte is the record count.  The routine increments it, computes
; the new record address, writes the caller's byte plus two packed words, and
; returns the address of the final word.



_fd825 proc near
    push bp
    mov bp,sp
    push di
    mov ax,ds
    mov es,ax
    mov di,2f30h
    mov bl,[di]
    inc byte ptr [di]
    inc di
    mov al,bl
    shl al,1
    shl al,1
    add al,bl
    sub ah,ah
    add di,ax
    mov al,[bp+4]
    stosb
    mov bx,[bp+8]
    mov ax,[bp+6]
    mov ah,bl
    stosw
    mov bx,[bp+12]
    mov ax,[bp+10]
    mov ah,bl
    stosw
    mov ax,di
    sub ax,4
    pop di
    pop bp
    ret
_fd825 endp

_TEXT ends

end

