; F_C755 -- reset per-voice state, then service each configured voice.
;
; The names retain their DGROUP-offset provenance.  F_C27D and F_C834
; independently establish the same sound-state offsets and F_C6B9 entry.

_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

public _f_c755
extrn _f_c6b9:near
extrn _g1784:word
extrn _g1786:word
extrn _g177a:word
extrn _g17ac:word

_f_c755 proc near
    push bp
    mov bp,sp
    mov word ptr DGROUP:_g1784,0
    mov word ptr DGROUP:_g1786,-1
    xor si,si
    mov cx,DGROUP:_g177a
reset_voice:
    mov word ptr [si+DGROUP:_g17ac],0
    call _f_c6b9
    add si,2
    loop reset_voice
    pop bp
    ret
_f_c755 endp

_TEXT ends
end
