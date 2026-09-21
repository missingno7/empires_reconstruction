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
     push si
     push di
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
     mov si,bx
    mov bp,1000h
    ; The planar clear loop follows. Its raw ES:[SI]/DI operations can
; now be transcribed directly in TASM; no compiler register wrapper exists.
    mov dx,3ceh
    mov al,8
    out dx,al
    inc dx
    mov al,0ch
    out dx,al
    mov ah,byte ptr es:[si]
    mov byte ptr es:[si],bl
    mov dx,3ceh
    mov al,8
    out dx,al
    inc dx
    mov al,3
    out dx,al
    mov ah,byte ptr es:[si]
    mov byte ptr es:[si],cl
    mov dx,3ceh
    mov al,8
    out dx,al
    inc dx
    mov al,0c0h
    out dx,al
    mov ah,byte ptr es:[si]
    mov byte ptr es:[si],bh
    inc si
    inc bh
    ; SI/DI accesses and their short back-edges remain literal; the nested
; plane/column counters can use ordinary TASM spellings.
    cmp bh,10h
    jl $-36h
    xor bh,bh
    inc cl
    cmp cl,10h
     jl $-3fh
    ; The original used the byte-register form (32 C9), not XOR CX,CX.
     xor cl,cl
    inc bl
    cmp bl,10h
     jl $-48h
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
     pop di
     pop si
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
     push si
     push di
    push ds
    cld
    mov bx,[bp+6]
    shl bx,1
    shl bx,1
    les si,[bx+3924h]
    shl bx,1
    shl bx,1
    ; DI-derived row addressing remains a tracked raw recovery candidate.
    mov di,bx
    shl di,1
    shl di,1
    add di,bx
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
    add si,ax
    add si,ax
    add di,ax
    push es
    mov ax,0a000h
    mov es,ax
    ; Address the selected planar page and enter its unrolled blit tail.
     add di,4000h
    mov bp,50h
    sub bp,bx
    mov dx,0a0h
    shl bx,1
    sub dx,bx
    mov ax,cs:[bx+runtime_planar_entry_deltas-_f039c+RT_CS]
    ; TASM 1.0 needs the historical absolute CS-store spelling preserved.
     mov word ptr cs:[0b5fh],ax
    add ax,rt_07c5-_f039c+RT_CS
    pop ds
    jmp ax
; UNROLLED_CODE: 80 repetitions of 15-byte pixel transfer; falls into row tail
rt_0307 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_0316 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_0325 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_0334 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_0343 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_0352 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_0361 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_0370 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_037f label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_038e label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_039d label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_03ac label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_03bb label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_03ca label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_03d9 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_03e8 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_03f7 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_0406 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_0415 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_0424 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_0433 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_0442 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_0451 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_0460 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_046f label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_047e label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_048d label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_049c label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_04ab label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_04ba label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_04c9 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_04d8 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_04e7 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_04f6 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_0505 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_0514 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_0523 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_0532 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_0541 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_0550 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_055f label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_056e label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_057d label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_058c label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_059b label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_05aa label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_05b9 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_05c8 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_05d7 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_05e6 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_05f5 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_0604 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_0613 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_0622 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_0631 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_0640 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_064f label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_065e label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_066d label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_067c label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_068b label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_069a label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_06a9 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_06b8 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_06c7 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_06d6 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_06e5 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_06f4 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_0703 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_0712 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_0721 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_0730 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_073f label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_074e label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_075d label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_076c label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_077b label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_078a label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_0799 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
rt_07a8 label byte
    lodsw
    mov bx,ax
    shr bx,1
    shr bx,1
    shr bx,1
    shr bx,1
    mov ah,es:[bx]
    stosb
; CODE: row strides; LOOP to mutable jump 07C2; fallthrough restores DS/DI/SI/BP and RET
     add si,dx
     add di,bp
     loop $+7h
     pop ds
     pop di
     pop si
     pop bp
     ret
; CODE: self-modified rel16 jump, written at 02FD
; Mutable rel16 operand at runtime 07C3, written by 02FD.
    JMP_NEAR rt_0307
