_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

g_c46e equ 0c46eh
g_c57e equ 0c57eh
g_13ba equ 13bah
g_13b8 equ 13b8h

public _faa1f
extrn _fa15e:near
extrn _f6b66:near
extrn _faf45:near
extrn _fa036:near
extrn _f86c9:near
extrn _fa24e:near

_faa1f proc near
 push bp
 mov bp,sp
 sub sp,0cch
 push si
 push di
 cmp word ptr ds:[g_c46e],1
 jne init_not_one
 mov si,word ptr ds:[g_c57e]
 mov di,si
 inc di
 jmp short init_done
init_not_one:
 xor si,si
 mov di,word ptr ds:[g_c57e]
init_done:
 jmp loop_entry

dispatch:
 mov ax,si
 mov [bp-0cch],ax
 push ax
 call _fa15e
 pop cx
 call _f6b66
 call _faf45
 mov cx,5
 mov bx,offset state_keys
find_state:
 cmp ax,cs:[bx]
 je state_found
 inc bx
 inc bx
 loop find_state
 jmp cleanup_state
state_found:
 jmp word ptr cs:[bx+state_handlers-state_keys]

org 49h
state_keys dw 0dh,12h,1bh,148h,150h
state_handlers dw state_first,state_mode_two,state_remove,state_decrement,state_increment

state_increment:
 inc si
 mov ax,si
 cwd
 idiv di
 mov si,dx
 jmp cleanup_state
state_decrement:
 dec si
 mov ax,si
 add ax,di
 cwd
 idiv di
 mov si,dx
 jmp cleanup_state
state_remove:
 push word ptr [bp-0cch]
 call _fa15e
 pop cx
 cmp word ptr ds:[g_c46e],1
 jne remove_not_one
 mov ax,-1
 jmp finish
remove_not_one:
 mov word ptr ds:[g_c46e],1
 mov ax,-2
 jmp finish
state_first:
 push word ptr [bp-0cch]
 call _fa15e
 pop cx
 cmp si,word ptr ds:[g_c57e]
 jne mode_two_logic
 mov word ptr ds:[g_c46e],0
 mov ax,-2
 jmp finish
remove_not_last:
 jmp short continue_mode
mode_two_logic:
 cmp word ptr ds:[g_c46e],2
 jne continue_mode
 xor ax,ax
 push ax
 push ax
 push ds
 mov ax,1356h
 push ax
 push ds
 mov ax,si
 mov dx,1bh
 mul dx
 add ax,0c470h
 push ax
 push ds
 mov ax,12e5h
 push ax
 push ss
 lea ax,[bp-0c8h]
 push ax
 call _fa036
 add sp,14h
 mov bx,ss
 mov es,bx
 lea bx,[bp-0c8h]
 mov word ptr ds:[g_13ba],es
 mov word ptr ds:[g_13b8],bx
 push ds
 mov ax,13b1h
 push ax
 call _f86c9
 pop cx
 pop cx
 mov [bp-0cah],ax
 or ax,ax
 jl mode_two_finish
 mov word ptr ds:[g_c46e],1
 cmp word ptr [bp-0cah],0
 je mode_two_finish
 push si
 call _fa24e
 pop cx
mode_two_finish:
 mov ax,-2
 jmp short finish
continue_mode:
 mov ax,si
 jmp short finish
state_mode_two:
 cmp word ptr ds:[g_c46e],1
 jne cleanup_state
 mov word ptr ds:[g_c46e],2
 push word ptr [bp-0cch]
 call _fa15e
 pop cx
 mov ax,-2
 jmp short finish
cleanup_state:
 push word ptr [bp-0cch]
 call _fa15e
 pop cx
loop_entry:
 jmp dispatch
finish:
 pop di
 pop si
 mov sp,bp
 pop bp
 ret
_faa1f endp
_TEXT ends
end
