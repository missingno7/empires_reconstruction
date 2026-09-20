; F_643A -- rewrite one indexed persistent record, then refresh its metadata.

_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
extrn _gb40:word, _gb3e:word, _gc0c9:word, _gc0c8:byte
extrn _gb33:word, _gb31:word, _gb2a:word
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

public _f643a
extrn _f6266:near, _lseek:near, _read:near, _write:near, _close:near
extrn _f652a:near, _f86c9:near

_f643a proc near
    push bp
    mov bp,sp
    sub sp,0ch
    push si
    push di
    mov si,[bp+4]
    mov di,si
    mov cl,0ch
    shr di,cl
    mov ax,_gb40
    mov [bp-2],ax
    and si,0fffh
    mov word ptr _gb40,1
F643A_retry:
    mov word ptr _gb3e,0
    push di
    call _f6266
    pop cx

    xor ax,ax
    push ax
    mov ax,si
    shl ax,1
    shl ax,1
    xor dx,dx
    push dx
    push ax
    push word ptr _gc0c9
    call _lseek
    add sp,8

    mov ax,4
    push ax
    push ss
    lea ax,[bp-0ah]
    push ax
    push word ptr _gc0c9
    call _read
    add sp,8
    mov ax,4
    push ax
    push ss
    lea ax,[bp-6]
    push ax
    push word ptr _gc0c9
    call _read
    add sp,8

    mov ax,[bp-6]
    sub ax,[bp-0ah]
    mov [bp-0ch],ax
    xor ax,ax
    push ax
    mov dx,[bp-8]
    mov ax,[bp-0ah]
    add ax,2
    adc dx,0
    push dx
    push ax
    push word ptr _gc0c9
    call _lseek
    add sp,8

    mov ax,[bp-0ch]
    add ax,0fffeh
    push ax
    push word ptr [bp+8]
    push word ptr [bp+6]
    push word ptr _gc0c9
    call _write
    add sp,8
    push word ptr _gc0c9
    call _close
    pop cx

    cmp word ptr _gb3e,0
    jz short F643A_after_metadata
    mov al,_gc0c8
    cbw
    push ax
    call _f652a
    pop cx
    mov _gb33,ds
    mov word ptr _gb31,0ac7h
    push ds
    mov ax,0b2ah
    push ax
    call _f86c9
    pop cx
    pop cx
    mov al,_gc0c8
    cbw
    push ax
    call _f652a
    pop cx
F643A_after_metadata:
    cmp word ptr _gb3e,0
    jz short F643A_done
    jmp F643A_retry

F643A_done:
    mov ax,[bp-2]
    mov _gb40,ax
    sti
    pop di
    pop si
    mov sp,bp
    pop bp
    ret
_f643a endp

_TEXT ends
end
