; SOUND.ASM -- the sound driver's one hand-written TASM translation unit,
; original file offsets 0xC3A0..0xCD5C.
;
; This module used to be reconstructed as 18 separate production modules: 11
; TASM fragments (M_C1A0_C232, M_C27D_C567, M_C5A8_C5C6, M_C5D1_C706,
; F_C755, M_C77A_C898, F_C8D4, F_C914, M_C9A4_CA91, F_CA9B, F_CAF1) plus 7
; tiny "C" files (SNDSCL4, SNDTICK, SNDNOTE, SPKON, SPKOFF, PITDIV2,
; SNDSTOP) whose entire bodies are inline asm, kept alive as C translation
; units only by an invented -k flag or a declared-but-ignored parameter
; solely to obtain a push bp/mov bp,sp frame from the compiler.  The byte
; evidence says all 18 are one contiguous, hand-written assembler module:
;
;   - every routine here opens push bp/mov bp,sp and closes pop bp/ret,
;     including the seven former "C" wrappers -- their C signatures exist
;     for no reason but to force that exact frame;
;   - _sound_tick_entry (the tick entry point) explicitly saves and restores
;     AX/CX/DX/BX/SI/DI/ES around its body, the signature of a hand-written
;     routine protecting a caller's registers across an unknown-effect
;     call, not of a C-ABI function (which would rely on the callee-saves
;     convention instead);
;   - SI and DI are read and written throughout this cluster as an
;     implicit per-voice index and a stream cursor, by routines that never
;     save them -- Turbo C always wraps inline asm naming SI/DI in
;     push si/push di, so their total absence here is only possible in
;     genuine, hand-written assembler;
;   - calls from one routine in this cluster to another were encoded by
;     the original assembler as bare same-segment relative displacements
;     with no OMF fixup at all (here plain `call _label` to a proc of this
;     file, which assembles to the same fixup-free relative call) -- only
;     possible when caller and callee were assembled together, in the same
;     module;
;   - _opl_register_write's OPL data-port write ends in "rept 35 / in al,dx / endm", a
;     macro-generated settling delay a hand assembler reaches for and a C
;     compiler's inline-asm expander does not produce.
;
; See REPORT.md next to this file for the full former-source-to-proc table
; and the extern symbols this module still reaches outside the cluster.

_TEXT   segment byte public 'CODE'
_TEXT   ends
_DATA   segment word public 'DATA'
; Every data symbol below is initialized DATA (D*.OBJ), declared inside _DATA so
; that `offset _notetab` immediates get a DGROUP-consistent frame (TLINK reports
; a fixup overflow for an offset immediate declared under _BSS).
    extrn _v_a:word, _v_b:word, _v_ctr:word, _v_hold:word, _v_len:word
    extrn _snd_flag2:word, _snd_mode:word, _snd_backend_mode:word
    extrn _snd_seg2:word, _snd_nvoices:word, _snd_hi:word
    extrn _voice_stream_base_table:word, _voice_stream_cursor_table:word
    extrn _g1784:word, _g1786:word, _g1788:word, _g178a:word
    extrn _g179c:word, _g177a:word
    extrn _g17a4:word, _g17ac:word, _g17b4:word, _g17bc:word, _voice_rest_table:word, _g17f4:word
    extrn _opl_port:word
    extrn _mus_ptr:word, _mus_arg:word, _mus_flag:word
    extrn _snd_delay:word, _snd_seg:word, _snd_on:word, _snd_len:word, _snd_one:word
    extrn _notetab:word
    extrn _sound_enabled:word
    extrn _music_enabled:word
    extrn _snd_base:word, _snd_base2:word, _g1e84:word, _g1e86:word, _g1e88:word
    extrn _g1e8a:word, _g1e8c:word, _stream_note_delay:word
_DATA   ends
_BSS    segment word public 'BSS'
_BSS    ends
DGROUP  group _DATA,_BSS
        assume cs:_TEXT, ds:DGROUP

_TEXT   segment byte public 'CODE'
    extrn _voice_bank_retune_on:near, _voice_bank_update:near     ; OPLVOICE.C
    extrn _opl_init:near                                          ; OPLINIT.C
    extrn _record_panel_rebuild:near                              ; RECTTAB.C
        public _sound_tick_entry, _sound_voice_pump_loop, _sound_voice_table_prime
        public _sound_voice_service_loop, _sound_command_stream_dispatch, _sound_control_value_select, _sound_command_value_derive, _sound_control_block_advance, _sound_secondary_cmd_dispatch, _sound_command_flags_update, _voice_percent_scale_store
        public _sound_param_scale4
        public _f_c5a8, _sound_table_word_select_store, _f_c5c6
        public _voice_command_decode_apply, _voice_enable, _voice_disable, _sound_pit_divisor_program
        public _sound_voices_reset_and_service
        public _sound_backend_select_init, _sound_voice_table_reload, _sound_voices_reset, _sound_voices_disable_all, _opl_register_write
        public _opl_port_write_byte
        public _sound_tick_step
        public _sound_stream_command_step
        public _sound_note_dispatch
        public _sound_stream_delay_decode, _sound_ctlblock_command_dispatch, _sound_ctlblock_flags_latch, _stream_percent_scale_store, _stream_base_value_set, _stream_note_delay_set
        public _stream_note_program
        public _speaker_gate_on
        public _speaker_gate_off
        public _pit_channel2_set_divisor
        public _stream_control_block_arm
        public _sound_stop_reset

; ============================================================================
; former asm/M_C1A0_C232.ASM -- contiguous sound-tick, voice-pump, and
; voice-table scanner.  The first two entries call directly into the
; following entries and all three share the 0x1764..0x17F4 sound-control
; BSS cluster.
; ============================================================================

