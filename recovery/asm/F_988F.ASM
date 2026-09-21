; Original entry F_988F -- draw two fixed rectangles using recovered BSS coordinates.

_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

public _marker_cell_draw_highlight
extrn _gfx_copy_rect_flip_h:near
extrn _gfx_wipe_rect:near
extrn _score_panel_y:word
extrn _score_panel_x:word

_marker_cell_draw_highlight proc near
    mov ax,168h
    push ax
    mov ax,50h
    push ax
    mov ax,1dh
    push ax
    mov ax,28h
    push ax
    mov ax,168h
    push ax
    xor ax,ax
    push ax
    call _gfx_copy_rect_flip_h
    add sp,0ch
    push word ptr ds:_score_panel_y
    push word ptr ds:_score_panel_x
    mov ax,1dh
    push ax
    mov ax,28h
    push ax
    mov ax,168h
    push ax
    mov ax,50h
    push ax
    call _gfx_wipe_rect
    add sp,0ch
    ret
_marker_cell_draw_highlight endp

_TEXT ends
end
