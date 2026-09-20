; F_98CB -- issue the two fixed parameterized display/service calls.
; Both callees are historical nonrelocating same-segment calls.
_TEXT segment byte public 'CODE'
assume cs:_TEXT, ds:DGROUP
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA, _BSS
public _f_98cb
public _f_98cb_end
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
