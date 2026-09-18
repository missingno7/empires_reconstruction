; F_6B4A -- the non-blocking INT 16h poll.  Census R5 (int): not a C
; translation unit; written as a TASM module in TCC's own segment layout.
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
_f6b4a  proc    near
        mov     ah,1
        int     16h
        jz      @@none
        or      al,al
        jnz     @@ascii
        mov     al,ah
        mov     ah,1
        jmp     short @@out
@@ext:
        mov     ax,0100h
        jmp     short @@ascii
@@none:
        xor     ax,ax
        jmp     short @@out
@@ascii:
        xor     ah,ah
@@out:
        ret
_f6b4a  endp
_TEXT   ends
        public  _f6b4a
        end
