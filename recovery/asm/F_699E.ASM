; F_699E -- the keyboard ISR (INT 09h handler).  Saves all nine registers by
; hand, builds a 4-byte local frame, re-enables interrupts, reads the
; keyboard controller (port 60h) and either services the E0h/E1h extended-
; scancode prefix directly (port 61h ack toggle, EOI, iret), or masks the
; break bit and looks the scancode up in a 15-entry cs-relative jump table
; (embedded in this module: a value array then a parallel target array,
; found by cmp word ptr cs:[bx],ax / loop, dispatched by
; jmp word ptr cs:[bx+30]) that services the numeric-keypad/cursor block,
; conditionally calls fF68CF, chains through the far pointer at
; DGROUP:0C0CCh when neither DGROUP:0B72 nor di gates it, and falls into a
; shared port-61h ack / EOI tail before the nine-register epilogue and IRET.
; docs/agent-context/substrate.md's F_699E task: 1,038 call sites from inside
; the master-tick ISR fC1A0's own caller (timer_irq_handler) and 88 from the BIOS chain;
; no callees inside its own 380-byte extent.
;
; Historical reference only. src/F_699E.C now matches the full extent using
; Turbo C interrupt support, native I/O intrinsics, and an ordinary switch.
; Register saves, CS-relative dispatch, and IRET are compiler-generated.
DGROUP	GROUP	_DATA,_BSS
_DATA	SEGMENT WORD PUBLIC 'DATA'
_DATA	ENDS
_BSS	SEGMENT WORD PUBLIC 'BSS'
	EXTRN	_key_up_held:WORD		; DS:0B68
	EXTRN	_gb6a:WORD		; DS:0B6A
	EXTRN	_key_up_left_held:WORD		; DS:0B6C
	EXTRN	_key_up_right_held:WORD		; DS:0B6E
	EXTRN	_key_up_released:WORD		; DS:0B70
	EXTRN	_keyboard_state:WORD		; DS:0B72
	EXTRN	_int9_saved_vector:DWORD		; DS:0C0CC .. 0C0CE, the old INT 09h vector chain
_BSS	ENDS

_TEXT	SEGMENT BYTE PUBLIC 'CODE'
	ASSUME	CS:_TEXT, DS:DGROUP
	EXTRN	_sound_effects_toggle:NEAR
	PUBLIC	_keyboard_irq_handler
_keyboard_irq_handler	PROC	NEAR
	push	ax
	push	bx
	push	cx
	push	dx
	push	es
	push	ds
	push	si
	push	di
	push	bp
	mov	bp,DGROUP
	mov	ds,bp
	mov	bp,sp
	sub	sp,4
	sti
	in	al,60h
	mov	byte ptr [bp-1],al
	mov	di,1
	cmp	byte ptr [bp-1],0E0h
	je	prefix
	cmp	byte ptr [bp-1],0E1h
	jne	noprefix
prefix:
	in	al,61h
	mov	ah,0
	mov	[bp-4],ax
	mov	al,byte ptr [bp-4]
	or	al,80h
	out	61h,al
	mov	al,byte ptr [bp-4]
	out	61h,al
	mov	al,20h
	out	20h,al
	jmp	near ptr epilogue
noprefix:
	cmp	byte ptr [bp-1],80h
	jae	makecode
	mov	si,1
	jmp	short havecode
makecode:
	xor	si,si
havecode:
	mov	al,byte ptr [bp-1]
	mov	ah,0
	and	ax,7Fh
	mov	cx,0Fh
	mov	bx,offset scantab
scan:
	cmp	word ptr cs:[bx],ax
	je	found
	inc	bx
	inc	bx
	loop	scan
	jmp	near ptr nomatch
found:
	jmp	word ptr cs:[bx+30]

scantab	dw	01Dh,01Fh,029h,02Bh,046h,047h,048h,049h
	dw	04Ah,04Bh,04Dh,04Eh,050h,054h,058h
jmptab	dw	case1D,case1F,case29,case2B,case46,case47,case48,case49
	dw	case4A,case4B,case4D,case4E,case50,case54,case58

case58:
	cmp	byte ptr ds:[0856h],0
	jne	case47
	jmp	near ptr tail
case47:
	mov	ax,si
	mov	_key_up_left_held,ax
	mov	_key_up_held,ax
	test	byte ptr [bp-1],80h
	je	case47done
	mov	word ptr _key_up_released,1
case47done:
	jmp	short tail
	nop
case49:
	mov	ax,si
	mov	_key_up_right_held,ax
	mov	_key_up_held,ax
	test	byte ptr [bp-1],80h
	je	case49done
	mov	word ptr _key_up_released,1
case49done:
	jmp	short tail
	nop
case29:
	cmp	byte ptr ds:[0856h],0
	jne	case48
	jmp	short tail
case48:
	mov	_key_up_held,si
	test	byte ptr [bp-1],80h
	je	case48done
	mov	word ptr _key_up_released,1
case48done:
	jmp	short tail
case2B:
	cmp	byte ptr ds:[0856h],0
	jne	case4B
	jmp	short tail
case4B:
	mov	_key_up_left_held,si
	jmp	short tail
case4E:
	cmp	byte ptr ds:[0856h],0
	jne	case4D
	jmp	short tail
case4D:
	mov	_key_up_right_held,si
	jmp	short tail
case4A:
	cmp	byte ptr ds:[0856h],0
	jne	case50
	jmp	short tail
case50:
	mov	_gb6a,si
	jmp	short tail
case46:
case54:
	jmp	short tail
case1F:
	cmp	word ptr ds:[0B74h],0
	je	xordi
	or	si,si
	je	xordi
	call	_sound_effects_toggle
	jmp	short jmptail
xordi:
	xor	di,di
jmptail:
	jmp	short tail
case1D:
	mov	ds:[0B74h],si
nomatch:
	xor	di,di
	jmp	short tail
tail:
	or	di,di
	je	chain
	cmp	word ptr _keyboard_state,0
	je	ack
chain:
	pushf
	call	dword ptr _int9_saved_vector
	jmp	short epilogue
ack:
	in	al,61h
	mov	ah,0
	mov	[bp-4],ax
	mov	al,byte ptr [bp-4]
	or	al,80h
	out	61h,al
	mov	al,byte ptr [bp-4]
	out	61h,al
	mov	al,20h
	out	20h,al
epilogue:
	mov	sp,bp
	pop	bp
	pop	di
	pop	si
	pop	ds
	pop	es
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	iret
_keyboard_irq_handler	ENDP
_TEXT	ENDS
	END
