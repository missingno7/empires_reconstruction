; Original entry F_652A -- retry a one-sector BIOS disk read three times.

_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

public _f652a
extrn _gc0c8:byte

_f652a proc near
    push bp
    mov bp,sp
    sub sp,200h
    push si
    xor si,si
    jmp short retry_test
retry_read:
    mov ah,0
    mov dl,ds:_gc0c8
    int 13h
    mov dx,ss
    lea ax,[bp-200h]
    mov es,dx
    mov dx,ss
    lea ax,[bp-200h]
    mov bx,ax
    mov ah,2
    mov al,1
    mov ch,1
    mov cl,1
    mov dh,0
    mov dl,[bp+4]
    int 13h
    inc si
retry_test:
    cmp si,3
    jl retry_read
    mov ah,0dh
    int 21h
    pop si
    mov sp,bp
    pop bp
    ret
_f652a endp

_TEXT ends
end
