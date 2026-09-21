; Original entry F_C5B3 -- select a word from the DS:1832h table by AL,
; then store it through SI at DS:17C4h.

_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

public _sound_table_word_select_store

_sound_table_word_select_store proc near
    push bp
    mov bp,sp
    mov bl,al
    xor bh,bh
    shl bx,1
    mov bx,[bx+1832h]
    mov [si+17c4h],bx
    pop bp
    ret
_sound_table_word_select_store endp

_TEXT ends
end
