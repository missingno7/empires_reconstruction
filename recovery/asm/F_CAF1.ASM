; F_CAF1 -- initialize sound-tick state for one pending stream entry.
_TEXT segment byte public 'CODE'
assume cs:_TEXT, ds:DGROUP
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
extrn _sound_enabled:word, _g1760:word, _g1e8a:word, _snd_base:word
extrn _g1e88:word, _snd_on:word, _g1e8e:word, _stream_note_delay:word
extrn _g1e86:word, _g1e90:word, _g1e94:word, _g1e8c:word
_BSS ends
DGROUP group _DATA, _BSS
public _stream_control_block_arm
_stream_control_block_arm:
 push bp
 mov bp,sp
 push ax
 push cx
 push si
 push di
 push es
 cmp _sound_enabled,0
 jz short done
 mov es,_g1760
 mov di,[bp+4]
 cmp di,_g1e8a
 ja short done
 mov _g1e8a,di
 shl di,1
 add di,_snd_base
 mov ax,es:[di]
 add ax,_snd_base
 mov _g1e88,ax
 mov _snd_on,2
 mov _g1e8e,6
 mov _stream_note_delay,1
 xor ax,ax
 mov _g1e86,ax
 mov _g1e90,ax
 mov _g1e94,ax
 mov _g1e8c,ax
done:
 pop es
 pop di
 pop si
 pop cx
 pop ax
 pop bp
 ret
_TEXT ends
end
