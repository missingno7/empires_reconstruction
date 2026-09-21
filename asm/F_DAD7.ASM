; F_DAD7 -- clamp a byte selection and submit its associated table entry.
_TEXT segment byte public 'CODE'
assume cs:_TEXT, ds:DGROUP
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA, _BSS
public _voice_channel_set_gate
public _f_dad7_end
_voice_channel_set_gate:
 push bp
 mov bp,sp
 push si
 cmp word ptr [bp+6],0
 jz short zero_value
 mov ax,7fh
 jmp short store_value
zero_value:
 xor ax,ax
store_value:
 mov [bp+6],ax
 mov bx,[bp+4]
 shl bx,1
 add bx,2fd2h
 push ds
 pop es
 mov al,es:[bx+1]
 cbw
 mov si,ax
 cmp word ptr [bp+6],7fh
 jna short bounded
 mov word ptr [bp+6],7fh
bounded:
 mov al,[bp+6]
 mov [si-35b0h],al
 push si
 call $+6e2h
 pop cx
_f_dad7_end:
 pop si
 pop bp
 ret
_TEXT ends
end
