; M_6B1A_6B4A -- adjacent BIOS keyboard input helpers.
; _f6b1a blocks and dispatches F1..F10; _f6b4a polls without consuming.
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
        public  _f6b1a, _f6b4a
        extrn   _menu_list_active:near
        extrn   _menu_loop_run:near

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

_f6b4a  proc    near
        mov     ah,1
        int     16h
        jz      Kpoll_none
        or      al,al
        jnz     Kpoll_ascii
        mov     al,ah
        mov     ah,1
        jmp     short Kpoll_out
Kpoll_ext:
        mov     ax,0100h
        jmp     short Kpoll_ascii
Kpoll_none:
        xor     ax,ax
        jmp     short Kpoll_out
Kpoll_ascii:
        xor     ah,ah
Kpoll_out:
        ret
_f6b4a  endp
_TEXT   ends
        end
