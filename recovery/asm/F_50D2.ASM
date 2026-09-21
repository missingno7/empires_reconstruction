; F_50D2 -- display-adapter detection, as a hand-written TASM module.
; Segment/EXTRN layout copied from what TCC itself emits (code first, EXTRN
; at the end), so every reference to _bbfcd is a forward reference: TASM
; sizes  mov _bbfcd,0  as a word on pass 1 and as a byte on pass 2, and pads
; the byte it saved with NOP.  Forward  jmp  is likewise sized near on pass 1
; and short on pass 2, and padded.  No NOP is written by hand.
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
_f50d2  proc    near
        push    si
        mov     _bbfcd,0
        mov     ah,0fh
        int     10h
        cmp     al,7
        jne     @@1
        jmp     @@done
@@1:
        mov     ax,0f000h
        mov     es,ax
        mov     si,0fffeh
        cmp     byte ptr es:[si],0ffh
        jne     @@pc
        mov     si,0c000h
        cmp     byte ptr es:[si],21h
        jne     @@pc
        mov     _bbfcd,3
        mov     byte ptr _b856,1
        jmp     @@done
@@pc:
        mov     ax,1a00h
        int     10h
        cmp     al,1ah
        jne     @@ega
        cmp     bl,0ah
        jl      @@2
        mov     _bbfcd,4
        jmp     @@done
@@2:
        cmp     bl,4
        je      @@3
        cmp     bl,5
        jne     @@4
@@3:
        mov     _bbfcd,1
        jmp     @@done
@@4:
        cmp     bl,7
        je      @@5
        cmp     bl,8
        jne     @@6
@@5:
        mov     _bbfcd,5
        jmp     @@done
@@6:
        cmp     bl,2
        jne     @@ega
        mov     _bbfcd,2
        jmp     @@done
@@ega:
        mov     bh,0ffh
        mov     cl,0ffh
        mov     ah,12h
        mov     bl,10h
        int     10h
        cmp     bh,1
        ja      @@crtc
        cmp     cl,0fh
        ja      @@crtc
        mov     _bbfcd,1
        jmp     @@done
@@crtc:
        mov     dx,3d4h
        mov     al,0fh
        out     dx,al
        inc     dx
        in      al,dx
        mov     ah,al
        mov     al,66h
        out     dx,al
        mov     cx,100h
@@spin:
        loop    @@spin
        in      al,dx
        xchg    ah,al
        out     dx,al
        cmp     ah,66h
        jne     @@done
        mov     _bbfcd,2
@@done:
        cmp     word ptr _bbfcd,1
        jne     @@7
        mov     ah,12h
        mov     bl,10h
        int     10h
        or      bl,bl
        jne     @@fin
        mov     _bbfcd,2
        jmp     @@fin
@@7:
        cmp     word ptr _bbfcd,4
        jne     @@8
        jmp     @@fin
@@8:
        cmp     word ptr _bbfcd,5
        jne     @@fin
@@fin:
        pop     si
        ret
_f50d2  endp
_TEXT   ends
        extrn   _bbfcd:byte
        extrn   _b856:byte
        public  _f50d2
        end
