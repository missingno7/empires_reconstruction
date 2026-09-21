; Word alignment lets TLINK supply the preceding inter-module pad.
; M_4AA8_4EEB -- four contiguous sprite/window routines assembled as one
; hand-written TASM module: F_4AA8 (clip+wipe), F_4B0C (bytecode
; interpreter), F_4E9F (wipe-table walk), F_4EEB (draw-table walk).
; F_4AA8 (0x4CA8) starts at an even address with the pad TLINK supplies
; before a word-aligned module; F_4B0C (0x4D0C), F_4E9F (0x509F, odd) and
; F_4EEB (0x50EB, odd) all start where the routine before them in this same
; module left off, so none of them can be the head of its own word-aligned
; module.
_TEXT   segment word public 'CODE'
DGROUP  group   _DATA,_BSS
        assume  cs:_TEXT,ds:DGROUP
_TEXT   ends
_DATA   segment word public 'DATA'
d@      label   byte
_DATA   ends
_BSS    segment word public 'BSS'
b@      label   byte
_BSS    ends
_TEXT   segment word public 'CODE'

; F_4AA8 -- clip a rectangle to the play window (8..137h, 10h..9Fh) and hand
; the clipped rect to wipe() = f03B4, with the y origin biased by 0B8h.
;
; NOT a Turbo C translation unit: AX, BX, CX, DX hold four SIMULTANEOUSLY
; LIVE variables (x1, y1, x2, y2) across the four guard branches and the four
; clamps, and each is written in place (mov ax,8 / mov cx,137h / mov bx,10h /
; mov dx,9Fh).  TC 2.0 keeps a variable only in SI or DI, and it saves them;
; this body saves nothing but BP and uses exactly the four registers the
; Turbo C convention lets a callee clobber.  Hand-written assembly.
_play_window_wipe_clipped  proc    near
        push    bp
        mov     bp,sp
        mov     ax,[bp+4]               ; x1
        cmp     ax,137h
        jg      pw_out
        mov     bx,[bp+6]               ; y1
        cmp     bx,9Fh
        jg      pw_out
        mov     cx,[bp+8]               ; w
        add     cx,ax                   ; x2 = x1 + w - 1
        dec     cx
        cmp     cx,8
        jl      pw_out
        mov     dx,[bp+0Ah]             ; h
        add     dx,bx                   ; y2 = y1 + h - 1
        dec     dx
        cmp     dx,10h
        jl      pw_out
        cmp     ax,8
        jge     pw_x1ok
        mov     ax,8
pw_x1ok:
        cmp     cx,137h
        jle     pw_x2ok
        mov     cx,137h
pw_x2ok:
        cmp     bx,10h
        jge     pw_y1ok
        mov     bx,10h
pw_y1ok:
        cmp     dx,9Fh
        jle     pw_y2ok
        mov     dx,9Fh
pw_y2ok:
        sub     cx,ax                   ; w = x2 - x1 + 1
        inc     cx
        sub     dx,bx                   ; h = y2 - y1 + 1
        inc     dx
        push    bx
        push    ax
        push    dx
        push    cx
        add     bx,0B8h
        push    bx
        push    ax
        call    _gfx_wipe_rect
        add     sp,0Ch
pw_out:
        pop     bp
        ret
_play_window_wipe_clipped  endp

; F_4B0C -- the "sprite script" bytecode interpreter's per-frame driver.
; Walks the 20h-byte record table at 0B3AEh (count in its first byte, one
; record per active sprite/actor) and for each record: skips it if it names
; a different board (byte+1 != gBFBA), renders it directly if it's already
; "rendered" (byte 0 == 1), ticks a countdown timer at byte+0Ah, and
; otherwise resumes its bytecode program (16-bit offset at byte+0Dh into the
; same 0B3AEh table) through dispatch_opcode's jump table at 74Ah, one
; opcode per loop iteration until the program yields via
; save_program_counter (stores si back to byte+0Dh, re-checks the board id,
; and falls into the render/collision path). The opcode handlers cover
; relative/absolute jumps, delay countdowns, calls into F_CAF1/F_2A2D+
; F_338A/F_36F0, flag set/test, movement-limit and velocity setup, clamped
; and absolute position moves (with an optional redraw), enable/disable,
; bit-test-and-skip, and a random skip. near proc, no args/return; called
; once per interpreter tick; clobbers ax/bx/cx/dx/si/di, all pushed/popped.
_sprite_script_frame_driver proc near
 push bp
 mov bp,sp
 push si
 push di
 push bp
 cld