; CODE: dispatch slot 3; bounded copy routine with loops and RET
rt_07c5 label byte
    push bp
    mov bp,sp
    push si
    push di
    push ds
    cld
    mov bx,word ptr [bp+6]
    shl bx,1
    shl bx,1
    les si,dword ptr [bx+3924h]
    mov ax,word ptr [bp+4]
    mov dx,ax
    shr dx,1
    add si,dx
    shr bx,1
    shr bx,1
    xchg bh,bl
    shr bx,1
    shr bx,1
    add ah,bl
    xor bl,bl
    add ax,bx
    shr bx,1
    shr bx,1
    add ax,bx
    mov dx,word ptr [bp+8]
    shr ax,1
    adc dx,0
    mov di,ax
    push es
    mov ax,0b800h
    mov es,ax
    pop ds
    shr dx,1
    adc dx,0
    mov cx,word ptr [bp+0ah]
    mov ax,0a0h
    sub ax,dx
    mov bx,02000h
    sub bx,dx
    mov bp,080a0h
    push cx
    mov cx,dx
    shr cx,1
    rep movsw
    rcl cx,1
     rep movsb
     pop cx
     add si,ax
     add di,bx
     js $+6h
     loop $-12h
     jmp $+6h
     add di,bp
     loop $-18h
     pop ds
     pop di
     pop si
     pop bp
     ret
; CODE: dispatch slot 4; setup ending at computed JMP AX
rt_083b label byte
    push bp
    mov bp,sp
    push si
    push di
    push ds
    cld
    mov bx,word ptr [bp+6]
    shl bx,1
    shl bx,1
    les si,dword ptr [bx+3924h]
    shl bx,1
    shl bx,1
    shl bx,1
    shl bx,1
    mov di,bx
    shl bx,1
    shl bx,1
    add di,bx
    mov ax,word ptr [bp+4]
    mov bx,word ptr [bp+8]
    add bx,ax
    dec bx
    shr ax,1
    shl ax,1
    add di,ax
    shr ax,1
    add si,ax
    shr ax,1
    shr bx,1
    shr bx,1
    sub bx,ax
    inc bx
    shl bx,1
    mov ax,word ptr cs:[bx+48eh]
    mov word ptr cs:[12d9h],ax
    mov dx,0a0h
    sub dx,bx
    push es
    mov cx,0a000h
    mov es,cx
    pop ds
    mov cx,word ptr [bp+0ah]
    shl bx,1
    mov bp,0140h
    sub bp,bx
    add ax,12dbh
    jmp ax
; UNROLLED_CODE: 80 repetitions of 21-byte pixel transform; falls into row tail
rt_089f label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_08b4 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_08c9 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_08de label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_08f3 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0908 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_091d label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0932 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0947 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_095c label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0971 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0986 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_099b label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_09b0 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_09c5 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_09da label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_09ef label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0a04 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0a19 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0a2e label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0a43 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0a58 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0a6d label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0a82 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0a97 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0aac label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0ac1 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0ad6 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0aeb label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0b00 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0b15 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0b2a label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0b3f label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0b54 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0b69 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0b7e label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0b93 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0ba8 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0bbd label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0bd2 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0be7 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0bfc label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0c11 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0c26 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0c3b label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0c50 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0c65 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0c7a label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0c8f label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0ca4 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0cb9 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0cce label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0ce3 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0cf8 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0d0d label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0d22 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0d37 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0d4c label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0d61 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0d76 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0d8b label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0da0 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0db5 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0dca label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0ddf label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0df4 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0e09 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0e1e label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0e33 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0e48 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0e5d label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0e72 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0e87 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0e9c label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0eb1 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0ec6 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0edb label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0ef0 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0f05 label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
rt_0f1a label byte
    lodsw
    mov ch,ah
    mov bx,ax
    rol bx,1
    rol bx,1
    rol bx,1
    rol bx,1
    mov ah,bl
    stosw
    mov al,ch
    mov ah,bh
    stosw
; CODE: row strides; LOOP to mutable jump 0F3C; epilogue RET
     add si,dx
     add di,bp
     xor ch,ch
     loop $+7h
 rt_0f37 label byte
     pop ds
     pop di
     pop si
     pop bp
     ret
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
    les di,dword ptr [bx+3924h]
    mov bx,word ptr [bp+4]
    shr bx,1
    jae runtime_0fd4_next_row
    mov cx,word ptr [bp+0ah]
    push di
    add di,bx
    mov dh,0f0h
    mov dl,al
    and dl,0fh
