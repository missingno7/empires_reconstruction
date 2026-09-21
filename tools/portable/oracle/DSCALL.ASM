; DSCALL.ASM -- DS-switching thunk for the historical oracle harness.
;
; RUNTIME_BLOCK.ASM primitives address the game's DGROUP through absolute
; DS-relative offsets (ds:[3924h], ds:[40C8h], ds:[0BCh], ...).  ORACLE.C
; runs in its OWN DGROUP (Turbo C compact model: DS is the program's data
; segment for the whole run).  To call a gfx_* primitive against a fake
; DGROUP block, DS must be switched to that block for the duration of the
; call only -- no C code may run while DS points away from the real DGROUP.
;
; int ds_call(unsigned seg, void (near *fn)(), int nwords, int near *words);
;
;   1. Push words[nwords-1..0] (so words[0] ends up at the callee's [bp+4],
;      matching normal left-to-right C parameter layout when the caller
;      built `words[]` in declaration order).
;   2. Set DS = seg.
;   3. CALL fn (near; callee uses push bp/mov bp,sp and a plain RET, so the
;      caller -- this thunk -- cleans the stack afterward).
;   4. Discard the pushed words, restore the real DS, return AX (the
;      primitive's return value for gfx_draw_char/gfx_get_pixel; ignored
;      otherwise).
;
; No C statement executes between step 2 and the DS restore in step 4: the
; thunk is the only code that runs while DS is switched.

_TEXT   segment byte public 'CODE'
        assume  cs:_TEXT

        public  _ds_call
_ds_call proc near
        push    bp
        mov     bp,sp
        push    si
        push    di
        push    ds                      ; save the real (ORACLE.C) DS

        mov     bx,[bp+0ah]             ; words (near ptr; DS is still real here)
        mov     cx,[bp+8]               ; nwords
        ; 8086 addressing allows only [bx/bp + si/di]; cx cannot index, so the
        ; word array is walked with si while cx counts the `loop` iterations.
        or      cx,cx
        jz      no_args
        mov     si,cx
        dec     si
        shl     si,1                    ; si = (nwords-1)*2 = last word's byte index
pushloop:
        push    word ptr [bx+si]
        sub     si,2
        loop    pushloop                ; pushes words[nwords-1..0], decrements cx to 0
no_args:
        mov     ax,[bp+4]               ; seg
        mov     si,[bp+6]               ; fn (near code pointer)
        mov     ds,ax                   ; DS now names the fake DGROUP
        call    si                      ; the only code that runs with DS switched
        mov     cx,[bp+8]
        shl     cx,1
        add     sp,cx                   ; drop the args this thunk pushed

        pop     ds                      ; restore the real DS
        pop     di
        pop     si
        pop     bp
        ret                             ; caller (Turbo C) cleans its own args
_ds_call endp

_TEXT   ends
        end
