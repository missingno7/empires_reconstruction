; F_4B0C symbolic command interpreter loop.
; Reconstructed control flow retains branch, state-record, and opcode structure.
; CALL_REL retains original
; direct near-call encoding in this independently emitted contribution.
_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

CALL_REL macro delta
 call $+delta
endm

public _f4b0c
_f4b0c proc near
 push bp
L_4B0D:
 mov bp,sp
L_4B0F:
 push si
L_4B10:
 push di
L_4B11:
 push bp
L_4B12:
 cld
L_4B13:
 mov ax,0fh
L_4B16:
 push ax
L_4B17:
 CALL_REL -18761 ; F_01CE
L_4B1A:
 pop ax
L_4B1B:
 mov di,0b3aeh
L_4B1E:
 xor cx,cx
L_4B20:
 mov cl,[di]
L_4B22:
 or cx,cx
L_4B24:
 jz interpreter_return
L_4B26:
 inc di
record_loop:
 push cx
L_4B28:
 xor ax,ax
L_4B2A:
 mov al,[di+1h]
L_4B2D:
 cmp ax,ds:[0bfbah]
L_4B31:
 jnz next_record
L_4B33:
 mov al,[di]
L_4B35:
 cmp al,1h
L_4B37:
 jz render_record
L_4B39:
 cmp byte ptr [di+0ah],0h
L_4B3D:
 jnz decrement_delay
L_4B3F:
 mov si,[di+0dh]
L_4B42:
 add si,0b3aeh
dispatch_opcode:
 xor bx,bx
L_4B48:
 lodsb
L_4B49:
 mov bl,al
L_4B4B:
 shl bx,1
L_4B4D:
 add bx,74ah
L_4B51:
 jmp [bx]
save_program_counter:
 sub si,0b3aeh
L_4B57:
 mov [di+0dh],si
L_4B5A:
 xor ax,ax
L_4B5C:
 mov al,[di+1h]
L_4B5F:
 cmp ax,ds:[0bfbah]
L_4B63:
 jz update_record
next_record:
 pop cx
L_4B66:
 add di,byte ptr +20h
L_4B69:
 loop record_loop
interpreter_return:
 pop bp
L_4B6C:
 pop di
L_4B6D:
 pop si
L_4B6E:
 pop bp
L_4B6F:
 ret
decrement_delay:
 dec byte ptr [di+0ah]
render_record:
 cmp byte ptr [di+8h],1h
L_4B77:
 jz next_record
L_4B79:
 xor bx,bx
L_4B7B:
 mov bl,[di+7h]
L_4B7E:
 push bx
L_4B7F:
 mov bl,[di+6h]
L_4B82:
 shl bx,1
L_4B84:
 shl bx,1
L_4B86:
 les ax,[bx-4f84h]
L_4B8A:
 push es
L_4B8B:
 push ax
L_4B8C:
 push word ptr [di+4h]
L_4B8F:
 push word ptr [di+2h]
L_4B92:
 CALL_REL -18374 ; code_03CC
L_4B95:
 add sp,byte ptr +0ah
update_record:
 cmp byte ptr [di+8h],1h
L_4B9C:
 jz next_record
L_4B9E:
 cmp byte ptr [di+1ah],0h
L_4BA2:
 jz collision_check
L_4BA4:
 mov ax,[di+2h]
L_4BA7:
 add ax,10h
L_4BAA:
 mov bx,[di+4h]
L_4BAD:
 mov dl,[di+1ah]
L_4BB0:
 xor dh,dh
L_4BB2:
 sub bx,dx
L_4BB4:
 inc bx
L_4BB5:
 push bx
L_4BB6:
 push dx
L_4BB7:
 push ax
L_4BB8:
 CALL_REL -18451 ; code_03A5
L_4BBB:
 add sp,byte ptr +6h
collision_check:
 xor bx,bx
L_4BC0:
 mov bl,[di+6h]
L_4BC3:
 shl bx,1
L_4BC5:
 add bl,[di+7h]
L_4BC8:
 shl bx,1
L_4BCA:
 shl bx,1
L_4BCC:
 mov ax,[bx+40d4h]
L_4BD0:
 add bx,byte ptr +2h
L_4BD3:
 mov cx,[bx+40d4h]
L_4BD7:
 xor dx,dx
L_4BD9:
 xchg dl,ch
L_4BDB:
 mov bp,dx
L_4BDD:
 mov dl,ah
L_4BDF:
 xor ah,ah
L_4BE1:
 mov bx,[di+2h]
L_4BE4:
 add ax,bx
L_4BE6:
 add cx,bx
L_4BE8:
 mov bx,[di+4h]
