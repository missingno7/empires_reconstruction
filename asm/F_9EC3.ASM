; F_9EC3 -- copy a clipped row through indexed source and destination tables.

_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
extrn _g3924:dword
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

public _f9ec3

_f9ec3 proc near
    push bp
    mov bp,sp
    sub sp,2
    push si
    push di
    push ds
    cld
    mov bx,[bp+0eh]
    shl bx,1
    shl bx,1
    les di,_g3924[bx]
    add di,[bp+0ch]
    mov bx,[bp+6]
    shl bx,1
    shl bx,1
    lds si,_g3924[bx]
    add si,[bp+4]
    mov dx,[bp+8]
    mov cx,[bp+0ah]
    mov ax,[bp+12h]
    sub ax,dx
    mov [bp-2],ax
    mov ax,[bp+10h]
    push ds
    mov bx,DGROUP
    mov ds,bx
    lea bx,ds:[12b0h]
    xlat
    pop ds
F9_row:
    push ds
    mov bx,DGROUP
    mov ds,bx
    lea bx,ds:[12c0h]
    xlat
    pop ds
F9_segment:
    cmp dx,ax
    jng short F9_tail
    add si,ax
    add di,ax
    movsb
    add si,0fh
    sub si,ax
    add di,0fh
    sub di,ax
    sub dx,10h
    jg short F9_segment
F9_tail:
    add si,dx
    add di,dx
    mov dx,[bp+8]
    add si,[bp-2]
    add di,[bp-2]
    loop F9_row
    pop ds
    pop di
    pop si
    mov sp,bp
    pop bp
    ret
_f9ec3 endp

_TEXT ends
end
