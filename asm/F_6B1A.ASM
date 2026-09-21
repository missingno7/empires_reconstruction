; F_6B1A -- the blocking INT 16h read with the F1..F10 hot-key check.
; Census R5 (int).
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
_f6b1a  proc    near
        push    si
        xor     ah,ah
        int     16h
        or      al,al
        jnz     @@ascii
        mov     al,ah
        mov     ah,1
        cmp     al,3bh
        jb      @@out
        cmp     al,44h
        jg      @@out
        mov     si,ax
        call    _menu_list_active
        or      ax,ax
        mov     ax,si
        jz      @@out
        sub     ax,013bh
        push    ax
        call    _menu_loop_run
        pop     ax
        mov     ax,si
        jmp     short @@out
@@ascii:
        xor     ah,ah
@@out:
        pop     si
        ret
_f6b1a  endp
_TEXT   ends
        public  _f6b1a
        extrn   _menu_list_active:near
        extrn   _menu_loop_run:near
        end
