; Production source. src/RUNTIME_BLOCK.C is the frozen migration oracle.
JMP_NEAR macro target
 db 0e9h
 dw target-$-2
endm

CALL_NEAR macro target
 db 0e8h
 dw target-$-2
endm

_TEXT segment byte public 'CODE'
_TEXT ends
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT,ds:DGROUP
_TEXT segment byte public 'CODE'
public _f039c
_f039c label near
; RUNTIME_BLOCK -- exact startup/runtime dispatch block.
    db 0e9h,091h,001h
    public _f039f
    _f039f label near
    public _box
    _box label near
    db 0e9h,094h,002h
    public _f03a2
    _f03a2 label near
    public _bar
    _bar label near
    db 0e9h,036h,00fh
    public _f03a5
    _f03a5 label near
    db 0e9h,07ch,00fh
    public _f03a8
    _f03a8 label near
    public _clear
    _clear label near
    db 0e9h,0b2h,00fh
    public _f03ab
    _f03ab label near
    public _fill
    _fill label near
    db 0e9h,026h,010h
    public _f03ae
    _f03ae label near
    db 0e9h,092h,010h
    public _f03b1
    _f03b1 label near
    db 0e9h,0d6h,010h
    public _f03b4
    _f03b4 label near
    public _wipe
    _wipe label near
    db 0e9h,008h,011h
    public _f03b7
    _f03b7 label near
    db 0e9h,07eh,011h
    public _f03ba
    _f03ba label near
    db 0e9h,0d4h,011h
    public _f03bd
    _f03bd label near
    db 0e9h,02ch,012h
    public _f03c0
    _f03c0 label near
    db 0e9h,093h,012h
    public _f03c3
    _f03c3 label near
    db 0e9h,031h,013h
    public _f03c6
    _f03c6 label near
    db 0e9h,0dbh,013h
    public _f03c9
    _f03c9 label near
    public _blit
    _blit label near
    db 0e9h,064h,015h
    public _f03cc
    _f03cc label near
    public _copy
    _copy label near
    db 0e9h,0c9h,016h
    public _f03cf
    _f03cf label near
    db 0e9h,02ah,018h
    public _f03d2
    _f03d2 label near
    db 0e9h,0f5h,018h
    public _f03d5
    _f03d5 label near
    db 0e9h,033h,019h
    db 000h,000h,03dh,005h,000h,000h,0f4h,005h,030h,006h,000h,000h
    db 043h,006h,000h,000h,061h,00bh,0d7h,00bh,0e3h,0ffh,0e3h,0ffh,0d4h,0ffh,0c5h,0ffh,0b6h,0ffh,0a7h,0ffh,098h,0ffh,089h,0ffh
    db 07ah,0ffh,06bh,0ffh,05ch,0ffh,04dh,0ffh,03eh,0ffh,02fh,0ffh,020h,0ffh,011h,0ffh,002h,0ffh,0f3h,0feh,0e4h,0feh,0d5h,0feh
    db 0c6h,0feh,0b7h,0feh,0a8h,0feh,099h,0feh,08ah,0feh,07bh,0feh,06ch,0feh,05dh,0feh,04eh,0feh,03fh,0feh,030h,0feh,021h,0feh
    db 012h,0feh,003h,0feh,0f4h,0fdh,0e5h,0fdh,0d6h,0fdh,0c7h,0fdh,0b8h,0fdh,0a9h,0fdh,09ah,0fdh,08bh,0fdh,07ch,0fdh,06dh,0fdh
    db 05eh,0fdh,04fh,0fdh,040h,0fdh,031h,0fdh,022h,0fdh,013h,0fdh,004h,0fdh,0f5h,0fch,0e6h,0fch,0d7h,0fch,0c8h,0fch,0b9h,0fch
    db 0aah,0fch,09bh,0fch,08ch,0fch,07dh,0fch,06eh,0fch,05fh,0fch,050h,0fch,041h,0fch,032h,0fch,023h,0fch,014h,0fch,005h,0fch
    db 0f6h,0fbh,0e7h,0fbh,0d8h,0fbh,0c9h,0fbh,0bah,0fbh,0abh,0fbh,09ch,0fbh,08dh,0fbh,07eh,0fbh,06fh,0fbh,060h,0fbh,051h,0fbh
    db 042h,0fbh,0f8h,0ffh,0dbh,0ffh,0c6h,0ffh,0b1h,0ffh,09ch,0ffh,087h,0ffh,072h,0ffh,05dh,0ffh,048h,0ffh,033h,0ffh,01eh,0ffh
    db 009h,0ffh,0f4h,0feh,0dfh,0feh,0cah,0feh,0b5h,0feh,0a0h,0feh,08bh,0feh,076h,0feh,061h,0feh,04ch,0feh,037h,0feh,022h,0feh
    db 00dh,0feh,0f8h,0fdh,0e3h,0fdh,0ceh,0fdh,0b9h,0fdh,0a4h,0fdh,08fh,0fdh,07ah,0fdh,065h,0fdh,050h,0fdh,03bh,0fdh,026h,0fdh
    db 011h,0fdh,0fch,0fch,0e7h,0fch,0d2h,0fch,0bdh,0fch,0a8h,0fch,093h,0fch,07eh,0fch,069h,0fch,054h,0fch,03fh,0fch,02ah,0fch
    db 015h,0fch,000h,0fch,0ebh,0fbh,0d6h,0fbh,0c1h,0fbh,0ach,0fbh,097h,0fbh,082h,0fbh,06dh,0fbh,058h,0fbh,043h,0fbh,02eh,0fbh
    db 019h,0fbh,004h,0fbh,0efh,0fah,0dah,0fah,0c5h,0fah,0b0h,0fah,09bh,0fah,086h,0fah,071h,0fah,05ch,0fah,047h,0fah,032h,0fah
    db 01dh,0fah,008h,0fah,0f3h,0f9h,0deh,0f9h,0c9h,0f9h,0b4h,0f9h,09fh,0f9h,08ah,0f9h,075h,0f9h,060h,0f9h
    runtime_dispatch:
    xor bh,bh
    ; mov bl, byte ptr ds:[0BFCDh] (TASM absolute-address encoding).
    db 08ah,01eh,0cdh,0bfh
    shl bx,1
    jmp word ptr cs:[bx+3d8h]
    ; EGA/VGA planar-mode initializer reached from the dispatch table.
    push bp
    mov bp,sp
    db 056h,057h
    push ds
    cld
    mov ax,0eh
    int 10h
    mov ah,5
    mov al,1
    int 10h
    mov ax,1000h
    mov bx,1
    int 10h
    mov ax,1000h
    mov bx,100h
    int 10h
    mov ax,1000h
    mov bx,170eh
    int 10h
    mov ax,1000h
    mov bx,160fh
    int 10h
    mov dx,3ceh
    mov al,5
    out dx,al
    inc dx
    mov al,2
    out dx,al
    mov dx,3c4h
    mov al,2
    out dx,al
    inc dx
    mov al,0fh
    out dx,al
    mov ax,0a000h
    mov es,ax
    xor bx,bx
    mov cx,bx
    ; SI assignment remains a tracked raw recovery candidate.
    db 08bh,0f3h
    mov bp,1000h
    ; The planar clear loop follows. Its raw ES:[SI]/DI operations can
