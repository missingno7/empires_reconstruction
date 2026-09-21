; F_01A4 -- DOS write setup and process handoff following F_019C.
_TEXT   segment byte public 'CODE'
DGROUP  group   _DATA,_BSS
        assume  cs:_TEXT,ds:DGROUP
_DATA   segment word public 'DATA'
_DATA   ends
_BSS    segment word public 'BSS'
_BSS    ends
_TEXT   segment byte public 'CODE'
_f01a4  proc near
        mov     cx,01eh
        nop
        mov     dx,03dh
        mov     ds,word ptr cs:01bah
        call    _f019c
        mov     ax,3
        push    ax
        call    near ptr _f0104
_f01a4  endp
        public  _f01a4
        extrn   _f019c:near
        extrn   _f0104:near
_TEXT   ends
        end
