; F_C914 -- decode one command from the secondary music stream.
_TEXT segment byte public 'CODE'
assume cs:_TEXT, ds:DGROUP
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
extrn _mus_ptr:word, _mus_arg:word, _mus_flag:word, _snd_delay:word
_BSS ends
DGROUP group _DATA, _BSS
extrn _sound_note_dispatch:near, _sound_stream_delay_decode:near, _speaker_gate_off:near, _sound_ctlblock_command_dispatch:near
extrn _stream_note_program:near, _stream_control_block_arm:near, _sound_stop_reset:near
public _sound_stream_command_step
_sound_stream_command_step:
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
 call _sound_stream_delay_decode
 jmp short C914_advance
 nop
C914_command_zero: call _speaker_gate_off
 call _sound_stream_delay_decode
 jmp short C914_advance
 nop
C914_command_d: call _sound_ctlblock_command_dispatch
 cmp _snd_delay,0
 jmp short C914_advance
 nop
C914_command_e: call _stream_note_program
 jmp short C914_advance
 nop
C914_command_f: call _speaker_gate_off
 cmp _mus_flag,0
 jz short C914_stop
 mov ax,_mus_arg
 push ax
 call _stream_control_block_arm
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
