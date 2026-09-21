; F_AB66 -- initialise the indexed record workspace, run the selection loop,
; then retain the selected record's two descriptor fields for later rendering.

_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

public _slot_menu_run

_slot_menu_run proc near
    push bp
    mov bp,sp
    sub sp,4
    push si
    push di
    call $+0c006h
    mov [bp-2],ax
    call $+0cdb1h
    call $+01fd1h
    call $+01cbah
    mov word ptr ds:[237eh],0ffffh
    call $+02a10h
    mov word ptr ds:[98h],0
    mov word ptr ds:[9ah],9fh
    call $+0f50bh

    push word ptr ds:[0c5c8h]
    push word ptr ds:[0c5c6h]
    mov ax,0c8h
    push ax
    xor ax,ax
    push ax
    call $+05825h
    add sp,8
    xor ax,ax
    push ax
    call $+05621h
    pop cx
    call $+0f672h
    mov word ptr ds:[0c57eh],ax

    mov word ptr [bp-4],0f9h
    xor di,di
    jmp short AB66_record_test
AB66_record_body:
    mov ax,1
    push ax
    add word ptr [bp-4],0bh
    mov ax,[bp-4]
    push ax
    push ds
    mov ax,di
    mov dx,1bh
    mul dx
    add ax,0c470h
    push ax
    call $+0f6b5h
    add sp,8
    inc di
AB66_record_test:
    cmp di,word ptr ds:[0c57eh]
    jl short AB66_record_body

    xor ax,ax
    push ax
    xor ax,ax
    push ax
    mov ax,0c8h
    push ax
    mov ax,140h
    push ax
    mov ax,0c8h
    push ax
    xor ax,ax
    push ax
    call $+0f346h
    add sp,0ch

    push word ptr ds:[0c5c8h]
    push word ptr ds:[0c5c6h]
    mov ax,0c8h
    push ax
    xor ax,ax
    push ax
    call $+057bah
    add sp,8
    mov word ptr ds:[0c46eh],1
    call $+0bd75h
AB66_refresh_records:
    call $+0f605h
    mov word ptr ds:[0c57eh],ax
    or ax,ax
    jnz short AB66_have_records
    mov word ptr ds:[0c46eh],0
AB66_have_records:
    call $+0bf38h
    call $+0f70eh
    cmp word ptr ds:[0c46eh],0
    jz short AB66_no_special_mode
    call $+0fde4h
    mov si,ax
    jmp short AB66_selection_ready
AB66_no_special_mode:
    call $+0fc1ch
    mov si,ax
AB66_selection_ready:
    cmp si,0ffffh
    jnz short AB66_selection_valid
    push ds
    mov ax,139dh
    push ax
    call $+0da78h
    pop cx
    pop cx
    cmp ax,1
    je short AB66_selection_valid
    mov si,0fffeh
AB66_selection_valid:
    cmp si,0ffffh
    jl short AB66_refresh_records

    call $+0f4dch
    cmp word ptr [bp-2],0
    jnz short AB66_keep_display
    call $+0bd2bh
AB66_keep_display:
    mov ax,1
    push ax
    call $+0555bh
    pop cx

    mov ax,0c8h
    push ax
    mov ax,140h
    push ax
    xor ax,ax
    push ax
    xor ax,ax
    push ax
    call $+05723h
    add sp,8
    mov ax,0c8h
    push ax
    mov ax,140h
    push ax
    xor ax,ax
    push ax
    xor ax,ax
    push ax
    call $+05706h
    add sp,8

    mov word ptr ds:[98h],4
    mov word ptr ds:[9ah],9bh
    mov ax,si
    mov dx,1bh
    mul dx
    mov bx,ax
    add bx,0c470h
    push ds
    pop es
    mov ax,es:[bx+0dh]
    mov word ptr ds:[176eh],ax
    mov ax,si
    mov dx,1bh
    mul dx
    mov bx,ax
    add bx,0c470h
    push ds
    pop es
    mov ax,es:[bx+0fh]
    mov word ptr ds:[1772h],ax
    call $+028cfh
    mov ax,si
    mov word ptr ds:[13edh],ax
    jmp short AB66_done
AB66_done:
    pop di
    pop si
    mov sp,bp
    pop bp
    ret
_slot_menu_run endp

_TEXT ends
end
