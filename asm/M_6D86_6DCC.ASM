; Word alignment lets TLINK supply the preceding inter-module pad.
; M_6D86_6DCC -- contiguous two-stage decoder module.
;
; The PackBits stage ends at 6DC5h.  The seven following bytes are the
; bit-reader state addressed through CS by the LZ stage at 6DCCh, so they are
; module-local initialized code-segment storage, not alignment padding.
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
	PUBLIC	_f6d86, _f6dcc

_f6d86	PROC	NEAR
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
_f6d86	ENDP

; LZ bit-reader state at module offsets 3Fh..45h.
bitlen	dw	0
inleft	dw	0
bitcnt	dw	0
bitbuf	db	0

_f6dcc	PROC	NEAR
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
_f6dcc	ENDP
_TEXT	ENDS
	END
