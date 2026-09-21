; Word alignment lets TLINK supply the preceding inter-module pad.
; M_6D86_6F4B -- five contiguous decoder routines assembled as one
; hand-written TASM module: F_6D86 (PackBits expansion), PAD_006FC5 (the
; LZ stage's module-local CS-addressed bit-reader state, not alignment
; padding), F_6DCC (LZ-style decompressor), F_6EFF (4bpp planar sprite
; decode), F_6F4B (4bpp mode-13h sprite decode).
;
; The PackBits stage ends at 6DC5h.  The seven following bytes are the
; bit-reader state addressed through CS by the LZ stage at 6DCCh, so they are
; module-local initialized code-segment storage, not alignment padding.
;
; F_6D86 (0x6F86) starts at an even address with the pad TLINK supplies
; before a word-aligned module; F_6DCC (0x6FCC) follows the inline bit-reader
; state; F_6EFF (0x70FF, odd) and F_6F4B (0x714B, odd) both start where the
; routine before them in this same module left off, so neither can be the
; head of its own word-aligned module.
_TEXT	SEGMENT word PUBLIC 'CODE'
DGROUP	GROUP	_DATA,_BSS
	ASSUME	CS:_TEXT,DS:DGROUP
_TEXT	ENDS
_DATA	SEGMENT WORD PUBLIC 'DATA'
d@	LABEL	BYTE
_DATA	ENDS
_BSS	SEGMENT WORD PUBLIC 'BSS'
b@	LABEL	BYTE
_BSS	ENDS
_TEXT	SEGMENT word PUBLIC 'CODE'
	PUBLIC	_rle_packbits_decode, _lz_decompress
	PUBLIC	_sprite_decode_4bpp_planar, _sprite_decode_4bpp_mode13h

; F_6D86 -- PackBits-style RLE expansion. far src=[bp+4], far dst=[bp+8],
; word max_len=[bp+0Ch] (end-of-src bound for bx). Reads signed control
; bytes: negative -> (1-n) repeats of the following byte (rep stosb);
; non-negative -> copy n+1 literal bytes (rep movsb). Loops until si
; reaches the src+max_len bound. Returns ax = bytes written (di - dst).
; near proc, C calling convention; clobbers ax/bx/cx/dx; si/di/es/ds saved.
_rle_packbits_decode	PROC	NEAR
	push	bp
	mov	bp,sp
	push	si
	push	di
	push	es
	push	ds
	cld
	lds	si,[bp+4]
	les	di,[bp+8]
	mov	bx,si
	mov	dx,di
	mov	ax,[bp+0ch]
	add	bx,ax
L_item:
	lodsb
	cbw
	cmp	ax,0
	jg	L_lit
	neg	ax
	inc	ax
	mov	cx,ax
	lodsb
	rep	stosb
	cmp	si,bx
	jge	L_done
	jmp	short L_item
L_lit:
	mov	cx,ax
	rep	movsb
	cmp	si,bx
	jl	L_item
L_done:
	mov	ax,di
	sub	ax,dx
	clc
	pop	ds
	pop	es
	pop	di
	pop	si
	pop	bp
	ret
_rle_packbits_decode	ENDP

; LZ bit-reader state at module offsets 3Fh..45h.
bitlen	dw	0
inleft	dw	0
bitcnt	dw	0
bitbuf	db	0

