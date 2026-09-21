; F_8BAB symbolic candidate
_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP
public _puzzle_run
extrn _cur_color_index_get:near
extrn _sprite_sheet_index_get:near
extrn _menu_list_active:near
extrn _keyboard_chain_active:near
extrn _keyboard_chain_enable:near
extrn _keyboard_buffer_drain:near
extrn _campaign_node_index:near
extrn _puzzle_display_init:near
extrn _slot_is_new_game:near
extrn _tutorial_hint_dialog_show:near
extrn _menu_list_disable:near
extrn _timer_wait_ticks:near
extrn _keyboard_read_blocking_hotkeys:near
extrn _f9440:near
extrn _menu_list_enable:near
extrn _puzzle_cell_backing_swap:near
extrn _puzzle_cell_highlight_draw:near
extrn _puzzle_clear_cell:near
extrn _stream_control_block_arm:near
extrn _timer_deadline_arm:near
extrn _f9402:near
extrn _player_select_restart_confirm:near
extrn _puzzle_draw_piece:near
extrn _timer_deadline_reached:near
extrn _keyboard_poll_nonblocking:near
extrn _puzzle_check_solved:near
extrn _puzzle_draw_tray_piece:near
extrn _sound_stop_reset:near
extrn _puzzle_free_resources:near
extrn _gfx_color_select:near
extrn _sprite_sheet_select:near
extrn _keyboard_chain_disable:near
extrn _resource_record_cache_reset:near
JMP_NEAR macro target
 db 0e9h
 dw target-$-2
endm
_puzzle_run proc near
 push bp
 mov bp,sp
 sub sp,16h
 push si
 push di
 mov word ptr [bp-14h],0h
 mov word ptr [bp-12h],1h
 mov word ptr [bp-10h],1h
 mov word ptr [bp-4h],0h
 mov ax,word ptr ds:[237eh]
 mov [bp-2h],ax
  call _cur_color_index_get
 mov [bp-0eh],ax
  call _sprite_sheet_index_get
 mov [bp-0ch],ax
  call _menu_list_active
 mov [bp-8h],ax
  call _keyboard_chain_active
 mov [bp-0ah],ax
  call _keyboard_chain_enable
  call _keyboard_buffer_drain
  call _campaign_node_index
 mov [bp-6h],ax
  call _puzzle_display_init
  call _slot_is_new_game
 or ax,ax
 jz short $+12
 mov ax,8h
 push ax
  call _tutorial_hint_dialog_show
 pop cx
 jmp short $+10
L_8C05:
 mov ax,6h
 push ax
  call _tutorial_hint_dialog_show
 pop cx
L_8C0D:
 jmp near ptr $+1112
L_8C10:
 cmp byte ptr ds:[0c134h],0h
 jz short $+80
  call _menu_list_disable
 jmp short $+55
L_8C1C:
 mov ax,76h
 push ax
  call _timer_wait_ticks
 pop cx
  call _keyboard_buffer_drain
  call _keyboard_read_blocking_hotkeys
 mov [bp-16h],ax
 cmp ax,0dh
 jnz short $+22
 inc word ptr [bp-4h]
 mov ax,[bp-4h]
 cmp ax,2h
 jnl short $+9
 push word ptr [bp-4h]
  call _f9440
 pop cx
L_8C44:
 jmp short $+13
L_8C46:
 cmp word ptr [bp-16h],1bh
 jnz short $+7
 mov word ptr [bp-4h],2h
L_8C51:
 cmp word ptr [bp-4h],2h
 jl short $-57
 cmp word ptr [bp-8h],0h
 jz short $+5
  call _menu_list_enable
L_8C60:
 mov word ptr [bp-14h],1h
L_8C65:
 jmp near ptr $+413
