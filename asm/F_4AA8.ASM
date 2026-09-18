; F_4AA8 -- clip a rectangle to the play window (8..137h, 10h..9Fh) and hand
; the clipped rect to wipe() = f03B4, with the y origin biased by 0B8h.
;
; NOT a Turbo C translation unit: AX, BX, CX, DX hold four SIMULTANEOUSLY
; LIVE variables (x1, y1, x2, y2) across the four guard branches and the four
; clamps, and each is written in place (mov ax,8 / mov cx,137h / mov bx,10h /
; mov dx,9Fh).  TC 2.0 keeps a variable only in SI or DI, and it saves them;
; this body saves nothing but BP and uses exactly the four registers the
; Turbo C convention lets a callee clobber.  Hand-written assembly.
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
_f4aa8  proc    near
        push    bp
        mov     bp,sp
        mov     ax,[bp+4]               ; x1
        cmp     ax,137h
        jg      @@out
        mov     bx,[bp+6]               ; y1
        cmp     bx,9Fh
        jg      @@out
        mov     cx,[bp+8]               ; w
        add     cx,ax                   ; x2 = x1 + w - 1
        dec     cx
        cmp     cx,8
        jl      @@out
        mov     dx,[bp+0Ah]             ; h
        add     dx,bx                   ; y2 = y1 + h - 1
        dec     dx
        cmp     dx,10h
        jl      @@out
        cmp     ax,8
        jge     @@x1ok
        mov     ax,8
@@x1ok:
        cmp     cx,137h
        jle     @@x2ok
        mov     cx,137h
@@x2ok:
        cmp     bx,10h
        jge     @@y1ok
        mov     bx,10h
@@y1ok:
        cmp     dx,9Fh
        jle     @@y2ok
        mov     dx,9Fh
@@y2ok:
        sub     cx,ax                   ; w = x2 - x1 + 1
        inc     cx
        sub     dx,bx                   ; h = y2 - y1 + 1
        inc     dx
        push    bx
        push    ax
        push    dx
        push    cx
        add     bx,0B8h
        push    bx
        push    ax
        call    _f03b4
        add     sp,0Ch
@@out:
        pop     bp
        ret
_f4aa8  endp
_TEXT   ends
        public  _f4aa8
        extrn   _f03b4:near
        end
