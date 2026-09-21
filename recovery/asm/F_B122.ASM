; F_B122 -- initialise and expand the descriptor tables used by the board
; record stream at B07C/B07E.

_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

public _board_resource_expand

; These calls were direct same-segment calls in the original contribution.
; Keep the relation symbolic without introducing an OMF external reference.
CALL_REL macro delta
    call $+delta
endm

; Expand one word-offset table into consecutive B07C/B07E far-pointer records.
COPY_TABLE macro table, count
local copy_body, copy_test
    xor si,si
    jmp short copy_test
copy_body:
    mov ax,si
    shl ax,1
    les bx,dword ptr ds:[table]
    add bx,ax
    mov ax,es:[bx]
    mov cx,word ptr ds:[table+2]
    mov bx,word ptr ds:[table]
    add bx,ax
    inc bx
    inc bx
    push cx
    push bx
    mov bx,di
    shl bx,1
    shl bx,1
    pop ax
    pop dx
    mov [bx-4f82h],dx
    mov [bx-4f84h],ax
    inc di
    inc si
copy_test:
    cmp si,count
    jl short copy_body
endm

; Register one table before expanding it. The caller provides the original
; selector used by the descriptor-registration helper.
REGISTER_TABLE macro table, selector, count, call_delta
    push ds
    mov ax,table
    push ax
    mov ax,selector
    push ax
    CALL_REL call_delta
    add sp,6
    COPY_TABLE table,count
endm

_board_resource_expand proc near
    push si
    push di
    xor di,di

    mov ax,52h
    push ax
    CALL_REL 0b442h
    pop cx
    mov ax,54h
    push ax
    push word ptr ds:[0c5c8h]
    push word ptr ds:[0c5c6h]
    push ds
    mov ax,9bfch
    push ax
    CALL_REL 0425dh
    add sp,0ah

    mov ax,53h
    push ax
    CALL_REL 0b423h
    pop cx
    mov ax,54h
    push ax
    push word ptr ds:[0c5c8h]
    push word ptr ds:[0c5c6h]
    push ds
    mov ax,0bf66h
    push ax
    CALL_REL 0423eh
    add sp,0ah

    REGISTER_TABLE 0c5a8h,49h,0ch,0b6ddh
    REGISTER_TABLE 0c580h,4ah,8,0b698h
    REGISTER_TABLE 0c5ach,4bh,0ah,0b653h
    REGISTER_TABLE 0c59ah,4ch,11h,0b60eh
    REGISTER_TABLE 0c5a4h,4dh,4,0b5c9h

    push ds
    mov ax,0c58ah
    push ax
    mov ax,4eh
    push ax
    CALL_REL 0b584h
    add sp,6
    mov dx,word ptr ds:[0c58ch]
    mov ax,word ptr ds:[0c58ah]
    mov bx,di
    shl bx,1
    shl bx,1
    mov [bx-4f82h],dx
    mov [bx-4f84h],ax
    inc di

    REGISTER_TABLE 0c58eh,4fh,0ch,0b55fh
    REGISTER_TABLE 0c584h,50h,4,0b51ah
    REGISTER_TABLE 0c59eh,51h,1,0b4d5h

    jmp short B122_tail_test
B122_tail_copy:
    mov dx,word ptr ds:[0b07eh]
    mov ax,word ptr ds:[0b07ch]
    mov bx,di
    shl bx,1
    shl bx,1
    mov [bx-4f82h],dx
    mov [bx-4f84h],ax
    inc di
B122_tail_test:
    cmp di,54h
    jl short B122_tail_copy
    mov word ptr ds:[720h],2
    pop di
    pop si
    ret
_board_resource_expand endp

_TEXT ends
end
