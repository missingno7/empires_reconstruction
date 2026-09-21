; F_4E9F -- walk the 20h-stride record table at gB3AE (count in its first
; byte) and, for every record whose byte at +1 equals gBFBA and whose byte at
; +8 is not 1, call f4AA8 with the record's two words at +2 / +4 and two bytes
; looked up in the parallel tables at gBF66 and g9BFC by the record's byte +6.
;
; NOT a Turbo C translation unit: DI is the record cursor and CX the loop
; count, both live across the call and preserved by an explicit push cx /
; pop cx around it; TC 2.0 would spill the count to a stack local.
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
_sprite_table_wipe_active  proc    near
        push    bp
        mov     bp,sp
        push    si
        push    di
        mov     di,offset DGROUP:_gb3ae
        xor     cx,cx
        mov     cl,[di]
        or      cx,cx
        je      @@out
        inc     di
@@top:  xor     ax,ax
        mov     al,[di+1]
        cmp     ax,_board_record_index
        jne     @@next
        cmp     byte ptr [di+8],1
        je      @@next
        push    cx
        xor     ax,ax
        mov     al,[di+6]
        mov     bx,ax
        mov     al,[bx+_gbf66]
        push    ax
        mov     al,[bx+_g9bfc]
        push    ax
        mov     ax,[di+4]
        push    ax
        mov     ax,[di+2]
        push    ax
        call    _play_window_wipe_clipped
        add     sp,8
        pop     cx
@@next: add     di,20h
        loop    @@top
@@out:  pop     di
        pop     si
        pop     bp
        ret
_sprite_table_wipe_active  endp
_TEXT   ends
        public  _sprite_table_wipe_active
        extrn   _play_window_wipe_clipped:near
        extrn   _gb3ae:byte
        extrn   _board_record_index:word
        extrn   _gbf66:byte
        extrn   _g9bfc:byte
        end
