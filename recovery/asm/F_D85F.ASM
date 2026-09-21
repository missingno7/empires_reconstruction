; F_D85F -- delete the 5-byte record whose key byte equals arg1 from the
; length-prefixed table at g2F30, by shifting the tail down over it with
; rep movsb (count = 5 * remaining) and decrementing the count byte.
;
; NOT a Turbo C translation unit: ES is loaded from DS by hand, the table is
; walked with lodsb / loop / jcxz with CX live across the whole body, and the
; move is rep movsb with SI and DI both live.  Census R6 lodsb, R6 loop.
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
_record_table_delete_compact  proc    near
        push    bp
        mov     bp,sp
        push    si
        push    di
        mov     ax,ds
        mov     es,ax
        mov     si,offset DGROUP:_g2f30
        lodsb
        mov     cl,al
        sub     ch,ch
        jcxz    @@out
@@scan: mov     al,[si]
        cmp     al,[bp+4]
        je      @@hit
        add     si,5
        loop    @@scan
        jcxz    @@out
@@hit:  mov     di,si
        add     si,5
        dec     cx
        mov     ax,cx
        shl     ax,1
        shl     ax,1
        add     cx,ax
        rep     movsb
        mov     di,offset DGROUP:_g2f30
        dec     byte ptr es:[di]
@@out:  pop     di
        pop     si
        pop     bp
        ret
_record_table_delete_compact  endp
_TEXT   ends
        public  _record_table_delete_compact
        extrn   _g2f30:byte
        end
