RT_CS equ 039ch ; Fixed loader ABI: replacement drivers are copied to CS:039C.
; Table addresses derive from instruction labels; only the loader ABI base is fixed.
; Production source. src/RUNTIME_BLOCK.C is the frozen migration oracle.
JMP_NEAR macro target
 db 0e9h
 dw target-$-2
endm

CALL_NEAR macro target
 db 0e8h
 dw target-$-2
endm

_TEXT segment byte public 'CODE'
_TEXT ends
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT,ds:DGROUP
_TEXT segment byte public 'CODE'
public _f039c
_f039c label near
; RUNTIME_BLOCK -- exact startup/runtime dispatch block.
    JMP_NEAR $+193h
    public _f039f
    _f039f label near
    public _box
    _box label near
    JMP_NEAR $+296h
    public _f03a2
    _f03a2 label near
    public _bar
    _bar label near
    JMP_NEAR $+0f38h
    public _f03a5
    _f03a5 label near
    JMP_NEAR $+0f7eh
    public _f03a8
    _f03a8 label near
    public _clear
    _clear label near
    JMP_NEAR $+0fb4h
    public _f03ab
    _f03ab label near
    public _fill
    _fill label near
    JMP_NEAR $+1028h
    public _f03ae
    _f03ae label near
    JMP_NEAR $+1094h
    public _f03b1
    _f03b1 label near
    JMP_NEAR $+10d8h
    public _f03b4
    _f03b4 label near
    public _wipe
    _wipe label near
    JMP_NEAR $+110ah
    public _f03b7
    _f03b7 label near
    JMP_NEAR $+1180h
    public _f03ba
    _f03ba label near
    JMP_NEAR $+11d6h
    public _f03bd
    _f03bd label near
    JMP_NEAR $+122eh
    public _f03c0
    _f03c0 label near
    JMP_NEAR $+1295h
    public _f03c3
    _f03c3 label near
    JMP_NEAR $+1333h
    public _f03c6
    _f03c6 label near
    JMP_NEAR $+13ddh
    public _f03c9
    _f03c9 label near
    public _blit
    _blit label near
    JMP_NEAR $+1566h
    public _f03cc
    _f03cc label near
    public _copy
    _copy label near
    JMP_NEAR $+16cbh
    public _f03cf
    _f03cf label near
    JMP_NEAR $+182ch
    public _f03d2
    _f03d2 label near
    JMP_NEAR $+18f7h
    public _f03d5
    _f03d5 label near
    JMP_NEAR $+1935h
; 0194 clears BH, loads display selector, doubles BX; 019C indexes CS:03D8. Five stored slots; selector is not locally bounded.
runtime_mode_dispatch label word
    dw 0,(rt_01a1-_f039c+RT_CS),0,(rt_0258-_f039c+RT_CS),(rt_0294-_f039c+RT_CS)
; 029A clears BH, loads display selector, doubles BX; 02A2 indexes CS:03E2. Mode 5 spills into the following table and remains unproven.
runtime_blit_dispatch label word
    dw 0,(rt_02a7-_f039c+RT_CS),0,(rt_07c5-_f039c+RT_CS),(rt_083b-_f039c+RT_CS)
; 02F8 reads CS:03EC+2*width; 02FD writes the rel16 at CS:0B5F; 0301 adds CS:0B61. 81 words: indices 0/1 alias, then 15-byte strides. Valid stored domain 0..80; caller width bounds remain unproven.
runtime_planar_entry_deltas label word
    dw rt_07a8-rt_07c5,rt_07a8-rt_07c5,rt_0799-rt_07c5,rt_078a-rt_07c5
    dw rt_077b-rt_07c5,rt_076c-rt_07c5,rt_075d-rt_07c5,rt_074e-rt_07c5
    dw rt_073f-rt_07c5,rt_0730-rt_07c5,rt_0721-rt_07c5,rt_0712-rt_07c5
    dw rt_0703-rt_07c5,rt_06f4-rt_07c5,rt_06e5-rt_07c5,rt_06d6-rt_07c5
    dw rt_06c7-rt_07c5,rt_06b8-rt_07c5,rt_06a9-rt_07c5,rt_069a-rt_07c5
    dw rt_068b-rt_07c5,rt_067c-rt_07c5,rt_066d-rt_07c5,rt_065e-rt_07c5
    dw rt_064f-rt_07c5,rt_0640-rt_07c5,rt_0631-rt_07c5,rt_0622-rt_07c5
    dw rt_0613-rt_07c5,rt_0604-rt_07c5,rt_05f5-rt_07c5,rt_05e6-rt_07c5
    dw rt_05d7-rt_07c5,rt_05c8-rt_07c5,rt_05b9-rt_07c5,rt_05aa-rt_07c5
    dw rt_059b-rt_07c5,rt_058c-rt_07c5,rt_057d-rt_07c5,rt_056e-rt_07c5
    dw rt_055f-rt_07c5,rt_0550-rt_07c5,rt_0541-rt_07c5,rt_0532-rt_07c5
    dw rt_0523-rt_07c5,rt_0514-rt_07c5,rt_0505-rt_07c5,rt_04f6-rt_07c5
    dw rt_04e7-rt_07c5,rt_04d8-rt_07c5,rt_04c9-rt_07c5,rt_04ba-rt_07c5
    dw rt_04ab-rt_07c5,rt_049c-rt_07c5,rt_048d-rt_07c5,rt_047e-rt_07c5
    dw rt_046f-rt_07c5,rt_0460-rt_07c5,rt_0451-rt_07c5,rt_0442-rt_07c5
    dw rt_0433-rt_07c5,rt_0424-rt_07c5,rt_0415-rt_07c5,rt_0406-rt_07c5
    dw rt_03f7-rt_07c5,rt_03e8-rt_07c5,rt_03d9-rt_07c5,rt_03ca-rt_07c5
    dw rt_03bb-rt_07c5,rt_03ac-rt_07c5,rt_039d-rt_07c5,rt_038e-rt_07c5
    dw rt_037f-rt_07c5,rt_0370-rt_07c5,rt_0361-rt_07c5,rt_0352-rt_07c5
    dw rt_0343-rt_07c5,rt_0334-rt_07c5,rt_0325-rt_07c5,rt_0316-rt_07c5
    dw rt_0307-rt_07c5
