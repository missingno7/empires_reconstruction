; F_C914 -- decode one command from the secondary music stream.
_TEXT segment byte public 'CODE'
assume cs:_TEXT, ds:DGROUP
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
extrn _mus_ptr:word, _mus_arg:word, _mus_flag:word, _snd_delay:word
_BSS ends
DGROUP group _DATA, _BSS
extrn _sound_note_dispatch:near, _f_c9a4:near, _speaker_gate_off:near, _f_ca03:near
extrn _fca9b:near, _fcaf1:near, _sound_stop_reset:near
public _fc914
_fc914:
 push bp
 mov bp,sp
; Fetch the next command byte from the stream and split it into a low
; nibble (command code, AL) and high nibble (argument, AH).
C914_dispatch: mov di,_mus_ptr
 mov al,es:[di]
 mov ah,al
 shr ah,1
 shr ah,1
 shr ah,1
 shr ah,1
 and al,0fh
 cmp al,0
 jz short C914_command_zero
 cmp al,0dh
 jz short C914_command_d
 cmp al,0eh
 jz short C914_command_e
 cmp al,0fh
 jz short C914_command_f
; Default path: ordinary note command.
 call _sound_note_dispatch
 call _f_c9a4
 jmp short C914_advance
 nop
C914_command_zero: call _speaker_gate_off
 call _f_c9a4
 jmp short C914_advance
 nop
C914_command_d: call _f_ca03
 cmp _snd_delay,0
 jmp short C914_advance
 nop
C914_command_e: call _fca9b
 jmp short C914_advance
 nop
C914_command_f: call _speaker_gate_off
 cmp _mus_flag,0
 jz short C914_stop
 mov ax,_mus_arg
 push ax
 call _fcaf1
 pop ax
 inc _snd_delay
 pop bp
 ret
C914_stop: call _sound_stop_reset
 pop bp
 ret
C914_advance: add _mus_ptr,2
 cmp _snd_delay,0
 jz short C914_dispatch
 pop bp
 ret
_TEXT ends
end
