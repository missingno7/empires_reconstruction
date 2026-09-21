; F_50D2 -- display-adapter detection, as a hand-written TASM module.
; Segment/EXTRN layout copied from what TCC itself emits (code first, EXTRN
; at the end), so every reference to _bbfcd is a forward reference: TASM
; sizes  mov _bbfcd,0  as a word on pass 1 and as a byte on pass 2, and pads
; the byte it saved with NOP.  Forward  jmp  is likewise sized near on pass 1
; and short on pass 2, and padded.  No NOP is written by hand.
_TEXT   segment byte public 'CODE'
DGROUP  group   _DATA,_BSS
        assume  cs:_TEXT,ds:DGROUP
_TEXT   ends
_DATA   segment word public 'DATA'
d@      label   byte
_DATA   ends
_BSS    segment word public 'BSS'
b@      label   byte
_BSS    ends
_TEXT   segment byte public 'CODE'
_video_adapter_detect  proc    near
        push    si
        mov     _bbfcd,0
        mov     ah,0fh
        int     10h
        cmp     al,7
        jne     @@rom_sig_probe
        jmp     @@done
@@rom_sig_probe:
        mov     ax,0f000h
        mov     es,ax
        mov     si,0fffeh
        cmp     byte ptr es:[si],0ffh
        jne     @@pc
        mov     si,0c000h
        cmp     byte ptr es:[si],21h
        jne     @@pc
        mov     _bbfcd,3
        mov     byte ptr _b856,1
        jmp     @@done
@@pc:
        mov     ax,1a00h
        int     10h
        cmp     al,1ah
        jne     @@ega
        cmp     bl,0ah
        jl      @@check_ega_class
        mov     _bbfcd,4
        jmp     @@done
@@check_ega_class:
        cmp     bl,4
        je      @@mark_ega
        cmp     bl,5
        jne     @@check_vga_class
@@mark_ega:
        mov     _bbfcd,1
        jmp     @@done
@@check_vga_class:
        cmp     bl,7
        je      @@mark_vga
        cmp     bl,8
        jne     @@check_mcga_class
@@mark_vga:
        mov     _bbfcd,5
        jmp     @@done
@@check_mcga_class:
        cmp     bl,2
        jne     @@ega
        mov     _bbfcd,2
        jmp     @@done
@@ega:
        mov     bh,0ffh
        mov     cl,0ffh
        mov     ah,12h
        mov     bl,10h
        int     10h
        cmp     bh,1
        ja      @@crtc
        cmp     cl,0fh
        ja      @@crtc
        mov     _bbfcd,1
        jmp     @@done
@@crtc:
        mov     dx,3d4h
        mov     al,0fh
        out     dx,al
        inc     dx
        in      al,dx
        mov     ah,al
        mov     al,66h
        out     dx,al
        mov     cx,100h
@@spin:
        loop    @@spin
        in      al,dx
        xchg    ah,al
        out     dx,al
        cmp     ah,66h
        jne     @@done
        mov     _bbfcd,2
@@done:
        cmp     word ptr _bbfcd,1
        jne     @@check_pcjr
        mov     ah,12h
        mov     bl,10h
        int     10h
        or      bl,bl
        jne     @@fin
        mov     _bbfcd,2
        jmp     @@fin
@@check_pcjr:
        cmp     word ptr _bbfcd,4
        jne     @@check_mcga
        jmp     @@fin
@@check_mcga:
        cmp     word ptr _bbfcd,5
        jne     @@fin
@@fin:
        pop     si
        ret
_video_adapter_detect  endp
_TEXT   ends
        extrn   _bbfcd:byte
        extrn   _b856:byte
        public  _video_adapter_detect

; Original entry F_53BF -- select a display mode and probe VGA availability.

_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP

public _sound_backend_probe
extrn _g1778:word
extrn _display_mode:word
extrn _opl_detect:near

_sound_backend_probe proc near
    mov word ptr ds:_g1778,0
    cmp word ptr ds:_display_mode,3
    jne probe_mode
    mov word ptr ds:_g1778,1
    jmp short selected

probe_mode:
    call _opl_detect
    or ax,ax
    jz bios_probe
    mov word ptr ds:_g1778,2
    jmp short selected

bios_probe:
    mov ah,0c0h
    int 15h
    or ah,ah
    jnz selected
    cmp word ptr es:[bx+2],0bfch
    jne selected

    mov cx,10
    mov dx,203h
wait_vga_port:
    mov al,0a5h
    out dx,al
    in al,dx
    cmp al,0a5h
    jne selected
    loop wait_vga_port
    mov word ptr ds:_g1778,3

selected:
    ret
_sound_backend_probe endp

_TEXT ends
end