; 087B reads CS:048E+2*width; 0880 patches CS:12D9; 089A adds CS:12DB. 81 words: zero selects epilogue, then 21-byte strides. Valid stored domain 0..80; caller bounds remain unproven.
runtime_packed_entry_deltas label word
    dw rt_0f37-rt_0f3f,rt_0f1a-rt_0f3f,rt_0f05-rt_0f3f,rt_0ef0-rt_0f3f
    dw rt_0edb-rt_0f3f,rt_0ec6-rt_0f3f,rt_0eb1-rt_0f3f,rt_0e9c-rt_0f3f
    dw rt_0e87-rt_0f3f,rt_0e72-rt_0f3f,rt_0e5d-rt_0f3f,rt_0e48-rt_0f3f
    dw rt_0e33-rt_0f3f,rt_0e1e-rt_0f3f,rt_0e09-rt_0f3f,rt_0df4-rt_0f3f
    dw rt_0ddf-rt_0f3f,rt_0dca-rt_0f3f,rt_0db5-rt_0f3f,rt_0da0-rt_0f3f
    dw rt_0d8b-rt_0f3f,rt_0d76-rt_0f3f,rt_0d61-rt_0f3f,rt_0d4c-rt_0f3f
    dw rt_0d37-rt_0f3f,rt_0d22-rt_0f3f,rt_0d0d-rt_0f3f,rt_0cf8-rt_0f3f
    dw rt_0ce3-rt_0f3f,rt_0cce-rt_0f3f,rt_0cb9-rt_0f3f,rt_0ca4-rt_0f3f
    dw rt_0c8f-rt_0f3f,rt_0c7a-rt_0f3f,rt_0c65-rt_0f3f,rt_0c50-rt_0f3f
    dw rt_0c3b-rt_0f3f,rt_0c26-rt_0f3f,rt_0c11-rt_0f3f,rt_0bfc-rt_0f3f
    dw rt_0be7-rt_0f3f,rt_0bd2-rt_0f3f,rt_0bbd-rt_0f3f,rt_0ba8-rt_0f3f
    dw rt_0b93-rt_0f3f,rt_0b7e-rt_0f3f,rt_0b69-rt_0f3f,rt_0b54-rt_0f3f
    dw rt_0b3f-rt_0f3f,rt_0b2a-rt_0f3f,rt_0b15-rt_0f3f,rt_0b00-rt_0f3f
    dw rt_0aeb-rt_0f3f,rt_0ad6-rt_0f3f,rt_0ac1-rt_0f3f,rt_0aac-rt_0f3f
    dw rt_0a97-rt_0f3f,rt_0a82-rt_0f3f,rt_0a6d-rt_0f3f,rt_0a58-rt_0f3f
    dw rt_0a43-rt_0f3f,rt_0a2e-rt_0f3f,rt_0a19-rt_0f3f,rt_0a04-rt_0f3f
    dw rt_09ef-rt_0f3f,rt_09da-rt_0f3f,rt_09c5-rt_0f3f,rt_09b0-rt_0f3f
    dw rt_099b-rt_0f3f,rt_0986-rt_0f3f,rt_0971-rt_0f3f,rt_095c-rt_0f3f
    dw rt_0947-rt_0f3f,rt_0932-rt_0f3f,rt_091d-rt_0f3f,rt_0908-rt_0f3f
    dw rt_08f3-rt_0f3f,rt_08de-rt_0f3f,rt_08c9-rt_0f3f,rt_08b4-rt_0f3f
    dw rt_089f-rt_0f3f
    runtime_dispatch:
    xor bh,bh
    ; mov bl, byte ptr ds:[0BFCDh] (TASM absolute-address encoding).
    mov bl,byte ptr ds:[0bfcdh]
    shl bx,1
    jmp word ptr cs:[bx+runtime_mode_dispatch-_f039c+RT_CS]
    ; EGA/VGA planar-mode initializer reached from the dispatch table.
rt_01a1 label byte
    push bp
    mov bp,sp
    db 056h
    db 057h
    push ds
    cld
    mov ax,0eh
    int 10h
    mov ah,5
    mov al,1
    int 10h
    mov ax,1000h
    mov bx,1
    int 10h
    mov ax,1000h
    mov bx,100h
    int 10h
    mov ax,1000h
    mov bx,170eh
    int 10h
    mov ax,1000h
    mov bx,160fh
    int 10h
    mov dx,3ceh
    mov al,5
    out dx,al
    inc dx
    mov al,2
    out dx,al
    mov dx,3c4h
    mov al,2
    out dx,al
    inc dx
    mov al,0fh
    out dx,al
    mov ax,0a000h
    mov es,ax
    xor bx,bx
    mov cx,bx
    ; SI assignment remains a tracked raw recovery candidate.
    db 08bh,0f3h
    mov bp,1000h
    ; The planar clear loop follows. Its raw ES:[SI]/DI operations can
; now be transcribed directly in TASM; no compiler register wrapper exists.
    mov dx,3ceh
    mov al,8
    out dx,al
    inc dx
    mov al,0ch
    out dx,al
    db 026h,08ah,024h
    db 026h,088h,01ch
    mov dx,3ceh
    mov al,8
    out dx,al
    inc dx
    mov al,3
    out dx,al
    db 026h,08ah,024h
    db 026h,088h,00ch
    mov dx,3ceh
    mov al,8
    out dx,al
    inc dx
    mov al,0c0h
    out dx,al
    db 026h,08ah,024h
    db 026h,088h,03ch
    db 046h
    db 0feh,0c7h
    ; SI/DI accesses and their short back-edges remain literal; the nested
; plane/column counters can use ordinary TASM spellings.
    db 080h,0ffh,010h
    db 07ch,0c8h
    db 032h,0ffh
    inc cl
    cmp cl,10h
    db 07ch,0bfh
    ; The original used the byte-register form (32 C9), not XOR CX,CX.
    db 032h,0c9h
    inc bl
    cmp bl,10h
    db 07ch,0b6h
    mov dx,3ceh
    mov al,8
    out dx,al
    inc dx
    mov al,30h
    out dx,al
    mov dx,3ceh
    mov al,3
    out dx,al
    inc dx
    mov al,0
    out dx,al
    pop ds
    db 05fh
    db 05eh
    pop bp
    ret
    ; Restore the EGA palette register set, then provide the mode-13h entry.
rt_0258 label byte
    mov ax,9
    int 10h
    mov al,10h
    mov dx,3dah
    out dx,al
    mov al,1
    mov dx,3deh
    out dx,al
    mov al,11h
    mov dx,3dah
    out dx,al
    mov al,0
    mov dx,3deh
    out dx,al
    mov al,1eh
    mov dx,3dah
    out dx,al
    mov al,0fh
    mov dx,3deh
    out dx,al
    mov al,1fh
    mov dx,3dah
    out dx,al
    mov al,0eh
    mov dx,3deh
    out dx,al
    mov dx,3dah
    mov al,0fh
    out dx,al
    ret
rt_0294 label byte
    mov ax,13h
    int 10h
    ret
    runtime_dispatch_3e2:
    ; DI is intentionally cleared with its original encoding.
    xor bh,bh
    ; mov bl, byte ptr ds:[0BFCDh] (TASM absolute-address encoding).
    mov bl,byte ptr ds:[0bfcdh]
    shl bx,1
    jmp word ptr cs:[bx+runtime_blit_dispatch-_f039c+RT_CS]
    runtime_3e2_handler:
