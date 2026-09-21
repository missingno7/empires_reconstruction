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
_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

extrn _gfx_color_select:near
extrn _gfx_copy_rect:near
extrn _gfx_vline:near
extrn _stream_control_block_arm:near
extrn _rand:near
extrn _record_field_skip_n:near
extrn _board_run_unit_script:near
extrn _board_advance_unit_moves:near

public _sprite_script_frame_driver
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
_TEXT ends
end _sprite_script_frame_driver
