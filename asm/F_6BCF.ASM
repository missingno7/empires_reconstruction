; F_6BCF -- timer interrupt: chain the prior handler, update the tick clock,
; service sound when needed, then acknowledge the master PIC.
_TEXT segment byte public 'CODE'
assume cs:_TEXT, ds:DGROUP
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
extrn _gc0d4:word, _g0b7a:dword, _g0b76:word, _g0b78:word
extrn _g176e:word, _g1772:word, _g237c:word
_BSS ends
DGROUP group _DATA, _BSS
extrn _fc1a0:near
public _f6bcf
_f6bcf:
 push ax
 push bx
 push cx
 push dx
 push es
 push ds
 push si
 push di
 push bp
 mov bp,DGROUP
 mov ds,bp
 pushf
 sti
 inc _gc0d4
 cmp _gc0d4,13
 jl short update_clock
 mov _gc0d4,0
 pushf
 call dword ptr _g0b7a
update_clock:
 add _g0b76,1
 adc _g0b78,0
 cmp _g237c,0
 jnz short acknowledge
 cmp _g176e,0
 jnz short service_sound
 cmp _g1772,0
 jz short acknowledge
service_sound:
 call _fc1a0
acknowledge:
 mov al,20h
 out 20h,al
 popf
 pop bp
 pop di
 pop si
 pop ds
 pop es
 pop dx
 pop cx
 pop bx
 pop ax
 iret
_TEXT ends
end