L_8C68:
 xor ax,ax
 push ax
 xor ax,ax
 push ax
 mov al,word ptr ds:[0c135h]
 cbw
 push ax
 mov al,word ptr ds:[0c346h]
 cbw
 push ax
  call _puzzle_cell_backing_swap
 add sp,8h
 xor ax,ax
 push ax
 mov ax,1h
 push ax
 mov al,word ptr ds:[0c135h]
 cbw
 push ax
 mov al,word ptr ds:[0c346h]
 cbw
 push ax
  call _puzzle_cell_highlight_draw
 add sp,8h
 mov al,word ptr ds:[0c135h]
 cbw
 push ax
 mov al,word ptr ds:[0c346h]
 cbw
 push ax
  call _puzzle_clear_cell
 pop cx
 pop cx
 mov al,word ptr ds:[0c346h]
 cbw
 mov si,ax
 mov al,word ptr ds:[0c135h]
 cbw
 mov di,ax
  call _keyboard_read_blocking_hotkeys
 mov [bp-16h],ax
 mov ax,[bp-16h]
 mov cx,6h
 mov bx,8ccfh
L_8CBF:
 cmp ax,[cs:bx]
 jz short $+9
 inc bx
 inc bx
 loop short $-7
 jmp near ptr $+239
L_8CCB:
 jmp word ptr cs:[bx+first_handlers-first_keys]
org 124h
first_keys dw 0dh,1bh,148h,14bh,14dh,150h
first_handlers dw L_8D29,L_8DB2,L_8CE7,L_8D08,L_8D16,L_8CF5
L_8CE7:
 dec byte ptr ds:[0c346h]
 jnl short L_8CF2
 mov byte ptr ds:[0c346h],3h
L_8CF2:
 jmp near ptr $+197
L_8CF5:
 inc byte ptr ds:[0c346h]
 mov al,word ptr ds:[0c346h]
 cmp al,4h
 jl short $+7
 mov byte ptr ds:[0c346h],0h
L_8D05:
 jmp near ptr $+178
L_8D08:
 dec byte ptr ds:[0c135h]
 jnl short $+7
 mov byte ptr ds:[0c135h],5h
L_8D13:
 jmp near ptr $+164
L_8D16:
 inc byte ptr ds:[0c135h]
 mov al,word ptr ds:[0c135h]
 cmp al,6h
 jl short $+7
 mov byte ptr ds:[0c135h],0h
L_8D26:
 jmp near ptr $+145
L_8D29:
 mov al,word ptr ds:[0c346h]
 cbw
 mov dx,0ch
 mul dx
 mov bx,ax
 add bx,0c316h
 push ds
 pop es
 mov al,word ptr ds:[0c135h]
 cbw
 shl ax,1
 add bx,ax
 cmp byte ptr [es:bx],0ffh
 jnz short $+12
 mov ax,17h
 push ax
  call _stream_control_block_arm
 pop cx
 jmp short $+96
L_8D52:
 mov al,word ptr ds:[0c346h]
 cbw
 mov dx,0ch
 mul dx
 mov bx,ax
 add bx,0c316h
 push ds
 pop es
 mov al,word ptr ds:[0c135h]
 cbw
 shl ax,1
 add bx,ax
 mov ax,[es:bx]
 mov word ptr ds:[0c132h],ax
 mov al,word ptr ds:[0c346h]
 cbw
 mov dx,0ch
 mul dx
 mov bx,ax
 add bx,0c316h
 push ds
 pop es
 mov al,word ptr ds:[0c135h]
 cbw
 shl ax,1
 add bx,ax
 mov byte ptr [es:bx],0ffh
 mov word ptr [bp-12h],1h
 mov word ptr [bp-10h],1h
 mov ax,76h
 push ax
  call _timer_deadline_arm
 pop cx
 mov ax,0eh
 push ax
  call _stream_control_block_arm
 pop cx
 mov ax,1h
 push ax
  call _f9402
 pop cx
L_8DB0:
 jmp short $+7
L_8DB2:
  call _player_select_restart_confirm
 jmp short $+2
L_8DB7:
 mov ax,1h
 push ax
 xor ax,ax
 push ax
 push di
 push si
  call _puzzle_cell_backing_swap
 add sp,8h
 cmp byte ptr ds:[0c132h],0ffh
 jnz short $+11
 push di
 push si
  call _puzzle_clear_cell
 pop cx
 pop cx
 jmp short $+46
L_8DD6:
 mov al,word ptr ds:[0c135h]
 cbw
 push ax
 mov al,word ptr ds:[0c346h]
 cbw
 push ax
 mov al,word ptr ds:[0c346h]
 cbw
 mov dx,0ch
 mul dx
 mov bx,ax
 add bx,0c316h
 push ds
 pop es
 mov al,word ptr ds:[0c135h]
 cbw
 shl ax,1
 add bx,ax
 push word ptr [es:bx]
  call _puzzle_draw_piece
 add sp,6h