L_4BEB:
 add dx,bx
L_4BED:
 add bp,bx
L_4BEF:
 mov bx,ds:[72eh]
L_4BF3:
 shl bx,1
L_4BF5:
 add bx,ds:[73ah]
L_4BF9:
 shl bx,1
L_4BFB:
 shl bx,1
L_4BFD:
 push si
L_4BFE:
 push di
L_4BFF:
 mov si,bx
L_4C01:
 add si,79eh
L_4C05:
 mov di,ax
L_4C07:
 lodsw
L_4C08:
 xor bx,bx
L_4C0A:
 xchg bl,ah
L_4C0C:
 add ax,ds:[736h]
L_4C10:
 cmp ax,cx
L_4C12:
 jg skip_collision
L_4C14:
 add bx,ds:[738h]
L_4C18:
 cmp bx,bp
L_4C1A:
 jg skip_collision
L_4C1C:
 lodsw
L_4C1D:
 xor bx,bx
L_4C1F:
 xchg bl,ah
L_4C21:
 add ax,ds:[736h]
L_4C25:
 cmp di,ax
L_4C27:
 jg skip_collision
L_4C29:
 add bx,ds:[738h]
L_4C2D:
 cmp dx,bx
L_4C2F:
 jg skip_collision
L_4C31:
 mov ax,di
L_4C33:
 pop di
L_4C34:
 pop si
L_4C35:
 cmp word ptr ds:[40ceh],0h
L_4C3A:
 jnz L_4C42
L_4C3C:
 pop bx
L_4C3D:
 push bx
L_4C3E:
 mov ds:[40ceh],bx
L_4C42:
 mov bl,[di+19h]
L_4C45:
 or bl,bl
L_4C47:
 jz check_bounds
L_4C49:
 cmp byte ptr [di+1bh],1h
L_4C4D:
 jz check_bounds
L_4C4F:
 cmp bl,1h
L_4C52:
 jnz L_4C5B
L_4C54:
 cmp byte ptr ds:[72ch],0h
L_4C59:
 jz check_bounds
L_4C5B:
 xor bx,bx
L_4C5D:
 mov [di+0fh],bx
L_4C60:
 mov [di+11h],bx
L_4C63:
 mov [di+13h],bx
L_4C66:
 mov [di+15h],bx
L_4C69:
 inc bl
L_4C6B:
 mov [di+1bh],bl
L_4C6E:
 mov si,[di+17h]
L_4C71:
 mov [di+0dh],si
L_4C74:
 jmp short check_bounds
skip_collision:
 mov ax,di
L_4C78:
 pop di
L_4C79:
 pop si
check_bounds:
 cmp byte ptr ds:[8feh],0h
L_4C7F:
 jz advance_record
L_4C81:
 cmp byte ptr [di+0ah],0h
L_4C85:
 jnz advance_record
L_4C87:
 mov bx,ds:[0c04eh]
L_4C8B:
 shl bx,1
L_4C8D:
 push bx
L_4C8E:
 mov bx,[bx-3fb0h]
L_4C92:
 cmp bx,ax
L_4C94:
 pop ax
L_4C95:
 jl advance_record
L_4C97:
 cmp bx,cx
L_4C99:
 jg advance_record
L_4C9B:
 mov bx,ax
L_4C9D:
 mov ax,[bx-3f80h]
L_4CA1:
 cmp ax,dx
L_4CA3:
 jl advance_record
L_4CA5:
 cmp ax,bp
L_4CA7:
 jg advance_record
L_4CA9:
 mov al,[di+9h]
L_4CAC:
 mov [di+0ah],al
advance_record:
 jmp near ptr next_record
set_program_counter:
 sub si,0b3aeh
L_4CB6:
 mov [di+0dh],si
L_4CB9:
 jmp near ptr render_record
opcode_add_offset:
 lodsw
L_4CBD:
 add si,ax
L_4CBF:
 jmp near ptr dispatch_opcode
opcode_store_offset:
 lodsw
L_4CC3:
 mov [di+0fh],si
L_4CC6:
 add si,ax
L_4CC8:
 jmp near ptr dispatch_opcode
opcode_restore_offset:
 mov si,[di+0fh]
L_4CCE:
 jmp near ptr dispatch_opcode
L_4CD1:
 mov bx,11h
L_4CD4:
 jmp short opcode_countdown
L_4CD6:
 mov bx,13h
L_4CD9:
 jmp short opcode_countdown
L_4CDB:
 mov bx,15h
opcode_countdown:
 lodsw
L_4CDF:
 mov cx,ax
L_4CE1:
 lodsw
L_4CE2:
 dec word ptr [bx+di]
