_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP
public _f6f4b
_f6f4b proc near
 push bp
 mov bp,sp
 sub sp,4
 push si
 push di
 push ds
 push bp
 cld
 lds bx,[bp+4]
 mov ax,ds
 mov es,ax
 mov di,bx
 xor al,al
 stosb
 mov cx,15
header_loop:
 mov al,es:[di]
 mov ah,al
 and ax,0c030h
 shr ax,1
 shr ax,1
 shr al,1
 shr al,1
 add ax,1001h
 or al,ah
 stosb
 loop header_loop
 add di,10h
 mov ax,es:[di]
 inc di
 inc di
 mov dh,al
 mul ah
 mov bp,ax
 xor cl,cl
 mov dl,dh
decode_loop:
 mov al,es:[di]
 mov ah,al
 and ax,0ff0h
 shr al,1
 shr al,1
 shr al,1
 shr al,1
 xlatb
 rol al,cl
 xchg al,ah
 xlatb
 rol al,cl
 and ax,0f00fh
 or al,ah
 stosb
 dec bp
 jz short done
 dec dl
 jnz short decode_loop
 mov dl,dh
 xor cl,4
 jmp short decode_loop
done:
 pop bp
 pop ds
 pop di
 pop si
 mov sp,bp
 pop bp
 ret
_f6f4b endp
_TEXT ends
end