; now be transcribed directly in TASM; no compiler register wrapper exists.
    mov dx,3ceh
    mov al,8
    out dx,al
    inc dx
    mov al,0ch
    out dx,al
    db 026h,08ah,024h,026h,088h,01ch
    mov dx,3ceh
    mov al,8
    out dx,al
    inc dx
    mov al,3
    out dx,al
    db 026h,08ah,024h,026h,088h,00ch
    mov dx,3ceh
    mov al,8
    out dx,al
    inc dx
    mov al,0c0h
    out dx,al
    db 026h,08ah,024h,026h,088h,03ch,046h,0feh,0c7h
    ; SI/DI accesses and their short back-edges remain literal; the nested
; plane/column counters can use ordinary TASM spellings.
    db 080h,0ffh,010h,07ch,0c8h,032h,0ffh
    inc cl
    cmp cl,10h
    db 07ch,0bfh
    ; The original used the byte-register form (32 C9), not XOR CX,CX.
    db 032h,0c9h
    inc bl
    cmp bl,10h
    db 07ch,0b6h
    mov dx,3ceh
    mov al,8
    out dx,al
    inc dx
    mov al,30h
    out dx,al
    mov dx,3ceh
    mov al,3
    out dx,al
    inc dx
    mov al,0
    out dx,al
    pop ds
    db 05fh,05eh
    pop bp
    ret
    ; Restore the EGA palette register set, then provide the mode-13h entry.
    mov ax,9
    int 10h
    mov al,10h
    mov dx,3dah
    out dx,al
    mov al,1
    mov dx,3deh
    out dx,al
    mov al,11h
    mov dx,3dah
    out dx,al
    mov al,0
    mov dx,3deh
    out dx,al
    mov al,1eh
    mov dx,3dah
    out dx,al
    mov al,0fh
    mov dx,3deh
    out dx,al
    mov al,1fh
    mov dx,3dah
    out dx,al
    mov al,0eh
    mov dx,3deh
    out dx,al
    mov dx,3dah
    mov al,0fh
    out dx,al
    ret
    mov ax,13h
    int 10h
    ret
    runtime_dispatch_3e2:
    ; DI is intentionally cleared with its original encoding.
    db 032h,0ffh
    ; mov bl, byte ptr ds:[0BFCDh] (TASM absolute-address encoding).
    db 08ah,01eh,0cdh,0bfh
    shl bx,1
    jmp word ptr cs:[bx+3e2h]
    runtime_3e2_handler:
    push bp
    mov bp,sp
    ; This handler owns its SI/DI preservation explicitly.
    db 056h,057h
    push ds
    cld
    mov bx,[bp+6]
    shl bx,1
    shl bx,1
    db 0c4h,0b7h,024h,039h
    shl bx,1
    shl bx,1
    ; DI-derived row addressing remains a tracked raw recovery candidate.
    db 08bh,0fbh,0d1h,0e7h,0d1h,0e7h,003h,0fbh
    mov ax,[bp+4]
    mov bx,[bp+8]
    mov cx,[bp+0ah]
    add bx,ax
    dec bx
    shr ax,1
    shr ax,1
    shr bx,1
    shr bx,1
    sub bx,ax
    inc bx
    db 003h,0f0h,003h,0f0h,003h,0f8h
    push es
    mov ax,0a000h
    mov es,ax
    ; Address the selected planar page and enter its unrolled blit tail.
    db 081h,0c7h,000h,040h
    mov bp,50h
    sub bp,bx
    mov dx,0a0h
    shl bx,1
    sub dx,bx
    mov ax,cs:[bx+3ech]
    ; TASM 1.0 needs the historical absolute CS-store spelling preserved.
    db 02eh,0a3h,05fh,00bh
    add ax,0b61h
    pop ds
    jmp ax
    db 0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh
    db 0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh
    db 026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh
    db 0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h
    db 0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h
    db 0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh
    db 0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh
    db 026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh
    db 0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h
    db 0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h
    db 0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh
    db 0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh
    db 026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh
    db 0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h
    db 0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h
    db 0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh
    db 0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh
    db 026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh
    db 0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h
    db 0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h
    db 0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh
    db 0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh
    db 026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh
    db 0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h
    db 0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h
    db 0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh
    db 0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh
    db 026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh
    db 0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h
    db 0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h
    db 0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh
    db 0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh
    db 026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh
    db 0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h
    db 0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h
    db 0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh
    db 0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh
    db 026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh
    db 0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h
    db 0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h
    db 0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh
    db 0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh
    db 026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh
    db 0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h
    db 0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h
    db 0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh
    db 0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh
    db 026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh
    db 0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h
    db 0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h
    db 0ebh,0d1h,0ebh,026h,08ah,027h,0aah,003h,0f2h,003h,0fdh,0e2h,005h,01fh,05fh,05eh,05dh,0c3h,0e9h,042h,0fbh,055h,08bh,0ech
    db 056h,057h,01eh,0fch,08bh,05eh,006h,0d1h,0e3h,0d1h,0e3h,0c4h,0b7h,024h,039h,08bh,046h,004h,08bh,0d0h,0d1h,0eah,003h,0f2h
    db 0d1h,0ebh,0d1h,0ebh,086h,0fbh,0d1h,0ebh,0d1h,0ebh,002h,0e3h,032h,0dbh,003h,0c3h,0d1h,0ebh,0d1h,0ebh,003h,0c3h,08bh,056h
    db 008h,0d1h,0e8h,083h,0d2h,000h,08bh,0f8h,006h,0b8h,000h,0b8h,08eh,0c0h,01fh,0d1h,0eah,083h,0d2h,000h,08bh,04eh,00ah,0b8h
    db 0a0h,000h,02bh,0c2h,0bbh,000h,020h,02bh,0dah,0bdh,0a0h,080h,051h,08bh,0cah,0d1h,0e9h,0f3h,0a5h,0d1h,0d1h,0f3h,0a4h,059h
    db 003h,0f0h,003h,0fbh,078h,004h,0e2h,0ech,0ebh,004h,003h,0fdh,0e2h,0e6h,01fh,05fh,05eh,05dh,0c3h,055h,08bh,0ech,056h,057h
    db 01eh,0fch,08bh,05eh,006h,0d1h,0e3h,0d1h,0e3h,0c4h,0b7h,024h,039h,0d1h,0e3h,0d1h,0e3h,0d1h,0e3h,0d1h,0e3h,08bh,0fbh,0d1h
    db 0e3h,0d1h,0e3h,003h,0fbh,08bh,046h,004h,08bh,05eh,008h,003h,0d8h,04bh,0d1h,0e8h,0d1h,0e0h,003h,0f8h,0d1h,0e8h,003h,0f0h
    db 0d1h,0e8h,0d1h,0ebh,0d1h,0ebh,02bh,0d8h,043h,0d1h,0e3h,02eh,08bh,087h,08eh,004h,02eh,0a3h,0d9h,012h,0bah,0a0h,000h,02bh
    db 0d3h,006h,0b9h,000h,0a0h,08eh,0c1h,01fh,08bh,04eh,00ah,0d1h,0e3h,0bdh,040h,001h,02bh,0ebh,005h,0dbh,012h,0ffh,0e0h,0adh
    db 08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh
    db 0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h
    db 0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h
    db 0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h
    db 08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh
    db 08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah
    db 0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh
    db 08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh
    db 0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h
    db 0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h
    db 0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h
    db 08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh
    db 08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah
    db 0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh
    db 08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh
    db 0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h
    db 0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h
    db 0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h
    db 08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh
    db 08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah
    db 0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh
    db 08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh
    db 0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h
    db 0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h
    db 0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h
    db 08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh
    db 08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah
    db 0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh
    db 08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh
    db 0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h
    db 0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h
    db 0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h
    db 08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh
    db 08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah
    db 0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh
    db 08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh
    db 0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h
    db 0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h
    db 0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h
    db 08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh
    db 08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah
    db 0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh
    db 08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh
    db 0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h
    db 0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h
    db 0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h
    db 08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh
    db 08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah
    db 0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh
    db 08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh
    db 0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h
    db 0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h
    db 0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h
    db 08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh
    db 08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah
    db 0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh
    db 08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh
    db 0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h
    db 0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h
    db 0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h
    db 08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh
    db 08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah
    db 0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh
    db 08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh
    db 0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h
    db 0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h
    db 0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h
    db 08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh
    db 08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah
    db 0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,003h
    db 0f2h,003h,0fdh,032h,0edh,0e2h,005h,01fh,05fh,05eh,05dh,0c3h,0e9h,060h,0f9h
    ; F_03A2 / bar: planar horizontal-bar primitive.
    runtime_bar_primitive:
    push bp
    mov bp,sp
    db 057h
    cld
    ; Numeric absolute memory requires an explicit segment in TASM; see ABSOLUTE_MEMORY.
    db 0a1h,0c8h,040h
    mov cx,[bp+8]
    mov bx,[bp+6]
    shl bx,1
    shl bx,1
    ; LES DI selects the row-pointer table; native TASM may expose DI directly.
    les di,dword ptr [bx+3924h]
    mov bx,[bp+4]
    shr bx,1
    jae runtime_f03a2_add_row
    mov dl,al
    and dl,0fh
    and byte ptr es:[bx+di],0f0h
    or byte ptr es:[bx+di],dl
    inc di
    dec cx
    runtime_f03a2_add_row:
    add di,bx
    sub bx,bx
    shr cx,1
    jae runtime_f03a2_fill
    mov bx,1
    runtime_f03a2_fill:
    rep stosb
    cmp bx,0
    je runtime_f03a2_done
    and al,0f0h
    and byte ptr es:[di],0fh
    or byte ptr es:[di],al
    runtime_f03a2_done:
    pop di
    pop bp
    ret
    ; F_03A5: adjacent planar-bar variant with mirrored nibble handling.
    runtime_f03a5:
    push bp
    mov bp,sp
    db 057h
    cld
    db 0a1h,0c8h,040h
    mov cx,[bp+8]
    mov bx,[bp+6]
    shl bx,1
    shl bx,1
    db 0c4h,0bfh,024h,039h
    db 08bh,05eh,004h,0d1h,0ebh,072h,006h,024h,0f0h,0b2h
    db 00fh,0ebh,004h,024h,00fh,0b2h,0f0h,003h,0fbh,0bbh,0a0h,000h,026h,020h,015h,026h,008h,005h,003h,0fbh,0e2h,0f6h,05fh,05dh
    db 0c3h
    ; F_03A8 / clear: planar clear primitive.
    runtime_clear_primitive:
    push bp
    mov bp,sp
    db 057h,056h
    cld
    db 0a1h,0c8h,040h
    ; SI supplies the 160-byte planar stride; raw bytes remain tracked.
    db 0beh,0a0h,000h
    mov bx,[bp+6]
    shl bx,1
    shl bx,1
    db 0c4h,0bfh,024h,039h
    db 08bh,05eh,004h,0d1h,0ebh,073h,01ch,08bh,04eh,00ah,057h,003h,0fbh,0b6h,0f0h,08ah,0d0h,080h,0e2h,00fh,026h,020h,035h,026h
    db 008h,015h,003h,0feh,0e2h,0f6h,05fh,047h,0ffh,04eh,008h,003h,0fbh,02ah,0e4h,08bh,05eh,008h,0d1h,0ebh,073h,002h,0b4h,001h
    db 08bh,056h,00ah,02bh,0f3h,08bh,0cbh,0f3h,0aah,003h,0feh,04ah,075h,0f7h,080h,0fch,000h,074h,019h,08bh,04eh,00ah,02bh,0feh
    db 0b6h,00fh,08ah,0d0h,080h,0e2h,0f0h,0beh,0a0h,000h,026h,020h,035h,026h,008h,015h,02bh,0feh,0e2h,0f6h,05eh,05fh,05dh,0c3h
    ; F_03AB / fill: planar XOR-fill primitive.
    runtime_f03ab:
    push bp
    mov bp,sp
    db 057h,056h
    cld
    mov dx,[bp+0ah]
    db 0beh,0a0h,000h
    mov bx,[bp+6]
    shl bx,1
    shl bx,1
    db 0c4h,0bfh,024h,039h
    db 08bh,05eh,004h,0d1h,0ebh,073h,013h,08bh,0cah,057h,003h,0fbh,0b0h,00fh,026h,030h,005h,003h,0feh,0e2h,0f9h,05fh,047h,0ffh,04eh
    db 008h,003h,0fbh,057h,02ah,0e4h,08bh,04eh,008h,0d1h,0e9h,073h,002h,0b4h,001h,08bh,0d9h,0e3h,011h,0b0h,0ffh,02bh,0f1h,026h
    db 030h,005h,047h,0e2h,0fah,003h,0feh,08bh,0cbh,04ah,075h,0f3h,05fh,003h,0fbh,080h,0fch,000h,074h,00fh,08bh,04eh,00ah,0b0h
    db 0f0h,0beh,0a0h,000h,026h,030h,005h,003h,0feh,0e2h,0f9h,05eh,05fh,05dh,0c3h
    ; F_03AE: planar rectangle-copy primitive.
    runtime_f03ae:
    push bp
    mov bp,sp
    db 057h,056h
    push ds
    cld
    mov ax,[bp+4]
    mov bx,[bp+8]
    add bx,ax
    dec bx
    shr ax,1
    shr bx,1
    sub bx,ax
    inc bx
    mov dx,[bp+0ah]
    mov si,[bp+6]
    shl si,1
    shl si,1
    lds si,dword ptr [si+3924h]
    add si,ax
    les di,dword ptr [bp+0ch]
    mov word ptr es:[di],bx
    mov word ptr es:[di+2],dx
    add di,4
    mov ax,0a0h
    sub ax,bx
    runtime_f03ae_copy_loop:
    mov cx,bx
    rep movsb
    add si,ax
    dec dx
    jne runtime_f03ae_copy_loop
    pop ds
    pop si
    pop di
    pop bp
    ret
    ; F_03B1: copy from a source rectangle descriptor to a planar row.
    runtime_f03b1:
    push bp
    mov bp,sp
    db 057h,056h
    push ds
    cld
    mov ax,[bp+4]
    shr ax,1
    mov di,[bp+6]
    shl di,1
    shl di,1
    les di,dword ptr [di+3924h]
    add di,ax
    lds si,dword ptr [bp+8]
    lodsw
    mov bx,ax
    lodsw
    mov dx,ax
    mov ax,0a0h
    sub ax,bx
    runtime_f03b1_copy_loop:
    mov cx,bx
    rep movsb
    add di,ax
    dec dx
    jne runtime_f03b1_copy_loop
    pop ds
    pop si
    pop di
    pop bp
    ret
    ; F_03B4 / wipe: planar rectangle copy with word-aligned interior.
    runtime_f03b4:
    push bp
    mov bp,sp
    db 057h,056h
    push ds
    cld
    mov bx,[bp+0eh]
    shl bx,1
    shl bx,1
    db 0c4h,0bfh,024h,039h
    mov ax,[bp+0ch]
    shr ax,1
    db 003h,0f8h
    mov bx,[bp+6]
    shl bx,1
    shl bx,1
    lds si,dword ptr [bx+3924h]
    mov ax,[bp+4]
    shr ax,1
    add si,ax
    mov bx,[bp+8]
    shr bx,1
    mov dx,[bp+0ah]
    mov ax,0a0h
    sub ax,bx
    runtime_f03b4_copy_loop:
    mov cx,bx
    shr cx,1
    rep movsw
    rcl cx,1
    rep movsb
    add si,ax
    add di,ax
    dec dx
    jne runtime_f03b4_copy_loop
    pop ds
    cmp word ptr ds:[0bch],1
    jne runtime_f03b4_done
    mov bx,[bp+0eh]
    cmp bx,0c8h
    jae runtime_f03b4_done
    les di,dword ptr ds:[40c4h]
    mov ax,[bp+0ch]
    shr ax,1
    mov ah,bl
    stosw
    mov ax,[bp+8]
    shr ax,1
    mov bx,[bp+0ah]
    mov ah,bl
    stosw
    mov word ptr ds:[40c4h],di
    runtime_f03b4_done:
    pop si
    pop di
    pop bp
    ret
    ; F_03B7: reverse-direction planar rectangle copy.
    runtime_f03b7:
    push bp
    mov bp,sp
    db 057h,056h
    push ds
    cld
    mov bx,[bp+0eh]
    shl bx,1
    shl bx,1
    db 0c4h,0bfh,024h,039h
    mov ax,[bp+0ch]
    shr ax,1
    db 003h,0f8h
    mov bx,[bp+6]
    shl bx,1
    shl bx,1
    db 0c5h,0b7h,024h,039h
    mov ax,[bp+4]
    shr ax,1
    db 003h,0f0h
    mov bx,[bp+8]
    shr bx,1
    mov dx,[bp+0ah]
    mov ax,dx
    dec ax
    xchg ah,al
    shr ax,1
    add di,ax
    shr ax,1
    shr ax,1
    add di,ax
    mov ax,0a0h
    runtime_f03b7_copy_loop:
    mov cx,bx
    rep movsb
    add si,ax
    sub si,bx
    sub di,ax
    sub di,bx
    dec dx
    jne runtime_f03b7_copy_loop
    pop ds
    pop si
    pop di
    pop bp
    ret
    ; F_03BA: reverse-direction planar nibble transform.
    runtime_f03ba:
    push bp
    mov bp,sp
    db 057h,056h
    push ds
    cld
    mov bx,[bp+0eh]
    shl bx,1
    shl bx,1
    db 0c4h,0bfh,024h,039h
    mov ax,[bp+0ch]
    shr ax,1
    db 003h,0f8h
    mov bx,[bp+6]
    shl bx,1
    shl bx,1
    db 0c5h,0b7h,024h,039h
    mov ax,[bp+4]
    shr ax,1
    db 003h,0f0h
    mov bx,[bp+8]
    shr bx,1
    mov dx,[bp+0ah]
    db 003h,0fbh,04fh,08bh,0cbh,0ach,0d0h,0c0h
    db 0d0h,0c0h,0d0h,0c0h,0d0h,0c0h,026h,088h,005h,04fh,0e2h,0f1h,081h,0c6h,0a0h,000h,02bh,0f3h,081h,0c7h,0a0h,000h,003h,0fbh
    db 04ah,075h,0e0h,01fh,05eh,05fh,05dh,0c3h
    ; F_03BD: mirrored reverse-direction planar nibble transform.
    runtime_f03bd:
    push bp
    mov bp,sp
    db 057h,056h
    push ds
    cld
    mov bx,[bp+0eh]
    shl bx,1
    shl bx,1
    db 0c4h,0bfh,024h,039h
    mov ax,[bp+0ch]
    shr ax,1
    db 003h,0f8h
    mov bx,[bp+6]
    shl bx,1
    shl bx,1
    db 0c5h,0b7h,024h,039h
    mov ax,[bp+4]
    shr ax,1
    db 003h,0f0h
    mov bx,[bp+8]
    shr bx,1
    mov dx,[bp+0ah]
    mov ax,dx
    dec ax
    xchg ah,al
    shr ax,1
    add di,ax
    shr ax,1
    shr ax,1
    add di,ax
    add di,bx
    dec di
    runtime_f03bd_outer_loop:
    mov cx,bx
    runtime_f03bd_inner_loop:
    lodsb
    rol al,1
    rol al,1
    rol al,1
    rol al,1
    mov byte ptr es:[di],al
    dec di
    loop runtime_f03bd_inner_loop
    add si,0a0h
    sub si,bx
    sub di,0a0h
    add di,bx
    dec dx
    jne runtime_f03bd_outer_loop
    pop ds
    pop si
    pop di
    pop bp
    ret
    ; F_03C0: dual-planar reflected pixel transform.
    runtime_f03c0:
    push bp
    mov bp,sp
    db 057h,056h
    push ds
    cld
    mov bx,[bp+0eh]
    shl bx,1
    shl bx,1
    db 0c4h,0bfh,024h,039h
    mov ax,[bp+0ch]
    shr ax,1
    db 003h,0f8h
    mov bx,[bp+6]
    shl bx,1
    shl bx,1
    db 0c5h,0b7h,024h,039h
    mov ax,[bp+4]
    shr ax,1
    db 003h,0f0h
    mov bx,[bp+8]
    shr bx,1
    mov dx,[bp+0ah]
    shr dx,1
    add di,dx
    dec di
    runtime_f03c0_outer_loop:
    push di
    mov cx,bx
    runtime_f03c0_low_loop:
    lodsb
    mov ah,al
    and ax,0f00fh
    shr ah,1
    shr ah,1
    shr ah,1
    shr ah,1
    and byte ptr es:[di],0f0h
    or byte ptr es:[di],ah
    add di,0a0h
    and byte ptr es:[di],0f0h
    or byte ptr es:[di],al
    add di,0a0h
    loop runtime_f03c0_low_loop
    add si,0a0h
    sub si,bx
    pop di
    push di
    mov cx,bx
    runtime_f03c0_high_loop:
    lodsb
    mov ah,al
    and ax,0f00fh
    shl al,1
    shl al,1
    shl al,1
    shl al,1
    and byte ptr es:[di],0fh
    or byte ptr es:[di],ah
    add di,0a0h
    and byte ptr es:[di],0fh
    or byte ptr es:[di],al
    add di,0a0h
    loop runtime_f03c0_high_loop
    add si,0a0h
    sub si,bx
    pop di
    dec di
    dec dx
    jne runtime_f03c0_outer_loop
    pop ds
    pop si
    pop di
    pop bp
    ret
    ; F_03C3: mirrored dual-planar reflected pixel transform.
    runtime_f03c3:
    push bp
    mov bp,sp
    db 057h,056h
    push ds
    cld
    mov bx,[bp+0eh]
    shl bx,1
    shl bx,1
    db 0c4h,0bfh,024h,039h
    mov ax,[bp+0ch]
    shr ax,1
    db 003h,0f8h
    mov bx,[bp+6]
    shl bx,1
    shl bx,1
    db 0c5h,0b7h,024h,039h
    mov ax,[bp+4]
    shr ax,1
    db 003h,0f0h
    mov bx,[bp+8]
    mov ax,bx
    shr bx,1
    mov dx,[bp+0ah]
    shr dx,1
    dec ax
    xchg ah,al
    shr ax,1
    add di,ax
    shr ax,1
    shr ax,1
    add di,ax
    push di
    mov cx,bx
    runtime_f03c3_low_loop:
    lodsb
    mov ah,al
    and ax,0f00fh
    shl al,1
    shl al,1
    shl al,1
    shl al,1
    and byte ptr es:[di],0fh
    or byte ptr es:[di],ah
    sub di,0a0h
    and byte ptr es:[di],0fh
    or byte ptr es:[di],al
    sub di,0a0h
    loop runtime_f03c3_low_loop
    add si,0a0h
    sub si,bx
    pop di
    push di
    mov cx,bx
    runtime_f03c3_high_loop:
    lodsb
    mov ah,al
    and ax,0f00fh
    shr ah,1
    shr ah,1
    shr ah,1
    shr ah,1
    and byte ptr es:[di],0f0h
    or byte ptr es:[di],ah
    sub di,0a0h
    and byte ptr es:[di],0f0h
    or byte ptr es:[di],al
    sub di,0a0h
    loop runtime_f03c3_high_loop
    db 081h,0c6h,0a0h,000h,02bh,0f3h,05fh,047h,04ah,075h,09ch,01fh,05eh,05fh,05dh,0c3h
    ; F_03C6: state-driven planar compositing primitive.
    runtime_f03c6:
    push bp
    mov bp,sp
    sub sp,4
    db 057h,056h
    push ds
    cld
    ; Far state base at C0E0h, with the historical no-fixup encoding.
    db 0a1h,0e0h,0c0h
    mov es,ax
    mov bx,[bp+8]
    mov di, word ptr ds:[0c0e4h]
    mov cl, byte ptr es:[bx + di]
    sub ch, ch
    mov di, word ptr ds:[0c0e6h]
    mov al, byte ptr es:[bx + di]
    mov di, word ptr ds:[0c0e2h]
    mov ah, byte ptr es:[bx + di]
    mov si, word ptr ds:[0c0deh]
    add si, ax
    mov dx, word ptr ds:[0c0e8h]
    mov ax, word ptr ds:[040c8h]
    mov ah, al
    mov bx, word ptr [bp + 6]
    shl bx, 1
    shl bx, 1
    lds di, dword ptr [bx + 03924h]
    mov word ptr [bp - 2], 0
    mov bx, word ptr [bp + 4]
    shr bx, 1
    jae rb_145c
    mov word ptr [bp - 2], 1
