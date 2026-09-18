; F_CAF1 -- the sound-tick service body, called with every register live.
; Census R1-R4 (module): it saves ax/cx/si/di/es itself, which no Turbo C
; prologue does.
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
_fcaf1  proc    near
        push    bp
        mov     bp,sp
        push    ax
        push    cx
        push    si
        push    di
        push    es
        cmp     _g176e,0
        je      @@out
        mov     es,_g1760
        mov     di,[bp+4]
        cmp     di,_g1e8a
        ja      @@out
        mov     _g1e8a,di
        shl     di,1
        add     di,_g175e
        mov     ax,es:[di]
        add     ax,_g175e
        mov     _g1e88,ax
        mov     _g1770,2
        mov     _g1e8e,6
        mov     _g1e92,1
        xor     ax,ax
        mov     _g1e86,ax
        mov     _g1e90,ax
        mov     _g1e94,ax
        mov     _g1e8c,ax
@@out:
        pop     es
        pop     di
        pop     si
        pop     cx
        pop     ax
        pop     bp
        ret
_fcaf1  endp
_TEXT   ends
        public  _fcaf1
        extrn   _g176e:word
        extrn   _g1760:word
        extrn   _g1e8a:word
        extrn   _g175e:word
        extrn   _g1e88:word
        extrn   _g1770:word
        extrn   _g1e8e:word
        extrn   _g1e92:word
        extrn   _g1e86:word
        extrn   _g1e90:word
        extrn   _g1e94:word
        extrn   _g1e8c:word
        end
