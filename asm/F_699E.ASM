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
; the master-tick ISR fC1A0's own caller (f6bcf) and 88 from the BIOS chain;
; no callees inside its own 380-byte extent.
;
; Historical reference only. src/F_699E.C now matches the full extent using
; Turbo C interrupt support, native I/O intrinsics, and an ordinary switch.
; Register saves, CS-relative dispatch, and IRET are compiler-generated.
DGROUP	GROUP	_DATA,_BSS
_DATA	SEGMENT WORD PUBLIC 'DATA'
_DATA	ENDS
_BSS	SEGMENT WORD PUBLIC 'BSS'
	EXTRN	_gb68:WORD		; DS:0B68
	EXTRN	_gb6a:WORD		; DS:0B6A
	EXTRN	_gb6c:WORD		; DS:0B6C
	EXTRN	_gb6e:WORD		; DS:0B6E
	EXTRN	_gb70:WORD		; DS:0B70
	EXTRN	_gb72:WORD		; DS:0B72
	EXTRN	_gc0cc:DWORD		; DS:0C0CC .. 0C0CE, the old INT 09h vector chain
_BSS	ENDS

_TEXT	SEGMENT BYTE PUBLIC 'CODE'
	ASSUME	CS:_TEXT, DS:DGROUP
	EXTRN	_f68cf:NEAR
	PUBLIC	_f699e
_f699e	PROC	NEAR
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
	mov	_gb6c,ax
	mov	_gb68,ax
	test	byte ptr [bp-1],80h
	je	case47done
	mov	word ptr _gb70,1
case47done:
	jmp	short tail
	nop
case49:
	mov	ax,si
	mov	_gb6e,ax
	mov	_gb68,ax
	test	byte ptr [bp-1],80h
	je	case49done
	mov	word ptr _gb70,1
case49done:
	jmp	short tail
	nop
case29:
	cmp	byte ptr ds:[0856h],0
	jne	case48
	jmp	short tail
case48:
	mov	_gb68,si
	test	byte ptr [bp-1],80h
	je	case48done
	mov	word ptr _gb70,1
case48done:
	jmp	short tail
case2B:
	cmp	byte ptr ds:[0856h],0
	jne	case4B
	jmp	short tail
case4B:
	mov	_gb6c,si
	jmp	short tail
case4E:
	cmp	byte ptr ds:[0856h],0
	jne	case4D
	jmp	short tail
case4D:
	mov	_gb6e,si
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
	call	_f68cf
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
	cmp	word ptr _gb72,0
	je	ack
chain:
	pushf
	call	dword ptr _gc0cc
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
_f699e	ENDP
_TEXT	ENDS
	END
