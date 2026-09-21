; Original entry F_53BF -- select a display mode and probe VGA availability.

_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

public _f53bf
extrn _g1778:word
extrn _display_mode:word
extrn _opl_detect:near

_f53bf proc near
    mov word ptr ds:_g1778,0
    cmp word ptr ds:_display_mode,3
    jne probe_mode
    mov word ptr ds:_g1778,1
    jmp short selected

probe_mode:
    call _opl_detect
    or ax,ax
    jz bios_probe
    mov word ptr ds:_g1778,2
    jmp short selected

bios_probe:
    mov ah,0c0h
    int 15h
    or ah,ah
    jnz selected
    cmp word ptr es:[bx+2],0bfch
    jne selected

    mov cx,10
    mov dx,203h
wait_vga_port:
    mov al,0a5h
    out dx,al
    in al,dx
    cmp al,0a5h
    jne selected
    loop wait_vga_port
    mov word ptr ds:_g1778,3

selected:
    ret
_f53bf endp

_TEXT ends
end
