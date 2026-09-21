; F_25B3 -- update ten three-byte moving records and redraw their affected cells.
; The historical object contained the called helpers in the same contribution;
; retain those calls as direct local relations, not external OMF fixups.
_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP
public _board_update_moving_records
CALL_REL macro delta
    call $+delta
endm
_board_update_moving_records proc near
 push bp
 mov bp,sp
 sub sp,0ah
 push si
 push di
 les bx,dword ptr ds:[0bfbcH]
 add bx,2ach
 mov [bp-2],es
 mov [bp-4],bx
 mov word ptr [bp-0ah],0
 jmp near ptr F25_test
F25_body:
 les bx,dword ptr [bp-4]
 mov al,es:[bx]
 mov [bp-5],al
 test al,0fh
 jnz short F25_decrement
 jmp near ptr F25_after_move
F25_decrement:
 les bx,dword ptr [bp-4]
 dec byte ptr es:[bx]
 mov al,es:[bx]
 test al,0fh
 jnz short F25_coordinates
 les bx,dword ptr [bp-4]
 xor byte ptr es:[bx],20h
F25_coordinates:
 les bx,dword ptr [bp-4]
 mov al,es:[bx+1]
 xor ah,ah
 xor dx,dx
 mov si,ax
 shl si,1
 les bx,dword ptr [bp-4]
 mov al,es:[bx+2]
 xor ah,ah
 xor dx,dx
 mov di,ax
 mov ax,si
 shr ax,1
 shr ax,1
 shr ax,1
 push ax
 mov ax,di
 shr ax,1
 shr ax,1
 shr ax,1
 add ax,0fffeh
 mov dx,26h
 mul dx
 pop dx
 add dx,ax
 dec dx
 mov [bp-8],dx
 add di,0b4h
 sub si,4
 test byte ptr [bp-5],80h
 jnz short F25_horizontal
 jmp near ptr F25_vertical
F25_horizontal:
 test byte ptr [bp-5],20h
 jnz short F25_horizontal_plus
 jmp near ptr F25_horizontal_minus
F25_horizontal_plus:
 mov ax,di
 add ax,0ff40h
 push ax
 mov ax,si
 add ax,4
 push ax
 ; F_257D: test whether this cell can be updated.
 CALL_REL 0ff27h
 pop cx
 pop cx
 or ax,ax
 jz short F25_hplus_draw
 mov al,[bp-5]
 les bx,dword ptr [bp-4]
 mov es:[bx],al
 jmp short F25_after_hplus
F25_hplus_draw:
 push di
 push si
 mov ax,38h
 push ax
 mov ax,10h
 push ax
 mov ax,di
 add ax,90h
 push ax
 push si
 ; RUNTIME_BLOCK cell draw helper.
 CALL_REL 0dd39h
 add sp,0ch
 sub di,8
 les bx,dword ptr [bp-4]
 sub byte ptr es:[bx+2],8
 mov word ptr ds:[96h],167h
 xor ax,ax
 push ax
 push ds
 mov ax,6ca6h
 push ax
 push di
 push si
 ; RUNTIME_BLOCK sprite blit helper.
 CALL_REL 0dd30h
 add sp,0ah
 mov word ptr ds:[96h],9fh
 mov ax,di
 add ax,0ff48h
 push ax
 push si
 mov ax,40h
 push ax
 mov ax,10h
 push ax
 push di
 push si
 CALL_REL 0dcfbH
 add sp,0ch
 les bx,dword ptr ds:[0bfbcH]
 add bx,[bp-8]
 add bx,-26h
 mov byte ptr es:[bx],7
 les bx,dword ptr ds:[0bfbcH]
 add bx,[bp-8]
 mov byte ptr es:[bx+0beh],0
F25_after_hplus:
 jmp near ptr F25_hminus_done
F25_horizontal_minus:
 mov ax,di
 add ax,0ff88h
 push ax
 mov ax,si
 add ax,4
 push ax
 CALL_REL 0fe94h
 pop cx
 pop cx
 or ax,ax
 jz short F25_hminus_draw
 mov al,[bp-5]
 les bx,dword ptr [bp-4]
 mov es:[bx],al
 jmp short F25_hminus_done