runtime_0fd4_pixel_loop:
    and byte ptr es:[di],dh
    or byte ptr es:[di],dl
    add di,si
    loop runtime_0fd4_pixel_loop
    pop di
    inc di
    dec word ptr [bp+8]
runtime_0fd4_next_row:
    add di,bx
    sub ah,ah
    mov bx,word ptr [bp+8]
    shr bx,1
    jae runtime_0fd4_fill_start
    mov ah,1
runtime_0fd4_fill_start:
    mov dx,word ptr [bp+0ah]
    sub si,bx
runtime_0fd4_fill_loop:
    mov cx,bx
    rep stosb
    add di,si
    dec dx
    jne runtime_0fd4_fill_loop
     cmp ah,0
     je $+1bh
     mov cx,word ptr ss:[bp+0ah]
     sub di,si
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
    les di,dword ptr [bx+3924h]
    mov bx,word ptr [bp+4]
    shr bx,1
    jae runtime_104b_next_row
    mov cx,dx
    push di
    add di,bx
    mov al,0fh
runtime_104b_pixel_loop:
    xor byte ptr es:[di],al
    add di,si
    loop runtime_104b_pixel_loop
    pop di
    inc di
    dec word ptr [bp+8]
runtime_104b_next_row:
    add di,bx
    push di
    sub ah,ah
    mov cx,word ptr [bp+8]
    shr cx,1
    jae runtime_104b_fill_start
    mov ah,1
runtime_104b_fill_start:
    mov bx,cx
    jcxz $+13h
     mov al,0ffh
     sub si,cx
     xor byte ptr es:[di],al
     inc di
     loop $-4h
     add di,si
     mov cx,bx
     dec dx
     jne $-0bh
     pop di
     add di,bx
     cmp ah,0
     je $+11h
     mov cx,word ptr ss:[bp+0ah]
     mov al,0f0h
     mov si,0a0h
     xor byte ptr es:[di],al
     add di,si
     loop $-5h
     pop si
     pop di
     pop bp
     ret
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
    and byte ptr ds:[di],0f0h
    or byte ptr ds:[di],dl
rt_14a7 label byte
    inc di
    shl al,1
    jae $+7h
    and byte ptr ds:[di],0fh
    or byte ptr ds:[di],dh
    shl al,1
    jae $+7h
    and byte ptr ds:[di],0f0h
    or byte ptr ds:[di],dl
rt_14ba label byte
    inc di
    shl al,1
    jae $+7h
    and byte ptr ds:[di],0fh
    or byte ptr ds:[di],dh
    shl al,1
    jae $+7h
     and byte ptr ds:[di],0f0h
     or byte ptr ds:[di],dl
rt_14cd label byte
     inc di
     shl al,1
     jae $+7h
     and byte ptr ds:[di],0fh
     or byte ptr ds:[di],dh
     cmp ah,0
     jne $+10h
     shl al,1
     jae $+7h
     and byte ptr ds:[di],0f0h
     or byte ptr ds:[di],dl
rt_14e5 label byte
     sub bx,10h
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
     and byte ptr ds:[di],0f0h
     or byte ptr ds:[di],dl
     inc di
     shl al,1
     jae $+7h
     and byte ptr ds:[di],0fh
     or byte ptr ds:[di],dh
rt_152e label byte
     shl al,1
     jae $+7h
     and byte ptr ds:[di],0f0h
     or byte ptr ds:[di],dl
     inc di
     shl al,1
     jae $+7h
     and byte ptr ds:[di],0fh
     or byte ptr ds:[di],dh
rt_1541 label byte
     shl al,1
     jae $+7h
    and byte ptr ds:[di],0f0h
    or byte ptr ds:[di],dl
    inc di
    cmp ah,0
    jne $+10h
    shl al,1
    jae $+7h
    and byte ptr ds:[di],0fh
    or byte ptr ds:[di],dh
rt_1559 label byte
    sub bx,10h
    jg $-67h
    pop bx
    add di,word ptr ss:[bp-4]
    loop $-70h
    mov ax,bx
    shr ax,1
    pop ds
    pop si
    pop di
    add sp,4
    pop bp
    ret
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
    add di,bx
    lds si,dword ptr [bp+8]
    add si,20h
    lodsw
    sub bh,bh
    mov bl,al
    mov dh,bh
    mov dl,ah
    push dx
    mov ax,0a0h
    sub ax,bx