L_8E02:
 cmp byte ptr ds:[0c132h],0ffh
 jnz short $+11
 cmp word ptr [bp-14h],0h
 jnz short $+5
 jmp near ptr $-423
L_8E12:
 jmp near ptr $+579
L_8E15:
 cmp word ptr [bp-12h],0h
 jz short $+110
 xor ax,ax
 push ax
 xor ax,ax
 push ax
 mov al,word ptr ds:[0c135h]
 cbw
 push ax
 mov al,word ptr ds:[0c346h]
 cbw
 push ax
  call _puzzle_cell_backing_swap
 add sp,8h
 mov al,word ptr ds:[0c135h]
 cbw
 push ax
 mov al,word ptr ds:[0c346h]
 cbw
 push ax
 push word ptr ds:[0c132h]
  call _puzzle_draw_piece
 add sp,6h
 xor ax,ax
 push ax
 mov ax,1h
 push ax
 mov al,word ptr ds:[0c135h]
 cbw
 push ax
 mov al,word ptr ds:[0c346h]
 cbw
 push ax
  call _puzzle_cell_backing_swap
 add sp,8h
 mov ax,1h
 push ax
 push word ptr [bp-10h]
 mov al,word ptr ds:[0c135h]
 cbw
 push ax
 mov al,word ptr ds:[0c346h]
 cbw
 push ax
  call _puzzle_cell_highlight_draw
 add sp,8h
 mov al,word ptr ds:[0c135h]
 cbw
 push ax
 mov al,word ptr ds:[0c346h]
 cbw
 push ax
  call _puzzle_clear_cell
 pop cx
 pop cx
 mov word ptr [bp-12h],0h
L_8E87:
  call _timer_deadline_reached
 or ax,ax
 jz short $+83
 mov ax,1h
 push ax
 mov ax,1h
 push ax
 mov al,word ptr ds:[0c135h]
 cbw
 push ax
 mov al,word ptr ds:[0c346h]
 cbw
 push ax
  call _puzzle_cell_backing_swap
 add sp,8h
 mov ax,[bp-10h]
 neg ax
 sbb ax,ax
 inc ax
 mov [bp-10h],ax
 mov ax,1h
 push ax
 push word ptr [bp-10h]
 mov al,word ptr ds:[0c135h]
L_8EBB:
 cbw
 push ax
 mov al,word ptr ds:[0c346h]
 cbw
 push ax
  call _puzzle_cell_highlight_draw
 add sp,8h
 mov al,word ptr ds:[0c135h]
 cbw
 push ax
 mov al,word ptr ds:[0c346h]
 cbw
 push ax
  call _puzzle_clear_cell
 pop cx
 pop cx
 mov ax,76h
 push ax
  call _timer_deadline_arm
 pop cx
L_8EDF:
  call _keyboard_poll_nonblocking
 or ax,ax
 jnz short $+5
 jmp near ptr $+367
L_8EE9:
 mov al,word ptr ds:[0c346h]
 cbw
 mov si,ax
 mov al,word ptr ds:[0c135h]
 cbw
 mov di,ax
  call _keyboard_read_blocking_hotkeys
 mov [bp-16h],ax
 mov ax,[bp-16h]
 mov cx,8h
 mov bx,8f14h
L_8F04:
 cmp ax,[cs:bx]
 jz short $+9
 inc bx
 inc bx
 loop short $-7
 jmp near ptr $+231
L_8F10:
 jmp word ptr cs:[bx+second_handlers-second_keys]
org 369h
second_keys dw 0dh,1bh,46h,66h,148h,14bh,14dh,150h
second_handlers dw L_8F8F,L_8FEF,L_8F76,L_8F76
 dw L_8F34,L_8F55,L_8F63,L_8F42
L_8F34:
 dec byte ptr ds:[0c346h]
 jnl short $+7
 mov byte ptr ds:[0c346h],3h
L_8F3F:
 jmp near ptr $+181
L_8F42:
 inc byte ptr ds:[0c346h]
 mov al,word ptr ds:[0c346h]
 cmp al,4h
 jl short $+7
 mov byte ptr ds:[0c346h],0h
