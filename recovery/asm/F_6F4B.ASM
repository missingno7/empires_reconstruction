; F_6F4B -- decode a packed 4-bit-per-pixel sprite/tile record in place,
; the display_mode==2 (mode-13h) sibling of F_6EFF. far ptr q = [bp+4] points
; at a record starting with a 0 byte, 15 header/palette bytes remapped from
; VGA-attribute form to mode-13h form, then width/height bytes, then packed
; nibble pixel data. Each output byte is produced by two xlatb lookups
; through the remapped palette at q (rotated by a shifting nibble-select
; count that flips 0/4 every "width" pixels), so the routine both rewrites
; the 15-byte palette table and expands the trailing nibble stream to one
; byte per pixel, overwriting the source buffer as it goes.
; near proc, C calling convention: far ptr arg q at [bp+4], no return value.
; Clobbers ax/bx/cx/dx; si/di/ds/bp saved via push/pop.
_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP
public _sprite_decode_4bpp_mode13h
_sprite_decode_4bpp_mode13h proc near
 push bp
 mov bp,sp
 sub sp,4
 push si
 push di
 push ds
 push bp
 cld
; es:di = ds:bx = q (record start); bx doubles as the xlatb table base below
 lds bx,[bp+4]
 mov ax,ds
 mov es,ax
 mov di,bx
 xor al,al
 stosb
 mov cx,15
; remap the 15-byte header/palette in place: attribute nibble -> mode-13h form
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
; skip to the width/height byte pair; bp = pixel count = width*height
 add di,10h
 mov ax,es:[di]
 inc di
 inc di
 mov dh,al
 mul ah
 mov bp,ax
 xor cl,cl
 mov dl,dh
; expand each packed nibble to a byte via two xlatb lookups through the
; remapped palette (table base = bx), rotating by cl which flips 0/4 every
; "width" (dh) output pixels
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
_sprite_decode_4bpp_mode13h endp
_TEXT ends
end