rt_02a7 label byte
    push bp
    mov bp,sp
    ; This handler owns its SI/DI preservation explicitly.
    db 056h
    db 057h
    push ds
    cld
    mov bx,[bp+6]
    shl bx,1
    shl bx,1
    db 0c4h,0b7h,024h,039h
    shl bx,1
    shl bx,1
    ; DI-derived row addressing remains a tracked raw recovery candidate.
    db 08bh,0fbh
    db 0d1h,0e7h
    db 0d1h,0e7h
    db 003h,0fbh
    mov ax,[bp+4]
    mov bx,[bp+8]
    mov cx,[bp+0ah]
    add bx,ax
    dec bx
    shr ax,1
    shr ax,1
    shr bx,1
    shr bx,1
    sub bx,ax
    inc bx
    db 003h,0f0h
    db 003h,0f0h
    db 003h,0f8h
    push es
    mov ax,0a000h
    mov es,ax
    ; Address the selected planar page and enter its unrolled blit tail.
    db 081h,0c7h,000h,040h
    mov bp,50h
    sub bp,bx
    mov dx,0a0h
    shl bx,1
    sub dx,bx
    mov ax,cs:[bx+runtime_planar_entry_deltas-_f039c+RT_CS]
    ; TASM 1.0 needs the historical absolute CS-store spelling preserved.
    db 02eh,0a3h,05fh,00bh
    add ax,rt_07c5-_f039c+RT_CS
    pop ds
    jmp ax
; UNROLLED_CODE: 80 repetitions of 15-byte pixel transfer; falls into row tail
rt_0307 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_0316 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_0325 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_0334 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_0343 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_0352 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_0361 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_0370 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_037f label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_038e label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_039d label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_03ac label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_03bb label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_03ca label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_03d9 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_03e8 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_03f7 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_0406 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_0415 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_0424 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_0433 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_0442 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_0451 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_0460 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_046f label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_047e label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_048d label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_049c label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_04ab label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_04ba label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_04c9 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_04d8 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_04e7 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_04f6 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_0505 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_0514 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_0523 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_0532 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_0541 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_0550 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_055f label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_056e label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_057d label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_058c label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_059b label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_05aa label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_05b9 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_05c8 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_05d7 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_05e6 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_05f5 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_0604 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_0613 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_0622 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_0631 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_0640 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_064f label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_065e label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_066d label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_067c label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_068b label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_069a label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_06a9 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_06b8 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_06c7 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_06d6 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_06e5 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_06f4 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_0703 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_0712 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_0721 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_0730 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_073f label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_074e label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_075d label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_076c label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_077b label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_078a label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_0799 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
rt_07a8 label byte
    db 0adh
    db 08bh,0d8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 026h,08ah,027h
    db 0aah
; CODE: row strides; LOOP to mutable jump 07C2; fallthrough restores DS/DI/SI/BP and RET
    db 003h,0f2h
    db 003h,0fdh
    db 0e2h,005h
    db 01fh
    db 05fh
    db 05eh
    db 05dh
    db 0c3h
; CODE: self-modified rel16 jump, written at 02FD
; Mutable rel16 operand at runtime 07C3, written by 02FD.
    JMP_NEAR rt_0307
; CODE: dispatch slot 3; bounded copy routine with loops and RET
rt_07c5 label byte
    db 055h
    db 08bh,0ech
    db 056h
    db 057h
    db 01eh
    db 0fch
    db 08bh,05eh,006h
    db 0d1h,0e3h
    db 0d1h,0e3h
    db 0c4h,0b7h,024h,039h
    db 08bh,046h,004h
    db 08bh,0d0h
    db 0d1h,0eah
    db 003h,0f2h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 086h,0fbh
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 002h,0e3h
    db 032h,0dbh
    db 003h,0c3h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 003h,0c3h
    db 08bh,056h,008h
    db 0d1h,0e8h
    db 083h,0d2h,000h
    db 08bh,0f8h
    db 006h
    db 0b8h,000h,0b8h
    db 08eh,0c0h
    db 01fh
    db 0d1h,0eah
    db 083h,0d2h,000h
    db 08bh,04eh,00ah
    db 0b8h,0a0h,000h
    db 02bh,0c2h
    db 0bbh,000h,020h
    db 02bh,0dah
    db 0bdh,0a0h,080h
    db 051h
    db 08bh,0cah
    db 0d1h,0e9h
    db 0f3h,0a5h
    db 0d1h,0d1h
    db 0f3h,0a4h
    db 059h
    db 003h,0f0h
    db 003h,0fbh
    db 078h,004h
    db 0e2h,0ech
    db 0ebh,004h
    db 003h,0fdh
    db 0e2h,0e6h
    db 01fh
    db 05fh
    db 05eh
    db 05dh
    db 0c3h
; CODE: dispatch slot 4; setup ending at computed JMP AX
rt_083b label byte
    db 055h
    db 08bh,0ech
    db 056h
    db 057h
    db 01eh
    db 0fch
    db 08bh,05eh,006h
    db 0d1h,0e3h
    db 0d1h,0e3h
    db 0c4h,0b7h,024h,039h
    db 0d1h,0e3h
    db 0d1h,0e3h
    db 0d1h,0e3h
    db 0d1h,0e3h
    db 08bh,0fbh
    db 0d1h,0e3h
    db 0d1h,0e3h
    db 003h,0fbh
    db 08bh,046h,004h
    db 08bh,05eh,008h
    db 003h,0d8h
    db 04bh
    db 0d1h,0e8h
    db 0d1h,0e0h
    db 003h,0f8h
    db 0d1h,0e8h
    db 003h,0f0h
    db 0d1h,0e8h
    db 0d1h,0ebh
    db 0d1h,0ebh
    db 02bh,0d8h
    db 043h
    db 0d1h,0e3h
    db 02eh,08bh,087h,08eh,004h
    db 02eh,0a3h,0d9h,012h
    db 0bah,0a0h,000h
    db 02bh,0d3h
    db 006h
    db 0b9h,000h,0a0h
    db 08eh,0c1h
    db 01fh
    db 08bh,04eh,00ah
    db 0d1h,0e3h
    db 0bdh,040h,001h
    db 02bh,0ebh
    db 005h,0dbh,012h
    jmp ax
; UNROLLED_CODE: 80 repetitions of 21-byte pixel transform; falls into row tail
rt_089f label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_08b4 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_08c9 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_08de label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_08f3 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0908 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_091d label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0932 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0947 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_095c label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0971 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0986 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_099b label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_09b0 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_09c5 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_09da label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_09ef label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0a04 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0a19 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0a2e label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0a43 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0a58 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0a6d label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0a82 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0a97 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0aac label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0ac1 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0ad6 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0aeb label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0b00 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0b15 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0b2a label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0b3f label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0b54 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0b69 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0b7e label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0b93 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0ba8 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0bbd label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0bd2 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0be7 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0bfc label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0c11 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0c26 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0c3b label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0c50 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0c65 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0c7a label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0c8f label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0ca4 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0cb9 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0cce label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0ce3 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0cf8 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0d0d label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0d22 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0d37 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0d4c label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0d61 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0d76 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0d8b label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0da0 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0db5 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0dca label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0ddf label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0df4 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0e09 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0e1e label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0e33 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0e48 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0e5d label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0e72 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0e87 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0e9c label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0eb1 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0ec6 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0edb label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0ef0 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0f05 label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
rt_0f1a label byte
    db 0adh
    db 08ah,0ech
    db 08bh,0d8h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 0d1h,0c3h
    db 08ah,0e3h
    db 0abh
    db 08ah,0c5h
    db 08ah,0e7h
    db 0abh
