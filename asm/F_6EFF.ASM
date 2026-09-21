; F_6EFF -- decode a packed 4-bit-per-pixel sprite/tile record in place, the
; display_mode!=2 (EGA/VGA planar) sibling of F_6F4B. far ptr q = [bp+4]
; points at a record: 8 packed header/palette words whose upper nibble of
; each byte is masked off (and 0F0Fh), then a width/height byte pair, then
; packed nibble pixel data. bx is left pointing at the (now-masked) header,
; which doubles as the two xlatb translation tables used to expand each
; packed byte's low and high nibble into a full output byte (low nibble via
; table+((byte&0F00Fh) form, high nibble via table+(byte>>4)), OR'd together.
; near proc, C calling convention: far ptr arg q at [bp+4], no return value.
; Clobbers ax/bx/cx/dx; si/di/ds/bp saved via push/pop.

_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

public _sprite_decode_4bpp_planar

_sprite_decode_4bpp_planar proc near
    push bp
    mov bp,sp
    push si
    push di
    push ds
    cld
    les di,dword ptr [bp+4]
    mov ax,es
    mov ds,ax
    mov bx,di
    mov cx,8
clear_upper_nibbles:
    mov ax,es:[di]
    and ax,0f0fh
    stosw
    loop clear_upper_nibbles
    add di,10h
    mov si,di
    lodsw
    mul ah
    mov cx,ax
    mov di,si
expand_next_byte:
    lodsb
    mov ah,al
    and ax,0f00fh
    xlatb
    xchg ah,al
    shr al,1
    shr al,1
    shr al,1
    shr al,1
    xlatb
    shl al,1
    shl al,1
    shl al,1
    shl al,1
    or al,ah
    stosb
    loop expand_next_byte
    pop ds
    pop di
    pop si
    pop bp
    ret
_sprite_decode_4bpp_planar endp

_TEXT ends
end
