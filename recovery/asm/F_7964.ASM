_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

JMP_NEAR macro target
 db 0e9h
 dw target-$-2
endm

public _menu_loop_run
gc0fe equ 0c0feh
extrn _menu_list_active:near, _menu_list_disable:near, _keyboard_chain_active:near, _keyboard_chain_enable:near
extrn _keyboard_buffer_drain:near, _ui_overlay_show:near, _sound_start:near, _menu_list_draw:near
extrn _dialog_draw_panel:near, _gfx_fill_rect:near, _gfx_box:near, _keyboard_read_blocking_hotkeys:near
extrn _dialog_restore_screen:near, _f_7dd3:near

_menu_loop_run proc near
 push bp
 mov bp,sp
 sub sp,14h
 push si
 push di
 mov si,[bp+4]
 or si,si
 jl cleanup_entry
 les bx,ds:[gc0fe]
 mov ax,es:[bx]
 cmp ax,si
 jg initialize
cleanup_entry:
 jmp near ptr _f_7dd3+23h
initialize:
 call _menu_list_active
 mov [bp-12h],ax
 call _menu_list_disable
 call _keyboard_chain_active
 mov [bp-14h],ax
 call _keyboard_chain_enable
 call _keyboard_buffer_drain
 call _ui_overlay_show
 call _sound_start
 JMP_NEAR finish_check

loop_start:
 mov [bp-10h],si
 les bx,ds:[gc0fe]
 les bx,es:[bx+2]
 mov ax,si
 mov dx,14h
 mul dx
 add bx,ax
 mov [bp-0ch],es
 mov [bp-0eh],bx
 push si
 call _menu_list_draw
 pop cx
 push word ptr [bp-0ch]
 push word ptr [bp-0eh]
 call _dialog_draw_panel
 pop cx
 pop cx
 mov word ptr [bp-2],0
 xor di,di
 mov word ptr [bp-8],0
 JMP_NEAR initial_loop

key_loop:
 mov [bp-0ah],di
 mov ax,0ah
 push ax
 les bx,[bp-0eh]
 push word ptr es:[bx+10h]
 mov ax,di
 mov dx,0ah
 mul dx
 add ax,11h
 push ax
 les bx,[bp-0eh]
 mov ax,es:[bx+12h]
 add ax,4
 push ax
 call _gfx_fill_rect
 add sp,8
 mov ax,0ah
 push ax
 les bx,[bp-0eh]
 push word ptr es:[bx+10h]
 mov ax,di
 mov dx,0ah
 mul dx
 add ax,11h
 push ax
 les bx,[bp-0eh]
 mov ax,es:[bx+12h]
 add ax,4
 push ax
 call _gfx_box
 add sp,8
 call _keyboard_read_blocking_hotkeys
 mov [bp-6],ax
 mov ax,[bp-6]
 mov cx,10h
 mov bx,offset key_values
find_key:
 cmp ax,cs:[bx]
 je key_found
 inc bx
 inc bx
 loop find_key
 JMP_NEAR dispatch_done
key_found:
 jmp word ptr cs:[bx+key_handlers-key_values]

org 0e4h
key_values dw 000dh,001bh,013bh,013ch,013dh,013eh,013fh,0140h
           dw 0141h,0142h,0143h,0144h,0148h,014bh,014dh,0150h
key_handlers dw help_key, cancel_key, numeric_key, numeric_key
             dw numeric_key, numeric_key, numeric_key, numeric_key
             dw numeric_key, numeric_key, numeric_key, numeric_key
             dw previous_key, next_key, prior_page_key, first_key

previous_key:
 dec di
 jnl previous_done
 les bx,[bp-0eh]
 mov di,es:[bx+6]
previous_ready:
 dec di
previous_done:
 JMP_NEAR dispatch_done
first_key:
 les bx,[bp-0eh]
 mov ax,es:[bx+6]
 inc di
 mov dx,di
 cmp ax,dx
 jg first_done
 xor di,di
first_done:
 JMP_NEAR dispatch_done
next_key:
 dec si
 jnl next_done
 les bx,ds:[gc0fe]
 mov si,es:[bx]
 dec si
next_done:
 mov word ptr [bp-8],1
 JMP_NEAR dispatch_done
prior_page_key:
 les bx,ds:[gc0fe]
 mov ax,es:[bx]
 inc si
 mov dx,si
 cmp ax,dx
 jg prior_page_done
 xor si,si
prior_page_done:
 mov word ptr [bp-8],1
 JMP_NEAR dispatch_done
numeric_key:
 mov si,[bp-6]
 add si,0fec5h
 or si,si
 jl numeric_restore
 les bx,ds:[gc0fe]
 mov ax,es:[bx]
 cmp ax,si
 jg numeric_check_changed
numeric_restore:
 mov si,[bp-10h]
 jmp short numeric_done
numeric_check_changed:
 cmp si,[bp-10h]
 je numeric_done
 mov word ptr [bp-8],1
numeric_done:
 jmp short dispatch_done
cancel_key:
 mov word ptr [bp-8],1
 mov si,0ffffh
 jmp short redraw
help_key:
 call _dialog_restore_screen
 mov word ptr [bp-2],1
 mov ax,0ffffh
 push ax
 call _menu_list_draw
 pop cx
 les bx,[bp-0eh]
 les bx,es:[bx+0ch]
 mov ax,di
 shl ax,1
 add bx,ax
 cmp word ptr es:[bx],0
 je no_callback
 les bx,[bp-0eh]
 les bx,es:[bx+0ch]
 mov ax,di
 shl ax,1
 add bx,ax
 call word ptr es:[bx]
 mov [bp-4],ax
 jmp short callback_done
no_callback:
 mov word ptr [bp-4],1
callback_done:
 mov ax,[bp-4]
 or ax,ax
 jz callback_zero
 cmp ax,1
 jz callback_one
 jmp short callback_zero
callback_zero:
 mov word ptr [bp-8],1
 mov si,0ffffh
 jmp short callback_join
callback_one:
 mov word ptr [bp-8],1
 jmp short callback_join
callback_join:
 jmp short redraw

dispatch_done:
redraw:
 cmp word ptr [bp-8],0
 jne initial_loop
 mov ax,0ah
 push ax
 les bx,[bp-0eh]
 push word ptr es:[bx+10h]
 mov ax,[bp-0ah]
 mov dx,0ah
 mul dx
 add ax,11h
 push ax
 les bx,[bp-0eh]
 mov ax,es:[bx+12h]
 add ax,4
 push ax
 call _gfx_fill_rect
 add sp,8
 mov ax,0ah
 push ax
 les bx,[bp-0eh]
 push word ptr es:[bx+10h]
 mov ax,[bp-0ah]
 mov dx,0ah
 mul dx
 add ax,11h
 push ax
 les bx,[bp-0eh]
 mov ax,es:[bx+12h]
 add ax,4
 push ax
 call _gfx_box
 add sp,8
initial_loop:
 cmp word ptr [bp-8],0
 jne finish_or_loop
 JMP_NEAR key_loop
finish_or_loop:
 cmp word ptr [bp-2],0
 jne finish_check
 call _dialog_restore_screen
finish_check:
 cmp si,-1
 je cleanup_tail
 JMP_NEAR loop_start
cleanup_tail:
 ; The historically adjacent F_7DD3 owner begins at this fall-through label.
_menu_loop_run endp
_TEXT ends
end