L_4CE4:
 jz L_4CEC
L_4CE6:
 jns L_4CEA
L_4CE8:
 mov [bx+di],ax
L_4CEA:
 add si,cx
L_4CEC:
 jmp near ptr dispatch_opcode
opcode_call_caf1:
 xor ah,ah
L_4CF1:
 lodsb
L_4CF2:
 push ax
L_4CF3:
 CALL_REL 32254 ; F_CAF1
L_4CF6:
 pop ax
L_4CF7:
 jmp near ptr dispatch_opcode
opcode_dispatch_2a2d:
 xor ah,ah
L_4CFC:
 lodsb
L_4CFD:
 push ax
L_4CFE:
 CALL_REL -8913 ; F_2A2D
L_4D01:
 pop cx
L_4D02:
 push dx
L_4D03:
 push ax
L_4D04:
 CALL_REL -6522 ; F_338A
L_4D07:
 pop cx
L_4D08:
 pop cx
L_4D09:
 jmp near ptr dispatch_opcode
opcode_dispatch_36f0:
 xor ah,ah
L_4D0E:
 lodsb
L_4D0F:
 push ax
L_4D10:
 CALL_REL -5664 ; F_36F0
L_4D13:
 pop ax
L_4D14:
 jmp near ptr dispatch_opcode
opcode_set_flag:
 mov dl,1h
L_4D19:
 jmp short L_4D1D
L_4D1B:
 mov dl,0h
L_4D1D:
 xor ah,ah
L_4D1F:
 lodsb
L_4D20:
 cmp al,80h
L_4D22:
 jnc L_4D39
L_4D24:
 mov bx,ax
L_4D26:
 shl bx,1
L_4D28:
 shl bx,1
L_4D2A:
 shl bx,1
L_4D2C:
 shl bx,1
L_4D2E:
 shl bx,1
L_4D30:
 add bx,0b3afh
L_4D34:
 mov [bx],dl
L_4D36:
 jmp near ptr dispatch_opcode
L_4D39:
 and al,7fh
L_4D3B:
 mov bx,ax
L_4D3D:
 shl bx,1
L_4D3F:
 shl bx,1
L_4D41:
 shl bx,1
L_4D43:
 shl bx,1
L_4D45:
 shl bx,1
L_4D47:
 add bx,di
L_4D49:
 mov [bx],dl
L_4D4B:
 jmp near ptr dispatch_opcode
opcode_set_limits:
 lodsw
L_4D4F:
 mov [di+0bh],al
L_4D52:
 mov [di+0ch],ah
L_4D55:
 jmp near ptr dispatch_opcode
opcode_set_velocity:
 xor ax,ax
L_4D5A:
 lodsb
L_4D5B:
 shl ax,1
L_4D5D:
 shr al,1
L_4D5F:
 mov [di+6h],al
L_4D62:
 mov [di+7h],ah
L_4D65:
 jmp near ptr dispatch_opcode
opcode_move_clamped:
 mov dx,[di+2h]
L_4D6B:
 mov cx,[di+4h]
L_4D6E:
 lodsw
L_4D6F:
 mov bl,ah
L_4D71:
 cbw
L_4D72:
 add dx,ax
L_4D74:
 mov al,bl
L_4D76:
 cbw
L_4D77:
 add cx,ax
L_4D79:
 mov [di+2h],dx
L_4D7C:
 mov [di+4h],cx
L_4D7F:
 xor ax,ax
L_4D81:
 lodsb
L_4D82:
 shl ax,1
L_4D84:
 shr al,1
L_4D86:
 mov [di+7h],ah
L_4D89:
 add al,[di+6h]
L_4D8C:
 cmp al,[di+0bh]
L_4D8F:
 jl L_4DBB
L_4D91:
 cmp al,[di+0ch]
L_4D94:
 jg L_4DC0
L_4D96:
 mov [di+6h],al
L_4D99:
 cmp byte ptr [di+8h],1h
L_4D9D:
 jz L_4DB8
L_4D9F:
 xor bx,bx
L_4DA1:
 mov bl,ah
L_4DA3:
 push bx
L_4DA4:
 mov bl,al
L_4DA6:
 shl bx,1
L_4DA8:
 shl bx,1
L_4DAA:
 les ax,[bx-4f84h]
L_4DAE:
 push es
L_4DAF:
 push ax
L_4DB0:
 push cx
L_4DB1:
 push dx
L_4DB2:
 CALL_REL -18918 ; code_03CC
L_4DB5:
 add sp,byte ptr +0ah
L_4DB8:
 jmp near ptr save_program_counter
L_4DBB:
 mov al,[di+0ch]
