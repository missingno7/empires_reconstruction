; F_6036 -- traverse the sprite record table at objtab (count byte first,
; 3-byte records: word offset/id, byte flags) and, for each one, look up its
; frame within sprbase (byte flags & 1Fh, scaled by 1E6h, +2, added to the
; sprbase far pointer), then call draw_queue_append(id=bp, x=bx, y=dx, color=0Fh,
; height=1Eh) to fetch/validate the sprite frame, and append a compact draw
; record via gfx_copy_rect(x=shl(bx,1), y=dx+0B8h, ptr=es:di, w=0Fh-derived, flag=0).
; bp is a running sprite-slot counter starting at 2Fh, incremented once per
; sprite. Leaves g0a20 = 10 once the table is exhausted.
; near proc, no args/return; matches src/F_6036.C's Turbo C inline-asm body.
_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
 extrn _sprbase:dword, _objtab:dword, _g0a20:word
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP
extrn _draw_queue_append:near, _gfx_copy_rect:near
public _sprite_table_queue_draws
_sprite_table_queue_draws proc near
 push bp
 mov bp,sp
 push si
 push di
 push ds
 cld
; bp = running sprite-slot counter, es:di = sprbase, ds:si = objtab,
; cx = record count (first byte of objtab)
 mov bp,2fh
 les di,dword ptr ds:_sprbase
 lds si,dword ptr ds:_objtab
 lodsb
 xor ch,ch
 mov cl,al
sprite_loop:
 push di
 inc bp
; read the record: bx = word offset/id, al = flag byte
 lodsw
 mov bx,ax
 lodsb
; locate the sprite frame within sprbase: es:di += (al&1Fh)*1E6h + 2
 xor ah,ah
 and al,1fh
 mov dx,1e6h
 mul dx
 add ax,2
 add di,ax
; dx = bh (high byte of bx), bh cleared
 xor dx,dx
 mov dl,bh
 mov bh,dh
 push ds
 push cx
 push es
; fetch/validate the sprite frame: draw_queue_append(id=bp, x=bx, y=dx, color=0Fh, height=1Eh)
 mov ax,DGROUP
 mov ds,ax
 mov ax,1eh
 push ax
 mov ax,0fh
 push ax
 push dx
 push bx
 push bp
 call _draw_queue_append
 pop ax
 pop bx
 pop dx
 add sp,4
 pop es
 push es
; append a compact draw record: gfx_copy_rect(x=bx*2, y=dx+0B8h, ptr=es:di, flag=0)
 xor ax,ax
 push ax
 push es
 push di
 add dx,0b8h
 push dx
 shl bx,1
 push bx
 call _gfx_copy_rect
 add sp,0ah
 pop es
 pop cx
 pop ds
 pop di
 loop sprite_loop
; mark the draw list ready
 mov word ptr ds:_g0a20,10
 pop ds
 pop di
 pop si
 pop bp
 ret
_sprite_table_queue_draws endp
_TEXT ends
end