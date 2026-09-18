; F_C1A0 -- the master sound tick.  Saves ax,cx,dx,bx,si,di,es by hand after
; the frame.  TC 2.01 always emits its own push si / push di immediately after
; the frame and before any asm, so this order is not reachable from C.
DGROUP	GROUP	_DATA,_BSS
_DATA	SEGMENT WORD PUBLIC 'DATA'
_DATA	ENDS
_BSS	SEGMENT WORD PUBLIC 'BSS'
	EXTRN	_v_b:WORD		; DS:177C .. 1782
	; /*@SYM _v_b=0x177C kind=g key=storage_objects/G_P212AC.phys*/
	EXTRN	_snd_flag:WORD		; DS:1770
	; /*@SYM _snd_flag=0x1770 kind=g key=storage_objects/G_P212A0.phys*/
	EXTRN	_snd_mode:WORD		; DS:1784
	; /*@SYM _snd_mode=0x1784 kind=g key=storage_objects/G_P212B4.phys*/
	EXTRN	_snd_paused:WORD	; DS:1778
	; /*@SYM _snd_paused=0x1778 kind=g key=storage_objects/G_P212A8.phys*/
	EXTRN	_snd_seg2:WORD		; DS:1764
	; /*@SYM _snd_seg2=0x1764 kind=g key=storage_objects/G_P21294.phys*/
_BSS	ENDS

_TEXT	SEGMENT BYTE PUBLIC 'CODE'
	ASSUME	CS:_TEXT, DS:DGROUP
	EXTRN	_fc1f7:NEAR
	EXTRN	_fc8e2:NEAR
	PUBLIC	_fc1a0
_fc1a0	PROC	NEAR
	push	bp
	mov	bp,sp
	push	ax
	push	cx
	push	dx
	push	bx
	push	si
	push	di
	push	es
	xor	ax,ax
	mov	word ptr _v_b,ax
	mov	word ptr _v_b+2,ax
	mov	word ptr _v_b+4,ax
	mov	word ptr _v_b+6,ax
	cmp	word ptr _snd_flag,0
	je	L2
	call	near ptr _fc8e2
	cmp	word ptr _snd_mode,0
	je	L3
	cmp	word ptr _snd_paused,0
	jne	L1
	mov	word ptr _v_b,1
L1:	mov	es,word ptr _snd_seg2
	call	near ptr _fc1f7
	jmp	L3
L2:	cmp	word ptr _snd_mode,0
	je	L3
	mov	es,word ptr _snd_seg2
	call	near ptr _fc1f7
L3:	pop	es
	pop	di
	pop	si
	pop	bx
	pop	dx
	pop	cx
	pop	ax
	pop	bp
	ret
_fc1a0	ENDP
_TEXT	ENDS
	END
