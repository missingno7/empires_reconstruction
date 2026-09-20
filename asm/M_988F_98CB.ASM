; M_988F_98CB -- paired fixed-rectangle display helpers.
_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
 extrn _gc350:word, _gc34e:word
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP
extrn _f03ba:near, _f03b4:near
public _f_988f, _f_98cb, _f_98cb_end

_f_988f proc near
    mov ax,168h
    push ax
    mov ax,50h
    push ax
    mov ax,1dh
    push ax
    mov ax,28h
    push ax
    mov ax,168h
    push ax
    xor ax,ax
    push ax
    call _f03ba
    add sp,0ch
    push word ptr ds:_gc350
    push word ptr ds:_gc34e
    mov ax,1dh
    push ax
    mov ax,28h
    push ax
    mov ax,168h
    push ax
    mov ax,50h
    push ax
    call _f03b4
    add sp,0ch
    ret
_f_988f endp



_f_98cb:
 mov ax,168h
 push ax
 mov ax,50h
 push ax
 mov ax,1dh
 push ax
 mov ax,28h
 push ax
 mov ax,168h
 push ax
 mov ax,1
 push ax
 call $+6ad4h
 add sp,0ch
 push word ptr ds:[0c350h]
 push word ptr ds:[0c34eh]
 mov ax,1dh
 push ax
 mov ax,28h
 push ax
 mov ax,168h
 push ax
 mov ax,50h
 push ax
 call $+6ab3h
 add sp,0ch
_f_98cb_end:
 ret

_TEXT ends
end
