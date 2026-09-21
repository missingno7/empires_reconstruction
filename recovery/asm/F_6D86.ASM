; F_6D86 -- the PackBits-style RLE stage of the resource decoder, as a
; hand-written TASM module.  cld / lodsb / rep stosb / rep movsb are all in
; runF's R6 set (instructions Turbo C 2.0 never emits), so this cannot be a C
; translation unit.  In the shipped image it shares one module with F_6DCC
; (see M6D86.ASM); this file is the per-function row.
;
;   int far_rle(char far *src, char far *dst, int srclen)
;      n = *src++ (signed);  n > 0 -> n literal bytes
;                            n <= 0 -> (1-n) copies of the next byte
;      until the source is exhausted; returns the number of bytes written.
_TEXT	SEGMENT BYTE PUBLIC 'CODE'
	ASSUME	CS:_TEXT
	PUBLIC	_rle_packbits_decode
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
_TEXT	ENDS
	END