; F_C1A0 -- sound-tick entry point: save the caller's registers, drive one
; sound_tick_step, then pump ES:_snd_seg2 voices unless newly paused.
_sound_tick_entry	PROC	NEAR
	push	bp
	mov	bp,sp
	push	ax
	push	cx
	push	dx
	push	bx
	push	si
	push	di
	push	es
	xor	ax,ax
	mov	word ptr _v_b,ax
	mov	word ptr _v_b+2,ax
	mov	word ptr _v_b+4,ax
	mov	word ptr _v_b+6,ax
	cmp	word ptr _snd_on,0
	je	tick_l2
	call	near ptr _sound_tick_step
	cmp	word ptr _snd_mode,0
	je	tick_l3
	cmp	word ptr _snd_backend_mode,0
	jne	tick_l1
	mov	word ptr _v_b,1
tick_l1:	mov	es,word ptr _snd_seg2
	call	near ptr _sound_voice_pump_loop
	jmp	tick_l3
tick_l2:	cmp	word ptr _snd_mode,0
	je	tick_l3
	mov	es,word ptr _snd_seg2
	call	near ptr _sound_voice_pump_loop
tick_l3:	pop	es
	pop	di
	pop	si
	pop	bx
	pop	dx
	pop	cx
	pop	ax
	pop	bp
	ret
_sound_tick_entry	ENDP

; F_C1F7 -- pump loop: rescan the voice table in mode 2, dispatch one
; command batch, then optionally freeze the just-read state into _voice_stream_cursor_table.
_sound_voice_pump_loop	PROC	NEAR
	push	bp
	mov	bp,sp
	cmp	word ptr _snd_mode,2
	je	pump_l2
pump_l1:	call	near ptr _sound_voice_table_prime
pump_l2:	call	near ptr _sound_voice_service_loop
	cmp	word ptr _snd_mode,0
	jne	pump_l3
	mov	ax,word ptr _snd_flag2
	cmp	ax,0
	je	pump_l3
	mov	ax,word ptr _voice_stream_base_table
	mov	word ptr _voice_stream_cursor_table,ax
	mov	ax,word ptr _voice_stream_base_table+2
	mov	word ptr _voice_stream_cursor_table+2,ax
	mov	ax,word ptr _voice_stream_base_table+4
	mov	word ptr _voice_stream_cursor_table+4,ax
	mov	ax,word ptr _voice_stream_base_table+6
	mov	word ptr _voice_stream_cursor_table+6,ax
	jmp	pump_l1
pump_l3:	pop	bp
	ret
_sound_voice_pump_loop	ENDP

; F_C232 -- voice-table scanner: (re)prime every configured voice's
; SI-relative control slots from the ES:[si+_voice_stream_cursor_table] entry table.
_sound_voice_table_prime proc near
    push bp
    mov bp,sp
    xor ax,ax
    mov bx,2
    mov dx,6
    mov ds:_g1784,bx
    mov ds:_g178a,ax
    mov cx,ds:_g177a
    xor si,si
scan_entry:
    mov ds:[si+_g17ac],ax
    mov di,ds:[si+_voice_stream_cursor_table]
    cmp byte ptr es:[di],0ffh
    je next_entry
    mov ds:[si+_g17ac],bx
    mov ds:[si+_g179c],ax
    mov word ptr ds:[si+_g17a4],1
    mov ds:[si+_g17b4],ax
    mov ds:[si+_voice_rest_table],ax
    mov ds:[si+_g17f4],ax
    mov ds:[si+_g17bc],dx
next_entry:
    add si,2
    loop scan_entry
    pop bp
    ret
_sound_voice_table_prime endp

; ============================================================================
; former asm/M_C27D_C567.ASM -- contiguous sound service, command dispatch,
; and control helpers.
; ============================================================================

; command_cursor, command_value, command_guard, command_state and command_index
; each duplicated an existing DGROUP extern (_voice_stream_cursor_table, _v_ctr,
; _voice_rest_table, _v_a and _g17f4 respectively); retired in favor of the
; extern name used directly below.
; voice_pending_table is the sole surviving name for DS:17E4 -- no data symbol
; was ever declared for it (state_mode/command_mode were two equates for the
; same unbound address); kept as a plain equate, not an extern.
voice_pending_table equ 17e4h

; F_C27D -- per-voice service loop: advance each voice's counter, retrigger
; command dispatch and the voice-off entry as their timers expire, and
; drive the paused/idle renderer once every voice has been serviced.
_sound_voice_service_loop	PROC	NEAR
	push	bp
	mov	bp,sp
	xor	si,si
L1:	cmp	word ptr [si+_v_a],0
	je	L3
	cmp	word ptr [si+_v_ctr],0
	jne	L2
	call	near ptr _sound_command_stream_dispatch
	cmp	word ptr [si+_v_a],0
	je	L3
L2:	dec	word ptr [si+_v_ctr]
	cmp	word ptr _snd_backend_mode,1
	je	L3
	cmp	word ptr [si+_v_hold],1
	je	L3
	mov	ax,word ptr [si+_v_len]
	dec	ax
	cmp	word ptr [si+_v_ctr],ax
	jne	L3
	call	near ptr _voice_disable
L3:	add	si,2
	mov	ax,si
	shr	ax,1
	cmp	ax,word ptr _snd_nvoices
	jl	L1
	mov	ax,word ptr _v_a
	or	ax,word ptr _v_a+2
	or	ax,word ptr _v_a+4
	or	ax,word ptr _v_a+6
	jne	L4
	call	near ptr _sound_voices_reset_and_service
	jmp	L5
L4:	cmp	word ptr _snd_backend_mode,1
	jne	L5
	call	near ptr _sound_control_block_advance