; select drawing color 0Fh; di = record-table cursor, cx = record count
 mov ax,0fh
 push ax
 call _gfx_color_select
 pop ax
 mov di,0b3aeh
 xor cx,cx
 mov cl,[di]
 or cx,cx
 jz interpreter_return
 inc di
record_loop:
; keep only records belonging to the current board (byte+1 == gBFBA)
 push cx
 xor ax,ax
 mov al,[di+1h]
 cmp ax,ds:[0bfbah]
 jnz next_record
 mov al,[di]
 cmp al,1h
 jz render_record
 cmp byte ptr [di+0ah],0h
 jnz decrement_delay
; resume the record's bytecode program at its saved offset
 mov si,[di+0dh]
 add si,0b3aeh
dispatch_opcode:
; fetch one opcode byte and jump through the handler table at 74Ah
 xor bx,bx
 lodsb
 mov bl,al
 shl bx,1
 add bx,74ah
 jmp [bx]
save_program_counter:
; yield: save si back into byte+0Dh, then re-check the board id before
; falling into the render/collision path below
 sub si,0b3aeh
 mov [di+0dh],si
 xor ax,ax
 mov al,[di+1h]
 cmp ax,ds:[0bfbah]
 jz update_record
next_record:
; advance to the next 20h-byte record
 pop cx
 add di,byte ptr +20h
 loop record_loop
interpreter_return:
 pop bp
 pop di
 pop si
 pop bp
 ret
decrement_delay:
; countdown not yet expired for this tick; nothing else to do this record
 dec byte ptr [di+0ah]
render_record:
; record already rendered (byte 0 == 1): just blit its sprite frame
 cmp byte ptr [di+8h],1h
 jz next_record
 xor bx,bx
 mov bl,[di+7h]
 push bx
 mov bl,[di+6h]
 shl bx,1
 shl bx,1
 les ax,[bx-4f84h]
 push es
 push ax
 push word ptr [di+4h]
 push word ptr [di+2h]
 call _gfx_copy_rect
 add sp,byte ptr +0ah
update_record:
; skip the redraw/collision path when already rendered
 cmp byte ptr [di+8h],1h
 jz next_record
