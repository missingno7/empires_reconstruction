; F_60A9 -- the animated-tile ticker.  Every 10th call it walks a
; length-prefixed table of 3-byte animation records at the far pointer
; g40D0, advances each record's 5-bit frame field up or down (bit 6 selects
; the direction, bit 7 arms the record), recomputes the tile's 1E6h-byte
; slot in the far sprite bank at gBFC8, and repaints the tile in both the
; play window and the 0B8h-biased mirror.
;
; NOT a Turbo C translation unit: BP is used as a DATA register (mov bp,dx /
; mov dx,bp) and then re-pointed at the stack mid-body (mov bp,sp /
; add word ptr [bp],0B8h) to rewrite an already-pushed argument between two
; calls; SI/DI are driven by lodsb/lodsw with DS reloaded by lds; and the
; loop count lives in CX across two calls.  Census R1-R4 + R6 + R7.
;
; The fixup at 6122h is a SEGMENT fixup, not a byte constant: the image's
; relocation table patches that word with the program's data paragraph
; (stored 0FA3h, image offset 0FA30h -- which is where the Borland
; "Turbo-C - Copyright (c) 1988" banner sits, i.e. the head of _DATA).
; So the source is `mov ax,DGROUP` -- the DS-restore an asm module must do
; after `lds` before it may call a C function.
_TEXT   segment byte public 'CODE'
DGROUP  group   _DATA,_BSS
        assume  cs:_TEXT,ds:DGROUP
_TEXT   ends
_DATA   segment word public 'DATA'
d@      label   byte
_DATA   ends
_BSS    segment word public 'BSS'
b@      label   byte
_BSS    ends
_TEXT   segment byte public 'CODE'
_f60a9  proc    near
        push    bp
        mov     bp,sp
        push    si
        push    di
        push    ds
        cld
        cmp     _g8fe,0
        je      @@armed
        jmp     @@done
@@armed:
        dec     _ga20
        cmp     _ga20,0
        je      @@tick
        jmp     @@done
@@tick:
        mov     _ga20,0Ah
        les     di,_gbfc8
        lds     si,_g40d0
        lodsb
        xor     cx,cx
        mov     cl,al                   ; record count
@@rec:
        push    di
        lodsw
        xor     dx,dx
        mov     dl,ah                   ; y
        mov     bh,dh
        mov     bl,al
        shl     bx,1                    ; x*2
        mov     bp,dx
        lodsb                           ; flags/frame byte
        xor     ah,ah
        test    al,80h
        jne     @@active
        jmp     @@next
@@active:
        test    al,40h
        je      @@up
        and     al,1Fh
        dec     al
        jge     @@dnok
        mov     al,17h
@@dnok:
        jmp     short @@store
@@up:
        and     al,1Fh
        inc     al
        cmp     al,17h
        jle     @@store
        mov     al,0
@@store:
        and     byte ptr [si-1],0E0h
        or      byte ptr [si-1],al
        mov     dx,1E6h
        mul     dx
        add     ax,2
        add     di,ax
        mov     dx,bp
        push    ds
        push    cx
        push    es
        push    dx
        mov     ax,DGROUP
        mov     ds,ax
        push    dx
        mov     bp,sp
        push    bx
        mov     ax,1Eh
        push    ax
        mov     ax,1Eh
        push    ax
        mov     ax,dx
        add     ax,148h
        push    ax
        push    bx
        call    _f03b4
        add     word ptr [bp],0B8h
        call    _f03b4
        pop     bx
        add     sp,0Ah
        pop     dx
        pop     es
        push    es
        xor     ax,ax
        push    ax
        push    es
        push    di
        push    dx
        push    bx
        call    _f03cc
        pop     bx
        pop     dx
        add     sp,6
        mov     ax,dx
        add     ax,0B8h
        push    ax
        push    bx
        mov     ax,1Eh
        push    ax
        mov     ax,1Eh
        push    ax
        push    dx
        push    bx
        call    _f03b4
        add     sp,0Ch
        pop     es
        pop     cx
        pop     ds
@@next:
        pop     di
        dec     cx
        jcxz    @@done
        jmp     @@rec
@@done:
        pop     ds
        pop     di
        pop     si
        pop     bp
        ret
_f60a9  endp
_TEXT   ends
        public  _f60a9
        extrn   _f03b4:near
        extrn   _f03cc:near
        extrn   _g8fe:word
        extrn   _ga20:word
        extrn   _gbfc8:dword
        extrn   _g40d0:dword
        end