rb_145c:
    add di, bx
    mov bx, cx
    shl bx, 1
    inc cx
    shr cx, 1
    mov word ptr [bp - 4], 0a0h
    sub word ptr [bp - 4], cx
    mov cl, dl
    mov dh, al
    mov dl, dh
    and dx, 0f00fh
    cmp word ptr [bp - 2], 0
    jne _f039c+014f2h
    dec di
    push bx
    sub ah, ah
    mov al, byte ptr es:[si]
    inc si
    cmp bx, 010h
    db 07dh,00ah,08ah,0e3h,080h,0e4h,002h,02eh,0ffh,0a7h,00ch,019h,047h,0d0h,0e0h,073h,005h,080h,025h,00fh,008h,035h,0d0h,0e0h
    db 073h,005h,080h,025h,0f0h,008h,015h,047h,0d0h,0e0h,073h,005h,080h,025h,00fh,008h,035h,0d0h,0e0h,073h,005h,080h,025h,0f0h
    db 008h,015h,047h,0d0h,0e0h,073h,005h,080h,025h,00fh,008h,035h,0d0h,0e0h,073h,005h,080h,025h,0f0h,008h,015h,047h,0d0h,0e0h
    db 073h,005h,080h,025h,00fh,008h,035h,080h,0fch,000h,075h,00eh,0d0h,0e0h,073h,005h,080h,025h,0f0h,008h,015h,083h,0ebh,010h
    db 07fh,097h,05bh,003h,07eh,0fch,0e2h,08eh,0ebh,072h,053h,02ah,0e4h,026h,08ah,004h,046h,083h,0fbh,010h,07dh,00ah,08ah,0e3h
    db 080h,0e4h,002h,02eh,0ffh,0a7h,01eh,019h,0d0h,0e0h,073h,005h,080h,025h,0f0h,008h,015h,047h,0d0h,0e0h,073h,005h,080h,025h
    db 00fh,008h,035h,0d0h,0e0h,073h,005h,080h,025h,0f0h,008h,015h,047h,0d0h,0e0h,073h,005h,080h,025h,00fh,008h,035h,0d0h,0e0h
    db 073h,005h,080h,025h,0f0h,008h,015h,047h,0d0h,0e0h,073h,005h,080h,025h,00fh,008h,035h,0d0h,0e0h,073h,005h,080h,025h,0f0h
    db 008h,015h,047h,080h,0fch,000h,075h,00eh,0d0h,0e0h,073h,005h,080h,025h,00fh,008h,035h,083h,0ebh,010h,07fh,097h,05bh,003h
    db 07eh,0fch,0e2h,08eh,08bh,0c3h,0d1h,0e8h,01fh,05eh,05fh,083h,0c4h,004h,05dh,0c3h,081h,018h,069h,018h,069h,018h,056h,018h
    db 056h,018h,043h,018h,043h,018h,030h,018h,030h,018h,0f5h,018h,0ddh,018h,0ddh,018h,0cah,018h,0cah,018h,0b7h,018h,0b7h,018h
    db 0a4h,018h,0a4h,018h
    ; F_03C9 / blit: planar bitmap blit from a 32-byte source header.
    runtime_blit_primitive:
    push bp
    mov bp,sp
    db 057h,056h
    push ds
    cld
    mov bx,[bp+6]
    shl bx,1
    shl bx,1
    db 0c4h,0bfh,024h,039h
    mov bx,[bp+4]
    shr bx,1
    db 003h,0fbh
    db 0c5h,076h,008h,083h,0c6h,020h,0adh,02ah,0ffh,08ah,0d8h,08ah,0f7h,08ah,0d4h,052h,0b8h,0a0h,000h
    db 02bh,0c3h,08bh,0cbh,0d1h,0e9h,0f3h,0a5h,0d1h,0d1h,0f3h,0a4h,003h,0f8h,04ah,075h,0f1h,05ah,01fh,083h,03eh,0bch,000h,001h
    db 075h,01eh,08bh,04eh,006h,081h,0f9h,0c8h,000h,073h,015h,0c4h,03eh,0c4h,040h,08bh,046h,004h,0d1h,0e8h,08ah,0e1h,0abh,08ah
    db 0c3h,08ah,0e2h,0abh,089h,03eh,0c4h,040h,05eh,05fh,05dh,0c3h,0ffh,0f0h,0f0h,0f0h,0f0h,0f0h,0f0h,0f0h,0f0h,0f0h,0f0h,0f0h
    db 0f0h,0f0h,0f0h,0f0h,00fh,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,00fh,000h,000h,000h
    db 000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,00fh,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h
    db 000h,000h,000h,000h,00fh,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,00fh,000h,000h,000h
    db 000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,00fh,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h
    db 000h,000h,000h,000h,00fh,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,00fh,000h,000h,000h
    db 000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,00fh,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h
    db 000h,000h,000h,000h,00fh,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,00fh,000h,000h,000h
    db 000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,00fh,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h
    db 000h,000h,000h,000h,00fh,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,00fh,000h,000h,000h
    db 000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,00fh,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h
    db 000h,000h,000h,000h
    ; F_03CC / copy: clipped source-rectangle copy primitive.
    runtime_copy_primitive:
    push bp
    mov bp,sp
    db 056h,057h
    push ds
    push bp
    les si,dword ptr [bp+8]
    add si,20h
    mov ax,word ptr es:[si]
    add si,2
    xor dx,dx
    mov cx,dx
    mov cl,al
    mov dl,ah
    mov di,[bp+6]
    mov ax,word ptr ds:[96h]
    sub ax,di
    js $+48h
    inc ax
    cmp dx,ax
    jbe $+4
    mov dx,ax
    mov ax,word ptr ds:[94h]
    sub ax,di
    jle $+0ch
    sub dx,ax
    jbe $+36h
    add di,ax
    mul cl
    add si,ax
    mov bx,[bp+4]
    cmp word ptr [bp+0ch],0
    db 074h,003h,0e9h,082h,000h,0d1h,0fbh,0fch,033h,0edh,0a1h,09ah,000h,02bh,0c3h,078h,016h,040h,03bh,0c8h,076h,006h,08bh,0e9h
    sub bp,ax
    mov cx,ax
    mov ax,word ptr ds:[98h]
    sub ax,bx
    jle runtime_copy_skip_forward
    sub cx,ax
    ja runtime_copy_return
    pop bp
    pop ds
    pop di
    pop si
    cld
    pop bp
    ret
    runtime_copy_return:
    add bx,ax
    add bp,ax
    add si,ax
    runtime_copy_skip_forward:
    push es
    push di
    cmp word ptr ds:[0bch],1
    jne runtime_copy_done
    cmp di,0c8h
    jae runtime_copy_done
    mov ax,di
    mov ah,al
    mov al,bl
    les di,dword ptr ds:[40c4h]
    stosw
    mov al,cl
    mov ah,dl
    stosw
    mov word ptr ds:[40c4h],di
    runtime_copy_done:
    pop di
    shl di,1
    shl di,1
    les di,dword ptr [di+3924h]
    db 003h,0fbh,01fh,0bbh,098h,019h,057h,051h,0ach,08ah,0e0h,02eh,0d7h,026h,022h,005h,00ah,0c4h,0aah,0e2h,0f3h,059h,05fh,081h
    db 0c7h,0a0h,000h,003h,0f5h,04ah,075h,0e6h,05dh,01fh,05fh,05eh,0fch,05dh,0c3h,0fdh,033h,0edh,003h,0d9h,003h,0d9h,0d1h,0ebh
    db 04bh,08bh,0c3h,02bh,006h,098h,000h,078h,017h,040h,03bh,0c8h,076h,006h,08bh,0e9h,02bh,0e8h,08bh,0c8h,08bh,0c3h,02bh,006h
    db 09ah,000h,076h,011h,02bh,0c8h,077h,007h,05dh,01fh,05fh,05eh,0fch,05dh,0c3h,02bh,0d8h,003h,0e8h,003h,0f0h,006h,057h,083h
    db 03eh,0bch,000h,001h,075h,020h,081h,0ffh,0c8h,000h,073h,01ah,08bh,0c7h,08ah,0e0h,08ah,0c3h,02ah,0c1h,0feh,0c0h,0c4h,03eh
    db 0c4h,040h,0fch,0abh,08ah,0c1h,08ah,0e2h,0abh,0fdh,089h,03eh,0c4h,040h,05fh,0d1h,0e7h,0d1h,0e7h,0c4h,0bdh,024h,039h,003h
    db 0fbh,01fh,0bbh,098h,019h,057h,051h,08ah,004h,046h,0d0h,0c8h,0d0h,0c8h,0d0h,0c8h,0d0h,0c8h,08ah,0e0h,02eh,0d7h,026h,022h
    db 005h,00ah,0c4h,0aah,0e2h,0e9h,059h,05fh,081h,0c7h,0a0h,000h,003h,0f5h,04ah,075h,0dch,05dh,01fh,05fh,05eh,0fch,05dh,0c3h
    ; F_03CF: state-keyed planar pixel compositor. Remaining raw SI/DI