; CODE: row strides; LOOP to mutable jump 0F3C; epilogue RET
    db 003h,0f2h
    db 003h,0fdh
    db 032h,0edh
    db 0e2h,005h
rt_0f37 label byte
    db 01fh
    db 05fh
    db 05eh
    db 05dh
    db 0c3h
; CODE: self-modified rel16 jump, written at 0880
; Mutable rel16 operand at runtime 0F3D, written by 0880.
    JMP_NEAR rt_089f
    ; F_03A2 / bar: planar horizontal-bar primitive.
    runtime_bar_primitive:
rt_0f3f label byte
    push bp
    mov bp,sp
    push di
    cld
    ; Numeric absolute memory requires an explicit segment in TASM; see ABSOLUTE_MEMORY.
    mov ax,word ptr ds:[40c8h]
    mov cx,[bp+8]
    mov bx,[bp+6]
    shl bx,1
    shl bx,1
    ; LES DI selects the row-pointer table; native TASM may expose DI directly.
    les di,dword ptr [bx+3924h]
    mov bx,[bp+4]
    shr bx,1
    jae runtime_f03a2_add_row
    mov dl,al
    and dl,0fh
    and byte ptr es:[bx+di],0f0h
    or byte ptr es:[bx+di],dl
    inc di
    dec cx
    runtime_f03a2_add_row:
    add di,bx
    sub bx,bx
    shr cx,1
    jae runtime_f03a2_fill
    mov bx,1
    runtime_f03a2_fill:
    rep stosb
    cmp bx,0
    je runtime_f03a2_done
    and al,0f0h
    and byte ptr es:[di],0fh
    or byte ptr es:[di],al
    runtime_f03a2_done:
    pop di
    pop bp
    ret
    ; F_03A5: adjacent planar-bar variant with mirrored nibble handling.
    runtime_f03a5:
    push bp
    mov bp,sp
    push di
    cld
    mov ax,word ptr ds:[40c8h]
    mov cx,[bp+8]
    mov bx,[bp+6]
    shl bx,1
    shl bx,1
    les di,dword ptr [bx+3924h]
    mov bx,[bp+4]
    shr bx,1
    jb $+8
    and al,0f0h
    mov dl,0fh
    jmp $+6
    and al,0fh
    mov dl,0f0h
    add di,bx
    mov bx,0a0h
    runtime_f03a5_loop:
    and byte ptr es:[di],dl
    or byte ptr es:[di],al
    add di,bx
    loop runtime_f03a5_loop
    pop di
    pop bp
    ret
    ; F_03A8 / clear: planar clear primitive.
    runtime_clear_primitive:
    push bp
    mov bp,sp
    push di
    push si
    cld
    mov ax,word ptr ds:[40c8h]
    ; SI supplies the 160-byte planar stride; raw bytes remain tracked.
    mov si,0a0h
    mov bx,[bp+6]
    shl bx,1
    shl bx,1
    db 0c4h,0bfh,024h,039h
    db 08bh,05eh,004h
    db 0d1h,0ebh
    db 073h,01ch
    db 08bh,04eh,00ah
    db 057h
    db 003h,0fbh
    db 0b6h,0f0h
    db 08ah,0d0h
    db 080h,0e2h,00fh
    db 026h,020h,035h
    db 026h,008h,015h
    db 003h,0feh
    db 0e2h,0f6h
    db 05fh
    db 047h
    db 0ffh,04eh,008h
    db 003h,0fbh
    db 02ah,0e4h
    db 08bh,05eh,008h
    db 0d1h,0ebh
    db 073h,002h
    db 0b4h,001h
    db 08bh,056h,00ah
    db 02bh,0f3h
    db 08bh,0cbh
    db 0f3h,0aah
    db 003h,0feh
    db 04ah
    db 075h,0f7h
    db 080h,0fch,000h
    db 074h,019h
    db 08bh,04eh,00ah
    db 02bh,0feh
    mov dh,0fh
    mov dl,al
    and dl,0f0h
    mov si,0a0h
    runtime_f03a8_clear_loop:
    and byte ptr es:[di],dh
    or byte ptr es:[di],dl
    sub di,si
    loop runtime_f03a8_clear_loop
    pop si
    pop di
    pop bp
    ret
    ; F_03AB / fill: planar XOR-fill primitive.
    runtime_f03ab:
    push bp
    mov bp,sp
    push di
    push si
    cld
    mov dx,[bp+0ah]
    mov si,0a0h
    mov bx,[bp+6]
    shl bx,1
    shl bx,1
    db 0c4h,0bfh,024h,039h
    db 08bh,05eh,004h
    db 0d1h,0ebh
    db 073h,013h
    db 08bh,0cah
    db 057h
    db 003h,0fbh
    db 0b0h,00fh
    db 026h,030h,005h
    db 003h,0feh
    db 0e2h,0f9h
    db 05fh
    db 047h
    db 0ffh,04eh,008h
    db 003h,0fbh
    db 057h
    db 02ah,0e4h
    db 08bh,04eh,008h
    db 0d1h,0e9h
    db 073h,002h
    db 0b4h,001h
    db 08bh,0d9h
    db 0e3h,011h
    db 0b0h,0ffh
    db 02bh,0f1h
    db 026h,030h,005h
    db 047h
    db 0e2h,0fah
    db 003h,0feh
    db 08bh,0cbh
    db 04ah
    db 075h,0f3h
    db 05fh
    db 003h,0fbh
    db 080h,0fch,000h
    db 074h,00fh
    db 08bh,04eh,00ah
    db 0b0h,0f0h
    db 0beh,0a0h,000h
    db 026h,030h,005h
    db 003h,0feh
    db 0e2h,0f9h
    db 05eh
    db 05fh
    db 05dh
    db 0c3h
    ; F_03AE: planar rectangle-copy primitive.
    runtime_f03ae:
    push bp
    mov bp,sp
    push di
    push si
    push ds
    cld
    mov ax,[bp+4]
    mov bx,[bp+8]
    add bx,ax
    dec bx
    shr ax,1
    shr bx,1
    sub bx,ax
    inc bx
    mov dx,[bp+0ah]
    mov si,[bp+6]
    shl si,1
    shl si,1
    lds si,dword ptr [si+3924h]
    add si,ax
    les di,dword ptr [bp+0ch]
    mov word ptr es:[di],bx
    mov word ptr es:[di+2],dx
    add di,4
    mov ax,0a0h
    sub ax,bx
    runtime_f03ae_copy_loop:
    mov cx,bx
    rep movsb
    add si,ax
    dec dx
    jne runtime_f03ae_copy_loop
    pop ds
    pop si
    pop di
    pop bp
    ret
    ; F_03B1: copy from a source rectangle descriptor to a planar row.
    runtime_f03b1:
    push bp
    mov bp,sp
    push di
    push si
    push ds
    cld
    mov ax,[bp+4]
    shr ax,1
    mov di,[bp+6]
    shl di,1
    shl di,1
    les di,dword ptr [di+3924h]
    add di,ax
    lds si,dword ptr [bp+8]
    lodsw
    mov bx,ax
    lodsw
    mov dx,ax
    mov ax,0a0h
    sub ax,bx
    runtime_f03b1_copy_loop:
    mov cx,bx
    rep movsb
    add di,ax
    dec dx
    jne runtime_f03b1_copy_loop
    pop ds
    pop si
    pop di
    pop bp
    ret
    ; F_03B4 / wipe: planar rectangle copy with word-aligned interior.
    runtime_f03b4:
    push bp
    mov bp,sp
    push di
    push si
    push ds
    cld
    mov bx,[bp+0eh]
    shl bx,1
    shl bx,1
    les di,dword ptr [bx+3924h]
    mov ax,[bp+0ch]
    shr ax,1
    add di,ax
    mov bx,[bp+6]
    shl bx,1
    shl bx,1
    lds si,dword ptr [bx+3924h]
    mov ax,[bp+4]
    shr ax,1
    add si,ax
    mov bx,[bp+8]
    shr bx,1
    mov dx,[bp+0ah]
    mov ax,0a0h
    sub ax,bx
    runtime_f03b4_copy_loop:
    mov cx,bx
    shr cx,1
    rep movsw
    rcl cx,1
    rep movsb
    add si,ax
    add di,ax
    dec dx
    jne runtime_f03b4_copy_loop
    pop ds
    cmp word ptr ds:[0bch],1
    jne runtime_f03b4_done
    mov bx,[bp+0eh]
    cmp bx,0c8h
    jae runtime_f03b4_done
    les di,dword ptr ds:[40c4h]
    mov ax,[bp+0ch]
    shr ax,1
    mov ah,bl
    stosw
    mov ax,[bp+8]
    shr ax,1
    mov bx,[bp+0ah]
    mov ah,bl
    stosw
    mov word ptr ds:[40c4h],di
    runtime_f03b4_done:
    pop si
    pop di
    pop bp
    ret
    ; F_03B7: reverse-direction planar rectangle copy.
    runtime_f03b7:
    push bp
    mov bp,sp
    push di
    push si
    push ds
    cld
    mov bx,[bp+0eh]
    shl bx,1
    shl bx,1
    les di,dword ptr [bx+3924h]
    mov ax,[bp+0ch]
    shr ax,1
    add di,ax
    mov bx,[bp+6]
    shl bx,1
    shl bx,1
    lds si,dword ptr [bx+3924h]
    mov ax,[bp+4]
    shr ax,1
    add si,ax
    mov bx,[bp+8]
    shr bx,1
    mov dx,[bp+0ah]
    mov ax,dx
    dec ax
    xchg ah,al
    shr ax,1
    add di,ax
    shr ax,1
    shr ax,1
    add di,ax
    mov ax,0a0h
    runtime_f03b7_copy_loop:
    mov cx,bx
    rep movsb
    add si,ax
    sub si,bx
    sub di,ax
    sub di,bx
    dec dx
    jne runtime_f03b7_copy_loop
    pop ds
    pop si
    pop di
    pop bp
    ret
    ; F_03BA: reverse-direction planar nibble transform.
    runtime_f03ba:
    push bp
    mov bp,sp
    push di
    push si
    push ds
    cld
    mov bx,[bp+0eh]
    shl bx,1
    shl bx,1
    les di,dword ptr [bx+3924h]
    mov ax,[bp+0ch]
    shr ax,1
    add di,ax
    mov bx,[bp+6]
    shl bx,1
    shl bx,1
    lds si,dword ptr [bx+3924h]
    mov ax,[bp+4]
    shr ax,1
    add si,ax
    mov bx,[bp+8]
    shr bx,1
    mov dx,[bp+0ah]
    add di,bx
    dec di
    runtime_f03ba_outer_loop:
    mov cx,bx
    runtime_f03ba_inner_loop:
    lodsb
    rol al,1
    rol al,1
    rol al,1
    rol al,1
    mov byte ptr es:[di],al
    dec di
    loop runtime_f03ba_inner_loop
    add si,0a0h
    sub si,bx
    add di,0a0h
    add di,bx
    dec dx
    jne runtime_f03ba_outer_loop
    pop ds
    pop si
    pop di
    pop bp
    ret
    ; F_03BD: mirrored reverse-direction planar nibble transform.
    runtime_f03bd:
    push bp
    mov bp,sp
    push di
    push si
    push ds
    cld
    mov bx,[bp+0eh]
    shl bx,1
    shl bx,1
    les di,dword ptr [bx+3924h]
    mov ax,[bp+0ch]
    shr ax,1
    add di,ax
    mov bx,[bp+6]
    shl bx,1
    shl bx,1
    lds si,dword ptr [bx+3924h]
    mov ax,[bp+4]
    shr ax,1
    add si,ax
    mov bx,[bp+8]
    shr bx,1
    mov dx,[bp+0ah]
    mov ax,dx
    dec ax
    xchg ah,al
    shr ax,1
    add di,ax
    shr ax,1
    shr ax,1
    add di,ax
    add di,bx
    dec di
    runtime_f03bd_outer_loop:
    mov cx,bx
    runtime_f03bd_inner_loop:
    lodsb
    rol al,1
    rol al,1
    rol al,1
    rol al,1
    mov byte ptr es:[di],al
    dec di
    loop runtime_f03bd_inner_loop
    add si,0a0h
    sub si,bx
    sub di,0a0h
    add di,bx
    dec dx
    jne runtime_f03bd_outer_loop
    pop ds
    pop si
    pop di
    pop bp
    ret
    ; F_03C0: dual-planar reflected pixel transform.
    runtime_f03c0:
    push bp
    mov bp,sp
    push di
    push si
    push ds
    cld
    mov bx,[bp+0eh]
    shl bx,1
    shl bx,1
    les di,dword ptr [bx+3924h]
    mov ax,[bp+0ch]
    shr ax,1
    add di,ax
    mov bx,[bp+6]
    shl bx,1
    shl bx,1
    lds si,dword ptr [bx+3924h]
    mov ax,[bp+4]
    shr ax,1
    add si,ax
    mov bx,[bp+8]
    shr bx,1
    mov dx,[bp+0ah]
    shr dx,1
    add di,dx
    dec di
    runtime_f03c0_outer_loop:
    push di
    mov cx,bx
    runtime_f03c0_low_loop:
    lodsb
    mov ah,al
    and ax,0f00fh
    shr ah,1
    shr ah,1
    shr ah,1
    shr ah,1
    and byte ptr es:[di],0f0h
    or byte ptr es:[di],ah
    add di,0a0h
    and byte ptr es:[di],0f0h
    or byte ptr es:[di],al
    add di,0a0h
    loop runtime_f03c0_low_loop
    add si,0a0h
    sub si,bx
    pop di
    push di
    mov cx,bx
    runtime_f03c0_high_loop:
    lodsb
    mov ah,al
    and ax,0f00fh
    shl al,1
    shl al,1
    shl al,1
    shl al,1
    and byte ptr es:[di],0fh
    or byte ptr es:[di],ah
    add di,0a0h
    and byte ptr es:[di],0fh
    or byte ptr es:[di],al
    add di,0a0h
    loop runtime_f03c0_high_loop
    add si,0a0h
    sub si,bx
    pop di
    dec di
    dec dx
    jne runtime_f03c0_outer_loop
    pop ds
    pop si
    pop di
    pop bp
    ret
    ; F_03C3: mirrored dual-planar reflected pixel transform.
    runtime_f03c3:
    push bp
    mov bp,sp
    push di
    push si
    push ds
    cld
    mov bx,[bp+0eh]
    shl bx,1
    shl bx,1
    les di,dword ptr [bx+3924h]
    mov ax,[bp+0ch]
    shr ax,1
    add di,ax
    mov bx,[bp+6]
    shl bx,1
    shl bx,1
    lds si,dword ptr [bx+3924h]
    mov ax,[bp+4]
    shr ax,1
    add si,ax
    mov bx,[bp+8]
    mov ax,bx
    shr bx,1
    mov dx,[bp+0ah]
    shr dx,1
    dec ax
    xchg ah,al
    shr ax,1
    add di,ax
    shr ax,1
    shr ax,1
    add di,ax
    push di
    mov cx,bx
    runtime_f03c3_low_loop:
    lodsb
    mov ah,al
    and ax,0f00fh
    shl al,1
    shl al,1
    shl al,1
    shl al,1
    and byte ptr es:[di],0fh
    or byte ptr es:[di],ah
    sub di,0a0h
    and byte ptr es:[di],0fh
    or byte ptr es:[di],al
    sub di,0a0h
    loop runtime_f03c3_low_loop
    add si,0a0h
    sub si,bx
    pop di
    push di
    mov cx,bx
    runtime_f03c3_high_loop:
    lodsb
    mov ah,al
    and ax,0f00fh
    shr ah,1
    shr ah,1
    shr ah,1
    shr ah,1
    and byte ptr es:[di],0f0h
    or byte ptr es:[di],ah
    sub di,0a0h
    and byte ptr es:[di],0f0h
    or byte ptr es:[di],al
    sub di,0a0h
    loop runtime_f03c3_high_loop
    add si,0a0h
    sub si,bx
    pop di
    inc di
    dec dx
    jne $-62h
    pop ds
    pop si
    pop di
    pop bp
    ret
    ; F_03C6: state-driven planar compositing primitive.
    runtime_f03c6:
    push bp
    mov bp,sp
    sub sp,4
    push di
    push si
    push ds
    cld
    ; Far state base at C0E0h, with the historical no-fixup encoding.
    mov ax,word ptr ds:[0c0e0h]
    mov es,ax
    mov bx,[bp+8]
    mov di, word ptr ds:[0c0e4h]
    mov cl, byte ptr es:[bx + di]
    sub ch, ch
    mov di, word ptr ds:[0c0e6h]
    mov al, byte ptr es:[bx + di]
    mov di, word ptr ds:[0c0e2h]
    mov ah, byte ptr es:[bx + di]
    mov si, word ptr ds:[0c0deh]
    add si, ax
    mov dx, word ptr ds:[0c0e8h]
    mov ax, word ptr ds:[040c8h]
    mov ah, al
    mov bx, word ptr [bp + 6]
    shl bx, 1
    shl bx, 1
    lds di, dword ptr [bx + 03924h]
    mov word ptr [bp - 2], 0
    mov bx, word ptr [bp + 4]
    shr bx, 1
    jae rb_145c
    mov word ptr [bp - 2], 1
