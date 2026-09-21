; F_1F91 -- OR together the collision/attribute bytes of the 26h-stride cells
; the caller's clipped span covers in the far map at gBFBC, and return the
; accumulated byte.  x is clamped to 8..137h, y to 10h..9Fh, both are scaled
; by 8, and the row count is clamped at 13h.
;
; NOT a Turbo C translation unit: DS is saved and reloaded by lds, AX, BX, CX
; and DI are four simultaneously live scratch values across the clamps, and
; CX is the loop counter of a  loop  instruction (runK idiom (a)).
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
_board_collision_span_or  proc    near
        push    bp
        mov     bp,sp
        push    si
        push    di
        push    ds
        lds     si,_board_records
        mov     ax,[bp+4]
        cmp     ax,137h
        jg      @@xhi
        cmp     ax,8
        jge     @@xok
        mov     ax,8
        jmp     short @@xok
@@xhi:  mov     ax,137h
@@xok:  mov     bx,[bp+6]
        cmp     bx,9Fh
        jg      @@yhi
        cmp     bx,10h
        jge     @@yok
        mov     bx,10h
        jmp     short @@yok
@@yhi:  mov     bx,9Fh
@@yok:  shr     ax,1
        shr     ax,1
        shr     ax,1
        mov     cx,[bp+8]
        add     cx,bx
        dec     cx
        shr     bx,1
        shr     bx,1
        shr     bx,1
        shr     cx,1
        shr     cx,1
        shr     cx,1
        cmp     cx,13h
        jl      @@nlo
        mov     cx,13h
@@nlo:  sub     cx,bx
        inc     cx
        sub     bx,2
        dec     ax
        shl     bx,1
        mov     di,bx
        shl     bx,1
        add     di,bx
        shl     bx,1
        shl     bx,1
        shl     bx,1
        add     bx,di
        add     bx,ax
        add     si,bx
        xor     ax,ax
@@row:  or      al,[si]
        add     si,26h
        loop    @@row
        pop     ds
        pop     di
        pop     si
        pop     bp
        ret
_board_collision_span_or  endp
_TEXT   ends
        public  _board_collision_span_or
        extrn   _board_records:dword
        end
