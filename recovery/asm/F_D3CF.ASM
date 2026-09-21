; F_D3CF -- branch continuation for the F_D386 far-record decoder.
_TEXT   segment byte public 'CODE'
DGROUP  group   _DATA,_BSS
        assume  cs:_TEXT,ds:DGROUP
_DATA   segment word public 'DATA'
_DATA   ends
_BSS    segment word public 'BSS'
_BSS    ends
_TEXT   segment byte public 'CODE'
_icon_frame_reset_and_draw  proc near
        mov     byte ptr [di],1
        mov     si,di
        add     si,3
        lodsb
        jmp     short $-2fh
_icon_frame_reset_and_draw  endp
        public  _icon_frame_reset_and_draw
_TEXT   ends
        end