rb_145c:
    add di, bx
    mov bx, cx
    shl bx, 1
    inc cx
    shr cx, 1
    mov word ptr [bp - 4], 0a0h
    sub word ptr [bp - 4], cx
    mov cl, dl
    mov dh, al
    mov dl, dh
    and dx, 0f00fh
    cmp word ptr [bp - 2], 0
    jne rt_14f2
    dec di
    push bx
    sub ah, ah
    mov al, byte ptr es:[si]
    inc si
    cmp bx, 010h
    jge $+0ch
    mov ah,bl
    and ah,2
    jmp word ptr cs:[bx+runtime_even_pixel_dispatch-_f039c+RT_CS]
rt_1494 label byte
    inc di
    shl al,1
    jae $+7
    and byte ptr [di],0fh
    or byte ptr [di],dh
    shl al,1
    jae $+7
    db 080h,025h,0f0h
    db 008h,015h
rt_14a7 label byte
    db 047h
    db 0d0h,0e0h
    db 073h,005h
    db 080h,025h,00fh
    db 008h,035h
    db 0d0h,0e0h
    db 073h,005h
    db 080h,025h,0f0h
    db 008h,015h
rt_14ba label byte
    db 047h
    db 0d0h,0e0h
    db 073h,005h
    db 080h,025h,00fh
    db 008h,035h
    db 0d0h,0e0h
    db 073h,005h
    db 080h,025h,0f0h
    db 008h,015h