; optional extra strip copy when byte+1Ah is set (mirrors F_4EEB's copy step)
 cmp byte ptr [di+1ah],0h
 jz collision_check
 mov ax,[di+2h]
 add ax,10h
 mov bx,[di+4h]
 mov dl,[di+1ah]
 xor dh,dh
 sub bx,dx
 inc bx
 push bx
 push dx
 push ax
 call _gfx_vline
 add sp,byte ptr +6h
collision_check:
; compute this record's bounding box (ax/cx = x range, dx/bp = y range)
; from its sprite dimensions table (40D4h) and position (byte+2/+4)
 xor bx,bx
 mov bl,[di+6h]
 shl bx,1
 add bl,[di+7h]
 shl bx,1
 shl bx,1
 mov ax,[bx+40d4h]
 add bx,byte ptr +2h
 mov cx,[bx+40d4h]
 xor dx,dx
 xchg dl,ch
 mov bp,dx
 mov dl,ah
 xor ah,ah
 mov bx,[di+2h]
 add ax,bx
 add cx,bx
 mov bx,[di+4h]
 add dx,bx
 add bp,bx
 mov bx,ds:[72eh]
 shl bx,1
 add bx,ds:[73ah]
 shl bx,1
 shl bx,1
 push si
 push di
; walk the auxiliary bounds table at 79Eh and test this box against it twice
; (once per axis pair); any overlap failure jumps straight to skip_collision
 mov si,bx
 add si,79eh
 mov di,ax
 lodsw
 xor bx,bx
 xchg bl,ah
 add ax,ds:[736h]
 cmp ax,cx
 jg skip_collision
 add bx,ds:[738h]
 cmp bx,bp
 jg skip_collision
 lodsw
 xor bx,bx
 xchg bl,ah
 add ax,ds:[736h]
 cmp di,ax
 jg skip_collision
 add bx,ds:[738h]
 cmp dx,bx
 jg skip_collision
; overlap confirmed: latch this record as the active collision target
; (ds:[40CEh]) the first time one is found this tick
 mov ax,di
 pop di
 pop si
 cmp word ptr ds:[40ceh],0h
 jnz check_special_flags
 pop bx
 push bx
 mov ds:[40ceh],bx
check_special_flags:
; byte+19h selects special-case handling (e.g. explosion/death trigger),
; gated by byte+1Bh (already handled) and, for case 1, global flag ds:[72Ch]
 mov bl,[di+19h]
 or bl,bl
 jz check_bounds
 cmp byte ptr [di+1bh],1h
 jz check_bounds
 cmp bl,1h
 jnz reset_fields
 cmp byte ptr ds:[72ch],0h
 jz check_bounds
reset_fields:
; clear this record's per-instance state words, mark it handled (byte+1Bh),
; and restart its bytecode program from the offset stored at byte+17h
 xor bx,bx
 mov [di+0fh],bx
 mov [di+11h],bx
 mov [di+13h],bx
 mov [di+15h],bx
 inc bl
 mov [di+1bh],bl
 mov si,[di+17h]
 mov [di+0dh],si
 jmp short check_bounds
skip_collision:
 mov ax,di
 pop di
 pop si
check_bounds:
; optional level-bounds clamp: only when enabled (g8FEh) and not already
; delayed; drop the record outside the level's tile-map/scroll extent
 cmp byte ptr ds:[8feh],0h
 jz advance_record
 cmp byte ptr [di+0ah],0h
 jnz advance_record
 mov bx,ds:[0c04eh]
 shl bx,1
 push bx
 mov bx,[bx-3fb0h]
 cmp bx,ax
 pop ax
 jl advance_record
 cmp bx,cx
 jg advance_record
 mov bx,ax
 mov ax,[bx-3f80h]
 cmp ax,dx
 jl advance_record
 cmp ax,bp
 jg advance_record
 mov al,[di+9h]
 mov [di+0ah],al
advance_record:
 jmp near ptr next_record
set_program_counter:
; unconditional jump opcode: bytecode target already loaded into si; save
; it and fall into the render/collision path (does not re-enter dispatch)
 sub si,0b3aeh
 mov [di+0dh],si
 jmp near ptr render_record
; relative bytecode jump: si += word operand
opcode_add_offset:
 lodsw
 add si,ax
 jmp near ptr dispatch_opcode
; subroutine call: save return offset at byte+0Fh, then jump
opcode_store_offset:
 lodsw
 mov [di+0fh],si
 add si,ax
 jmp near ptr dispatch_opcode
; subroutine return: resume at the offset saved by opcode_store_offset
opcode_restore_offset:
 mov si,[di+0fh]
 jmp near ptr dispatch_opcode
; three countdown-timer opcode variants selecting a different per-record
; timer slot (bx = di+11h/+13h/+15h), then sharing opcode_countdown
 mov bx,11h
 jmp short opcode_countdown
 mov bx,13h
 jmp short opcode_countdown
 mov bx,15h
opcode_countdown:
; decrement the selected timer word; while running, reload it (ax) and skip
; the following "skip" block (cx words of bytecode) each tick; once it hits
; zero, fall through and execute that block
 lodsw
 mov cx,ax
 lodsw
 dec word ptr [bx+di]
 jz countdown_done
 jns countdown_skip
 mov [bx+di],ax
countdown_skip:
 add si,cx
countdown_done:
 jmp near ptr dispatch_opcode
; call F_CAF1(byte operand)
opcode_call_caf1:
 xor ah,ah
 lodsb
 push ax
 call _stream_control_block_arm
 pop ax
 jmp near ptr dispatch_opcode
; call F_2A2D(byte operand) then F_338A(dx, result)
opcode_dispatch_2a2d:
 xor ah,ah
 lodsb
 push ax
 call _record_field_skip_n
 pop cx
 push dx
 push ax
 call _board_run_unit_script
 pop cx
 pop cx
 jmp near ptr dispatch_opcode
; call F_36F0(byte operand)
opcode_dispatch_36f0:
 xor ah,ah
 lodsb
 push ax
 call _board_advance_unit_moves
 pop ax
 jmp near ptr dispatch_opcode
; two opcode variants (set flag = 1 / clear flag = 0) sharing the rest of
; the handler: operand byte selects a global flag slot (bit7 set) at
; 0B3AFh or a per-record flag byte (bit7 clear) at di+operand*20h
opcode_set_flag:
 mov dl,1h
 jmp short set_flag_read_index
 mov dl,0h
set_flag_read_index:
 xor ah,ah
 lodsb
 cmp al,80h
 jnc set_flag_local
 mov bx,ax
 shl bx,1
 shl bx,1
 shl bx,1
 shl bx,1
 shl bx,1
 add bx,0b3afh
 mov [bx],dl
 jmp near ptr dispatch_opcode
set_flag_local:
 and al,7fh
 mov bx,ax
 shl bx,1
 shl bx,1
 shl bx,1
 shl bx,1
 shl bx,1
 add bx,di
 mov [bx],dl
 jmp near ptr dispatch_opcode
; set the record's frame-index clamp range (low = byte+0Bh, high = byte+0Ch)
opcode_set_limits:
 lodsw
 mov [di+0bh],al
 mov [di+0ch],ah
 jmp near ptr dispatch_opcode
; set the record's current frame (byte+6h) and direction bit (byte+7h) from
; a signed operand byte doubled then halved (sign-extends via shl/shr pair)
opcode_set_velocity:
 xor ax,ax
 lodsb
 shl ax,1
 shr al,1
 mov [di+6h],al
 mov [di+7h],ah
 jmp near ptr dispatch_opcode
; move the record by a signed (dx,dy) delta operand, then advance its frame
; by a further signed delta and clamp it to [byte+0Bh, byte+0Ch]
opcode_move_clamped:
 mov dx,[di+2h]
 mov cx,[di+4h]
 lodsw
 mov bl,ah
 cbw
 add dx,ax
 mov al,bl
 cbw
 add cx,ax
 mov [di+2h],dx
 mov [di+4h],cx
 xor ax,ax
 lodsb
 shl ax,1
 shr al,1
 mov [di+7h],ah
 add al,[di+6h]
 cmp al,[di+0bh]
 jl clamp_high
 cmp al,[di+0ch]
 jg clamp_low
; store the (clamped) frame, optionally redraw the sprite immediately, then
; save the bytecode program counter and yield for this tick
velocity_store:
 mov [di+6h],al
 cmp byte ptr [di+8h],1h
 jz move_no_redraw
 xor bx,bx
 mov bl,ah
 push bx
 mov bl,al
 shl bx,1
 shl bx,1
 les ax,[bx-4f84h]
 push es
 push ax
 push cx
 push dx
 call _gfx_copy_rect
 add sp,byte ptr +0ah
move_no_redraw:
 jmp near ptr save_program_counter
clamp_high:
 mov al,[di+0ch]
 jmp short velocity_store
clamp_low:
 mov al,[di+0bh]
 jmp short velocity_store
; set the record's absolute position from a packed (x,y) operand, then set
; the frame from a further signed delta operand (no clamp)
opcode_set_position:
 lodsw
 xor cx,cx
 mov dx,cx
 mov dl,al
 mov cl,ah
 shl dx,1
 mov [di+2h],dx
 mov [di+4h],cx
 xor ax,ax
 lodsb
 shl ax,1
 shr al,1
 mov [di+7h],ah
 jmp short velocity_store
; set the record's absolute position and an explicit frame index/direction,
; redrawing (via velocity_store) only if the record's board id still
; matches gBFBA, otherwise just yielding without a redraw
opcode_set_position_and_frame:
 lodsw
 xor cx,cx
 mov dx,cx
 mov dl,al
 mov cl,ah
 shl dx,1
 mov [di+2h],dx
 mov [di+4h],cx
 lodsw
 mov [di+1h],ah
 xor bx,bx
 xchg bl,ah
 shl ax,1
 shr al,1
 mov [di+7h],ah
 cmp bx,ds:[0bfbah]
 jz velocity_store
 jmp short move_no_redraw
; mark the record active/rendered and clear its "special handled" flag,
; then save the program counter and yield
opcode_enable:
 mov byte ptr [di+8h],1h
 mov byte ptr [di+1bh],0h
 jmp near ptr save_program_counter
; mark the record inactive (not rendered)
opcode_disable:
 mov byte ptr [di+8h],0h
 jmp near ptr dispatch_opcode
; eight condition-test opcode variants: each reads a word/byte operand,
; tests a bit or compares a limit, and branches to opcode_skip_bytes (skip
; the following bytecode block) when the condition holds, else continues
; straight through to the next opcode. First four test bits 7/1/2/3 (07h)
; or bit 10h of a per-actor-class flag byte at gtab[operand]+4374h; the
; remaining four compare an operand against the scroll-position words at
; ds:[736h]/ds:[738h].
opcode_test_flags:
 lodsw
 mov bx,ax
 test byte ptr [bx+4374h],7h
 jz opcode_skip_bytes
 jmp near ptr dispatch_opcode
 lodsw
 mov bx,ax
 test byte ptr [bx+4374h],7h
 jnz opcode_skip_bytes
 jmp near ptr dispatch_opcode
 lodsw
 mov bx,ax
 test byte ptr [bx+4374h],10h
 jnz opcode_skip_bytes
 jmp near ptr dispatch_opcode
 lodsw
 mov bx,ax
 test byte ptr [bx+4374h],10h
 jz opcode_skip_bytes
 jmp near ptr dispatch_opcode
 xor ax,ax
 lodsb
 shl ax,1
 cmp word ptr ds:[736h],ax
 jng opcode_skip_bytes
 jmp near ptr dispatch_opcode
 xor ax,ax
 lodsb
 shl ax,1
 cmp word ptr ds:[736h],ax
 jnc opcode_skip_bytes
 jmp near ptr dispatch_opcode
 xor ax,ax
 lodsb
 cmp word ptr ds:[738h],ax
 jng opcode_skip_bytes
 jmp near ptr dispatch_opcode
 xor ax,ax
 lodsb
 cmp word ptr ds:[738h],ax
 jnc opcode_skip_bytes
 jmp near ptr dispatch_opcode
; skip a block of bytecode: operand byte indexes a length table at 782h,
; si += that length
opcode_skip_bytes:
 xor bx,bx
 lodsb
 mov bl,al
 mov bl,[bx+782h]
 add si,bx
 jmp near ptr dispatch_opcode
; roll a random value via F_F90F and skip the following block (as above)
; unless it is >= the operand byte
opcode_random_skip:
; random skip: rand() from the pinned CC.LIB (F_F90F)
 call _rand
 mov bx,ax
 lodsb
 cmp bl,al
 jna opcode_skip_bytes
 jmp near ptr dispatch_opcode
_sprite_script_frame_driver endp

; F_4E9F -- walk the 20h-stride record table at gB3AE (count in its first
; byte) and, for every record whose byte at +1 equals gBFBA and whose byte at
; +8 is not 1, call f4AA8 with the record's two words at +2 / +4 and two bytes
; looked up in the parallel tables at gBF66 and g9BFC by the record's byte +6.
;
; NOT a Turbo C translation unit: DI is the record cursor and CX the loop
; count, both live across the call and preserved by an explicit push cx /
; pop cx around it; TC 2.0 would spill the count to a stack local.
_sprite_table_wipe_active  proc    near
        push    bp
        mov     bp,sp
        push    si
        push    di
        mov     di,offset DGROUP:_actor_record_table
        xor     cx,cx
        mov     cl,[di]
        or      cx,cx
        je      wa_out
        inc     di
wa_top:  xor     ax,ax
        mov     al,[di+1]
        cmp     ax,_board_record_index
        jne     wa_next
        cmp     byte ptr [di+8],1
        je      wa_next
        push    cx
        xor     ax,ax
        mov     al,[di+6]
        mov     bx,ax
        mov     al,[bx+_tile_height_table]
        push    ax
        mov     al,[bx+_tile_width_table]
        push    ax
        mov     ax,[di+4]
        push    ax
        mov     ax,[di+2]
        push    ax
        call    _play_window_wipe_clipped
        add     sp,8
        pop     cx
wa_next: add     di,20h
        loop    wa_top
wa_out:  pop     di
        pop     si
        pop     bp
        ret
_sprite_table_wipe_active  endp

; F_4EEB -- walk the record table at actor_record_table (count in its first byte, 20h-byte
; records) and, for every record whose word at +1 equals board_record_index
; and whose byte at +8 is not 1, blit its sprite via gfx_copy_rect (color 0Fh
; selected first through gfx_color_select) at (word[+2], y0+word[+4]) using
; the far pointer looked up in resource_ptr_table[record byte +6 * 4]; when
; the record's byte at +1Ah is nonzero, additionally copy a strip of that
; sprite via gfx_vline.  Matches src/F_4EEB.C's original Turbo C inline-asm
; recovery (kept for provenance); this file restates it with labels instead
; of raw db bytes and TASM syntax.
;
; near proc, compact-model C calling convention: word arg y0 at [bp+4],
; no return value. Clobbers ax/bx/cx/dx; si/di/bp preserved via push/pop.
_board_actors_draw proc near
    push bp
    mov bp,sp
; select drawing color 0Fh for the sprite blits below
    mov ax,0fh
    push ax
    call _gfx_color_select
    pop ax
    push si
    push di
; di = cursor into the record table; cx = record count (first byte)
    mov di,offset DGROUP:_actor_record_table
    xor cx,cx
    mov cl,[di]
    or cx,cx
    je done
    inc di
loop_records:
; keep only records for the current board (word at +1 == board_record_index)
; and not flagged "skip" (byte at +8 == 1)
    xor ax,ax
    mov al,[di+1]
    cmp ax,word ptr DGROUP:_board_record_index
    jne skip_record
    cmp byte ptr [di+8],1
    je skip_record
    push cx
; look up the sprite's far pointer via resource_ptr_table[record byte+6]
    xor bx,bx
    mov bl,[di+7]
    push bx
    mov bl,[di+6]
    shl bx,1
    shl bx,1
    les ax,dword ptr DGROUP:_resource_ptr_table[bx]
    push es
    push ax
; blit the sprite at (word[+2], y0 + word[+4])
    mov ax,[bp+4]
    add ax,[di+4]
    push ax
    push word ptr [di+2]
    call _gfx_copy_rect
    add sp,0ah
; optional extra strip copy when byte at +1Ah is set
    cmp byte ptr [di+1ah],0
    je skip_copy
    mov ax,[di+2]
    add ax,10h
    mov bx,[di+4]
    mov dl,[di+1ah]
    xor dh,dh
    sub bx,dx
    inc bx
    push bx
    push dx
    push ax
    call _gfx_vline
    add sp,6
skip_copy:
    pop cx
skip_record:
; advance to the next 20h-byte record and loop
    add di,20h
    loop loop_records
done:
    pop di
    pop si
    pop bp
    ret
_board_actors_draw endp

_TEXT ends
        public  _play_window_wipe_clipped
        public  _sprite_script_frame_driver
        public  _sprite_table_wipe_active
        public  _board_actors_draw
        extrn   _gfx_wipe_rect:near
        extrn   _gfx_color_select:near
        extrn   _gfx_copy_rect:near
        extrn   _gfx_vline:near
        extrn   _stream_control_block_arm:near
        extrn   _rand:near
        extrn   _record_field_skip_n:near
        extrn   _board_run_unit_script:near
        extrn   _board_advance_unit_moves:near
        extrn   _actor_record_table:byte
        extrn   _board_record_index:word
        extrn   _tile_height_table:byte
        extrn   _tile_width_table:byte
        extrn   _resource_ptr_table:dword
        end
