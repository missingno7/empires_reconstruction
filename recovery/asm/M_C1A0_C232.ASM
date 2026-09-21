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
	EXTRN	_v_b:WORD,_snd_on:WORD,_snd_mode:WORD,_snd_backend_mode:WORD,_snd_seg2:WORD
	EXTRN	_snd_flag2:WORD,_voice_stream_base_table:WORD,_voice_stream_cursor_table:WORD
	EXTRN	_g1784:WORD,_g178a:WORD,_g177a:WORD,_voice_stream_cursor_table:WORD,_g179c:WORD
	EXTRN	_g17a4:WORD,_g17ac:WORD,_g17b4:WORD,_voice_rest_table:WORD,_g17f4:WORD,_g17bc:WORD
_BSS	ENDS
_TEXT	SEGMENT BYTE PUBLIC 'CODE'
	PUBLIC	_sound_tick_entry, _sound_voice_pump_loop, _sound_voice_table_prime
	EXTRN	_sound_tick_step:NEAR, _sound_voice_service_loop:NEAR

_sound_tick_entry	PROC	NEAR
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
	cmp	word ptr _snd_on,0
	je	tick_l2
	call	near ptr _sound_tick_step
	cmp	word ptr _snd_mode,0
	je	tick_l3
	cmp	word ptr _snd_backend_mode,0
	jne	tick_l1
	mov	word ptr _v_b,1
tick_l1:	mov	es,word ptr _snd_seg2
	call	near ptr _sound_voice_pump_loop
	jmp	tick_l3
tick_l2:	cmp	word ptr _snd_mode,0
	je	tick_l3
	mov	es,word ptr _snd_seg2
	call	near ptr _sound_voice_pump_loop
tick_l3:	pop	es
	pop	di
	pop	si
	pop	bx
	pop	dx
	pop	cx
	pop	ax
	pop	bp
	ret
_sound_tick_entry	ENDP

_sound_voice_pump_loop	PROC	NEAR
	push	bp
	mov	bp,sp
	cmp	word ptr _snd_mode,2
	je	pump_l2
pump_l1:	call	near ptr _sound_voice_table_prime
pump_l2:	call	near ptr _sound_voice_service_loop
	cmp	word ptr _snd_mode,0
	jne	pump_l3
	mov	ax,word ptr _snd_flag2
	cmp	ax,0
	je	pump_l3
	mov	ax,word ptr _voice_stream_base_table
	mov	word ptr _voice_stream_cursor_table,ax
	mov	ax,word ptr _voice_stream_base_table+2
	mov	word ptr _voice_stream_cursor_table+2,ax
	mov	ax,word ptr _voice_stream_base_table+4
	mov	word ptr _voice_stream_cursor_table+4,ax
	mov	ax,word ptr _voice_stream_base_table+6
	mov	word ptr _voice_stream_cursor_table+6,ax
	jmp	pump_l1
pump_l3:	pop	bp
	ret
_sound_voice_pump_loop	ENDP

_sound_voice_table_prime proc near
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
    mov di,ds:[si+_voice_stream_cursor_table]
    cmp byte ptr es:[di],0ffh
    je next_entry
    mov ds:[si+_g17ac],bx
    mov ds:[si+_g179c],ax
    mov word ptr ds:[si+_g17a4],1
    mov ds:[si+_g17b4],ax
    mov ds:[si+_voice_rest_table],ax
    mov ds:[si+_g17f4],ax
    mov ds:[si+_g17bc],dx
next_entry:
    add si,2
    loop scan_entry
    pop bp
    ret
_sound_voice_table_prime endp
_TEXT	ENDS
	END
