; F_6BCF -- the timer ISR (the master tick).  Saves every register, points DS
; at DGROUP, re-enables interrupts, bumps the /13 sub-counter gC0D4 and on
; wrap far-calls through the vector at g0B7A, advances the 32-bit tick at
; g0B76/g0B78, calls fC1A0 unless gated by g237C / g176E / g1772, sends EOI
; and irets.  233342 recorded executions, 411 call sites.
;
; NOT a Turbo C translation unit by construction: it ends in IRET, it saves
; all nine registers by hand, it uses pushf/sti/popf, and it writes OUT 20h.
; The  mov bp,0FA3h  is a SEGMENT fixup (an EXE base relocation), i.e.
; mov bp,DGROUP -- runK idiom (e).
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
_f6bcf  proc    near
        push    ax
        push    bx
        push    cx
        push    dx
        push    es
        push    ds
        push    si
        push    di
        push    bp
        mov     bp,DGROUP
        mov     ds,bp
        pushf
        sti
        inc     _gc0d4
        cmp     _gc0d4,0Dh
        jl      @@notick
        mov     _gc0d4,0
        pushf
        call    dword ptr _g0b7a
@@notick:
        add     _g0b76,1
        adc     _g0b78,0
        cmp     _g237c,0
        jne     @@done
        cmp     _g176e,0
        jne     @@call
        cmp     _g1772,0
        je      @@done
@@call: call    _fc1a0
@@done: mov     al,20h
        out     20h,al
        popf
        pop     bp
        pop     di
        pop     si
        pop     ds
        pop     es
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        iret
_f6bcf  endp
_TEXT   ends
        public  _f6bcf
        extrn   _fc1a0:near
        extrn   _gc0d4:word
        extrn   _g0b7a:dword
        extrn   _g0b76:word
        extrn   _g0b78:word
        extrn   _g237c:word
        extrn   _g176e:word
        extrn   _g1772:word
        end
