; M_6036_6181 -- three contiguous sprite-draw routines assembled as one
; hand-written TASM module: F_6036 (queue sprite draws), F_60A9 (animated-tile
; ticker), F_6181 (adjust a selected sprite record and issue two draw passes).
; F_6036 (0x6236) starts at an even address with no preceding pad needed;
; F_60A9 (0x62A9, odd) and F_6181 (0x6381, odd) both start where the routine
; before them in this same module left off, so neither can be the head of
; its own module.
_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

; F_6036 -- traverse the sprite record table at objtab (count byte first,
; 3-byte records: word offset/id, byte flags) and, for each one, look up its
; frame within sprbase (byte flags & 1Fh, scaled by 1E6h, +2, added to the
; sprbase far pointer), then call draw_queue_append(id=bp, x=bx, y=dx, color=0Fh,
; height=1Eh) to fetch/validate the sprite frame, and append a compact draw
; record via gfx_copy_rect(x=shl(bx,1), y=dx+0B8h, ptr=es:di, w=0Fh-derived, flag=0).
; bp is a running sprite-slot counter starting at 2Fh, incremented once per
; sprite. Leaves g0a20 = 10 once the table is exhausted.
; near proc, no args/return; matches src/F_6036.C's Turbo C inline-asm body.
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

; F_60A9 -- the animated-tile ticker.  Every 10th call it walks a
; length-prefixed table of 3-byte animation records at the far pointer
; g40D0, advances each record's 5-bit frame field up or down (bit 6 selects
; the direction, bit 7 arms the record), recomputes the tile's 1E6h-byte
; slot in the far sprite bank at gBFC8, and repaints the tile in both the
; play window and the 0B8h-biased mirror.
;
; NOT a Turbo C translation unit: BP is used as a DATA register (mov bp,dx /
; mov dx,bp) and then re-pointed at the stack mid-body (mov bp,sp /
; add word ptr [bp],0B8h) to rewrite an already-pushed argument between two
; calls; SI/DI are driven by lodsb/lodsw with DS reloaded by lds; and the
; loop count lives in CX across two calls.  Census R1-R4 + R6 + R7.
;
; The fixup at 6122h is a SEGMENT fixup, not a byte constant: the image's
; relocation table patches that word with the program's data paragraph
; (stored 0FA3h, image offset 0FA30h -- which is where the Borland
; "Turbo-C - Copyright (c) 1988" banner sits, i.e. the head of _DATA).
; So the source is `mov ax,DGROUP` -- the DS-restore an asm module must do
; after `lds` before it may call a C function.
_animated_tile_tick  proc    near
        push    bp
        mov     bp,sp
        push    si
        push    di
        push    ds
        cld
        cmp     _raycast_trail_active,0
        je      at_armed
        jmp     at_done
at_armed:
        dec     _ga20
        cmp     _ga20,0
        je      at_tick
        jmp     at_done
at_tick:
        mov     _ga20,0Ah
        les     di,_sprite_tile_bank
        lds     si,_g40d0
        lodsb
        xor     cx,cx
        mov     cl,al                   ; record count
at_rec:
        push    di
        lodsw
        xor     dx,dx
        mov     dl,ah                   ; y
        mov     bh,dh
        mov     bl,al
        shl     bx,1                    ; x*2
        mov     bp,dx
        lodsb                           ; flags/frame byte
        xor     ah,ah
        test    al,80h
        jne     at_active
        jmp     at_next
at_active:
        test    al,40h
        je      at_up
        and     al,1Fh
        dec     al
        jge     at_dnok
        mov     al,17h
at_dnok:
        jmp     short at_store
at_up:
        and     al,1Fh
        inc     al
        cmp     al,17h
        jle     at_store
        mov     al,0