F25_hminus_draw:
 push di
 push si
 mov ax,38h
 push ax
 mov ax,10h
 push ax
 mov ax,di
 add ax,90h
 push ax
 push si
 CALL_REL 0dca6h
 add sp,0ch
 les bx,dword ptr [bp-4]
 add byte ptr es:[bx+2],8
 mov word ptr ds:[96h],167h
 xor ax,ax
 push ax
 push ds
 mov ax,6ca6h
 push ax
 mov ax,di
 add ax,8
 push ax
 push si
 CALL_REL 0dc9bh
 add sp,0ah
 mov word ptr ds:[96h],9fh
 mov ax,di
 add ax,0ff48h
 push ax
 push si
 mov ax,40h
 push ax
 mov ax,10h
 push ax
 push di
 push si
 CALL_REL 0dc66h
 add sp,0ch
 les bx,dword ptr ds:[0bfbcH]
 add bx,[bp-8]
 mov byte ptr es:[bx+0e4h],7
 les bx,dword ptr ds:[0bfbcH]
 add bx,[bp-8]
 mov byte ptr es:[bx],0
F25_hminus_done:
 jmp near ptr F25_after_move
F25_vertical:
 test byte ptr [bp-5],20h
 jnz short F25_vertical_plus
 jmp near ptr F25_vertical_minus
F25_vertical_plus:
 mov ax,di
 add ax,0ff4ch
 push ax
 mov ax,si
 add ax,0fff8h
 push ax
 CALL_REL 0fdf9h
 pop cx
 pop cx
 or ax,ax
 jz short F25_vplus_draw
 mov al,[bp-5]
 les bx,dword ptr [bp-4]
 mov es:[bx],al
 jmp short F25_after_vplus
F25_vplus_draw:
 push di
 push si
 mov ax,10h
 push ax
 mov ax,38h
 push ax
 mov ax,di
 add ax,90h
 push ax
 push si
 CALL_REL 0dc0bh
 add sp,0ch
 sub si,8
 les bx,dword ptr [bp-4]
 sub byte ptr es:[bx+1],4
 mov word ptr ds:[96h],167h
 xor ax,ax
 push ax
 push ds
 mov ax,6ac4h
 push ax
 push di
 push si
 CALL_REL 0dc02h
 add sp,0ah
 mov word ptr ds:[96h],9fh
 mov ax,di
 add ax,0ff48h
 push ax
 push si
 mov ax,10h
 push ax
 mov ax,40h
 push ax
 push di
 push si
 CALL_REL 0dbcDh
 add sp,0ch
 les bx,dword ptr ds:[0bfbcH]
 add bx,[bp-8]
 dec bx
 mov byte ptr es:[bx],7
 les bx,dword ptr ds:[0bfbcH]
 add bx,[bp-8]
 mov byte ptr es:[bx+5],0
F25_after_vplus:
 jmp near ptr F25_after_move
F25_vertical_minus:
 mov ax,di
 add ax,0ff4ch
 push ax
 mov ax,si
 add ax,40h
 push ax
 CALL_REL 0fd69h
 pop cx
 pop cx
 or ax,ax
 jz short F25_vminus_draw
 mov al,[bp-5]
 les bx,dword ptr [bp-4]
 mov es:[bx],al
 jmp short F25_after_move
F25_vminus_draw:
 push di
 push si
 mov ax,10h
 push ax
 mov ax,38h
 push ax
 mov ax,di
 add ax,90h
 push ax
 push si
 CALL_REL 0db7bh
 add sp,0ch
 les bx,dword ptr [bp-4]
 add byte ptr es:[bx+1],4
 mov word ptr ds:[96h],167h
 xor ax,ax
 push ax
 push ds
 mov ax,6ac4h
 push ax
 push di
 mov ax,si
 add ax,8
 push ax
 CALL_REL 0db70h
 add sp,0ah
 mov word ptr ds:[96h],9fh
 mov ax,di
 add ax,0ff48h
 push ax
 push si
 mov ax,10h
 push ax
 mov ax,40h
 push ax
 push di
 push si
 CALL_REL 0db3bh
 add sp,0ch
 les bx,dword ptr ds:[0bfbcH]
 add bx,[bp-8]
 mov byte ptr es:[bx+6],7
 les bx,dword ptr ds:[0bfbcH]
 add bx,[bp-8]
 mov byte ptr es:[bx],0
F25_after_move:
 inc word ptr [bp-0ah]
 add word ptr [bp-4],3
F25_test:
 cmp word ptr [bp-0ah],0ah
 jnl short F25_done
 jmp near ptr F25_body
F25_done:
 pop di
 pop si
 mov sp,bp
 pop bp
 ret
_board_update_moving_records endp
_TEXT ends
end