; F_6DCC -- LZ-style decompressor (variable-width codes, growing from 9
; bits, code 100h = width-increase marker). far src=[bp+4], far dst=[bp+8],
; word src_len=[bp+0Ch]. bp = base of the src buffer, used to index a
; string table of {start,end} word pairs immediately following the 2-byte
; header (the encoder's back-reference table); codes <=0FFh are literal
; bytes, codes >0FFh copy a previously emitted string via that table. The
; bit reader's state (bitlen/inleft/bitcnt/bitbuf, declared as CS-relative
; storage between the two procs) is shared code-segment storage, not stack
; locals -- required so lz_decompress can be re-entered opcode-by-opcode while si
; tracks the compressed input. Returns ax = di (end of decompressed output).
; near proc, C calling convention; clobbers ax/bx/cx/dx; si/di/es/ds saved.
_lz_decompress	PROC	NEAR
	push	bp
	mov	bp,sp
	push	si
	push	di
	push	es
	push	ds
	cld
	lds	si,[bp+4]
	les	di,[bp+8]
	mov	ax,[bp+0ch]
	mov	bp,si
	mov	bx,si
	sub	ax,2
	inc	ax
	mov	cs:inleft,ax
	add	si,2
	mov	word ptr cs:bitcnt,0
	mov	word ptr cs:bitlen,9
L_head:
	mov	[bx],di
	add	bx,2
L_sym1:
	push	bx
	xor	ax,ax
	mov	dx,cs:bitcnt
	mov	bl,cs:bitbuf
	mov	cx,cs:bitlen
L_bit1:
	dec	dx
	cmp	dx,0
	jl	L_fill1
L_shift1:
	rcl	bl,1
	rcl	ax,1
	loop	L_bit1
	jmp	short L_done1
L_fill1:
	mov	bl,[si]
	inc	si
	mov	dx,7
	dec	word ptr cs:inleft
	jne	L_shift1
L_done1:
	mov	cs:bitcnt,dx
	mov	cs:bitbuf,bl
	pop	bx
	cmp	ax,100h
	jne	L_chk1
	inc	word ptr cs:bitlen
	jmp	L_sym1
L_chk1:
	cmp	word ptr cs:inleft,0
	jne	L_go1
	jmp	L_exit
L_go1:
	cmp	ax,0ffh
	jle	L_lit1
	sub	ax,101h
	shl	ax,1
	add	ax,bp
	push	si
	mov	si,ax
	mov	ax,[si]
	mov	cx,[si+2]
	sub	cx,ax
	mov	si,ax
	mov	dx,ds
	push	es
	pop	ds
	test	cx,1
	je	L_even1
	movsb
L_even1:
	shr	cx,1
	rep	movsw
	mov	ds,dx
	pop	si
	jmp	short L_sym2
L_lit1:
	stosb
L_sym2:
	push	bx
	xor	ax,ax
	mov	dx,cs:bitcnt
	mov	bl,cs:bitbuf
	mov	cx,cs:bitlen
L_bit2:
	dec	dx
	cmp	dx,0
	jl	L_fill2
L_shift2:
	rcl	bl,1
	rcl	ax,1
	loop	L_bit2
	jmp	short L_done2
L_fill2:
	mov	bl,[si]
	inc	si
	mov	dx,7
	dec	word ptr cs:inleft
	jne	L_shift2
L_done2:
	mov	cs:bitcnt,dx
	mov	cs:bitbuf,bl
	pop	bx
	cmp	ax,100h
	jne	L_chk2
	inc	word ptr cs:bitlen
	jmp	L_sym2
L_chk2:
	cmp	word ptr cs:inleft,0
	je	L_exit
	cmp	ax,0ffh
	jle	L_lit2
	sub	ax,101h
	shl	ax,1
	add	ax,bp
	push	si
	mov	si,ax
	mov	ax,[si]
	mov	cx,[si+2]
	sub	cx,ax
	mov	si,ax
	mov	dx,ds
	push	es
	pop	ds
	test	cx,1
	je	L_even2
	movsb
L_even2:
	shr	cx,1
	rep	movsw
	mov	ds,dx
	pop	si
	jmp	short L_lit2x
L_lit2:
	stosb
L_lit2x:
	jmp	L_head
L_exit:
	mov	ax,di
	pop	ds
	pop	es
	pop	di
	pop	si
	pop	bp
	ret
_lz_decompress	ENDP

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
_TEXT	ENDS
	END