L_8F52:
 jmp near ptr $+162
L_8F55:
 dec byte ptr ds:[0c135h]
 jnl short $+7
 mov byte ptr ds:[0c135h],5h
L_8F60:
 jmp near ptr $+148
L_8F63:
 inc byte ptr ds:[0c135h]
 mov al,word ptr ds:[0c135h]
 cmp al,6h
 jl short $+7
 mov byte ptr ds:[0c135h],0h
L_8F73:
 JMP_NEAR L_8FF4
L_8F76:
  call _slot_is_new_game
 or ax,ax
 jz short $+18
 inc byte ptr ds:[0c133h]
 mov al,word ptr ds:[0c133h]
 cmp al,3h
 jng short $+7
 mov byte ptr ds:[0c133h],0h
L_8F8D:
 jmp short $+103
L_8F8F:
 mov al,word ptr ds:[0c346h]
 cbw
 mov dx,0ch
 mul dx
 mov bx,ax
 add bx,0c316h
 push ds
 pop es
 mov al,word ptr ds:[0c135h]
 cbw
 shl ax,1
 add bx,ax
 cmp byte ptr [es:bx],0ffh
 jz short $+12
 mov ax,17h
 push ax
  call _stream_control_block_arm
 pop cx
 jmp short $+55
L_8FB8:
 mov ax,word ptr ds:[0c132h]
 push ax
 mov al,word ptr ds:[0c346h]
 cbw
 mov dx,0ch
 mul dx
 mov bx,ax
 add bx,0c316h
 push ds
 pop es
 mov al,word ptr ds:[0c135h]
 cbw
 shl ax,1
 add bx,ax
 pop ax
 mov [es:bx],ax
 mov byte ptr ds:[0c132h],0ffh
 mov ax,1ah
 push ax
  call _stream_control_block_arm
 pop cx
 xor ax,ax
 push ax
  call _f9402
 pop cx
L_8FED:
 jmp short $+7
L_8FEF:
  call _player_select_restart_confirm
 jmp short $+2
L_8FF4:
 cmp byte ptr ds:[0c132h],0ffh
 jz short $+26
 mov ax,1h
 push ax
 xor ax,ax
 push ax
 push di
 push si
  call _puzzle_cell_backing_swap
 add sp,8h
 push di
 push si
  call _puzzle_clear_cell
 pop cx
 pop cx
 jmp short $+63
L_9013:
 mov ax,1h
 push ax
 mov ax,1h
 push ax
 push di
 push si
  call _puzzle_cell_backing_swap
 add sp,8h
 push di
 push si
  call _puzzle_clear_cell
 pop cx
 pop cx
  call _puzzle_check_solved
 mov word ptr ds:[0c134h],al
 or al,al
 jz short $+30
  call _keyboard_buffer_drain
 push word ptr [bp-4h]
  call _f9440
 pop cx
 push word ptr [bp-6h]
  call _puzzle_draw_tray_piece
 pop cx
  call _sound_stop_reset
 mov ax,1bh
 push ax
  call _stream_control_block_arm
 pop cx
L_9050:
 mov word ptr [bp-12h],1h
L_9055:
 cmp byte ptr ds:[0c132h],0ffh
 jz short $+11
 cmp word ptr [bp-14h],0h
 jnz short $+5
 jmp near ptr $-589
L_9065:
 cmp word ptr [bp-14h],0h
 jnz short $+5
 jmp near ptr $-1115
L_906E:
 mov ax,7h
 push ax
  call _tutorial_hint_dialog_show
 pop cx
  call _puzzle_free_resources
 push word ptr [bp-0eh]
  call _gfx_color_select
 pop cx
 push word ptr [bp-0ch]
  call _sprite_sheet_select
 pop cx
 cmp word ptr [bp-0ah],0h
 jnz short $+5
  call _keyboard_chain_disable
L_9090:
  call _keyboard_buffer_drain
 push word ptr [bp-2h]
  call _resource_record_cache_reset
 pop cx
 mov al,word ptr ds:[0c134h]
 cbw
 jmp short $+2
L_90A0:
 pop di
 pop si
 mov sp,bp
 pop bp
 ret
_puzzle_run endp
_TEXT ends
end