L5:	pop	bp
	ret
_sound_voice_service_loop	ENDP

; F_C2EA -- command-stream dispatcher: split the byte at [si+_voice_stream_cursor_table]
; into a low-nibble opcode and act on it (0/D/E/F get dedicated handlers,
; anything else falls through to F_C359 + F_C3DB), then loop while more
; command bytes remain.
_sound_command_stream_dispatch:
    push bp
    mov bp, sp
C2EA_dispatch:
    mov di, [si+_voice_stream_cursor_table]
    mov al, es:[di]
    mov ah, al
    shr ah, 1
    shr ah, 1
    shr ah, 1
    shr ah, 1
    and al, 0fh
    cmp al, 0
    jz short C2EA_command_zero
    cmp al, 0dh
    jz short C2EA_command_d
    cmp al, 0eh
    jz short C2EA_command_e
    cmp al, 0fh
    jz short C2EA_command_f
    call _sound_control_value_select
    call _sound_command_value_derive
    mov word ptr [si+_voice_rest_table], 0
    jmp short C2EA_advance
    nop

C2EA_command_zero:
    call _voice_disable
    call _sound_command_value_derive
    mov word ptr [si+_voice_rest_table], 1
    jmp short C2EA_advance
    nop

C2EA_command_d:
    call _sound_secondary_cmd_dispatch
    cmp word ptr [si+_v_ctr], 0
    jmp short C2EA_advance
    nop

C2EA_command_e:
    call _voice_command_decode_apply
    jmp short C2EA_advance
    nop

C2EA_command_f:
    mov word ptr [si+_v_a], 0
    call _voice_disable
    jmp short C2EA_done
    nop

C2EA_advance:
    add word ptr [si+_voice_stream_cursor_table], 2
    cmp word ptr [si+_v_ctr], 0
    jz short C2EA_dispatch
C2EA_done:
    pop bp
    ret

; F_C359 -- select and submit one value from the sound control state.
; This routine consumes AL/AH/SI from its caller.  Its direct near calls
; were emitted without OMF fixups by the original compiler, so the same-
; segment offsets are deliberately retained as symbolic relative
; expressions.

; The two note tables addressed here are the same DS:17FC table SNDNOTE.C
; (F_C988) and F_CA9B index by note number, and a second, unnamed table at
; DS:1814 used only for the paused-mode path below.
voice_table_b equ 1814h            ; DS:1814, second note/frequency table

_sound_control_value_select:
    push bp
    mov bp, sp
    cmp word ptr ds:_snd_backend_mode, 1
    jz short C359_paused_mode
    cmp word ptr ds:_snd_backend_mode, 2
    jz short C359_alternate_table

    mov cl, ah
    mov bx, offset _notetab
    dec al
    shl al, 1
    xor ah, ah
    add bx, ax
    mov ax, [bx]
    shr ax, cl
    call _sound_pit_divisor_program
    call _voice_enable
    jmp short C359_done
    nop

C359_alternate_table:
    dec al
    xor cx, cx
    mov cl, ah
    shl cx, 1
    shl cx, 1
    mov bx, cx
    shl cx, 1
    add cx, bx
    add al, cl
    xor ah, ah
    call _sound_pit_divisor_program
    call _voice_enable
    jmp short C359_done
    nop

C359_paused_mode:
    cmp si, 6
    jz short C359_reset_voice
    cmp ah, 0
    jz short C359_select_table_b
    dec ah
C359_select_table_b:
    mov cl, ah
    mov bx, voice_table_b
    dec al
    shl al, 1
    xor ah, ah
    add bx, ax
    mov ax, [bx]
    shr ax, cl
    cmp ax, 3ffh
    jng short C359_submit_value
    shr ax, 1
C359_submit_value:
    call _sound_pit_divisor_program
    jmp short C359_done
    nop

C359_reset_voice:
    cmp word ptr [si+_v_b], 0
    jnz short C359_done
    dec al
    add al, 0e0h
    call _opl_port_write_byte
C359_done:
    pop bp
    ret

; F_C3DB -- derive a command value from ES:DI state and update its selector.
; Register inputs (SI, ES:DI) are part of this dispatch cluster's internal
; calling convention.  This contribution has no OMF fixups.

; Both _g1788 and _g178a are word memory operands.  TASM 1.0 discards the
; memory type of an EQU in these contexts, so retain typed absolute
; operands.

_sound_command_value_derive:
    push bp
    mov bp, sp
    mov al, es:[di+1]
    mov ah, al
    mov bx, word ptr ds:_g1788
    add bx, word ptr ds:_g178a
    test ah, 80h
    jz short C3DB_decode_flags
    and al, 7fh
    jmp short C3DB_store_value
C3DB_decode_flags:
    and al, 7
    mov cl, al
    shr bx, cl
    test ah, 8
    jz short C3DB_test_index_flag
    mov dx, bx
    shr dx, 1
    add bx, dx
C3DB_test_index_flag:
    test ah, 10h
    jz short C3DB_increment_index
    cmp word ptr [si+_g17f4], 3
    jz short C3DB_store_value
    cmp word ptr [si+_g17f4], 1
    jz short C3DB_force_index_three
    inc word ptr [si+_g17f4]
    jmp short C3DB_store_value
    nop
C3DB_force_index_three:
    mov word ptr [si+_g17f4], 3
    jmp short C3DB_store_value
    nop
C3DB_increment_index:
    cmp word ptr [si+_g17f4], 0
    jz short C3DB_store_value
    inc word ptr [si+_g17f4]
C3DB_store_value:
    mov [si+_v_ctr], bx
    mov word ptr [si+voice_pending_table], 1
    pop bp
    ret

