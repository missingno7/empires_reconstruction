; F_880A -- state-driven record-selection loop, reconstructed symbolic TASM.
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

public _dialog_list_run
extrn _keyboard_chain_active:near, _keyboard_chain_enable:near, _keyboard_buffer_drain:near, _ui_overlay_show:near
extrn _sound_start:near, _menu_list_active:near, _menu_list_disable:near, _dialog_draw:near
extrn _f6b1a:near, _dialog_fill_box:near, _dialog_blink_box:near

_dialog_list_run proc near
 push bp
 mov bp,sp
 sub sp,1eh
 push si
 push di
 xor si,si
 mov word ptr [bp-6],1
 xor di,di
 call _keyboard_chain_active
 mov [bp-0ah],ax
 call _keyboard_chain_enable
 call _keyboard_buffer_drain
 call _ui_overlay_show
 call _sound_start
 call _menu_list_active
 mov [bp-8],ax
 call _menu_list_disable
 les bx,[bp+4]
 les bx,es:[bx]
 mov [bp-1ah],es
 mov [bp-1ch],bx
 les bx,[bp+4]
 mov al,es:[bx+4]
 mov [bp-18h],al
 les bx,[bp+4]
 mov ax,es:[bx+0ah]
 mov [bp-12h],ax
 les bx,[bp+4]
 mov ax,es:[bx+0ch]
 mov [bp-10h],ax
 les bx,[bp+4]
 mov ax,es:[bx+0eh]
 mov [bp-0eh],ax
 les bx,[bp+4]
 mov ax,es:[bx+10h]
 mov [bp-0ch],ax
 JMP_NEAR state_loop

select_record:
 or si,si
 jnz select_nonzero
 mov word ptr [bp-1eh],4
 mov al,0
 mov [bp-13h],al
 mov ah,0
 mov di,ax
 jmp short load_record
select_nonzero:
 les bx,[bp+4]
 mov al,es:[bx+9]
 cbw
 dec ax
 cmp ax,si
 jle select_last
 mov word ptr [bp-1eh],5
 jmp short select_ready
select_last:
 mov word ptr [bp-1eh],6
select_ready:
 mov al,1
 mov [bp-13h],al
 mov ah,0
 mov di,ax
load_record:
 les bx,[bp+4]
 les bx,es:[bx+5]
 mov ax,si
 shl ax,1
 shl ax,1
 add bx,ax
 les bx,es:[bx]
 mov [bp-15h],es
 mov [bp-17h],bx
 push word ptr [bp-6]
 push ss
 lea ax,[bp-1eh]
 push ax
 call _dialog_draw
 add sp,6
 mov word ptr [bp-6],0
 mov word ptr [bp-4],0
 JMP_NEAR process_state

read_input:
 call _f6b1a
 mov [bp-2],ax
 mov ax,[bp-2]
 mov cx,10
 mov bx,offset state_keys
find_state:
 cmp ax,cs:[bx]
 je state_found
 inc bx
 inc bx
 loop find_state
 JMP_NEAR process_state
state_found:
 jmp word ptr cs:[bx+state_handlers-state_keys]

org 0f5h
state_keys dw 9,0dh,1bh,13bh,148h,149h,14bh,14dh,150h,151h
state_handlers dw state_move_neg,state_move_pos,state_set_count,state_set_from_record
 dw state_move_neg,state_reset,state_move_neg,state_move_neg,state_move_neg,state_zero_count

state_move_neg:
 or si,si
 jle short state_move_done
 push di
 call _dialog_fill_box
 pop cx
 mov ax,di
 neg ax
 sbb ax,ax
 inc ax
 mov di,ax
 push di
 call _dialog_fill_box
 pop cx
state_move_done:
 JMP_NEAR process_state
state_move_pos:
 push di
 call _dialog_blink_box
 pop cx
 mov word ptr [bp-4],1
 jmp short process_state
state_set_count:
 mov di,-1
 mov word ptr [bp-4],1
 jmp short process_state
state_set_from_record:
 mov di,2
 mov word ptr [bp-4],1
 jmp short process_state
state_reset:
 or si,si
 jle reset_done
 or di,di
 jle reset_after_move
 push di
 call _dialog_fill_box
 pop cx
reset_after_move:
 xor di,di
 xor ax,ax
 push ax
 call _dialog_blink_box
 pop cx
 mov word ptr [bp-4],1
reset_done:
 jmp short process_state
state_zero_count:
 or si,si
 jnz zero_count_done
 xor di,di
 xor ax,ax
 push ax
 call _dialog_blink_box
 pop cx
 mov word ptr [bp-4],1
 jmp short zero_count_finish
zero_count_done:
 les bx,[bp+4]
 mov al,es:[bx+9]
 cbw
 dec ax
 cmp ax,si
 jle zero_count_finish
 mov di,1
 mov ax,1
 push ax
 call _dialog_blink_box
 pop cx
 mov word ptr [bp-4],1
zero_count_finish:
 jmp short process_state

process_state:
 cmp word ptr [bp-4],0
 jne dispatch_direction
 JMP_NEAR read_input
dispatch_direction:
 mov ax,di
 inc ax
 cmp ax,3
 ja state_loop
 mov bx,ax
 shl bx,1
 jmp word ptr cs:[bx+mode_handlers]

mode_handlers dw mode_minus_one,mode_plus_one,mode_set_count,mode_from_record
mode_minus_one:
 mov si,-1
 jmp short state_loop
mode_plus_one:
 or si,si
 jnz mode_decrement
 inc si
 jmp short mode_continue
mode_decrement:
 dec si
mode_continue:
 jmp short state_loop
mode_set_count:
 inc si
 jmp short state_loop
mode_from_record:
 les bx,[bp+4]
 mov al,es:[bx+9]
 cbw
 mov si,ax
 jmp short state_loop

state_loop:
 or si,si
 jl short f880a_end
 les bx,[bp+4]
 mov al,es:[bx+9]
 cbw
 cmp ax,si
 jle short f880a_end
 JMP_NEAR select_record
f880a_end label byte
_dialog_list_run endp
_TEXT ends
end
