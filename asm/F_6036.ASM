_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
 extrn _sprbase:dword, _objtab:dword, _g0a20:word
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP
extrn _fd825:near, _f03cc:near
public _f6036
_f6036 proc near
 push bp
 mov bp,sp
 push si
 push di
 push ds
 cld
 mov bp,2fh
 les di,dword ptr ds:_sprbase
 lds si,dword ptr ds:_objtab
 lodsb
 xor ch,ch
 mov cl,al
sprite_loop:
 push di
 inc bp
 lodsw
 mov bx,ax
 lodsb
 xor ah,ah
 and al,1fh
 mov dx,1e6h
 mul dx
 add ax,2
 add di,ax
 xor dx,dx
 mov dl,bh
 mov bh,dh
 push ds
 push cx
 push es
 mov ax,DGROUP
 mov ds,ax
 mov ax,1eh
 push ax
 mov ax,0fh
 push ax
 push dx
 push bx
 push bp
 call _fd825
 pop ax
 pop bx
 pop dx
 add sp,4
 pop es
 push es
 xor ax,ax
 push ax
 push es
 push di
 add dx,0b8h
 push dx
 shl bx,1
 push bx
 call _f03cc
 add sp,0ah
 pop es
 pop cx
 pop ds
 pop di
 loop sprite_loop
 mov word ptr ds:_g0a20,10
 pop ds
 pop di
 pop si
 pop bp
 ret
_f6036 endp
_TEXT ends
end