; F_C440 -- advance three two-byte slots in the 177Ch control-state block.
; This owner has no OMF relocations; the local renderer call remains a
; same-segment relative expression to preserve that historical object
; topology.

; state_ready, state_mode, state_index, state_guard, state_allow, state_limit
; and state_value each duplicated an existing DGROUP extern (_v_a,
; voice_pending_table, _g17f4, _voice_rest_table, _v_hold, _v_len and _v_ctr
; respectively); retired in favor of the extern/equate name used directly
; below.  state_active, state_cursor_mode, state_cursor, state_cursor_next and
; state_text have no corresponding extern and stay as equates.
state_active equ 177ch
state_cursor_mode equ 17d4h
state_cursor equ 17c4h
state_cursor_next equ 17cch
state_text equ 17dch

_sound_control_block_advance:
    push bp
    mov bp, sp
    xor si, si
    mov bl, 90h
C440_slot_loop:
    cmp word ptr [si+state_active], 0
    jz short C440_slot_ready
    jmp C440_slot_done
C440_slot_ready:
    cmp word ptr [si+_v_a], 0
    jnz short C440_slot_mode
    jmp C440_slot_done
C440_slot_mode:
    cmp word ptr [si+voice_pending_table], 1
    jnz short C440_alternate_mode
    mov word ptr [si+voice_pending_table], 0
    cmp word ptr [si+_g17f4], 2
    jl short C440_reset_cursor
    cmp word ptr [si+_g17f4], 3
    jz short C440_select_char
    mov word ptr [si+_g17f4], 0
    jmp short C440_select_char
    nop
C440_reset_cursor:
    mov word ptr [si+state_cursor_mode], 0
    mov cx, [si+state_cursor]
    mov [si+state_cursor_next], cx
    jmp short C440_select_char
    nop
C440_alternate_mode:
    cmp word ptr [si+_voice_rest_table], 1
    jnz short C440_alternate_index
    mov al, 0fh
    jmp short C440_render
    nop
C440_alternate_index:
    cmp word ptr [si+_g17f4], 1
    jz short C440_select_char
    cmp word ptr [si+_g17f4], 3
    jz short C440_select_char
    cmp word ptr [si+_v_hold], 1
    jz short C440_select_char
    mov cx, [si+_v_len]
    cmp [si+_v_ctr], cx
    jng short C440_render_blank
C440_select_char:
    cmp word ptr [si+state_cursor_mode], 0
    jz short C440_read_char
    cmp word ptr [si+state_cursor_mode], 1
    jg short C440_render_blank
    mov ax, [si+state_text]
    jmp short C440_render
    nop
C440_render_blank:
    mov al, 0fh
    jmp short C440_render
    nop
C440_read_char:
    mov di, [si+state_cursor_next]
    inc word ptr [si+state_cursor_next]
    mov al, [di]
    cmp al, 0ffh
    jnz short C440_render
    mov word ptr [si+state_cursor_mode], 1
    jmp short C440_slot_done
    nop
C440_render:
    add al, bl
    call _opl_port_write_byte
C440_slot_done:
    add bl, 20h
    add si, 2
    cmp si, 6
    jg short C440_done
    jmp C440_slot_loop
C440_done:
    pop bp
    ret

; F_C501 -- dispatch one secondary stream command by the AH selector.
; The six helper calls are direct same-segment relationships in the
; original no-fixup object, retained here as symbolic relative
; expressions.
_sound_secondary_cmd_dispatch:
 push bp
 mov bp,sp
 mov al,es:[di+1]
 cmp ah,0
 jnz short C501_test_two
 call _sound_command_flags_update
 jmp short C501_done
 nop
C501_test_two:
 cmp ah,2
 jg short C501_test_three
 call _voice_percent_scale_store
 jmp short C501_done
 nop
C501_test_three:
 cmp ah,3
 jnz short C501_test_four
 call _f_c5a8
 jmp short C501_done
 nop
C501_test_four:
 cmp ah,4
 jnz short C501_test_five
 call _sound_param_scale4
 jmp short C501_done
 nop
C501_test_five:
 cmp ah,5
 jnz short C501_test_six
 call _sound_table_word_select_store
 jmp short C501_done
 nop
C501_test_six:
 cmp ah,6
 jnz short C501_done
 call _f_c5c6
C501_done:
 pop bp
 ret

; F_C549 -- update the paired command-control flags from AL.
_sound_command_flags_update proc near
    push bp
    mov bp, sp
    cmp al, 0
    je short C549_set_one_plus_one
    mov word ptr ds:[si+_g17b4], 0
    xor ah, ah
    mov ds:[si+17bch], ax                  ; DS:17BC
    jmp short C549_done
C549_set_one:
    nop
C549_set_one_plus_one:
    mov word ptr ds:[si+_g17b4], 1
C549_done:
    pop bp
    ret
_sound_command_flags_update endp

; F_C567 -- signed scale-and-store for the sound-data state pair.
_voice_percent_scale_store proc near
    push bp
    mov bp, sp
    mov bl, ah
    mov cl, al
    xor ch, ch
    cmp cx, 0
    je short C567_store_zero
    mov ax, ds:_g1788
    mul cx
    mov cx, 100
    div cx
    cmp dx, 31h
    jle short C567_no_round_up
    inc ax
C567_no_round_up:
    cmp bl, 1
    je short C567_store_result
    neg ax
C567_store_result:
    mov ds:_g178a, ax
    jmp short C567_done
    nop
C567_store_zero:
    mov word ptr ds:_g178a, 0
C567_done:
    pop bp
    ret
_voice_percent_scale_store endp

; ============================================================================
; former src/SNDSCL4.C -- F_C59A.  TC frame with a hand-written body, the
; F_CA83 shape: the frame is forced by the function HAVING A PARAMETER,
; which the asm body ignores and reads out of AL directly.
; ============================================================================