rt_14cd label byte
    db 047h
    db 0d0h,0e0h
    db 073h,005h
    db 080h,025h,00fh
    db 008h,035h
    db 080h,0fch,000h
    db 075h,00eh
    db 0d0h,0e0h
    db 073h,005h
    db 080h,025h,0f0h
    db 008h,015h
rt_14e5 label byte
    db 083h,0ebh,010h
    jg $-67h
    pop bx
    add di,word ptr [bp-4]
    loop $-70h
    jmp $+74h
rt_14f2:
    push bx
    sub ah,ah
    mov al,byte ptr es:[si]
    inc si
    cmp bx,10h
    jge $+0ch
    mov ah,bl
    and ah,2
    jmp word ptr cs:[bx+runtime_odd_pixel_dispatch-_f039c+RT_CS]
rt_1508 label byte
    shl al,1
    jae $+7
    and byte ptr [di],0f0h
    or byte ptr [di],dl
    inc di
    shl al,1
    jae $+7
    and byte ptr [di],0fh
    or byte ptr [di],dh
rt_151b label byte
    shl al,1
    jae $+7
    db 080h,025h,0f0h
    db 008h,015h
    db 047h
    db 0d0h,0e0h
    db 073h,005h
    db 080h,025h,00fh
    db 008h,035h
rt_152e label byte
    db 0d0h,0e0h
    db 073h,005h
    db 080h,025h,0f0h
    db 008h,015h
    db 047h
    db 0d0h,0e0h
    db 073h,005h
    db 080h,025h,00fh
    db 008h,035h