runtime_15ab_copy_loop:
    mov cx,bx
    shr cx,1
    rep movsw
    rcl cx,1
    rep movsb
    add di,ax
    dec dx
    jne runtime_15ab_copy_loop
    pop dx
    pop ds
    cmp word ptr ds:[0bch],1
    jne $+20h
    mov cx,word ptr [bp+6]
    cmp cx,0c8h
    jae $+17h
    les di,dword ptr ds:[40c4h]
    mov ax,word ptr [bp+4]
    shr ax,1
    mov ah,cl
    stosw
; CODE: reachable blit tail; instruction at 15EF straddles old raw boundary 15F0; RET at 15FB
    mov al,bl
    mov ah,dl
    stosw
    mov word ptr ds:[40c4h],di
    pop si
    pop di
    pop bp
    ret
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
    add di,bx
    pop ds
    mov bx,1998h
runtime_17a0_row_loop:
    push di
    push cx
runtime_17a0_pixel_loop:
    lodsb
    mov ah,al
     xlat cs:[bx]
    and al,byte ptr es:[di]
    or al,ah
    stosb
    loop runtime_17a0_pixel_loop
    pop cx
    pop di
    add di,0a0h
    add si,bp
    dec dx
    jne runtime_17a0_row_loop
    pop bp
    pop ds
    pop di
    pop si
    cld
    pop bp
    ret
    std
    xor bp,bp
    add bx,cx
    add bx,cx
    shr bx,1
    dec bx
    mov ax,bx
    sub ax,word ptr ds:[98h]
    js $+19h
    inc ax
    cmp cx,ax
    jbe $+8h
     mov bp,cx
     sub bp,ax
     mov cx,ax
     mov ax,bx
     sub ax,word ptr ds:[09ah]
     jbe $+13h
     sub cx,ax
     ja $+9h
     pop bp
     pop ds
     pop di
     pop si
     cld
     pop bp
     ret
     sub bx,ax
     add bp,ax
     add si,ax
     push es
     push di
     cmp word ptr ds:[0bch],1
     jne $+22h
     cmp di,0c8h
     jae $+1ch
    mov ax,di
    mov ah,al
    mov al,bl
    sub al,cl
    inc al
    les di,dword ptr ds:[40c4h]
    cld
    stosw
    mov al,cl
    mov ah,dl
    stosw
    std
    mov word ptr ds:[40c4h],di
    pop di
    shl di,1
    shl di,1
    les di,dword ptr [di+3924h]
    add di,bx
    pop ds
    mov bx,1998h
runtime_180c_row_loop:
    push di
    push cx
runtime_180c_pixel_loop:
    mov al,byte ptr [si]
    inc si
    ror al,1
    ror al,1
    ror al,1
    ror al,1
    mov ah,al
     xlat cs:[bx]
    and al,byte ptr es:[di]
    or al,ah
    stosb
    loop runtime_180c_pixel_loop
    pop cx
    pop di
    add di,0a0h
    add si,bp
    dec dx
    jne runtime_180c_row_loop
    pop bp
    pop ds
    pop di
    pop si
    cld
    pop bp
    ret
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
    and byte ptr ds:[di],0f0h
    or byte ptr ds:[di],dl
rt_18cb label byte
    inc di
    shl al,1
    jae $+7h
    and byte ptr ds:[di],0fh
    or byte ptr ds:[di],dh
    shl al,1
    jae $+7h
    and byte ptr ds:[di],0f0h
    or byte ptr ds:[di],dl
rt_18de label byte
    inc di
    shl al,1
    jae $+7h
    and byte ptr ds:[di],0fh
    or byte ptr ds:[di],dh
    shl al,1
    jae $+7h
     and byte ptr ds:[di],0f0h
     or byte ptr ds:[di],dl
rt_18f1 label byte
     inc di
     shl al,1
     jae $+7h
     and byte ptr ds:[di],0fh
     or byte ptr ds:[di],dh
     cmp ah,0
     jne $+10h
     shl al,1
     jae $+7h
     and byte ptr ds:[di],0f0h
     or byte ptr ds:[di],dl
rt_1909 label byte
     sub bx,10h
     jg $-67h
     pop bx
     add di,word ptr ss:[bp-2]
     loop runtime_f03cf+42h
     pop ds
     pop di
     pop si
     add sp,2
     pop bp
     ret
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

