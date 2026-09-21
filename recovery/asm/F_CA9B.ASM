; Original entry F_CA9B -- shift a note-derived value and update sound state.
; The 17FCh table base is a recovered literal whose data owner remains open.

_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

public _stream_note_program
extrn _speaker_gate_off:near
extrn _pit_channel2_set_divisor:near
extrn _speaker_gate_on:near
extrn _stream_note_delay:word
extrn _g1e90:word

_stream_note_program proc near
    push bp
    mov bp,sp
    mov dl,es:[di+1]
    xor dh,dh
    cmp dx,0
    jne shift_note
    call _speaker_gate_off
    jmp short store_current_value
    nop
shift_note:
    mov cl,7
    shl dx,cl
    neg dx
    mov cl,ah
    xor bx,bx
    mov bx,17fch
    mov ax,[bx]
    add ax,dx
    shr ax,cl
    call _pit_channel2_set_divisor
    call _speaker_gate_on
store_current_value:
    mov ax,ds:_stream_note_delay
    mov ds:_g1e90,ax
    pop bp
    ret
_stream_note_program endp

_TEXT ends
end