at_store:
        and     byte ptr [si-1],0E0h
        or      byte ptr [si-1],al
        mov     dx,1E6h
        mul     dx
        add     ax,2
        add     di,ax
        mov     dx,bp
        push    ds
        push    cx
        push    es
        push    dx
        mov     ax,DGROUP
        mov     ds,ax
        push    dx
        mov     bp,sp
        push    bx
        mov     ax,1Eh
        push    ax
        mov     ax,1Eh
        push    ax
        mov     ax,dx
        add     ax,148h
        push    ax
        push    bx
        call    _gfx_wipe_rect
        add     word ptr [bp],0B8h
        call    _gfx_wipe_rect
        pop     bx
        add     sp,0Ah
        pop     dx
        pop     es
        push    es
        xor     ax,ax
        push    ax
        push    es
        push    di
        push    dx
        push    bx
        call    _gfx_copy_rect
        pop     bx
        pop     dx
        add     sp,6
        mov     ax,dx
        add     ax,0B8h
        push    ax
        push    bx
        mov     ax,1Eh
        push    ax
        mov     ax,1Eh
        push    ax
        push    dx
        push    bx
        call    _gfx_wipe_rect
        add     sp,0Ch
        pop     es
        pop     cx
        pop     ds
at_next:
        pop     di
        dec     cx
        jcxz    at_done
        jmp     at_rec
at_done:
        pop     ds
        pop     di
        pop     si
        pop     bp
        ret
_animated_tile_tick  endp

; F_6181 -- adjust the selected sprite record and issue two draw passes.
;
; TASM emits FIXUPP records in source-contribution order. Turbo C placed the
; DGROUP load before the two far-pointer records, even though its instruction
; is later in memory. The three ORG contributions preserve both that record
; order and the original contiguous _TEXT layout; their offsets are shifted
; by 0x14B (the combined length of F_6036+F_60A9 preceding this member in
; the merged module) from F_6181's own former module-relative 0/0x57/0x5c.

; Offset 0x1A2 (0x14B+0x57): emit the DGROUP FIXUPP record before the
; low-address externals.
org 1a2h
    mov ax, DGROUP
    mov ds, ax

; Offset 0x14B: sprite-record selection and state update.
org 14bh
_sprite_record_adjust_draw:
    push bp
    mov bp, sp
    push si
    push di
    push ds
    cld
    les di, dword ptr ds:[_sprbase]
    lds si, dword ptr ds:[_objtab]
    inc si
    mov ax, [bp+4]
    mov bx, ax
    shl ax, 1
    add si, ax
    add si, bx
    lodsw
    xor dx, dx
    mov dl, ah
    mov bh, dh
    mov bl, al
    shl bx, 1
    mov bp, dx
    mov al, [si]
    xor ah, ah
    test al, 40h
    jz short increment
    and al, 1fh
    dec al
    jnl short negative_done
    mov al, 17h
negative_done:
    jmp short store
increment:
    and al, 1fh
    inc al
    cmp al, 17h
    jng short store
    mov al, 0
store:
    and byte ptr [si], 0e0h
    or [si], al
    mov dx, 1e6h
    mul dx
    add ax, 2
    add di, ax
    mov dx, bp
    push es
    push dx

; Offset 0x1A7 (0x14B+0x5c): draw calls follow the DGROUP reload in memory
; and in FIXUPP order.
org 1a7h
    push dx
    mov bp, sp
    push bx
    mov ax, 1eh
    push ax
    mov ax, 1eh
    push ax
    mov ax, dx
    add ax, 148h
    push ax
    push bx
    call near ptr _gfx_wipe_rect
    add word ptr [bp], 0b8h
    call near ptr _gfx_wipe_rect
    pop bx
    add sp, 10
    pop dx
    pop es
    xor ax, ax
    push ax
    push es
    push di
    push dx
    push bx
    call near ptr _gfx_copy_rect
    pop bx
    pop dx
    add sp, 6
    mov ax, dx
    add ax, 0b8h
    push ax
    push bx
    mov ax, 1eh
    push ax
    mov ax, 1eh
    push ax
    push dx
    push bx
    call near ptr _gfx_wipe_rect
    add sp, 12
    pop ds
    pop di
    pop si
    pop bp
    ret

_TEXT ends
        public  _sprite_table_queue_draws
        public  _animated_tile_tick
        public  _sprite_record_adjust_draw
        extrn   _sprbase:dword
        extrn   _objtab:dword
        extrn   _draw_queue_append:near
        extrn   _gfx_copy_rect:near
        extrn   _g0a20:word
        extrn   _gfx_wipe_rect:near
        extrn   _g40d0:dword
        extrn   _raycast_trail_active:word
        extrn   _ga20:word
        extrn   _sprite_tile_bank:dword
        end