L_4DBE:
 jmp short L_4D96
L_4DC0:
 mov al,[di+0bh]
L_4DC3:
 jmp short L_4D96
opcode_set_position:
 lodsw
L_4DC6:
 xor cx,cx
L_4DC8:
 mov dx,cx
L_4DCA:
 mov dl,al
L_4DCC:
 mov cl,ah
L_4DCE:
 shl dx,1
L_4DD0:
 mov [di+2h],dx
L_4DD3:
 mov [di+4h],cx
L_4DD6:
 xor ax,ax
L_4DD8:
 lodsb
L_4DD9:
 shl ax,1
L_4DDB:
 shr al,1
L_4DDD:
 mov [di+7h],ah
L_4DE0:
 jmp short L_4D96
opcode_set_position_and_frame:
 lodsw
L_4DE3:
 xor cx,cx
L_4DE5:
 mov dx,cx
L_4DE7:
 mov dl,al
L_4DE9:
 mov cl,ah
L_4DEB:
 shl dx,1
L_4DED:
 mov [di+2h],dx
L_4DF0:
 mov [di+4h],cx
L_4DF3:
 lodsw
L_4DF4:
 mov [di+1h],ah
L_4DF7:
 xor bx,bx
L_4DF9:
 xchg bl,ah
L_4DFB:
 shl ax,1
L_4DFD:
 shr al,1
L_4DFF:
 mov [di+7h],ah
L_4E02:
 cmp bx,ds:[0bfbah]
L_4E06:
 jz L_4D96
L_4E08:
 jmp short L_4DB8
opcode_enable:
 mov byte ptr [di+8h],1h
L_4E0E:
 mov byte ptr [di+1bh],0h
L_4E12:
 jmp near ptr save_program_counter
opcode_disable:
 mov byte ptr [di+8h],0h
L_4E19:
 jmp near ptr dispatch_opcode
opcode_test_flags:
 lodsw
L_4E1D:
 mov bx,ax
L_4E1F:
 test byte ptr [bx+4374h],7h
L_4E24:
 jz opcode_skip_bytes
L_4E26:
 jmp near ptr dispatch_opcode
L_4E29:
 lodsw
L_4E2A:
 mov bx,ax
L_4E2C:
 test byte ptr [bx+4374h],7h
L_4E31:
 jnz opcode_skip_bytes
L_4E33:
 jmp near ptr dispatch_opcode
L_4E36:
 lodsw
L_4E37:
 mov bx,ax
L_4E39:
 test byte ptr [bx+4374h],10h
L_4E3E:
 jnz opcode_skip_bytes
L_4E40:
 jmp near ptr dispatch_opcode
L_4E43:
 lodsw
L_4E44:
 mov bx,ax
L_4E46:
 test byte ptr [bx+4374h],10h
L_4E4B:
 jz opcode_skip_bytes
L_4E4D:
 jmp near ptr dispatch_opcode
L_4E50:
 xor ax,ax
L_4E52:
 lodsb
L_4E53:
 shl ax,1
L_4E55:
 cmp word ptr ds:[736h],ax
L_4E59:
 jng opcode_skip_bytes
L_4E5B:
 jmp near ptr dispatch_opcode
L_4E5E:
 xor ax,ax
L_4E60:
 lodsb
L_4E61:
 shl ax,1
L_4E63:
 cmp word ptr ds:[736h],ax
L_4E67:
 jnc opcode_skip_bytes
L_4E69:
 jmp near ptr dispatch_opcode
L_4E6C:
 xor ax,ax
L_4E6E:
 lodsb
L_4E6F:
 cmp word ptr ds:[738h],ax
L_4E73:
 jng opcode_skip_bytes
L_4E75:
 jmp near ptr dispatch_opcode
L_4E78:
 xor ax,ax
L_4E7A:
 lodsb
L_4E7B:
 cmp word ptr ds:[738h],ax
L_4E7F:
 jnc opcode_skip_bytes
L_4E81:
 jmp near ptr dispatch_opcode
opcode_skip_bytes:
 xor bx,bx
L_4E86:
 lodsb
L_4E87:
 mov bl,al
L_4E89:
 mov bl,[bx+782h]
L_4E8D:
 add si,bx
L_4E8F:
 jmp near ptr dispatch_opcode
opcode_random_skip:
 CALL_REL 43645 ; code_F90F
L_4E95:
 mov bx,ax
L_4E97:
 lodsb
L_4E98:
 cmp bl,al
L_4E9A:
 jna opcode_skip_bytes
L_4E9C:
 jmp near ptr dispatch_opcode
_f4b0c endp
_TEXT ends
end _f4b0c
