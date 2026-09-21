; M_988F_98CB -- paired fixed-rectangle display helpers.
_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
 extrn _score_panel_y:word, _score_panel_x:word
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP
extrn _gfx_copy_rect_flip_h:near, _gfx_wipe_rect:near
public _marker_cell_draw_highlight, _marker_cell_draw_plain, _f_98cb_end

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



_marker_cell_draw_plain:
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
 mov ax,1
 push ax
 call $+6ad4h
 add sp,0ch
 push word ptr ds:[0c350h]
 push word ptr ds:[0c34eh]
 mov ax,1dh
 push ax
 mov ax,28h
 push ax
 mov ax,168h
 push ax
 mov ax,50h
 push ax
 call $+6ab3h
 add sp,0ch
_f_98cb_end:
 ret

_TEXT ends
end
