; F_4EEB -- called as board_actors_draw(y0) from F_329F/F_3986/F_31C4: walks a fixed
; 0x20-byte-stride table at gb3ae (count byte at [0], records from [1]),
; and for every live record whose "screen id" word matches board_record_index, does a
; copy() (03CC) of a jump-table sprite selected by [di+6] and, if a
; trailing shadow-height byte at [di+0x1a] is set, an gfx_vline() strip too.
; census R6 (loop): not a C translation unit.
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
_board_actors_draw  proc    near
        push    bp
        mov     bp,sp
        mov     ax,0fh
        push    ax
        call    _gfx_color_select
        pop     ax
        push    si
        push    di
        mov     di,offset _gb3ae
        xor     cx,cx
        mov     cl,byte ptr [di]
        or      cx,cx
        je      @@end
        inc     di
@@loop:
        xor     ax,ax
        mov     al,byte ptr [di+1]
        cmp     ax,_board_record_index
        jne     @@skip
        cmp     byte ptr [di+8],1
        je      @@skip
        push    cx
        xor     bx,bx
        mov     bl,byte ptr [di+7]
        push    bx
        mov     bl,byte ptr [di+6]
        shl     bx,1
        shl     bx,1
        les     ax,dword ptr _resource_ptr_table[bx]
        push    es
        push    ax
        mov     ax,[bp+4]
        add     ax,word ptr [di+4]
        push    ax
        push    word ptr [di+2]
        call    _gfx_copy_rect
        add     sp,0ah
        cmp     byte ptr [di+1ah],0
        je      @@skip2
        mov     ax,word ptr [di+2]
        add     ax,10h
        mov     bx,word ptr [di+4]
        mov     dl,byte ptr [di+1ah]
        xor     dh,dh
        sub     bx,dx
        inc     bx
        push    bx
        push    dx
        push    ax
        call    _gfx_vline
        add     sp,6
@@skip2:
        pop     cx
@@skip:
        add     di,20h
        loop    @@loop
@@end:
        pop     di
        pop     si
        pop     bp
        ret
_board_actors_draw  endp
_TEXT   ends
        public  _board_actors_draw
        extrn   _gfx_color_select:near
        extrn   _gfx_copy_rect:near
        extrn   _gfx_vline:near
        extrn   _gb3ae:byte
        extrn   _board_record_index:word
        extrn   _resource_ptr_table:word
        end
