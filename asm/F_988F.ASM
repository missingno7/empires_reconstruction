; Original entry F_988F -- draw two fixed rectangles using recovered BSS coordinates.

_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

public _f_988f
extrn _f03ba:near
extrn _f03b4:near
extrn _gc350:word
extrn _gc34e:word

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

_TEXT ends
end