; F_C59A -- scale a raw 0..63 value by 4 and store it through _g1788.
_sound_param_scale4 proc near
    push bp
    mov bp,sp
    xor ah,ah
    shl ax,1
    shl ax,1
    mov word ptr ds:_g1788,ax
    pop bp
    ret
_sound_param_scale4 endp

; ============================================================================
; former asm/M_C5A8_C5C6.ASM -- adjacent setters for the SI-relative
; command-control state.
; ============================================================================

; F_C5A8 -- store AL (zero-extended) through SI at DS:_g17a4.
_f_c5a8 proc near
    push bp
    mov bp,sp
    xor ah,ah
    mov [si+_g17a4],ax
    pop bp
    ret
_f_c5a8 endp

; F_C5B3 -- select a word from the DS:1832h table by AL, then store it
; through SI at DS:17C4h.
_sound_table_word_select_store proc near
    push bp
    mov bp,sp
    mov bl,al
    xor bh,bh
    shl bx,1
    mov bx,[bx+1832h]                      ; DS:1832, unnamed word table
    mov [si+17c4h],bx                      ; DS:17C4
    pop bp
    ret
_sound_table_word_select_store endp

; F_C5C6 -- store AL (zero-extended) through SI at DS:17DCh.
_f_c5c6 proc near
    push bp
    mov bp,sp
    xor ah,ah
    mov [si+17dch],ax                      ; DS:17DC
    pop bp
    ret
_f_c5c6 endp

; ============================================================================
; former asm/M_C5D1_C706.ASM -- sound command dispatch, speaker control,
; and PIT output.
;
; These four adjacent entries form one 388-byte no-fixup contribution.  The
; historical compiler encoded calls outside this contribution as direct
; same-segment displacements, retained below as symbolic relative
; expressions.
; ============================================================================

; voice_retune_base_table -- per-voice queued PIT-divisor/tone value awaiting
; delivery to the 9-voice OPL bank; no data symbol exists at DS:1766, so this
; stays a plain equate rather than an extern.
voice_retune_base_table equ 1766h

; F_C5D1 -- decode one command byte and update the selected SI-relative
; voice-control slot.
_voice_command_decode_apply proc near
    push bp
    mov bp,sp
    mov dl,es:[di+1]
    xor dh,dh
    cmp dx,0
    jnz short C5D1_have_value
    call _voice_disable
    mov word ptr [si+_voice_rest_table],1
    jmp C5D1_store_result
C5D1_have_value:
    cmp word ptr ds:_snd_backend_mode,1
    jz short C5D1_mode_one
    cmp word ptr ds:_snd_backend_mode,2
    jz short C5D1_mode_two
    mov cl,7
    shl dx,cl
    neg dx
    mov cl,ah
    mov bx,word ptr ds:_snd_backend_mode
    shl bx,1
    mov bx,[bx+182ch]                      ; DS:182C, unnamed word table
    mov ax,[bx]
    add ax,dx
    shr ax,cl
    call _sound_pit_divisor_program
    call _voice_enable
    jmp short C5D1_store_result
    nop
C5D1_mode_two:
    mov word ptr [si+17e4h],1              ; DS:17E4
    jmp short C5D1_store_result
    nop
C5D1_mode_one:
    mov bx,dx
    shl bx,1
    add bx,dx
    shr bx,1
    shr bx,1
    inc bx
    mov cl,ah
    cmp cl,0
    jnz short C5D1_have_shift
    inc cl
C5D1_have_shift:
    mov dx,0d70h
    shr dx,cl
    cmp cl,4
    jz short C5D1_scaled
    jg short C5D1_shift_down
    mov al,4
    sub al,cl
    mov cl,al
    shl bx,cl
    jmp short C5D1_scaled
    nop
C5D1_shift_down:
    sub cl,4
    shr bx,cl
C5D1_scaled:
    sub dx,bx
    cmp dx,400h
    jng short C5D1_frequency_ready
    shr dx,1
C5D1_frequency_ready:
    mov ax,dx
    call _sound_pit_divisor_program
    mov word ptr [si+17e4h],1              ; DS:17E4
    mov word ptr [si+_voice_rest_table],0
C5D1_store_result:
    mov ax,[si+_g17a4]
    mov [si+_g179c],ax
    pop bp
    ret
_voice_command_decode_apply endp

; F_C678 -- enable a voice, either by opening the speaker gate or by
; queueing its selected value for the alternate sound backend.
_voice_enable proc near
    push bp
    mov bp,sp
    cmp word ptr [si+_v_b],0
    jnz short C678_done
    cmp word ptr ds:_snd_backend_mode,1
    jz short C678_open_alt_gate
    cmp word ptr ds:_snd_backend_mode,2
    jz short C678_queue_value
    in al,61h
    or al,3
    out 61h,al
    jmp short C678_done
    nop
C678_queue_value:
    push si
    push di
    push es
    mov ax,[si+voice_retune_base_table]
    push ax
    mov ax,si
    shr ax,1
    push ax
    call _voice_bank_retune_on
    pop ax
    pop ax
    pop es
    pop di
    pop si
    jmp short C678_done
    nop
C678_open_alt_gate:
    in al,61h
    or al,60h
    out 61h,al
C678_done:
    pop bp
    ret
_voice_enable endp

; F_C6B9 -- disable a voice or submit an alternate-backend voice update.
_voice_disable proc near
    push bp
    mov bp,sp
    cmp word ptr [si+_v_b],0
    jnz short C6B9_done
    cmp word ptr ds:_snd_backend_mode,1
    jz short C6B9_queue_reset
    cmp word ptr ds:_snd_backend_mode,2
    jz short C6B9_queue_stop
    in al,61h
    and al,0fch
    out 61h,al
    jmp short C6B9_done
    nop
