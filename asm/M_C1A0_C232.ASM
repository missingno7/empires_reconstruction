; M_C1A0_C232 -- contiguous sound-tick, voice-pump, and voice-table scanner.
; The first two entries call directly into the following entries and all three
; share the 0x1764..0x17F4 sound-control BSS cluster.
_TEXT	SEGMENT BYTE PUBLIC 'CODE'
DGROUP	GROUP	_DATA,_BSS
	ASSUME	CS:_TEXT,DS:DGROUP
_TEXT	ENDS
_DATA	SEGMENT WORD PUBLIC 'DATA'
d@	LABEL	BYTE
_DATA	ENDS
_BSS	SEGMENT WORD PUBLIC 'BSS'
b@	LABEL	BYTE
	EXTRN	_v_b:WORD,_snd_flag:WORD,_snd_mode:WORD,_snd_paused:WORD,_snd_seg2:WORD
	EXTRN	_snd_flag2:WORD,_sv:WORD,_dv:WORD
	EXTRN	_g1784:WORD,_g178a:WORD,_g177a:WORD,_g178c:WORD,_g179c:WORD
	EXTRN	_g17a4:WORD,_g17ac:WORD,_g17b4:WORD,_g17ec:WORD,_g17f4:WORD,_g17bc:WORD
_BSS	ENDS
_TEXT	SEGMENT BYTE PUBLIC 'CODE'
	PUBLIC	_fc1a0, _fc1f7, _fc232
	EXTRN	_fc8e2:NEAR, _fc27d:NEAR

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
	je	tick_l2
	call	near ptr _fc8e2
	cmp	word ptr _snd_mode,0
	je	tick_l3
	cmp	word ptr _snd_paused,0
	jne	tick_l1
	mov	word ptr _v_b,1
tick_l1:	mov	es,word ptr _snd_seg2
	call	near ptr _fc1f7
	jmp	tick_l3
tick_l2:	cmp	word ptr _snd_mode,0
	je	tick_l3
	mov	es,word ptr _snd_seg2
	call	near ptr _fc1f7
tick_l3:	pop	es
	pop	di
	pop	si
	pop	bx
	pop	dx
	pop	cx
	pop	ax
	pop	bp
	ret
_fc1a0	ENDP

_fc1f7	PROC	NEAR
	push	bp
	mov	bp,sp
	cmp	word ptr _snd_mode,2
	je	pump_l2
pump_l1:	call	near ptr _fc232
pump_l2:	call	near ptr _fc27d
	cmp	word ptr _snd_mode,0
	jne	pump_l3
	mov	ax,word ptr _snd_flag2
	cmp	ax,0
	je	pump_l3
	mov	ax,word ptr _sv
	mov	word ptr _dv,ax
	mov	ax,word ptr _sv+2
	mov	word ptr _dv+2,ax
	mov	ax,word ptr _sv+4
	mov	word ptr _dv+4,ax
	mov	ax,word ptr _sv+6
	mov	word ptr _dv+6,ax
	jmp	pump_l1
pump_l3:	pop	bp
	ret
_fc1f7	ENDP

_fc232 proc near
    push bp
    mov bp,sp
    xor ax,ax
    mov bx,2
    mov dx,6
    mov ds:_g1784,bx
    mov ds:_g178a,ax
    mov cx,ds:_g177a
    xor si,si
scan_entry:
    mov ds:[si+_g17ac],ax
    mov di,ds:[si+_g178c]
    cmp byte ptr es:[di],0ffh
    je next_entry
    mov ds:[si+_g17ac],bx
    mov ds:[si+_g179c],ax
    mov word ptr ds:[si+_g17a4],1
    mov ds:[si+_g17b4],ax
    mov ds:[si+_g17ec],ax
    mov ds:[si+_g17f4],ax
    mov ds:[si+_g17bc],dx
next_entry:
    add si,2
    loop scan_entry
    pop bp
    ret
_fc232 endp
_TEXT	ENDS
	END
