/* RUNTIME_BLOCK -- exact startup/runtime dispatch block. */
void f039c()
{
    asm db 0e9h,091h,001h
    asm public _f039f
    asm _f039f label near
    asm public _box
    asm _box label near
    asm db 0e9h,094h,002h
    asm public _f03a2
    asm _f03a2 label near
    asm public _bar
    asm _bar label near
    asm db 0e9h,036h,00fh
    asm public _f03a5
    asm _f03a5 label near
    asm db 0e9h,07ch,00fh
    asm public _f03a8
    asm _f03a8 label near
    asm public _clear
    asm _clear label near
    asm db 0e9h,0b2h,00fh
    asm public _f03ab
    asm _f03ab label near
    asm public _fill
    asm _fill label near
    asm db 0e9h,026h,010h
    asm public _f03ae
    asm _f03ae label near
    asm db 0e9h,092h,010h
    asm public _f03b1
    asm _f03b1 label near
    asm db 0e9h,0d6h,010h
    asm public _f03b4
    asm _f03b4 label near
    asm public _wipe
    asm _wipe label near
    asm db 0e9h,008h,011h
    asm public _f03b7
    asm _f03b7 label near
    asm db 0e9h,07eh,011h
    asm public _f03ba
    asm _f03ba label near
    asm db 0e9h,0d4h,011h
    asm public _f03bd
    asm _f03bd label near
    asm db 0e9h,02ch,012h
    asm public _f03c0
    asm _f03c0 label near
    asm db 0e9h,093h,012h
    asm public _f03c3
    asm _f03c3 label near
    asm db 0e9h,031h,013h
    asm public _f03c6
    asm _f03c6 label near
    asm db 0e9h,0dbh,013h
    asm public _f03c9
    asm _f03c9 label near
    asm public _blit
    asm _blit label near
    asm db 0e9h,064h,015h
    asm public _f03cc
    asm _f03cc label near
    asm public _copy
    asm _copy label near
    asm db 0e9h,0c9h,016h
    asm public _f03cf
    asm _f03cf label near
    asm db 0e9h,02ah,018h
    asm public _f03d2
    asm _f03d2 label near
    asm db 0e9h,0f5h,018h
    asm public _f03d5
    asm _f03d5 label near
    asm db 0e9h,033h,019h
    asm db 000h,000h,03dh,005h,000h,000h,0f4h,005h,030h,006h,000h,000h
    asm db 043h,006h,000h,000h,061h,00bh,0d7h,00bh,0e3h,0ffh,0e3h,0ffh,0d4h,0ffh,0c5h,0ffh,0b6h,0ffh,0a7h,0ffh,098h,0ffh,089h,0ffh
    asm db 07ah,0ffh,06bh,0ffh,05ch,0ffh,04dh,0ffh,03eh,0ffh,02fh,0ffh,020h,0ffh,011h,0ffh,002h,0ffh,0f3h,0feh,0e4h,0feh,0d5h,0feh
    asm db 0c6h,0feh,0b7h,0feh,0a8h,0feh,099h,0feh,08ah,0feh,07bh,0feh,06ch,0feh,05dh,0feh,04eh,0feh,03fh,0feh,030h,0feh,021h,0feh
    asm db 012h,0feh,003h,0feh,0f4h,0fdh,0e5h,0fdh,0d6h,0fdh,0c7h,0fdh,0b8h,0fdh,0a9h,0fdh,09ah,0fdh,08bh,0fdh,07ch,0fdh,06dh,0fdh
    asm db 05eh,0fdh,04fh,0fdh,040h,0fdh,031h,0fdh,022h,0fdh,013h,0fdh,004h,0fdh,0f5h,0fch,0e6h,0fch,0d7h,0fch,0c8h,0fch,0b9h,0fch
    asm db 0aah,0fch,09bh,0fch,08ch,0fch,07dh,0fch,06eh,0fch,05fh,0fch,050h,0fch,041h,0fch,032h,0fch,023h,0fch,014h,0fch,005h,0fch
    asm db 0f6h,0fbh,0e7h,0fbh,0d8h,0fbh,0c9h,0fbh,0bah,0fbh,0abh,0fbh,09ch,0fbh,08dh,0fbh,07eh,0fbh,06fh,0fbh,060h,0fbh,051h,0fbh
    asm db 042h,0fbh,0f8h,0ffh,0dbh,0ffh,0c6h,0ffh,0b1h,0ffh,09ch,0ffh,087h,0ffh,072h,0ffh,05dh,0ffh,048h,0ffh,033h,0ffh,01eh,0ffh
    asm db 009h,0ffh,0f4h,0feh,0dfh,0feh,0cah,0feh,0b5h,0feh,0a0h,0feh,08bh,0feh,076h,0feh,061h,0feh,04ch,0feh,037h,0feh,022h,0feh
    asm db 00dh,0feh,0f8h,0fdh,0e3h,0fdh,0ceh,0fdh,0b9h,0fdh,0a4h,0fdh,08fh,0fdh,07ah,0fdh,065h,0fdh,050h,0fdh,03bh,0fdh,026h,0fdh
    asm db 011h,0fdh,0fch,0fch,0e7h,0fch,0d2h,0fch,0bdh,0fch,0a8h,0fch,093h,0fch,07eh,0fch,069h,0fch,054h,0fch,03fh,0fch,02ah,0fch
    asm db 015h,0fch,000h,0fch,0ebh,0fbh,0d6h,0fbh,0c1h,0fbh,0ach,0fbh,097h,0fbh,082h,0fbh,06dh,0fbh,058h,0fbh,043h,0fbh,02eh,0fbh
    asm db 019h,0fbh,004h,0fbh,0efh,0fah,0dah,0fah,0c5h,0fah,0b0h,0fah,09bh,0fah,086h,0fah,071h,0fah,05ch,0fah,047h,0fah,032h,0fah
    asm db 01dh,0fah,008h,0fah,0f3h,0f9h,0deh,0f9h,0c9h,0f9h,0b4h,0f9h,09fh,0f9h,08ah,0f9h,075h,0f9h,060h,0f9h
    asm runtime_dispatch:
    asm xor bh,bh
    /* mov bl, byte ptr ds:[0BFCDh] (TASM absolute-address encoding). */
    asm db 08ah,01eh,0cdh,0bfh
    asm shl bx,1
    asm jmp word ptr cs:[bx+3d8h]
    /* EGA/VGA planar-mode initializer reached from the dispatch table. */
    asm push bp
    asm mov bp,sp
    asm db 056h,057h
    asm push ds
    asm cld
    asm mov ax,0eh
    asm int 10h
    asm mov ah,5
    asm mov al,1
    asm int 10h
    asm mov ax,1000h
    asm mov bx,1
    asm int 10h
    asm mov ax,1000h
    asm mov bx,100h
    asm int 10h
    asm mov ax,1000h
    asm mov bx,170eh
    asm int 10h
    asm mov ax,1000h
    asm mov bx,160fh
    asm int 10h
    asm mov dx,3ceh
    asm mov al,5
    asm out dx,al
    asm inc dx
    asm mov al,2
    asm out dx,al
    asm mov dx,3c4h
    asm mov al,2
    asm out dx,al
    asm inc dx
    asm mov al,0fh
    asm out dx,al
    asm mov ax,0a000h
    asm mov es,ax
    asm xor bx,bx
    asm mov cx,bx
    /* Preserve the historical SI assignment without asking TC to manage SI. */
    asm db 08bh,0f3h
    asm mov bp,1000h
    /* The planar clear loop follows.  Its ES:[SI]/DI operations need a
       separate TC-safe conversion, because exposing them makes TC insert
       its own SI/DI preservation ahead of the raw dispatch veneer. */
    asm db 0bah,0ceh,003h,0b0h,008h,0eeh,042h,0b0h,00ch,0eeh,026h,08ah,024h,026h,088h,01ch,0bah,0ceh,003h,0b0h,008h,0eeh,042h,0b0h,003h,0eeh,026h
    asm db 08ah,024h,026h,088h,00ch,0bah,0ceh,003h,0b0h,008h,0eeh,042h,0b0h,0c0h,0eeh,026h,08ah,024h,026h,088h,03ch,046h,0feh,0c7h
    asm db 080h,0ffh,010h,07ch,0c8h,032h,0ffh,0feh,0c1h,080h,0f9h,010h,07ch,0bfh,032h,0c9h,0feh,0c3h,080h,0fbh,010h,07ch,0b6h,0bah
    asm db 0ceh,003h,0b0h,008h,0eeh,042h,0b0h,030h,0eeh,0bah,0ceh,003h,0b0h,003h,0eeh,042h,0b0h,000h,0eeh,01fh,05fh,05eh,05dh,0c3h
    /* Restore the EGA palette register set, then provide the mode-13h entry. */
    asm mov ax,9
    asm int 10h
    asm mov al,10h
    asm mov dx,3dah
    asm out dx,al
    asm mov al,1
    asm mov dx,3deh
    asm out dx,al
    asm mov al,11h
    asm mov dx,3dah
    asm out dx,al
    asm mov al,0
    asm mov dx,3deh
    asm out dx,al
    asm mov al,1eh
    asm mov dx,3dah
    asm out dx,al
    asm mov al,0fh
    asm mov dx,3deh
    asm out dx,al
    asm mov al,1fh
    asm mov dx,3dah
    asm out dx,al
    asm mov al,0eh
    asm mov dx,3deh
    asm out dx,al
    asm mov dx,3dah
    asm mov al,0fh
    asm out dx,al
    asm ret
    asm mov ax,13h
    asm int 10h
    asm ret
    asm runtime_dispatch_3e2:
    /* DI is intentionally cleared with its original encoding. */
    asm db 032h,0ffh
    /* mov bl, byte ptr ds:[0BFCDh] (TASM absolute-address encoding). */
    asm db 08ah,01eh,0cdh,0bfh
    asm shl bx,1
    asm jmp word ptr cs:[bx+3e2h]
    asm runtime_3e2_handler:
    asm push bp
    asm mov bp,sp
    /* TC must not infer SI/DI ownership from this manually framed handler. */
    asm db 056h,057h
    asm push ds
    asm cld
    asm mov bx,[bp+6]
    asm shl bx,1
    asm shl bx,1
    asm db 0c4h,0b7h,024h,039h
    asm db 0d1h,0e3h,0d1h,0e3h,08bh,0fbh,0d1h,0e7h,0d1h,0e7h,003h,0fbh,08bh,046h,004h,08bh,05eh,008h,08bh,04eh,00ah,003h,0d8h
    asm db 04bh,0d1h,0e8h,0d1h,0e8h,0d1h,0ebh,0d1h,0ebh,02bh,0d8h,043h,003h,0f0h,003h,0f0h,003h,0f8h,006h,0b8h,000h,0a0h,08eh,0c0h
    asm db 081h,0c7h,000h,040h,0bdh,050h,000h,02bh,0ebh,0bah,0a0h,000h,0d1h,0e3h,02bh,0d3h,02eh,08bh,087h,0ech,003h,02eh,0a3h,05fh
    asm db 00bh,005h,061h,00bh,01fh,0ffh,0e0h,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh
    asm db 0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh
    asm db 026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh
    asm db 0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h
    asm db 0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h
    asm db 0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh
    asm db 0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh
    asm db 026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh
    asm db 0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h
    asm db 0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h
    asm db 0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh
    asm db 0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh
    asm db 026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh
    asm db 0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h
    asm db 0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h
    asm db 0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh
    asm db 0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh
    asm db 026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh
    asm db 0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h
    asm db 0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h
    asm db 0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh
    asm db 0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh
    asm db 026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh
    asm db 0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h
    asm db 0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h
    asm db 0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh
    asm db 0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh
    asm db 026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh
    asm db 0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h
    asm db 0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h
    asm db 0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh
    asm db 0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh
    asm db 026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh
    asm db 0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h
    asm db 0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h
    asm db 0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh
    asm db 0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh
    asm db 026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh
    asm db 0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h
    asm db 0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h
    asm db 0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh
    asm db 0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh
    asm db 026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh
    asm db 0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h
    asm db 0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h
    asm db 0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh
    asm db 0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh
    asm db 026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh
    asm db 0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h
    asm db 0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,0d1h,0ebh,026h,08ah,027h,0aah,0adh,08bh,0d8h,0d1h,0ebh,0d1h,0ebh,0d1h
    asm db 0ebh,0d1h,0ebh,026h,08ah,027h,0aah,003h,0f2h,003h,0fdh,0e2h,005h,01fh,05fh,05eh,05dh,0c3h,0e9h,042h,0fbh,055h,08bh,0ech
    asm db 056h,057h,01eh,0fch,08bh,05eh,006h,0d1h,0e3h,0d1h,0e3h,0c4h,0b7h,024h,039h,08bh,046h,004h,08bh,0d0h,0d1h,0eah,003h,0f2h
    asm db 0d1h,0ebh,0d1h,0ebh,086h,0fbh,0d1h,0ebh,0d1h,0ebh,002h,0e3h,032h,0dbh,003h,0c3h,0d1h,0ebh,0d1h,0ebh,003h,0c3h,08bh,056h
    asm db 008h,0d1h,0e8h,083h,0d2h,000h,08bh,0f8h,006h,0b8h,000h,0b8h,08eh,0c0h,01fh,0d1h,0eah,083h,0d2h,000h,08bh,04eh,00ah,0b8h
    asm db 0a0h,000h,02bh,0c2h,0bbh,000h,020h,02bh,0dah,0bdh,0a0h,080h,051h,08bh,0cah,0d1h,0e9h,0f3h,0a5h,0d1h,0d1h,0f3h,0a4h,059h
    asm db 003h,0f0h,003h,0fbh,078h,004h,0e2h,0ech,0ebh,004h,003h,0fdh,0e2h,0e6h,01fh,05fh,05eh,05dh,0c3h,055h,08bh,0ech,056h,057h
    asm db 01eh,0fch,08bh,05eh,006h,0d1h,0e3h,0d1h,0e3h,0c4h,0b7h,024h,039h,0d1h,0e3h,0d1h,0e3h,0d1h,0e3h,0d1h,0e3h,08bh,0fbh,0d1h
    asm db 0e3h,0d1h,0e3h,003h,0fbh,08bh,046h,004h,08bh,05eh,008h,003h,0d8h,04bh,0d1h,0e8h,0d1h,0e0h,003h,0f8h,0d1h,0e8h,003h,0f0h
    asm db 0d1h,0e8h,0d1h,0ebh,0d1h,0ebh,02bh,0d8h,043h,0d1h,0e3h,02eh,08bh,087h,08eh,004h,02eh,0a3h,0d9h,012h,0bah,0a0h,000h,02bh
    asm db 0d3h,006h,0b9h,000h,0a0h,08eh,0c1h,01fh,08bh,04eh,00ah,0d1h,0e3h,0bdh,040h,001h,02bh,0ebh,005h,0dbh,012h,0ffh,0e0h,0adh
    asm db 08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh
    asm db 0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h
    asm db 0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h
    asm db 0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h
    asm db 08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh
    asm db 08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah
    asm db 0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh
    asm db 08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh
    asm db 0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h
    asm db 0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h
    asm db 0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h
    asm db 08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh
    asm db 08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah
    asm db 0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh
    asm db 08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh
    asm db 0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h
    asm db 0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h
    asm db 0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h
    asm db 08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh
    asm db 08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah
    asm db 0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh
    asm db 08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh
    asm db 0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h
    asm db 0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h
    asm db 0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h
    asm db 08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh
    asm db 08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah
    asm db 0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh
    asm db 08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh
    asm db 0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h
    asm db 0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h
    asm db 0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h
    asm db 08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh
    asm db 08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah
    asm db 0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh
    asm db 08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh
    asm db 0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h
    asm db 0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h
    asm db 0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h
    asm db 08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh
    asm db 08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah
    asm db 0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh
    asm db 08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh
    asm db 0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h
    asm db 0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h
    asm db 0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h
    asm db 08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh
    asm db 08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah
    asm db 0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh
    asm db 08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh
    asm db 0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h
    asm db 0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h
    asm db 0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h
    asm db 08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh
    asm db 08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah
    asm db 0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh
    asm db 08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh
    asm db 0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h
    asm db 0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h
    asm db 0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h
    asm db 08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh
    asm db 08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah
    asm db 0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh
    asm db 08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh
    asm db 0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h
    asm db 0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h
    asm db 0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h
    asm db 08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh
    asm db 08ah,0c5h,08ah,0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah
    asm db 0e7h,0abh,0adh,08ah,0ech,08bh,0d8h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,0d1h,0c3h,08ah,0e3h,0abh,08ah,0c5h,08ah,0e7h,0abh,003h
    asm db 0f2h,003h,0fdh,032h,0edh,0e2h,005h,01fh,05fh,05eh,05dh,0c3h,0e9h,060h,0f9h,055h,08bh,0ech,057h,0fch,0a1h,0c8h,040h,08bh
    asm db 04eh,008h,08bh,05eh,006h,0d1h,0e3h,0d1h,0e3h,0c4h,0bfh,024h,039h,08bh,05eh,004h,0d1h,0ebh,073h,00eh,08ah,0d0h,080h,0e2h
    asm db 00fh,026h,080h,021h,0f0h,026h,008h,011h,047h,049h,003h,0fbh,02bh,0dbh,0d1h,0e9h,073h,003h,0bbh,001h,000h,0f3h,0aah,083h
    asm db 0fbh,000h,074h,009h,024h,0f0h,026h,080h,025h,00fh,026h,008h,005h,05fh,05dh,0c3h,055h,08bh,0ech,057h,0fch,0a1h,0c8h,040h
    asm db 08bh,04eh,008h,08bh,05eh,006h,0d1h,0e3h,0d1h,0e3h,0c4h,0bfh,024h,039h,08bh,05eh,004h,0d1h,0ebh,072h,006h,024h,0f0h,0b2h
    asm db 00fh,0ebh,004h,024h,00fh,0b2h,0f0h,003h,0fbh,0bbh,0a0h,000h,026h,020h,015h,026h,008h,005h,003h,0fbh,0e2h,0f6h,05fh,05dh
    asm db 0c3h,055h,08bh,0ech,057h,056h,0fch,0a1h,0c8h,040h,0beh,0a0h,000h,08bh,05eh,006h,0d1h,0e3h,0d1h,0e3h,0c4h,0bfh,024h,039h
    asm db 08bh,05eh,004h,0d1h,0ebh,073h,01ch,08bh,04eh,00ah,057h,003h,0fbh,0b6h,0f0h,08ah,0d0h,080h,0e2h,00fh,026h,020h,035h,026h
    asm db 008h,015h,003h,0feh,0e2h,0f6h,05fh,047h,0ffh,04eh,008h,003h,0fbh,02ah,0e4h,08bh,05eh,008h,0d1h,0ebh,073h,002h,0b4h,001h
    asm db 08bh,056h,00ah,02bh,0f3h,08bh,0cbh,0f3h,0aah,003h,0feh,04ah,075h,0f7h,080h,0fch,000h,074h,019h,08bh,04eh,00ah,02bh,0feh
    asm db 0b6h,00fh,08ah,0d0h,080h,0e2h,0f0h,0beh,0a0h,000h,026h,020h,035h,026h,008h,015h,02bh,0feh,0e2h,0f6h,05eh,05fh,05dh,0c3h
    asm db 055h,08bh,0ech,057h,056h,0fch,08bh,056h,00ah,0beh,0a0h,000h,08bh,05eh,006h,0d1h,0e3h,0d1h,0e3h,0c4h,0bfh,024h,039h,08bh
    asm db 05eh,004h,0d1h,0ebh,073h,013h,08bh,0cah,057h,003h,0fbh,0b0h,00fh,026h,030h,005h,003h,0feh,0e2h,0f9h,05fh,047h,0ffh,04eh
    asm db 008h,003h,0fbh,057h,02ah,0e4h,08bh,04eh,008h,0d1h,0e9h,073h,002h,0b4h,001h,08bh,0d9h,0e3h,011h,0b0h,0ffh,02bh,0f1h,026h
    asm db 030h,005h,047h,0e2h,0fah,003h,0feh,08bh,0cbh,04ah,075h,0f3h,05fh,003h,0fbh,080h,0fch,000h,074h,00fh,08bh,04eh,00ah,0b0h
    asm db 0f0h,0beh,0a0h,000h,026h,030h,005h,003h,0feh,0e2h,0f9h,05eh,05fh,05dh,0c3h,055h,08bh,0ech,057h,056h,01eh,0fch,08bh,046h
    asm db 004h,08bh,05eh,008h,003h,0d8h,04bh,0d1h,0e8h,0d1h,0ebh,02bh,0d8h,043h,08bh,056h,00ah,08bh,076h,006h,0d1h,0e6h,0d1h,0e6h
    asm db 0c5h,0b4h,024h,039h,003h,0f0h,0c4h,07eh,00ch,026h,089h,01dh,026h,089h,055h,002h,083h,0c7h,004h,0b8h,0a0h,000h,02bh,0c3h
    asm db 08bh,0cbh,0f3h,0a4h,003h,0f0h,04ah,075h,0f7h,01fh,05eh,05fh,05dh,0c3h,055h,08bh,0ech,057h,056h,01eh,0fch,08bh,046h,004h
    asm db 0d1h,0e8h,08bh,07eh,006h,0d1h,0e7h,0d1h,0e7h,0c4h,0bdh,024h,039h,003h,0f8h,0c5h,076h,008h,0adh,08bh,0d8h,0adh,08bh,0d0h
    asm db 0b8h,0a0h,000h,02bh,0c3h,08bh,0cbh,0f3h,0a4h,003h,0f8h,04ah,075h,0f7h,01fh,05eh,05fh,05dh,0c3h,055h,08bh,0ech,057h,056h
    asm db 01eh,0fch,08bh,05eh,00eh,0d1h,0e3h,0d1h,0e3h,0c4h,0bfh,024h,039h,08bh,046h,00ch,0d1h,0e8h,003h,0f8h,08bh,05eh,006h,0d1h
    asm db 0e3h,0d1h,0e3h,0c5h,0b7h,024h,039h,08bh,046h,004h,0d1h,0e8h,003h,0f0h,08bh,05eh,008h,0d1h,0ebh,08bh,056h,00ah,0b8h,0a0h
    asm db 000h,02bh,0c3h,08bh,0cbh,0d1h,0e9h,0f3h,0a5h,0d1h,0d1h,0f3h,0a4h,003h,0f0h,003h,0f8h,04ah,075h,0efh,01fh,083h,03eh,0bch
    asm db 000h,001h,075h,024h,08bh,05eh,00eh,081h,0fbh,0c8h,000h,073h,01bh,0c4h,03eh,0c4h,040h,08bh,046h,00ch,0d1h,0e8h,08ah,0e3h
    asm db 0abh,08bh,046h,008h,0d1h,0e8h,08bh,05eh,00ah,08ah,0e3h,0abh,089h,03eh,0c4h,040h,05eh,05fh,05dh,0c3h,055h,08bh,0ech,057h
    asm db 056h,01eh,0fch,08bh,05eh,00eh,0d1h,0e3h,0d1h,0e3h,0c4h,0bfh,024h,039h,08bh,046h,00ch,0d1h,0e8h,003h,0f8h,08bh,05eh,006h
    asm db 0d1h,0e3h,0d1h,0e3h,0c5h,0b7h,024h,039h,08bh,046h,004h,0d1h,0e8h,003h,0f0h,08bh,05eh,008h,0d1h,0ebh,08bh,056h,00ah,08bh
    asm db 0c2h,048h,086h,0e0h,0d1h,0e8h,003h,0f8h,0d1h,0e8h,0d1h,0e8h,003h,0f8h,0b8h,0a0h,000h,08bh,0cbh,0f3h,0a4h,003h,0f0h,02bh
    asm db 0f3h,02bh,0f8h,02bh,0fbh,04ah,075h,0f1h,01fh,05eh,05fh,05dh,0c3h,055h,08bh,0ech,057h,056h,01eh,0fch,08bh,05eh,00eh,0d1h
    asm db 0e3h,0d1h,0e3h,0c4h,0bfh,024h,039h,08bh,046h,00ch,0d1h,0e8h,003h,0f8h,08bh,05eh,006h,0d1h,0e3h,0d1h,0e3h,0c5h,0b7h,024h
    asm db 039h,08bh,046h,004h,0d1h,0e8h,003h,0f0h,08bh,05eh,008h,0d1h,0ebh,08bh,056h,00ah,003h,0fbh,04fh,08bh,0cbh,0ach,0d0h,0c0h
    asm db 0d0h,0c0h,0d0h,0c0h,0d0h,0c0h,026h,088h,005h,04fh,0e2h,0f1h,081h,0c6h,0a0h,000h,02bh,0f3h,081h,0c7h,0a0h,000h,003h,0fbh
    asm db 04ah,075h,0e0h,01fh,05eh,05fh,05dh,0c3h,055h,08bh,0ech,057h,056h,01eh,0fch,08bh,05eh,00eh,0d1h,0e3h,0d1h,0e3h,0c4h,0bfh
    asm db 024h,039h,08bh,046h,00ch,0d1h,0e8h,003h,0f8h,08bh,05eh,006h,0d1h,0e3h,0d1h,0e3h,0c5h,0b7h,024h,039h,08bh,046h,004h,0d1h
    asm db 0e8h,003h,0f0h,08bh,05eh,008h,0d1h,0ebh,08bh,056h,00ah,08bh,0c2h,048h,086h,0e0h,0d1h,0e8h,003h,0f8h,0d1h,0e8h,0d1h,0e8h
    asm db 003h,0f8h,003h,0fbh,04fh,08bh,0cbh,0ach,0d0h,0c0h,0d0h,0c0h,0d0h,0c0h,0d0h,0c0h,026h,088h,005h,04fh,0e2h,0f1h,081h,0c6h
    asm db 0a0h,000h,02bh,0f3h,081h,0efh,0a0h,000h,003h,0fbh,04ah,075h,0e0h,01fh,05eh,05fh,05dh,0c3h,055h,08bh,0ech,057h,056h,01eh
    asm db 0fch,08bh,05eh,00eh,0d1h,0e3h,0d1h,0e3h,0c4h,0bfh,024h,039h,08bh,046h,00ch,0d1h,0e8h,003h,0f8h,08bh,05eh,006h,0d1h,0e3h
    asm db 0d1h,0e3h,0c5h,0b7h,024h,039h,08bh,046h,004h,0d1h,0e8h,003h,0f0h,08bh,05eh,008h,0d1h,0ebh,08bh,056h,00ah,0d1h,0eah,003h
    asm db 0fah,04fh,057h,08bh,0cbh,0ach,08ah,0e0h,025h,00fh,0f0h,0d0h,0ech,0d0h,0ech,0d0h,0ech,0d0h,0ech,026h,080h,025h,0f0h,026h
    asm db 008h,025h,081h,0c7h,0a0h,000h,026h,080h,025h,0f0h,026h,008h,005h,081h,0c7h,0a0h,000h,0e2h,0dah,081h,0c6h,0a0h,000h,02bh
    asm db 0f3h,05fh,057h,08bh,0cbh,0ach,08ah,0e0h,025h,00fh,0f0h,0d0h,0e0h,0d0h,0e0h,0d0h,0e0h,0d0h,0e0h,026h,080h,025h,00fh,026h
    asm db 008h,025h,081h,0c7h,0a0h,000h,026h,080h,025h,00fh,026h,008h,005h,081h,0c7h,0a0h,000h,0e2h,0dah,081h,0c6h,0a0h,000h,02bh
    asm db 0f3h,05fh,04fh,04ah,075h,09ch,01fh,05eh,05fh,05dh,0c3h,055h,08bh,0ech,057h,056h,01eh,0fch,08bh,05eh,00eh,0d1h,0e3h,0d1h
    asm db 0e3h,0c4h,0bfh,024h,039h,08bh,046h,00ch,0d1h,0e8h,003h,0f8h,08bh,05eh,006h,0d1h,0e3h,0d1h,0e3h,0c5h,0b7h,024h,039h,08bh
    asm db 046h,004h,0d1h,0e8h,003h,0f0h,08bh,05eh,008h,08bh,0c3h,0d1h,0ebh,08bh,056h,00ah,0d1h,0eah,048h,086h,0e0h,0d1h,0e8h,003h
    asm db 0f8h,0d1h,0e8h,0d1h,0e8h,003h,0f8h,057h,08bh,0cbh,0ach,08ah,0e0h,025h,00fh,0f0h,0d0h,0e0h,0d0h,0e0h,0d0h,0e0h,0d0h,0e0h
    asm db 026h,080h,025h,00fh,026h,008h,025h,081h,0efh,0a0h,000h,026h,080h,025h,00fh,026h,008h,005h,081h,0efh,0a0h,000h,0e2h,0dah
    asm db 081h,0c6h,0a0h,000h,02bh,0f3h,05fh,057h,08bh,0cbh,0ach,08ah,0e0h,025h,00fh,0f0h,0d0h,0ech,0d0h,0ech,0d0h,0ech,0d0h,0ech
    asm db 026h,080h,025h,0f0h,026h,008h,025h,081h,0efh,0a0h,000h,026h,080h,025h,0f0h,026h,008h,005h,081h,0efh,0a0h,000h,0e2h,0dah
    asm db 081h,0c6h,0a0h,000h,02bh,0f3h,05fh,047h,04ah,075h,09ch,01fh,05eh,05fh,05dh,0c3h,055h,08bh,0ech,083h,0ech,004h,057h,056h
    asm db 01eh,0fch,0a1h,0e0h,0c0h,08eh,0c0h,08bh,05eh,008h,08bh,03eh,0e4h,0c0h,026h,08ah,009h,02ah,0edh,08bh,03eh,0e6h,0c0h,026h
    asm db 08ah,001h,08bh,03eh,0e2h,0c0h,026h,08ah,021h,08bh,036h,0deh,0c0h,003h,0f0h,08bh,016h,0e8h,0c0h,0a1h,0c8h,040h,08ah,0e0h
    asm db 08bh,05eh,006h,0d1h,0e3h,0d1h,0e3h,0c5h,0bfh,024h,039h,0c7h,046h,0feh,000h,000h,08bh,05eh,004h,0d1h,0ebh,073h,005h,0c7h
    asm db 046h,0feh,001h,000h,003h,0fbh,08bh,0d9h,0d1h,0e3h,041h,0d1h,0e9h,0c7h,046h,0fch,0a0h,000h,029h,04eh,0fch,08ah,0cah,08ah
    asm db 0f0h,08ah,0d6h,081h,0e2h,00fh,0f0h,083h,07eh,0feh,000h,075h,075h,04fh,053h,02ah,0e4h,026h,08ah,004h,046h,083h,0fbh,010h
    asm db 07dh,00ah,08ah,0e3h,080h,0e4h,002h,02eh,0ffh,0a7h,00ch,019h,047h,0d0h,0e0h,073h,005h,080h,025h,00fh,008h,035h,0d0h,0e0h
    asm db 073h,005h,080h,025h,0f0h,008h,015h,047h,0d0h,0e0h,073h,005h,080h,025h,00fh,008h,035h,0d0h,0e0h,073h,005h,080h,025h,0f0h
    asm db 008h,015h,047h,0d0h,0e0h,073h,005h,080h,025h,00fh,008h,035h,0d0h,0e0h,073h,005h,080h,025h,0f0h,008h,015h,047h,0d0h,0e0h
    asm db 073h,005h,080h,025h,00fh,008h,035h,080h,0fch,000h,075h,00eh,0d0h,0e0h,073h,005h,080h,025h,0f0h,008h,015h,083h,0ebh,010h
    asm db 07fh,097h,05bh,003h,07eh,0fch,0e2h,08eh,0ebh,072h,053h,02ah,0e4h,026h,08ah,004h,046h,083h,0fbh,010h,07dh,00ah,08ah,0e3h
    asm db 080h,0e4h,002h,02eh,0ffh,0a7h,01eh,019h,0d0h,0e0h,073h,005h,080h,025h,0f0h,008h,015h,047h,0d0h,0e0h,073h,005h,080h,025h
    asm db 00fh,008h,035h,0d0h,0e0h,073h,005h,080h,025h,0f0h,008h,015h,047h,0d0h,0e0h,073h,005h,080h,025h,00fh,008h,035h,0d0h,0e0h
    asm db 073h,005h,080h,025h,0f0h,008h,015h,047h,0d0h,0e0h,073h,005h,080h,025h,00fh,008h,035h,0d0h,0e0h,073h,005h,080h,025h,0f0h
    asm db 008h,015h,047h,080h,0fch,000h,075h,00eh,0d0h,0e0h,073h,005h,080h,025h,00fh,008h,035h,083h,0ebh,010h,07fh,097h,05bh,003h
    asm db 07eh,0fch,0e2h,08eh,08bh,0c3h,0d1h,0e8h,01fh,05eh,05fh,083h,0c4h,004h,05dh,0c3h,081h,018h,069h,018h,069h,018h,056h,018h
    asm db 056h,018h,043h,018h,043h,018h,030h,018h,030h,018h,0f5h,018h,0ddh,018h,0ddh,018h,0cah,018h,0cah,018h,0b7h,018h,0b7h,018h
    asm db 0a4h,018h,0a4h,018h,055h,08bh,0ech,057h,056h,01eh,0fch,08bh,05eh,006h,0d1h,0e3h,0d1h,0e3h,0c4h,0bfh,024h,039h,08bh,05eh
    asm db 004h,0d1h,0ebh,003h,0fbh,0c5h,076h,008h,083h,0c6h,020h,0adh,02ah,0ffh,08ah,0d8h,08ah,0f7h,08ah,0d4h,052h,0b8h,0a0h,000h
    asm db 02bh,0c3h,08bh,0cbh,0d1h,0e9h,0f3h,0a5h,0d1h,0d1h,0f3h,0a4h,003h,0f8h,04ah,075h,0f1h,05ah,01fh,083h,03eh,0bch,000h,001h
    asm db 075h,01eh,08bh,04eh,006h,081h,0f9h,0c8h,000h,073h,015h,0c4h,03eh,0c4h,040h,08bh,046h,004h,0d1h,0e8h,08ah,0e1h,0abh,08ah
    asm db 0c3h,08ah,0e2h,0abh,089h,03eh,0c4h,040h,05eh,05fh,05dh,0c3h,0ffh,0f0h,0f0h,0f0h,0f0h,0f0h,0f0h,0f0h,0f0h,0f0h,0f0h,0f0h
    asm db 0f0h,0f0h,0f0h,0f0h,00fh,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,00fh,000h,000h,000h
    asm db 000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,00fh,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h
    asm db 000h,000h,000h,000h,00fh,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,00fh,000h,000h,000h
    asm db 000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,00fh,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h
    asm db 000h,000h,000h,000h,00fh,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,00fh,000h,000h,000h
    asm db 000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,00fh,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h
    asm db 000h,000h,000h,000h,00fh,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,00fh,000h,000h,000h
    asm db 000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,00fh,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h
    asm db 000h,000h,000h,000h,00fh,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,00fh,000h,000h,000h
    asm db 000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,00fh,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h
    asm db 000h,000h,000h,000h,055h,08bh,0ech,056h,057h,01eh,055h,0c4h,076h,008h,083h,0c6h,020h,026h,08bh,004h,083h,0c6h,002h,033h
    asm db 0d2h,08bh,0cah,08ah,0c8h,08ah,0d4h,08bh,07eh,006h,0a1h,096h,000h,02bh,0c7h,078h,046h,040h,03bh,0d0h,076h,002h,08bh,0d0h
    asm db 0a1h,094h,000h,02bh,0c7h,07eh,00ah,02bh,0d0h,076h,034h,003h,0f8h,0f6h,0e1h,003h,0f0h,08bh,05eh,004h,083h,07eh,00ch,000h
    asm db 074h,003h,0e9h,082h,000h,0d1h,0fbh,0fch,033h,0edh,0a1h,09ah,000h,02bh,0c3h,078h,016h,040h,03bh,0c8h,076h,006h,08bh,0e9h
    asm db 02bh,0e8h,08bh,0c8h,0a1h,098h,000h,02bh,0c3h,07eh,011h,02bh,0c8h,077h,007h,05dh,01fh,05fh,05eh,0fch,05dh,0c3h,003h,0d8h
    asm db 003h,0e8h,003h,0f0h,006h,057h,083h,03eh,0bch,000h,001h,075h,01ah,081h,0ffh,0c8h,000h,073h,014h,08bh,0c7h,08ah,0e0h,08ah
    asm db 0c3h,0c4h,03eh,0c4h,040h,0abh,08ah,0c1h,08ah,0e2h,0abh,089h,03eh,0c4h,040h,05fh,0d1h,0e7h,0d1h,0e7h,0c4h,0bdh,024h,039h
    asm db 003h,0fbh,01fh,0bbh,098h,019h,057h,051h,0ach,08ah,0e0h,02eh,0d7h,026h,022h,005h,00ah,0c4h,0aah,0e2h,0f3h,059h,05fh,081h
    asm db 0c7h,0a0h,000h,003h,0f5h,04ah,075h,0e6h,05dh,01fh,05fh,05eh,0fch,05dh,0c3h,0fdh,033h,0edh,003h,0d9h,003h,0d9h,0d1h,0ebh
    asm db 04bh,08bh,0c3h,02bh,006h,098h,000h,078h,017h,040h,03bh,0c8h,076h,006h,08bh,0e9h,02bh,0e8h,08bh,0c8h,08bh,0c3h,02bh,006h
    asm db 09ah,000h,076h,011h,02bh,0c8h,077h,007h,05dh,01fh,05fh,05eh,0fch,05dh,0c3h,02bh,0d8h,003h,0e8h,003h,0f0h,006h,057h,083h
    asm db 03eh,0bch,000h,001h,075h,020h,081h,0ffh,0c8h,000h,073h,01ah,08bh,0c7h,08ah,0e0h,08ah,0c3h,02ah,0c1h,0feh,0c0h,0c4h,03eh
    asm db 0c4h,040h,0fch,0abh,08ah,0c1h,08ah,0e2h,0abh,0fdh,089h,03eh,0c4h,040h,05fh,0d1h,0e7h,0d1h,0e7h,0c4h,0bdh,024h,039h,003h
    asm db 0fbh,01fh,0bbh,098h,019h,057h,051h,08ah,004h,046h,0d0h,0c8h,0d0h,0c8h,0d0h,0c8h,0d0h,0c8h,08ah,0e0h,02eh,0d7h,026h,022h
    asm db 005h,00ah,0c4h,0aah,0e2h,0e9h,059h,05fh,081h,0c7h,0a0h,000h,003h,0f5h,04ah,075h,0dch,05dh,01fh,05fh,05eh,0fch,05dh,0c3h
    asm db 055h,08bh,0ech,083h,0ech,002h,056h,057h,01eh,0fch,08bh,016h,0c8h,040h,08ah,0f2h,081h,0e2h,00fh,0f0h,0c4h,076h,008h,08bh
    asm db 05eh,006h,0d1h,0e3h,0d1h,0e3h,0c5h,0bfh,024h,039h,08bh,05eh,004h,0d1h,0ebh,003h,0fbh,033h,0dbh,08bh,0cbh,026h,08ah,01ch
    asm db 046h,0c7h,046h,0feh,0a0h,000h,029h,05eh,0feh,0d1h,0e3h,0d1h,0e3h,026h,08ah,00ch,046h,04fh,053h,02ah,0e4h,026h,08ah,004h
    asm db 046h,083h,0fbh,010h,07dh,00ah,08ah,0e3h,080h,0e4h,002h,02eh,0ffh,0a7h,00ch,019h,047h,0d0h,0e0h,073h,005h,080h,025h,00fh
    asm db 008h,035h,0d0h,0e0h,073h,005h,080h,025h,0f0h,008h,015h,047h,0d0h,0e0h,073h,005h,080h,025h,00fh,008h,035h,0d0h,0e0h,073h
    asm db 005h,080h,025h,0f0h,008h,015h,047h,0d0h,0e0h,073h,005h,080h,025h,00fh,008h,035h,0d0h,0e0h,073h,005h,080h,025h,0f0h,008h
    asm db 015h,047h,0d0h,0e0h,073h,005h,080h,025h,00fh,008h,035h,080h,0fch,000h,075h,00eh,0d0h,0e0h,073h,005h,080h,025h,0f0h,008h
    asm db 015h,083h,0ebh,010h,07fh,097h,05bh,003h,07eh,0feh,0e2h,08eh,01fh,05fh,05eh,083h,0c4h,002h,05dh,0c3h,0a5h,01ch,08dh,01ch
    asm db 08dh,01ch,07ah,01ch,07ah,01ch,067h,01ch,067h,01ch,054h,01ch,054h,01ch,055h,08bh,0ech,057h,08bh,05eh,006h,0d1h,0e3h,0d1h
    asm db 0e3h,0c4h,0bfh,024h,039h,08bh,05eh,004h,0d1h,0ebh,072h,015h,003h,0fbh,026h,08ah,005h,024h,00fh,08bh,01eh,0c8h,040h,0d0h
    asm db 0e3h,0d0h,0e3h,0d0h,0e3h,0d0h,0e3h,0ebh,00eh,003h,0fbh,026h,08ah,005h,024h,0f0h,08bh,01eh,0c8h,040h,080h,0e3h,00fh,00ah
    asm db 0c3h,026h,088h,005h,05fh,05dh,0c3h,055h,08bh,0ech,057h,08bh,05eh,006h,0d1h,0e3h,0d1h,0e3h,0c4h,0bfh,024h,039h,08bh,05eh
    asm db 004h,0d1h,0ebh,072h,012h,003h,0fbh,026h,08ah,005h,025h,0f0h,000h,0d1h,0e8h,0d1h,0e8h,0d1h,0e8h,0d1h,0e8h,0ebh,008h,003h
    asm db 0fbh,026h,08ah,005h,025h,00fh,000h,05fh,05dh,0c3h,0c3h,045h,047h,041h,02eh,044h,052h,056h,000h
    asm _runtime_block_end label byte
    asm public _runtime_block_end
}
