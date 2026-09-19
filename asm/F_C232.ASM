; Original entry F_C232 -- initialize and scan the ES:DI sound-data table.

_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

public _fc232
extrn _g1784:word, _g178a:word, _g177a:word, _g17ac:word
extrn _g178c:word, _g179c:word, _g17a4:word, _g17b4:word
extrn _g17ec:word, _g17f4:word, _g17bc:word

_fc232 proc near
    push bp
    mov bp,sp
    xor ax,ax
    mov bx,2
    mov dx,6
    mov ds:_g1784,bx
    mov ds:_g178a,ax
    mov cx,ds:_g177a
    xor si,si
scan_entry:
    mov ds:[si+_g17ac],ax
    mov di,ds:[si+_g178c]
    cmp byte ptr es:[di],0ffh
    je next_entry
    mov ds:[si+_g17ac],bx
    mov ds:[si+_g179c],ax
    mov word ptr ds:[si+_g17a4],1
    mov ds:[si+_g17b4],ax
    mov ds:[si+_g17ec],ax
    mov ds:[si+_g17f4],ax
    mov ds:[si+_g17bc],dx
next_entry:
    add si,2
    loop scan_entry
    pop bp
    ret
_fc232 endp

_TEXT ends
end
