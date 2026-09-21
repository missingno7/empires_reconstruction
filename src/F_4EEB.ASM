; Candidate symbolic assembly for F_4EEB.  This preserves the compiler's
; compact-model calling convention while exposing the recovered control flow.

_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

public _f4eeb
extrn _gfx_color_select:near
extrn _f03a5:near
extrn _f03cc:near
extrn _gb3ae:byte
extrn _board_record_index:word
extrn _resource_ptr_table:dword

_f4eeb proc near
    push bp
    mov bp,sp
    mov ax,0fh
    push ax
    call _gfx_color_select
    pop ax
    push si
    push di
    mov di,offset DGROUP:_gb3ae
    xor cx,cx
    mov cl,[di]
    or cx,cx
    je done
    inc di
loop_records:
    xor ax,ax
    mov al,[di+1]
    cmp ax,word ptr DGROUP:_board_record_index
    jne skip_record
    cmp byte ptr [di+8],1
    je skip_record
    push cx
    xor bx,bx
    mov bl,[di+7]
    push bx
    mov bl,[di+6]
    shl bx,1
    shl bx,1
    les ax,dword ptr DGROUP:_resource_ptr_table[bx]
    push es
    push ax
    mov ax,[bp+4]
    add ax,[di+4]
    push ax
    push word ptr [di+2]
    call _f03cc
    add sp,0ah
    cmp byte ptr [di+1ah],0
    je skip_copy
    mov ax,[di+2]
    add ax,10h
    mov bx,[di+4]
    mov dl,[di+1ah]
    xor dh,dh
    sub bx,dx
    inc bx
    push bx
    push dx
    push ax
    call _f03a5
    add sp,6
skip_copy:
    pop cx
skip_record:
    add di,20h
    loop loop_records
done:
    pop di
    pop si
    pop bp
    ret
_f4eeb endp

_TEXT ends
end
