; F_CC17 -- submit the selected row's 0Dh table field.
_TEXT segment byte public 'CODE'
assume cs:_TEXT, ds:DGROUP
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA, _BSS
public _options_toggle_sound
public _f_cc17_end
_options_toggle_sound:
 push si
 mov ax,word ptr ds:[13edh]
 mov dx,1bh
 mul dx
 mov bx,ax
 add bx,0c470h
 push ds
 pop es
 mov ax,es:[bx+0dh]
 neg ax
 sbb ax,ax
 inc ax
 mov byte ptr ds:[217fh],al
 push ds
 mov ax,2174h
 push ax
 call $+0ba90h
 pop cx
 pop cx
 mov si,ax
 or si,si
 jl short done
 mov ax,si
 push ax
 mov ax,word ptr ds:[13edh]
 mov dx,1bh
 mul dx
 mov bx,ax
 add bx,0c470h
 push ds
 pop es
 pop ax
 mov es:[bx+0dh],ax
 mov word ptr ds:[176eh],ax
 or si,si
 jnz short done
 call $+0fee5h
done:
 xor ax,ax
 jmp short _f_cc17_end
_f_cc17_end:
 pop si
 ret
_TEXT ends
end