; operations are ordinary native TASM recovery candidates.
    runtime_f03cf:
    push bp
    mov bp,sp
    sub sp,2
    db 056h,057h
    push ds
    cld
    db 08bh,016h,0c8h,040h
    mov dh,dl
    and dx,0f00fh
    ; Source and destination descriptors are both indexed by plane.
    db 0c4h,076h,008h
    mov bx,[bp+6]
    shl bx,1
    shl bx,1
    db 0c5h,0bfh,024h,039h
    mov bx,[bp+4]
    shr bx,1
    db 003h,0fbh
    xor bx,bx
    mov cx,bx
    db 026h,08ah,01ch
    db 046h
    mov word ptr [bp-2],0a0h
    sub [bp-2],bx
    shl bx,1
    shl bx,1
    db 026h,08ah,00ch,046h,04fh
    push bx
    sub ah,ah
    db 026h,08ah,004h
    db 046h,083h,0fbh,010h,07dh,00ah,08ah,0e3h,080h,0e4h,002h,02eh,0ffh,0a7h,00ch,019h,047h,0d0h,0e0h,073h,005h,080h,025h,00fh
    db 008h,035h,0d0h,0e0h,073h,005h,080h,025h,0f0h,008h,015h,047h,0d0h,0e0h,073h,005h,080h,025h,00fh,008h,035h,0d0h,0e0h,073h
    db 005h,080h,025h,0f0h,008h,015h,047h,0d0h,0e0h,073h,005h,080h,025h,00fh,008h,035h,0d0h,0e0h,073h,005h,080h,025h,0f0h,008h
    db 015h,047h,0d0h,0e0h,073h,005h,080h,025h,00fh,008h,035h,080h,0fch,000h,075h,00eh,0d0h,0e0h,073h,005h,080h,025h,0f0h,008h
    db 015h,083h,0ebh,010h,07fh,097h,05bh,003h,07eh,0feh,0e2h,08eh,01fh,05fh,05eh,083h,0c4h,002h,05dh,0c3h,0a5h,01ch,08dh,01ch
    db 08dh,01ch,07ah,01ch,07ah,01ch,067h,01ch,067h,01ch,054h,01ch,054h,01ch

    ; F_03D2: merge a nibble from a planar pixel with the current state.
    runtime_f03d2:
    push bp
    mov bp,sp
    db 057h
    mov bx,[bp+6]
    shl bx,1
    shl bx,1
    db 0c4h,0bfh,024h,039h
    mov bx,[bp+4]
    shr bx,1
    db 072h,015h
    db 003h,0fbh,026h,08ah,005h
    and al,0fh
    db 08bh,01eh,0c8h,040h
    shl bl,1
    shl bl,1
    shl bl,1
    shl bl,1
    db 0ebh,00eh
    db 003h,0fbh,026h,08ah,005h
    and al,0f0h
    db 08bh,01eh,0c8h,040h
    and bl,0fh
    or al,bl
    db 026h,088h,005h
    db 05fh
    pop bp
    ret
    ; F_03D5: fetch either half of a planar pixel as a normalized nibble.
    runtime_f03d5:
    push bp
    mov bp,sp
    db 057h
    mov bx,[bp+6]
    shl bx,1
    shl bx,1
    db 0c4h,0bfh,024h,039h
    mov bx,[bp+4]
    shr bx,1
    db 072h,012h
    db 003h,0fbh,026h,08ah,005h
    and ax,0f0h
    shr ax,1
    shr ax,1
    shr ax,1
    shr ax,1
    db 0ebh,008h
    db 003h,0fbh,026h,08ah,005h
    and ax,0fh
    db 05fh
    pop bp
    ret

    ; Unreferenced stub and driver name following the final code root.
    db 0c3h,045h,047h,041h,02eh,044h,052h,056h,000h
    _runtime_block_end label byte
    public _runtime_block_end
_TEXT ends
end