rt_1541 label byte
    db 0d0h,0e0h
    db 073h,005h
    db 080h,025h,0f0h
    db 008h,015h
    db 047h
    db 080h,0fch,000h
    db 075h,00eh
    db 0d0h,0e0h
    db 073h,005h
    db 080h,025h,00fh
    db 008h,035h
rt_1559 label byte
    db 083h,0ebh,010h
    db 07fh,097h
    db 05bh
    db 003h,07eh,0fch
    db 0e2h,08eh
    db 08bh,0c3h
    db 0d1h,0e8h
    db 01fh
    db 05eh
    db 05fh
    db 083h,0c4h,004h
    db 05dh
    db 0c3h
; Even BX pixel count indexes CS:190C after signed comparison with 16. Nine words include the full-width entry. Negative/wrapped counts are not proven impossible.
runtime_even_pixel_dispatch label word
    dw (rt_14e5-_f039c+RT_CS),(rt_14cd-_f039c+RT_CS),(rt_14cd-_f039c+RT_CS),(rt_14ba-_f039c+RT_CS),(rt_14ba-_f039c+RT_CS),(rt_14a7-_f039c+RT_CS),(rt_14a7-_f039c+RT_CS),(rt_1494-_f039c+RT_CS)
    dw (rt_1494-_f039c+RT_CS)
; Even BX pixel count indexes CS:191E; paired with table at 1570 and ends exactly at public blit target 1594.
runtime_odd_pixel_dispatch label word
    dw (rt_1559-_f039c+RT_CS),(rt_1541-_f039c+RT_CS),(rt_1541-_f039c+RT_CS),(rt_152e-_f039c+RT_CS),(rt_152e-_f039c+RT_CS),(rt_151b-_f039c+RT_CS),(rt_151b-_f039c+RT_CS),(rt_1508-_f039c+RT_CS)
    dw (rt_1508-_f039c+RT_CS)
    ; F_03C9 / blit: planar bitmap blit from a 32-byte source header.
    runtime_blit_primitive:
    push bp
    mov bp,sp
    push di
    push si
    push ds
    cld
    mov bx,[bp+6]
    shl bx,1
    shl bx,1
    les di,dword ptr [bx+3924h]
    mov bx,[bp+4]
    shr bx,1
    db 003h,0fbh
    db 0c5h,076h,008h
    db 083h,0c6h,020h
    db 0adh
    db 02ah,0ffh
    db 08ah,0d8h
    db 08ah,0f7h
    db 08ah,0d4h
    db 052h
    db 0b8h,0a0h,000h
    db 02bh,0c3h
    db 08bh,0cbh
    db 0d1h,0e9h
    db 0f3h,0a5h
    db 0d1h,0d1h
    db 0f3h,0a4h
    db 003h,0f8h
    db 04ah
    db 075h,0f1h
    db 05ah
    db 01fh
    db 083h,03eh,0bch,000h,001h
    db 075h,01eh
    db 08bh,04eh,006h
    db 081h,0f9h,0c8h,000h
    db 073h,015h
    db 0c4h,03eh,0c4h,040h
    db 08bh,046h,004h
    db 0d1h,0e8h
    db 08ah,0e1h
    db 0abh
; CODE: reachable blit tail; instruction at 15EF straddles old raw boundary 15F0; RET at 15FB
    db 08ah,0c3h
    db 08ah,0e2h
    db 0abh
    db 089h,03eh,0c4h,040h
    db 05eh
    db 05fh
    db 05dh
    db 0c3h
; BX=CS:1998 followed by CS XLAT at 17AB/1844; AL is an 8-bit index. For each value v, mask=(F0 if high nibble is zero else 0)|(0F if low nibble is zero else 0). RET at 15FB; next public root 16FC.
runtime_transparency_mask label byte
    db 0ffh,0f0h,0f0h,0f0h,0f0h,0f0h,0f0h,0f0h,0f0h,0f0h,0f0h,0f0h,0f0h,0f0h,0f0h,0f0h
    db 00fh,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h
    db 00fh,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h
    db 00fh,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h
    db 00fh,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h
    db 00fh,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h
    db 00fh,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h
    db 00fh,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h
    db 00fh,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h
    db 00fh,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h
    db 00fh,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h
    db 00fh,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h
    db 00fh,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h
    db 00fh,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h
    db 00fh,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h
    db 00fh,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h,000h
    ; F_03CC / copy: clipped source-rectangle copy primitive.
    runtime_copy_primitive:
    push bp
    mov bp,sp
    push si
    push di
    push ds
    push bp
    les si,dword ptr [bp+8]
    add si,20h
    mov ax,word ptr es:[si]
    add si,2
    xor dx,dx
    mov cx,dx
    mov cl,al
    mov dl,ah
    mov di,[bp+6]
    mov ax,word ptr ds:[96h]
    sub ax,di
    js $+48h
    inc ax
    cmp dx,ax
    jbe $+4
    mov dx,ax
    mov ax,word ptr ds:[94h]
    sub ax,di
    jle $+0ch
    sub dx,ax
    jbe $+36h
    add di,ax
    mul cl
    add si,ax
    mov bx,[bp+4]
    cmp word ptr [bp+0ch],0
    je $+5h
    JMP_NEAR $+84h
    sar bx,1
    cld
    xor bp,bp
    mov ax,word ptr ds:[9ah]
    sub ax,bx
    js $+18h
    inc ax
    cmp cx,ax
    jbe $+8h
    mov bp,cx
    sub bp,ax
    mov cx,ax
    mov ax,word ptr ds:[98h]
    sub ax,bx
    jle runtime_copy_skip_forward
    sub cx,ax
    ja runtime_copy_return
    pop bp
    pop ds
    pop di
    pop si
    cld
    pop bp
    ret
    runtime_copy_return:
    add bx,ax
    add bp,ax
    add si,ax
    runtime_copy_skip_forward:
    push es
    push di
    cmp word ptr ds:[0bch],1
    jne runtime_copy_done
    cmp di,0c8h
    jae runtime_copy_done
    mov ax,di
    mov ah,al
    mov al,bl
    les di,dword ptr ds:[40c4h]
    stosw
    mov al,cl
    mov ah,dl
    stosw
    mov word ptr ds:[40c4h],di
    runtime_copy_done:
    pop di
    shl di,1
    shl di,1
    les di,dword ptr [di+3924h]
    db 003h,0fbh
    db 01fh
    db 0bbh,098h,019h
    db 057h
    db 051h
    db 0ach
    db 08ah,0e0h
    db 02eh,0d7h
    db 026h,022h,005h
    db 00ah,0c4h
    db 0aah
    db 0e2h,0f3h
    db 059h
    db 05fh
    db 081h,0c7h,0a0h,000h
    db 003h,0f5h
    db 04ah
    db 075h,0e6h
    db 05dh
    db 01fh
    db 05fh
    db 05eh
    db 0fch
    db 05dh
    db 0c3h
    db 0fdh
    db 033h,0edh
    db 003h,0d9h
    db 003h,0d9h
    db 0d1h,0ebh
    db 04bh
    db 08bh,0c3h
    db 02bh,006h,098h,000h
    db 078h,017h
    db 040h
    db 03bh,0c8h
    db 076h,006h
    db 08bh,0e9h
    db 02bh,0e8h
    db 08bh,0c8h
    db 08bh,0c3h
    db 02bh,006h,09ah,000h
    db 076h,011h
    db 02bh,0c8h
    db 077h,007h
    db 05dh
    db 01fh
    db 05fh
    db 05eh
    db 0fch
    db 05dh
    db 0c3h
    db 02bh,0d8h
    db 003h,0e8h
    db 003h,0f0h
    db 006h
    db 057h
    db 083h,03eh,0bch,000h,001h
    db 075h,020h
    db 081h,0ffh,0c8h,000h
    db 073h,01ah
    db 08bh,0c7h
    db 08ah,0e0h
    db 08ah,0c3h
    db 02ah,0c1h
    db 0feh,0c0h
    db 0c4h,03eh,0c4h,040h
    db 0fch
    db 0abh
    db 08ah,0c1h
    db 08ah,0e2h
    db 0abh
    db 0fdh
    db 089h,03eh,0c4h,040h
    db 05fh
    db 0d1h,0e7h
    db 0d1h,0e7h
    db 0c4h,0bdh,024h,039h
    db 003h,0fbh
    db 01fh
    db 0bbh,098h,019h
    db 057h
    db 051h
    db 08ah,004h
    db 046h
    db 0d0h,0c8h
    db 0d0h,0c8h
    db 0d0h,0c8h
    db 0d0h,0c8h
    db 08ah,0e0h
    db 02eh,0d7h
    db 026h,022h,005h
    db 00ah,0c4h
    db 0aah
    db 0e2h,0e9h
    db 059h
    db 05fh
    db 081h,0c7h,0a0h,000h
    db 003h,0f5h
    db 04ah
    db 075h,0dch
    db 05dh
    db 01fh
    db 05fh
    db 05eh
    db 0fch
    db 05dh
    db 0c3h
    ; F_03CF: state-keyed planar pixel compositor. Remaining raw SI/DI
