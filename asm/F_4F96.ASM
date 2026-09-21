_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP
public _cmdline_parse_args

_cmdline_parse_args proc near
    push si
    mov si,1
    jmp scan_next

scan_option:
    mov ax,si
    shl ax,1
    shl ax,1
    les bx,dword ptr ds:[6dh]
    add bx,ax
    les bx,es:[bx]
    cmp byte ptr es:[bx],'-'
    je primary_dispatch
    mov ax,si
    shl ax,1
    shl ax,1
    les bx,dword ptr ds:[6dh]
    add bx,ax
    les bx,es:[bx]
    cmp byte ptr es:[bx],'/'
    je primary_dispatch
    jmp advance_option

primary_dispatch:
    mov ax,si
    shl ax,1
    shl ax,1
    les bx,dword ptr ds:[6dh]
    add bx,ax
    les bx,es:[bx]
    mov al,es:[bx+1]
    cbw
    mov cx,14
    mov bx,4ff4h
primary_find:
    cmp word ptr cs:[bx],ax
    je primary_found
    inc bx
    inc bx
    loop primary_find
    jmp advance_option
primary_found:
    jmp word ptr cs:[bx+1ch]

primary_keys dw 'C','E','I','M','S','T','V','c','e','i','m','s','t','v'
primary_handlers dw 5034h,502ch,5052h,5044h,505ah,503ch,504bh,5034h,502ch,5052h,5044h,505ah,503ch,504bh

set_mode_1:
    mov byte ptr ds:[0bfcdh],1
    jmp advance_option
set_mode_2:
    mov byte ptr ds:[0bfcdh],2
    jmp short advance_option
    nop
set_mode_3:
    mov byte ptr ds:[0bfcdh],3
    jmp short advance_option
    nop
set_mode_4:
    mov byte ptr ds:[0bfcdh],4
    jmp short advance_option
set_mode_5:
    mov byte ptr ds:[0bfcdh],5
    jmp short advance_option
set_state_0:
    mov word ptr ds:[1778h],0
    jmp short advance_option

secondary_dispatch:
    mov ax,si
    shl ax,1
    shl ax,1
    les bx,dword ptr ds:[6dh]
    add bx,ax
    les bx,es:[bx]
    mov al,es:[bx+2]
    cbw
    mov cx,6
    mov bx,5083h
secondary_find:
    cmp word ptr cs:[bx],ax
    je secondary_found
    inc bx
    inc bx
    loop secondary_find
    jmp short secondary_done
secondary_found:
    jmp word ptr cs:[bx+0ch]

secondary_keys dw 'A','I','T','a','i','t'
secondary_handlers dw 50a3h,509bh,50abh,50a3h,509bh,50abh

set_secondary_0:
    mov word ptr ds:[1778h],0
    jmp short secondary_done
set_secondary_2:
    mov word ptr ds:[1778h],2
    jmp short secondary_done
set_secondary_1:
    mov word ptr ds:[1778h],1
    jmp short secondary_done
secondary_done:
    jmp short advance_option
advance_option:
    inc si
scan_next:
    cmp si,word ptr ds:[6bh]
    jnl done
    jmp scan_option
done:
    pop si
    ret
_cmdline_parse_args endp
_TEXT ends
end
