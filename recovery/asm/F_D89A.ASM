; F_D89A -- the object hit-test (the retired identity brick's function).
; Walks the length-prefixed table of 5-byte rectangles at g2F30 and returns
; the id byte of the first rectangle that overlaps the caller's rect
; (x,y,w,h passed as four ints, narrowed to bytes into BL/BH/DL/DH), or 0.
;
; NOT a Turbo C translation unit: BP is used as a DATA register (mov bp,ax)
; after the frame is set up, the table is walked with cld/lodsb/lodsw, and
; BX and DX each carry TWO packed byte variables at once.  Census R6 cld,
; R6 lodsb, R6 lodsw, R6 loop, R7 bp write.
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
_rect_table_hit_id  proc    near
        push    bp
        mov     bp,sp
        push    si
        push    di
        cld
        mov     ax,[bp+4]
        mov     bl,al
        mov     ax,[bp+6]
        mov     bh,al
        mov     ax,[bp+8]
        mov     dl,al
        mov     ax,[bp+0Ah]
        mov     dh,al
        add     dx,bx
        dec     dl
        dec     dh
        mov     si,offset DGROUP:_g2f30
        lodsb
        sub     ch,ch
        mov     cl,al
        or      cx,cx
        je      @@zero
@@top:  sub     ah,ah
        lodsb
        mov     di,ax
        lodsw
        cmp     dl,al
        jb      @@skip
        cmp     dh,ah
        jb      @@skip
        mov     bp,ax
        lodsw
        add     ax,bp
        cmp     al,bl
        jbe     @@next
        cmp     ah,bh
        jbe     @@next
        mov     ax,di
        jmp     short @@done
@@skip: add     si,2
@@next: loop    @@top
@@zero: sub     ax,ax
@@done: pop     di
        pop     si
        pop     bp
        ret
_rect_table_hit_id  endp
_TEXT   ends
        public  _rect_table_hit_id
        extrn   _g2f30:byte
        end