; operations are ordinary native TASM recovery candidates.
    runtime_f03cf:
    push bp
    mov bp,sp
    sub sp,2
    push si
    push di
    push ds
    cld
    mov dx,word ptr ds:[40c8h]
    mov dh,dl
    and dx,0f00fh
    ; Source and destination descriptors are both indexed by plane.
    les si,dword ptr [bp+8]
    mov bx,[bp+6]
    shl bx,1
    shl bx,1
    lds di,dword ptr [bx+3924h]
    mov bx,[bp+4]
    shr bx,1
    add di,bx
    xor bx,bx
    mov cx,bx
    mov bl,byte ptr es:[si]
    inc si
    mov word ptr [bp-2],0a0h
    sub [bp-2],bx
    shl bx,1
    shl bx,1
    mov cl,byte ptr es:[si]
    inc si
    dec di
    push bx
    sub ah,ah
    mov al,byte ptr es:[si]
    inc si
    cmp bx,10h
    jge $+0ch
    mov ah,bl
    and ah,2
    jmp word ptr cs:[bx+runtime_even_pixel_dispatch-_f039c+RT_CS]
rt_18b8 label byte
    inc di
    shl al,1
    jae $+7
    and byte ptr [di],0fh
    or byte ptr [di],dh
    shl al,1
    jae $+7
    db 080h,025h,0f0h
    db 008h,015h
rt_18cb label byte
    db 047h
    db 0d0h,0e0h
    db 073h,005h
    db 080h,025h,00fh
    db 008h,035h
    db 0d0h,0e0h
    db 073h,005h
    db 080h,025h,0f0h
    db 008h,015h
rt_18de label byte
    db 047h
    db 0d0h,0e0h
    db 073h,005h
    db 080h,025h,00fh
    db 008h,035h
    db 0d0h,0e0h
    db 073h,005h
    db 080h,025h,0f0h
    db 008h,015h
rt_18f1 label byte
    db 047h
    db 0d0h,0e0h
    db 073h,005h
    db 080h,025h,00fh
    db 008h,035h
    db 080h,0fch,000h
    db 075h,00eh
    db 0d0h,0e0h
    db 073h,005h
    db 080h,025h,0f0h
    db 008h,015h
rt_1909 label byte
    db 083h,0ebh,010h
    db 07fh,097h
    db 05bh
    db 003h,07eh,0feh
    db 0e2h,08eh
    db 01fh
    db 05fh
    db 05eh
    db 083h,0c4h,002h
    db 05dh
    db 0c3h
; Unreferenced translated copy of runtime_even_pixel_dispatch (+0424h).
runtime_unreferenced_pixel_dispatch label word
    dw (rt_1909-_f039c+RT_CS),(rt_18f1-_f039c+RT_CS),(rt_18f1-_f039c+RT_CS)
    dw (rt_18de-_f039c+RT_CS),(rt_18de-_f039c+RT_CS),(rt_18cb-_f039c+RT_CS)
    dw (rt_18cb-_f039c+RT_CS),(rt_18b8-_f039c+RT_CS),(rt_18b8-_f039c+RT_CS)

    ; F_03D2: merge a nibble from a planar pixel with the current state.
    runtime_f03d2:
    push bp
    mov bp,sp
    push di
    mov bx,[bp+6]
    shl bx,1
    shl bx,1
    les di,dword ptr [bx+3924h]
    mov bx,[bp+4]
    shr bx,1
    jb $+17h
    add di,bx
    mov al,byte ptr es:[di]
    and al,0fh
    mov bx,word ptr ds:[40c8h]
    shl bl,1
    shl bl,1
    shl bl,1
    shl bl,1
    jmp $+10h
    add di,bx
    mov al,byte ptr es:[di]
    and al,0f0h
    mov bx,word ptr ds:[40c8h]
    and bl,0fh
    or al,bl
    mov byte ptr es:[di],al
    pop di
    pop bp
    ret
    ; F_03D5: fetch either half of a planar pixel as a normalized nibble.
    runtime_f03d5:
    push bp
    mov bp,sp
    push di
    mov bx,[bp+6]
    shl bx,1
    shl bx,1
    les di,dword ptr [bx+3924h]
    mov bx,[bp+4]
    shr bx,1
    jb $+14h
    add di,bx
    mov al,byte ptr es:[di]
    and ax,0f0h
    shr ax,1
    shr ax,1
    shr ax,1
    shr ax,1
    jmp $+0ah
    add di,bx
    mov al,byte ptr es:[di]
    and ax,0fh
    pop di
    pop bp
    ret

    ; Unreferenced stub and driver name following the final code root.
    db 0c3h
runtime_driver_name label byte
    db 'EGA.DRV',0
    _runtime_block_end label byte
    public _runtime_block_end
_TEXT ends
end

