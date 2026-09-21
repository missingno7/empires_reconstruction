; F_4EEB -- walk the record table at actor_record_table (count in its first byte, 20h-byte
; records) and, for every record whose word at +1 equals board_record_index
; and whose byte at +8 is not 1, blit its sprite via gfx_copy_rect (color 0Fh
; selected first through gfx_color_select) at (word[+2], y0+word[+4]) using
; the far pointer looked up in resource_ptr_table[record byte +6 * 4]; when
; the record's byte at +1Ah is nonzero, additionally copy a strip of that
; sprite via gfx_vline.  Matches src/F_4EEB.C's original Turbo C inline-asm
; recovery (kept for provenance); this file restates it with labels instead
; of raw db bytes and TASM syntax.
;
; near proc, compact-model C calling convention: word arg y0 at [bp+4],
; no return value. Clobbers ax/bx/cx/dx; si/di/bp preserved via push/pop.

_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

public _board_actors_draw
extrn _gfx_color_select:near
extrn _gfx_vline:near
extrn _gfx_copy_rect:near
extrn _actor_record_table:byte
extrn _board_record_index:word
extrn _resource_ptr_table:dword

_board_actors_draw proc near
    push bp
    mov bp,sp
; select drawing color 0Fh for the sprite blits below
    mov ax,0fh
    push ax
    call _gfx_color_select
    pop ax
    push si
    push di
; di = cursor into the record table; cx = record count (first byte)
    mov di,offset DGROUP:_actor_record_table
    xor cx,cx
    mov cl,[di]
    or cx,cx
    je done
    inc di
loop_records:
; keep only records for the current board (word at +1 == board_record_index)
; and not flagged "skip" (byte at +8 == 1)
    xor ax,ax
    mov al,[di+1]
    cmp ax,word ptr DGROUP:_board_record_index
    jne skip_record
    cmp byte ptr [di+8],1
    je skip_record
    push cx
; look up the sprite's far pointer via resource_ptr_table[record byte+6]
    xor bx,bx
    mov bl,[di+7]
    push bx
    mov bl,[di+6]
    shl bx,1
    shl bx,1
    les ax,dword ptr DGROUP:_resource_ptr_table[bx]
    push es
    push ax
; blit the sprite at (word[+2], y0 + word[+4])
    mov ax,[bp+4]
    add ax,[di+4]
    push ax
    push word ptr [di+2]
    call _gfx_copy_rect
    add sp,0ah
; optional extra strip copy when byte at +1Ah is set
    cmp byte ptr [di+1ah],0
    je skip_copy
    mov ax,[di+2]
    add ax,10h
    mov bx,[di+4]
    mov dl,[di+1ah]
    xor dh,dh
    sub bx,dx
    inc bx
    push bx
    push dx
    push ax
    call _gfx_vline
    add sp,6
skip_copy:
    pop cx
skip_record:
; advance to the next 20h-byte record and loop
    add di,20h
    loop loop_records
done:
    pop di
    pop si
    pop bp
    ret
_board_actors_draw endp

_TEXT ends
end
