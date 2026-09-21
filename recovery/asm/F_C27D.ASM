; F_C27D -- per-voice service loop.  Uses SI as the loop cursor WITHOUT saving
; it; TC 2.01 saves si/di in any function whose asm mentions them, so this is
; not reachable from C.  Separate TASM module.
DGROUP	GROUP	_DATA,_BSS
_DATA	SEGMENT WORD PUBLIC 'DATA'
_DATA	ENDS
_BSS	SEGMENT WORD PUBLIC 'BSS'
	EXTRN	_v_a:WORD		; DS:17AC
	; /*@SYM _v_a=0x17AC kind=g key=storage_objects/G_P212DC.phys*/
	EXTRN	_v_ctr:WORD		; DS:179C
	; /*@SYM _v_ctr=0x179C kind=g key=storage_objects/G_P212CC.phys*/
	EXTRN	_snd_backend_mode:WORD	; DS:1778
	; /*@SYM _snd_backend_mode=0x1778 kind=g key=storage_objects/G_P212A8.phys*/
	EXTRN	_v_hold:WORD		; DS:17B4
	; /*@SYM _v_hold=0x17B4 kind=g key=storage_objects/G_P212E4.phys*/
	EXTRN	_v_len:WORD		; DS:17BC
	; /*@SYM _v_len=0x17BC kind=g key=storage_objects/G_P212EC.phys*/
	EXTRN	_snd_nvoices:WORD	; DS:177A
	; /*@SYM _snd_nvoices=0x177A kind=g key=storage_objects/G_P212AA.phys*/
_BSS	ENDS

_TEXT	SEGMENT BYTE PUBLIC 'CODE'
	ASSUME	CS:_TEXT, DS:DGROUP
	EXTRN	_fc2ea:NEAR
	EXTRN	_fc6b9:NEAR
	EXTRN	_fc755:NEAR
	EXTRN	_fc440:NEAR
	PUBLIC	_sound_voice_service_loop
_sound_voice_service_loop	PROC	NEAR
	push	bp
	mov	bp,sp
	xor	si,si
L1:	cmp	word ptr [si+_v_a],0
	je	L3
	cmp	word ptr [si+_v_ctr],0
	jne	L2
	call	near ptr _fc2ea
	cmp	word ptr [si+_v_a],0
	je	L3
L2:	dec	word ptr [si+_v_ctr]
	cmp	word ptr _snd_backend_mode,1
	je	L3
	cmp	word ptr [si+_v_hold],1
	je	L3
	mov	ax,word ptr [si+_v_len]
	dec	ax
	cmp	word ptr [si+_v_ctr],ax
	jne	L3
	call	near ptr _fc6b9
L3:	add	si,2
	mov	ax,si
	shr	ax,1
	cmp	ax,word ptr _snd_nvoices
	jl	L1
	mov	ax,word ptr _v_a
	or	ax,word ptr _v_a+2
	or	ax,word ptr _v_a+4
	or	ax,word ptr _v_a+6
	jne	L4
	call	near ptr _fc755
	jmp	L5
L4:	cmp	word ptr _snd_backend_mode,1
	jne	L5
	call	near ptr _fc440
L5:	pop	bp
	ret
_sound_voice_service_loop	ENDP
_TEXT	ENDS
	END
