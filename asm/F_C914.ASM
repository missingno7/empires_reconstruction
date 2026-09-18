; F_C914 -- music-stream command decoder.  Uses DI as the stream cursor and ES
; as its segment without saving either; TC 2.01 would emit push di.  Separate
; TASM module.  The 90h fillers after the short jumps are TASM's own backfill
; of a forward JMP it first sized as near, not hand-written padding.
DGROUP	GROUP	_DATA,_BSS
_DATA	SEGMENT WORD PUBLIC 'DATA'
_DATA	ENDS
_BSS	SEGMENT WORD PUBLIC 'BSS'
	EXTRN	_mus_ptr:WORD		; DS:1E88
	; /*@SYM _mus_ptr=0x1E88 kind=g key=storage_objects/G_P219B8.phys*/
	EXTRN	_snd_delay:WORD		; DS:1E90
	; /*@SYM _snd_delay=0x1E90 kind=g key=storage_objects/G_P219C0.phys*/
	EXTRN	_mus_flag:WORD		; DS:1774
	; /*@SYM _mus_flag=0x1774 kind=g key=storage_objects/G_P212A4.phys*/
	EXTRN	_mus_arg:WORD		; DS:1E8A
	; /*@SYM _mus_arg=0x1E8A kind=g key=storage_objects/G_P219BA.phys*/
_BSS	ENDS

_TEXT	SEGMENT BYTE PUBLIC 'CODE'
	ASSUME	CS:_TEXT, DS:DGROUP
	EXTRN	_fc988:NEAR
	EXTRN	_fc9a4:NEAR
	EXTRN	_fcadb:NEAR
	EXTRN	_fca03:NEAR
	EXTRN	_fca9b:NEAR
	EXTRN	_fcaf1:NEAR
	EXTRN	_fcb48:NEAR
	PUBLIC	_fc914
_fc914	PROC	NEAR
	push	bp
	mov	bp,sp
L0:	mov	di,word ptr _mus_ptr
	mov	al,byte ptr es:[di]
	mov	ah,al
	shr	ah,1
	shr	ah,1
	shr	ah,1
	shr	ah,1
	and	al,0fh
	cmp	al,0
	je	L1
	cmp	al,0dh
	je	L2
	cmp	al,0eh
	je	L3
	cmp	al,0fh
	je	L4
	call	near ptr _fc988
	call	near ptr _fc9a4
	jmp	L7
L1:	call	near ptr _fcadb
	call	near ptr _fc9a4
	jmp	L7
L2:	call	near ptr _fca03
	cmp	word ptr _snd_delay,0
	jmp	L7
L3:	call	near ptr _fca9b
	jmp	L7
L4:	call	near ptr _fcadb
	cmp	word ptr _mus_flag,0
	je	L6
	mov	ax,word ptr _mus_arg
	push	ax
	call	near ptr _fcaf1
	pop	ax
	inc	word ptr _snd_delay
	pop	bp
	ret
L6:	call	near ptr _fcb48
	pop	bp
	ret
L7:	add	word ptr _mus_ptr,2
	cmp	word ptr _snd_delay,0
	je	L0
	pop	bp
	ret
_fc914	ENDP
_TEXT	ENDS
	END