C6B9_queue_stop:
    push si
    push di
    push es
    push cx
    mov ax,si
    shr ax,1
    push ax
    call _voice_bank_update
    pop ax
    pop cx
    pop es
    pop di
    pop si
    jmp short C6B9_done
    nop
C6B9_queue_reset:
    mov ax,si
    shl al,1
    shl al,1
    shl al,1
    shl al,1
    add al,90h
    or al,0fh
    call _opl_port_write_byte
    xor ax,ax
    call _sound_pit_divisor_program
C6B9_done:
    pop bp
    ret
_voice_disable endp

; F_C706 -- write a PIT divisor, queue it, or emit its two packed nibbles
; for the alternate sound backend.
_sound_pit_divisor_program proc near
    push bp
    mov bp,sp
    cmp word ptr [si+_v_b],0
    jnz short C706_done
    cmp word ptr ds:_snd_backend_mode,1
    jz short C706_emit_nibbles
    cmp word ptr ds:_snd_backend_mode,2
    jz short C706_store_queued
    out 42h,al
    mov al,ah
    out 42h,al
    jmp short C706_done
    nop
C706_store_queued:
    mov [si+voice_retune_base_table],ax
    jmp short C706_done
    nop
C706_emit_nibbles:
    mov bx,ax
    mov ax,si
    shl al,1
    shl al,1
    shl al,1
    shl al,1
    add al,80h
    mov ah,bl
    and ah,0fh
    or al,ah
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    call _opl_port_write_byte
    mov al,bl
    call _opl_port_write_byte
C706_done:
    pop bp
    ret
_sound_pit_divisor_program endp

; ============================================================================
; former src/F_C755.ASM -- reset per-voice state, then service each
; configured voice.
;
; The names retain their DGROUP-offset provenance.  F_C27D and F_C834
; independently establish the same sound-state offsets and F_C6B9 entry.
; ============================================================================

_sound_voices_reset_and_service proc near
    push bp
    mov bp,sp
    mov word ptr DGROUP:_g1784,0
    mov word ptr DGROUP:_g1786,-1
    xor si,si
    mov cx,DGROUP:_g177a
reset_voice:
    mov word ptr [si+DGROUP:_g17ac],0
    call _voice_disable
    add si,2
    loop reset_voice
    pop bp
    ret
_sound_voices_reset_and_service endp

; ============================================================================
; former asm/M_C77A_C898.ASM -- sound backend selection, voice-state
; reload, and OPL output.
; ============================================================================

; F_C77A -- select and initialise a sound backend.
_sound_backend_select_init proc near
    push bp
    mov bp,sp
    push ax
    mov ax,word ptr ds:_snd_backend_mode
    cmp ax,1
    jz short C77A_mode_one
    cmp ax,2
    jz short C77A_mode_two
    cmp ax,3
    jz short C77A_mode_three
    mov word ptr ds:_g177a,1
    jmp short C77A_done
    nop
C77A_mode_two:
    mov word ptr ds:_g177a,4
    call _opl_init
    jmp short C77A_done
    nop
C77A_mode_three:
    mov word ptr ds:_opl_port,205h
    mov word ptr ds:_g177a,4
    mov word ptr ds:_snd_backend_mode,1
    jmp short C77A_done
C77A_mode_one:
    mov word ptr ds:_opl_port,0c0h
    mov word ptr ds:_g177a,4
    call _voice_enable
C77A_done:
    pop ax
    pop bp
    ret
_sound_backend_select_init endp

; F_C7CB -- reload per-voice values from the current backend table into the
; two SI-relative sound-control arrays.
_sound_voice_table_reload proc near
    push bp
    mov bp,sp
    push ax
    push cx
    push si
    push di
    push es
    mov es,word ptr ds:_snd_seg2
    mov di,[bp+4]
    mov word ptr ds:_g1786,di
    cmp word ptr ds:_music_enabled,0
    jz short C7CB_done
    cmp word ptr ds:_snd_backend_mode,2
    jnz short C7CB_select_width
    push es
    push di
    call _record_panel_rebuild
    pop di
    pop es
    mov word ptr ds:_g177a,4
C7CB_select_width:
    mov cl,1
    cmp word ptr ds:_snd_backend_mode,0
    jz short C7CB_width_ready
    mov cl,3
C7CB_width_ready:
    shl di,cl
    xor si,si
    mov cx,word ptr ds:_g177a
    add di,word ptr ds:_snd_base2
C7CB_copy_voice:
    mov ax,es:[di]
    add ax,word ptr ds:_snd_base2
    mov [si+_voice_stream_cursor_table],ax
    mov [si+1794h],ax                      ; DS:1794, F_C7CB's second array
    add di,2
    add si,2
    loop C7CB_copy_voice
    mov word ptr ds:_g1784,1
C7CB_done:
    pop es
    pop di
    pop si
    pop cx
    pop ax
    pop bp
    ret
_sound_voice_table_reload endp

; F_C834 -- reset each configured voice and retain the voice count across
; the reset loop.
_sound_voices_reset proc near
    push bp
    mov bp,sp
    push ax
    push cx
    push si
    mov word ptr _snd_mode,0
    mov word ptr _snd_hi,-1
    xor si,si
    mov cx,_snd_nvoices
    push cx
    cmp word ptr _snd_backend_mode,2
    jnz short C834_reset_voice
    mov word ptr _snd_nvoices,4
C834_reset_voice:
    mov word ptr [si+_v_a],0
    mov word ptr [si+_v_b],0
    call _voice_disable
    add si,2
    loop C834_reset_voice
    pop _snd_nvoices
    pop si
    pop cx
    pop ax
    pop bp
    ret
