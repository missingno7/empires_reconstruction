; F_1ECD -- "d()" in F_233E's cast animation: drains a queue of pending
; box() invalidation rectangles between the read pointer ui_gfx_blob and the
; write pointer rect_queue_write_ptr (both far pointers, 4 bytes each), calling
; box(x,y,w,h) = f039f for each queued record, then resets the write
; pointer to the read pointer (queue emptied).  Record is 4 bytes:
; word1 lo=x/2 hi=y, word2 lo=w/2 hi=h -- census R6 (cld/lodsw/loop):
; not a C translation unit.
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
_f1ecd  proc    near
        push    si
        push    di
        cld
        mov     di,ds
        les     cx,dword ptr _rect_queue_write_ptr
        lds     si,dword ptr _ui_gfx_blob
        sub     cx,si
        shr     cx,1
        shr     cx,1
@@loop:
        push    cx
        push    ds
        lodsw
        mov     dx,ax
        lodsw
        xor     bx,bx
        mov     bl,ah
        push    bx
        mov     bl,al
        shl     bx,1
        push    bx
        xor     bx,bx
        mov     bl,dh
        push    bx
        mov     bl,dl
        shl     bx,1
        push    bx
        mov     ds,di
        call    _f039f
        add     sp,8
        pop     ds
        pop     cx
        loop    @@loop
        mov     ds,di
        les     bx,dword ptr _ui_gfx_blob
        mov     word ptr _g40c6,es
        mov     word ptr _rect_queue_write_ptr,bx
        pop     di
        pop     si
        ret
_f1ecd  endp
_TEXT   ends
        public  _f1ecd
        extrn   _f039f:near
        extrn   _rect_queue_write_ptr:word
        extrn   _g40c6:word
        extrn   _ui_gfx_blob:word
        end
