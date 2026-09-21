; F_C834 -- voice reset.  NOT expressible as a C function with an asm body:
; the register saves are push ax / push cx / push si, and TC 2.01 always emits
; its own push si / push di AFTER the frame and BEFORE any asm, so no C source
; can produce this order.  Separate TASM module.
	DOSSEG
DGROUP	GROUP	_DATA,_BSS
_DATA	SEGMENT WORD PUBLIC 'DATA'
_DATA	ENDS
_BSS	SEGMENT WORD PUBLIC 'BSS'
	EXTRN	_snd_mode:WORD		; DS:1784
	; /*@SYM _snd_mode=0x1784 kind=g key=storage_objects/G_P212B4.phys*/
	EXTRN	_snd_hi:WORD		; DS:1786
	; /*@SYM _snd_hi=0x1786 kind=g key=storage_objects/G_P212B6.phys*/
	EXTRN	_snd_nvoices:WORD	; DS:177A
	; /*@SYM _snd_nvoices=0x177A kind=g key=storage_objects/G_P212AA.phys*/
	EXTRN	_snd_backend_mode:WORD	; DS:1778
	; /*@SYM _snd_backend_mode=0x1778 kind=g key=storage_objects/G_P212A8.phys*/
	EXTRN	_v_a:WORD		; DS:17AC
	; /*@SYM _v_a=0x17AC kind=g key=storage_objects/G_P212DC.phys*/
	EXTRN	_v_b:WORD		; DS:177C
	; /*@SYM _v_b=0x177C kind=g key=storage_objects/G_P212AC.phys*/
_BSS	ENDS

_TEXT	SEGMENT BYTE PUBLIC 'CODE'
	ASSUME	CS:_TEXT, DS:DGROUP
	EXTRN	_fc6b9:NEAR
	PUBLIC	_sound_voices_reset
_sound_voices_reset	PROC	NEAR
	push	bp
	mov	bp,sp
	push	ax
	push	cx
	push	si
	mov	word ptr _snd_mode,0
	mov	word ptr _snd_hi,0ffffh
	xor	si,si
	mov	cx,word ptr _snd_nvoices
	push	cx
	cmp	word ptr _snd_backend_mode,2
	jne	L1
	mov	word ptr _snd_nvoices,4
L1:	mov	word ptr [si+_v_a],0
	mov	word ptr [si+_v_b],0
	call	near ptr _fc6b9
	add	si,2
	loop	L1
	pop	word ptr _snd_nvoices
	pop	si
	pop	cx
	pop	ax
	pop	bp
	ret
_sound_voices_reset	ENDP
_TEXT	ENDS
	END
