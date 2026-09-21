; F_C1F7 -- voice pump.  Reads as C output until C20E, where it tests a global
; with  mov ax,[1776] / cmp ax,0.  TC 2.01 tests a loaded word with OR AX,AX:
; over the whole 64,048-byte _TEXT there are 58 OR AX,AX and exactly 2 CMP AX,0,
; only one of them preceded by MOV AX,[mem] -- this one.  Hand assembly.
DGROUP	GROUP	_DATA,_BSS
_DATA	SEGMENT WORD PUBLIC 'DATA'
_DATA	ENDS
_BSS	SEGMENT WORD PUBLIC 'BSS'
	EXTRN	_snd_mode:WORD		; DS:1784
	; /*@SYM _snd_mode=0x1784 kind=g key=storage_objects/G_P212B4.phys*/
	EXTRN	_snd_flag2:WORD		; DS:1776
	; /*@SYM _snd_flag2=0x1776 kind=g key=storage_objects/G_P212A6.phys*/
	EXTRN	_sv:WORD		; DS:1794
	; /*@SYM _sv=0x1794 kind=g key=storage_objects/G_P212C4.phys*/
	EXTRN	_dv:WORD		; DS:178C
	; /*@SYM _dv=0x178C kind=g key=storage_objects/G_P212BC.phys*/
_BSS	ENDS

_TEXT	SEGMENT BYTE PUBLIC 'CODE'
	ASSUME	CS:_TEXT, DS:DGROUP
	EXTRN	_sound_voice_table_prime:NEAR
	EXTRN	_sound_voice_service_loop:NEAR
	PUBLIC	_sound_voice_pump_loop
_sound_voice_pump_loop	PROC	NEAR
	push	bp
	mov	bp,sp
	cmp	word ptr _snd_mode,2
	je	L2
L1:	call	near ptr _sound_voice_table_prime
L2:	call	near ptr _sound_voice_service_loop
	cmp	word ptr _snd_mode,0
	jne	L3
	mov	ax,word ptr _snd_flag2
	cmp	ax,0
	je	L3
	mov	ax,word ptr _sv
	mov	word ptr _dv,ax
	mov	ax,word ptr _sv+2
	mov	word ptr _dv+2,ax
	mov	ax,word ptr _sv+4
	mov	word ptr _dv+4,ax
	mov	ax,word ptr _sv+6
	mov	word ptr _dv+6,ax
	jmp	L1
L3:	pop	bp
	ret
_sound_voice_pump_loop	ENDP
_TEXT	ENDS
	END