_sound_voices_reset endp

; F_C877 -- submit one disabled voice update for every voice.
_sound_voices_disable_all proc near
    push bp
    mov bp,sp
    push cx
    push si
    xor si,si
    mov cx,_g177a
    cmp word ptr _snd_backend_mode,2
    jnz short C877_voice_count_ready
    mov cx,4
C877_voice_count_ready:
    call _voice_disable
    add si,2
    loop C877_voice_count_ready
    pop si
    pop cx
    pop bp
    ret
_sound_voices_disable_all endp

; F_C898 -- write OPL register and data bytes with the required
; input-port settling reads between them.
_opl_register_write proc near
    push bp
    mov bp,sp
    mov dx,_opl_port
    mov ax,[bp+4]
    out dx,al
    in al,dx
    in al,dx
    in al,dx
    in al,dx
    in al,dx
    in al,dx
    inc dx
    mov ax,[bp+6]
    out dx,al
    dec dx
    rept 35
        in al,dx
    endm
    pop bp
    ret
_opl_register_write endp

; ============================================================================
; former src/F_C8D4.ASM -- write the live AL value to the I/O port
; selected by the DS global _opl_port (the same OPL port as _opl_port above,
; under this contribution's original alias).
; ============================================================================

_opl_port_write_byte proc near
    push bp
    mov bp,sp
    push dx
    mov dx,word ptr ds:_opl_port
    out dx,al
    jmp short $+2
    pop dx
    pop bp
    ret
_opl_port_write_byte endp

; ============================================================================
; former src/SNDTICK.C -- F_C8E2, sound tick step.  Loads ES from a global
; (no C expression emits mov es,[mem]), so the body is asm; frame from the
; module's -k.
; ============================================================================

; F_C8E2 -- run one music-stream command via F_C914 when due, then age the
; current note's delay counter and gate the speaker off at its last tick.
_sound_tick_step proc near
    push bp
    mov bp,sp
    mov es,word ptr _snd_seg
    cmp word ptr _snd_delay,0
    jne C8E2_countdown
    call near ptr _sound_stream_command_step
    cmp word ptr _snd_on,0
    je  C8E2_done
C8E2_countdown:
    dec word ptr _snd_delay
    cmp word ptr _snd_one,1
    je  C8E2_done
    mov ax,word ptr _snd_len
    dec ax
    cmp word ptr _snd_delay,ax
    jne C8E2_done
    call near ptr _speaker_gate_off
C8E2_done:
    pop bp
    ret
_sound_tick_step endp

; ============================================================================
; former asm/F_C914.ASM -- decode one command from the secondary music
; stream.
; ============================================================================

; F_C914 -- fetch the next command byte, split it into a low nibble
; (command code, AL) and high nibble (argument, AH), dispatch to the
; matching handler, and loop while _snd_delay stays zero.
_sound_stream_command_step:
 push bp
 mov bp,sp
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

; ============================================================================
; former src/SNDNOTE.C -- F_C988, note dispatch in the sound engine.  Both
; arguments arrive in AH/AL; TC has no register convention, so the body is
; asm and the two declared parameters exist only to force the frame.
; ============================================================================

; F_C988 -- look up a divisor for (note=AL, shift=AH) in _notetab and
; program the PIT with it, then open the speaker gate.
_sound_note_dispatch proc near
    push bp
    mov bp,sp
    mov cl,ah
    mov bx,offset _notetab
    dec al
    shl al,1
    xor ah,ah
    add bx,ax
    mov ax,[bx]
    shr ax,cl
    call near ptr _pit_channel2_set_divisor
    call near ptr _speaker_gate_on
    pop bp
    ret
_sound_note_dispatch endp

; ============================================================================
; former asm/M_C9A4_CA91.ASM -- table-command decoder and four-way
; operation dispatcher.  The command byte at ES:DI+1 selects shared DGROUP
; state in the 1E84h..1E94h control block.  The dispatcher and all four
; operations are contiguous.
; ============================================================================

; F_C9A4 -- decode one ES:[DI+1] command byte into a scaled delta and store
; it through _snd_delay.
_sound_stream_delay_decode:
    push bp
    mov bp, sp
    mov al, es:[di+1]
    mov ah, al
    mov bx, word ptr ds:_g1e84
    add bx, word ptr ds:_g1e86
    test ah, 80h
    jz short decode_flags
    and al, 7fh
    jmp short store_value
decode_flags:
    and al, 7
    mov cl, al
    shr bx, cl
    test ah, 8
    jz short test_index_flag
    mov dx, bx
    shr dx, 1
    add bx, dx
test_index_flag:
    test ah, 10h
    jz short increment_index
    cmp word ptr ds:_g1e8c, 3
    jz short store_value
    cmp word ptr ds:_g1e8c, 1
    jz short force_index_three
    inc word ptr ds:_g1e8c
    jmp short store_value
    nop
force_index_three:
    mov word ptr ds:_g1e8c, 3
    jmp short store_value
    nop
increment_index:
    cmp word ptr ds:_g1e8c, 0
    jz short store_value
    inc word ptr ds:_g1e8c
store_value:
    mov word ptr ds:_snd_delay, bx
    pop bp
    ret

; F_CA03 -- dispatch one control-block command by the AH selector.
_sound_ctlblock_command_dispatch proc near
    push bp
    mov bp,sp
    mov al,es:[di+1]
    cmp ah,0
    jne check_two_or_less
    call _sound_ctlblock_flags_latch
    jmp short CA03_done
    nop
check_two_or_less:
    cmp ah,2
    jg check_three
    call _stream_percent_scale_store
    jmp short CA03_done
    nop
check_three:
    cmp ah,3
    jne check_four
    call _stream_note_delay_set
    jmp short CA03_done
    nop
check_four:
    cmp ah,4
    jne CA03_done
    call _stream_base_value_set
CA03_done:
    pop bp
    ret
_sound_ctlblock_command_dispatch endp

; F_CA35 -- latch or clear the control block's one-shot/length pair from AL.
_sound_ctlblock_flags_latch proc near
    push bp
    mov bp,sp
    cmp al,0
    je short set_one
    mov word ptr ds:_snd_one,0
    xor ah,ah
    mov ds:_snd_len,ax
    pop bp
    ret
set_one:
    mov word ptr ds:_snd_one,1
    pop bp
    ret
_sound_ctlblock_flags_latch endp

; F_CA51 -- signed percentage scale-and-store, the 1E84/1E86 twin of
; F_C567.
_stream_percent_scale_store proc near
    push bp
    mov bp,sp
    mov bl,ah
    mov cl,al
    xor ch,ch
    cmp cx,0
    je store_zero
    mov ax,ds:_g1e84
    mul cx
    mov cx,100
    div cx
    cmp dx,31h
    jle no_round_up
    inc ax
no_round_up:
    cmp bl,1
    je store_result
    neg ax
store_result:
    mov ds:_g1e86,ax
    pop bp
    ret
store_zero:
    mov word ptr ds:_g1e86,0
    pop bp
    ret
_stream_percent_scale_store endp

; F_CA83 -- scale a raw 0..63 value by 4 and store it through _g1e84.
_stream_base_value_set proc near
    push bp
    mov bp,sp
    xor ah,ah
    shl ax,1
    shl ax,1
    mov word ptr ds:_g1e84,ax
    pop bp
    ret
_stream_base_value_set endp

; F_CA91 -- store AL (zero-extended) through _stream_note_delay.
_stream_note_delay_set proc near
    push bp
    mov bp,sp
    xor ah,ah
    mov ds:_stream_note_delay,ax
    pop bp
    ret
_stream_note_delay_set endp

; ============================================================================
; former asm/F_CA9B.ASM -- shift a note-derived value and update sound
; state.
; ============================================================================

; F_CA9B -- look up a divisor in _notetab for a shifted note delta, program
; the PIT and speaker gate, then copy _stream_note_delay into _snd_delay.
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
    mov bx,offset _notetab
    mov ax,[bx]
    add ax,dx
    shr ax,cl
    call _pit_channel2_set_divisor
    call _speaker_gate_on
store_current_value:
    mov ax,ds:_stream_note_delay
    mov ds:_snd_delay,ax
    pop bp
    ret
_stream_note_program endp

; ============================================================================
; former src/SPKON.C -- F_CAD0, speaker gate on.  asm body; no parameter,
; so the frame is only there if the module was compiled -k.
; ============================================================================

; F_CAD0 -- open the PC-speaker gate (timer 2 output + speaker enable).
_speaker_gate_on proc near
    push bp
    mov bp,sp
    in  al,61h
    or  al,3
    out 61h,al
    pop bp
    ret
_speaker_gate_on endp

; ============================================================================
; former src/SPKOFF.C -- F_CADB, speaker gate off.  asm body, frame from
; the module's -k.
; ============================================================================

; F_CADB -- close the PC-speaker gate.
_speaker_gate_off proc near
    push bp
    mov bp,sp
    in  al,61h
    and al,0fch
    out 61h,al
    pop bp
    ret
_speaker_gate_off endp

; ============================================================================
; former src/PITDIV2.C -- F_CAE6, program PIT channel 2 with the divisor
; already in AX.  The parameter is declared (and ignored by the body)
; purely to force the frame.
; ============================================================================

; F_CAE6 -- write AX's low then high byte to PIT channel 2 (port 42h).
_pit_channel2_set_divisor proc near
    push bp
    mov bp,sp
    out 42h,al
    mov al,ah
    out 42h,al
    pop bp
    ret
_pit_channel2_set_divisor endp

; ============================================================================
; former asm/F_CAF1.ASM -- initialize sound-tick state for one pending
; stream entry.
; ============================================================================

; F_CAF1 -- when enabled and the caller's index is new, latch it, look up
; its far-pointer entry via _snd_base, and reset the 1E84..1E94 control
; block for that entry.
_stream_control_block_arm:
 push bp
 mov bp,sp
 push ax
 push cx
 push si
 push di
 push es
 cmp _sound_enabled,0
 jz short CAF1_done
 mov es,_snd_seg
 mov di,[bp+4]
 cmp di,_g1e8a
 ja short CAF1_done
 mov _g1e8a,di
 shl di,1
 add di,_snd_base
 mov ax,es:[di]
 add ax,_snd_base
 mov _g1e88,ax
 mov _snd_on,2
 mov _snd_len,6
 mov _stream_note_delay,1
 xor ax,ax
 mov _g1e86,ax
 mov _snd_delay,ax
 mov _snd_one,ax
 mov _g1e8c,ax
CAF1_done:
 pop es
 pop di
 pop si
 pop cx
 pop ax
 pop bp
 ret

; ============================================================================
; former src/SNDSTOP.C -- sound_stop_reset.  Genuine C (not asm-bodied);
; -k forces the frame TCC would otherwise omit from this three-statement
; leaf.
; ============================================================================

; F_CB48 -- clear the on-flag and invalidate the cached stream index, then
; close the speaker gate.
_sound_stop_reset proc near
    push bp
    mov bp,sp
    mov word ptr ds:_snd_on,0
    mov word ptr ds:_g1e8a,-1
    call _speaker_gate_off
    pop bp
    ret
_sound_stop_reset endp

_TEXT   ends
        end
