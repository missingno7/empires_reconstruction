; F_B99F symbolic candidate
_TEXT segment byte public 'CODE'
_DATA segment word public 'DATA'
_DATA ends
_BSS segment word public 'BSS'
_BSS ends
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP
public _level_run_loop
CALL_REL macro delta
 call $+delta
endm
_level_run_loop proc near
 push bp
 mov bp,sp
 sub sp,94h
 push si
 push di
 xor ax,ax
 mov word ptr ds:[8feh],ax
 mov [bp-0ch],ax
 mov [bp-8h],ax
 mov [bp-6h],ax
 mov [bp-4h],ax
 mov word ptr ds:[0c5a2h],ax
 mov ax,1h
 mov word ptr ds:[0bch],ax
 mov word ptr ds:[0c588h],ax
 mov [bp-2h],ax
 mov word ptr [bp-0ah],2h
 xor si,si
 jmp short $+49
L_B9D1:
 mov ax,20h
 push ax
 mov ax,si
 mov cl,5h
 shl ax,cl
 mov cx,ss
 lea bx,[bp-90h]
 add bx,ax
 push cx
 push bx
 mov ax,si
 shl ax,1
 add ax,4h
 mov cl,5h
 shl ax,cl
 mov bx,0b3afh
 mov cx,ds
 add bx,ax
 push cx
 push bx
 CALL_REL 14671 ; LIB_MOVMEM
 add sp,0ah
 inc si
L_BA00:
 cmp si,4h
 jl short $-50
 mov bx,word ptr ds:[0c5a2h]
 shl bx,1
 mov bx,[bx+1684h]
 mov cl,5h
 shl bx,cl
 mov byte ptr [bx-4c51h],0h
 mov word ptr [bp-92h],0h
 mov word ptr [bp-94h],0h
L_BA24:
 mov ax,18h
 push ax
 CALL_REL -19921 ; F_6C57
 pop cx
 xor di,di
 mov [bp-10h],di
 mov ax,di
 mov word ptr ds:[40ceh],ax
 cmp word ptr [bp-0ah],0h
 jz short $+67
 mov bx,word ptr ds:[0c5a2h]
 shl bx,1
 mov bx,[bx+1684h]
 mov cl,5h
 shl bx,cl
 cmp byte ptr [bx-4c51h],0h
 jz short $+44
 inc word ptr ds:[0c5a2h]
 mov bx,word ptr ds:[0c5a2h]
 shl bx,1
 cmp word ptr [bx+1684h],0h
 jnl short $+8
 mov word ptr ds:[0c5a2h],0h
L_BA68:
 mov bx,word ptr ds:[0c5a2h]
 shl bx,1
 mov bx,[bx+1684h]
 mov cl,5h
 shl bx,cl
 mov byte ptr [bx-4c51h],0h
L_BA7B:
 jmp short $+105
L_BA7D:
 cmp word ptr [bp-8h],0h
 jnz short $+21
 cmp byte ptr ds:[0b6afh],0h
 jnz short $+14
 mov word ptr [bp-8h],1h
 mov word ptr [bp-6h],14h
 jmp short $+80
L_BA96:
 cmp word ptr [bp-6h],0h
 jz short $+74
 dec word ptr [bp-6h]
 jnz short $+69
 CALL_REL -3514 ; F_ACE7
 or ax,ax
 jz short $+7
 CALL_REL -687 ; F_B7F9
 jmp short $+57
L_BAAD:
 mov word ptr ds:[0bch],0h
 CALL_REL -3300 ; F_ADCF
 mov word ptr ds:[1776h],0h
 jmp short $+2
L_BABE:
 cmp word ptr ds:[1784h],0h
 jnz short $-5
 mov word ptr ds:[1776h],1h
 push ds
 mov ax,1670h
 push ax
 CALL_REL -13319 ; F_86C9
 pop cx
 pop cx
 mov ax,1h
 push ax
 push ds
 mov ax,8bfeh
 push ax
 CALL_REL 16045 ; LIB_SETJMP
 add sp,6h
L_BAE4:
 CALL_REL -20378 ; F_6B4A
 or ax,ax
 jz short $+40
 mov word ptr ds:[0bch],0h
 CALL_REL -20439 ; F_6B1A
 mov [bp-10h],ax
 cmp word ptr [bp-10h],0dh
 jnz short $+7
 CALL_REL -18560 ; F_727D
 jmp short $+11
L_BB02:
 cmp word ptr [bp-10h],1bh
 jnz short $+5
 CALL_REL 4898 ; F_CE2A
L_BB0B:
 mov word ptr ds:[0bch],1h
L_BB11:
 les bx,word ptr ds:[0c5cah]
 mov word ptr ds:[40c6h],es
 mov word ptr ds:[40c4h],bx
 CALL_REL 26516 ; F_22B1
 CALL_REL -27777 ; F_4E9F
 cmp word ptr ds:[8feh],0h
 jz short $+5
 CALL_REL -23698 ; F_5E98
L_BB2D:
 cmp word ptr ds:[0b6eh],0h
 jnz short $+5
 jmp near ptr $+140
L_BB37:
 mov word ptr ds:[73ah],0h
 mov ax,27h
 push ax
 mov ax,word ptr ds:[738h]
 inc ax
 push ax
 mov ax,word ptr ds:[736h]
 add ax,1ch
 push ax
 CALL_REL 25668 ; F_1F91
 add sp,6h
 test ax,7h
 jnz short $+103
 mov ax,word ptr ds:[734h]
 add word ptr ds:[736h],ax
 mov di,1h
 cmp word ptr ds:[72eh],8h
 jg short $+20
 inc word ptr ds:[72eh]
 mov ax,word ptr ds:[72eh]
 cmp ax,8h
 jng short $+8
 mov word ptr ds:[72eh],1h
L_BB7B:
 cmp word ptr ds:[0c588h],0h
 jz short $+61
 xor si,si
 jmp short $+49
L_BB86:
 mov ax,20h
 push ax
 mov ax,si
 shl ax,1
 add ax,4h
 mov cl,5h
 shl ax,cl
 mov bx,0b3afh
 mov cx,ds
 add bx,ax
 push cx
 push bx
 mov ax,si
 mov cl,5h
 shl ax,cl
 mov cx,ss
 lea bx,[bp-90h]
 add bx,ax
 push cx
 push bx
 CALL_REL 14234 ; LIB_MOVMEM
 add sp,0ah
 inc si
L_BBB5:
 cmp si,4h
 jl short $-50
 CALL_REL -1096 ; F_B772
L_BBBD:
 jmp near ptr $+144
L_BBC0:
 cmp word ptr ds:[0b6ch],0h
 jnz short $+5
 jmp near ptr $+134
L_BBCA:
 mov word ptr ds:[73ah],1h
 mov ax,27h
 push ax
 mov ax,word ptr ds:[738h]
 inc ax
 push ax
 push word ptr ds:[736h]
 CALL_REL 25524 ; F_1F91
 add sp,6h
 test ax,7h
 jnz short $+103
 mov ax,word ptr ds:[734h]
 sub word ptr ds:[736h],ax
 mov di,0ffffh
 cmp word ptr ds:[72eh],8h
 jg short $+20
 inc word ptr ds:[72eh]
 mov ax,word ptr ds:[72eh]
 cmp ax,8h
 jng short $+8
 mov word ptr ds:[72eh],1h
L_BC0B:
 cmp word ptr ds:[0c588h],0h
 jz short $+61
 xor si,si
 jmp short $+49
L_BC16:
 mov ax,20h
 push ax
 mov ax,si
 shl ax,1
 add ax,4h
 mov cl,5h
 shl ax,cl
 mov bx,0b3afh
 mov cx,ds
 add bx,ax
 push cx
 push bx
 mov ax,si
 mov cl,5h
 shl ax,cl
 mov cx,ss
 lea bx,[bp-90h]
 add bx,ax
 push cx
 push bx
 CALL_REL 14090 ; LIB_MOVMEM
 add sp,0ah
 inc si
L_BC45:
 cmp si,4h
 jl short $-50
 CALL_REL -1240 ; F_B772
L_BC4D:
 cmp word ptr ds:[730h],0h
 jz short $+86
 mov ax,9h
 push ax
 mov ax,word ptr ds:[738h]
 dec ax
 push ax
 mov ax,word ptr ds:[736h]
 add ax,8h
 push ax
 CALL_REL 25267 ; F_1F17
 add sp,6h
 test ax,7h
 jnz short $+50
 mov bx,word ptr ds:[730h]
 mov al,[bx+740h]
 mov ah,0h
 sub word ptr ds:[738h],ax
 dec word ptr ds:[730h]
 inc word ptr ds:[72eh]
 mov ax,word ptr ds:[72eh]
 cmp ax,0bh
 jng short $+8
 mov word ptr ds:[72eh],0bh
L_BC93:
 or di,di
 jnz short $+8
 mov word ptr ds:[72eh],0ah
L_BC9D:
 jmp short $+8
L_BC9F:
 mov word ptr ds:[730h],0h
L_BCA5:
 jmp near ptr $+398
L_BCA8:
 mov ax,9h
 push ax
 mov ax,word ptr ds:[738h]
 add ax,2fh
 push ax
 mov ax,word ptr ds:[736h]
 add ax,8h
 push ax
 CALL_REL 25181 ; F_1F17
 add sp,6h
 test ax,7h
 jnz short $+41
 cmp word ptr ds:[732h],0h
 jz short $+9
 add word ptr ds:[738h],8h
 jmp short $+13
L_BCD3:
 mov word ptr ds:[732h],1h
 add word ptr ds:[738h],2h
L_BCDE:
 mov ax,di
 and ax,1h
 add ax,0ah
 mov word ptr ds:[72eh],ax
 jmp near ptr $+330
L_BCEC:
 mov ax,9h
 push ax
 mov ax,word ptr ds:[738h]
 add ax,28h
 push ax
 mov ax,word ptr ds:[736h]
 add ax,8h
 push ax
 CALL_REL 25113 ; F_1F17
 add sp,6h
 test ax,7h
 jnz short $+61
 mov ax,word ptr ds:[738h]
 add ax,30h
 mov bx,8h
 cwd
 idiv bx
 shl ax,1
 shl ax,1
 shl ax,1
 mov dx,word ptr ds:[738h]
 add dx,28h
 sub ax,dx
 add word ptr ds:[738h],ax
 mov word ptr ds:[732h],0h
 mov ax,di
 and ax,1h
 add ax,0ah
 mov word ptr ds:[72eh],ax
 mov ax,0bh
 push ax
 CALL_REL 3508 ; F_CAF1
 pop cx
 jmp near ptr $+242
L_BD44:
 cmp word ptr [bp-10h],20h
 jnz short $+83
 CALL_REL -19155 ; F_7277
 cmp ax,1h
 jnz short $+70
 mov ax,9h
 push ax
 mov ax,word ptr ds:[738h]
 dec ax
 push ax
 mov ax,word ptr ds:[736h]
 add ax,8h
 push ax
 CALL_REL 25013 ; F_1F17
 add sp,6h
 test ax,7h
 jnz short $+39
 mov ax,10h
 push ax
 CALL_REL 3456 ; F_CAF1
 pop cx
 mov word ptr ds:[730h],8h
 mov word ptr ds:[72eh],9h
 or di,di
 jz short $+7
 mov ax,8h
 jmp short $+5
L_BD8A:
 mov ax,4h
L_BD8D:
 mov word ptr ds:[734h],ax
 jmp short $+4
L_BD92:
 jmp short $+93
L_BD94:
 jmp short $+4
L_BD96:
 jmp short $+89
L_BD98:
 jmp near ptr $+155
L_BD9B:
 cmp word ptr ds:[0b68h],0h
 jz short $+79
 cmp word ptr ds:[0b70h],0h
 jz short $+72
 mov ax,9h
 push ax
 mov ax,word ptr ds:[738h]
 dec ax
 push ax
 mov ax,word ptr ds:[736h]
 add ax,8h
 push ax
 CALL_REL 24926 ; F_1F17
 add sp,6h
 test ax,7h
 jnz short $+45
 mov word ptr ds:[0b70h],0h
 mov word ptr ds:[730h],5h
 mov word ptr ds:[72eh],9h
 or di,di
 jz short $+7
 mov ax,8h
 jmp short $+5
L_BDDF:
 mov ax,4h
L_BDE2:
 mov word ptr ds:[734h],ax
 mov ax,0ch
 push ax
 CALL_REL 3336 ; F_CAF1
 pop cx
 jmp short $+70
L_BDEF:
 or di,di
 jz short $+17
 cmp word ptr ds:[72eh],8h
 jng short $+8
 mov word ptr ds:[72eh],1h
L_BE00:
 jmp short $+8
L_BE02:
 mov word ptr ds:[72eh],0h
L_BE08:
 mov word ptr ds:[734h],4h
 cmp word ptr ds:[738h],46h
 jnl short $+32
 cmp word ptr ds:[736h],5ah
 jng short $+25
 cmp word ptr ds:[736h],0d2h
 jnl short $+17
 CALL_REL -1213 ; F_B967
 mov word ptr ds:[0bch],0h
 mov ax,1h
 jmp near ptr $+682
L_BE33:
 cmp word ptr [bp-10h],20h
 jnz short $+96
 CALL_REL -19394 ; F_7277
 mov si,ax
 mov ax,si
 cmp ax,2h
 jnz short $+52
 cmp word ptr ds:[72ch],0h
 jnz short $+45
 mov ax,0ffffh
 push ax
 CALL_REL -19261 ; F_7313
 pop cx
 cmp ax,0ffffh
 jz short $+22
 mov word ptr ds:[72ch],3ah
 mov word ptr [bp-4h],0h
 xor ax,ax
 push ax
 CALL_REL 3210 ; F_CAF1
 pop cx
 jmp short $+10
L_BE6D:
 mov ax,11h
 push ax
 CALL_REL 3200 ; F_CAF1
 pop cx
L_BE75:
 jmp short $+34
L_BE77:
 or si,si
 jnz short $+30
 cmp word ptr ds:[8feh],0h
 jnz short $+15
 CALL_REL -25671 ; F_5A3B
 mov ax,14h
 push ax
 CALL_REL 3176 ; F_CAF1
 pop cx
 jmp short $+10
L_BE8F:
 mov ax,17h
 push ax
 CALL_REL 3166 ; F_CAF1
 pop cx
L_BE97:
 cmp word ptr ds:[72ch],1h
 jnz short $+8
 mov word ptr ds:[72ch],0h
L_BEA4:
 CALL_REL -29592 ; F_4B0C
 cmp word ptr [bp-2h],0h
 jz short $+63
 cmp word ptr ds:[0c588h],0h
 jnz short $+56
 cmp byte ptr ds:[0b52fh],0h
 jz short $+49
 mov byte ptr ds:[0b537h],1h
 mov word ptr [bp-2h],0h
 mov word ptr ds:[96h],190h
 xor ax,ax
 push ax
 push word ptr ds:[0b0f2h]
 push word ptr ds:[0b0f0h]
 mov ax,112h
 push ax
 mov ax,0a0h
 push ax
 CALL_REL 17646 ; RUNTIME_BLOCK
 add sp,0ah
 mov word ptr ds:[96h],9fh
L_BEEA:
 cmp word ptr ds:[40ceh],0h
 jz short $+25
 mov ax,[bp-0ch]
 cmp ax,word ptr ds:[40ceh]
 jz short $+16
 mov al,word ptr ds:[0b3aeh]
 mov ah,0h
 sub ax,word ptr ds:[40ceh]
 mov [bp-0eh],ax
 jmp short $+7
L_BF08:
 mov word ptr [bp-0eh],0ffffh
L_BF0D:
 mov ax,word ptr ds:[40ceh]
 mov [bp-0ch],ax
 cmp word ptr ds:[736h],8h
 jnl short $+10
 mov word ptr ds:[736h],8h
 jmp short $+16
L_BF22:
 cmp word ptr ds:[736h],110h
 jng short $+8
 mov word ptr ds:[736h],110h
L_BF30:
 cmp word ptr ds:[8feh],0h
 jz short $+5
 CALL_REL -25716 ; F_5AC3
L_BF3A:
 cmp word ptr [bp-4h],0h
 jz short $+91
 dec word ptr [bp-4h]
 cmp word ptr [bp-4h],1ah
 jng short $+34
 push word ptr ds:[73ah]
 mov dx,word ptr ds:[99d4h]
 mov ax,word ptr ds:[99d2h]
 add ax,39ech
 push dx
 push ax
 push word ptr ds:[738h]
 push word ptr ds:[736h]
 CALL_REL 17515 ; RUNTIME_BLOCK
 add sp,0ah
 jmp short $+47
L_BF69:
 test word ptr [bp-4h],1h
 jz short $+40
 push word ptr ds:[73ah]
 mov ax,word ptr ds:[72eh]
 mov dx,2a2h
 mul dx
 mov cx,word ptr ds:[99d4h]
 mov bx,word ptr ds:[99d2h]
 add bx,ax
 push cx
 push bx
 push word ptr ds:[738h]
 push word ptr ds:[736h]
 CALL_REL 17468 ; RUNTIME_BLOCK
 add sp,0ah
L_BF96:
 jmp near ptr $+210
L_BF99:
 cmp word ptr ds:[72ch],0h
 jz short $+69
 xor ax,ax
 push ax
 push ds
 mov ax,96eeh
 push ax
 push word ptr ds:[738h]
 push word ptr ds:[736h]
 CALL_REL 17436 ; RUNTIME_BLOCK
 add sp,0ah
 dec word ptr ds:[72ch]
 push word ptr ds:[73ah]
 mov ax,word ptr ds:[72eh]
 mov dx,2a2h
 mul dx
 mov cx,word ptr ds:[99d4h]
 mov bx,word ptr ds:[99d2h]
 add bx,ax
 push cx
 push bx
 push word ptr ds:[738h]
 push word ptr ds:[736h]
 CALL_REL 17394 ; RUNTIME_BLOCK
 add sp,0ah
 jmp near ptr $+136
L_BFE3:
 cmp word ptr [bp-0eh],4h
 jng short $+91
 cmp word ptr [bp-0eh],0bh
 jg short $+85
 push word ptr ds:[73ah]
 mov dx,word ptr ds:[99d4h]
 mov ax,word ptr ds:[99d2h]
 add ax,39ech
 push dx
 push ax
 push word ptr ds:[738h]
 push word ptr ds:[736h]
 CALL_REL 17349 ; RUNTIME_BLOCK
 add sp,0ah
 mov word ptr [bp-4h],1eh
 mov ax,1h
 push ax
 CALL_REL 2779 ; F_CAF1
 pop cx
 CALL_REL 24243 ; F_1ECD
 mov word ptr ds:[0bch],0h
 mov ax,0fffeh
 push ax
 CALL_REL -19673 ; F_734E
 pop cx
 or ax,ax
 jg short $+10
 CALL_REL 31063 ; F_3986
 xor ax,ax
 jmp near ptr $+166
L_C037:
 mov word ptr ds:[0bch],1h
 jmp near ptr $+140
 jmp short $+40
L_C042:
 push word ptr ds:[73ah]
 mov ax,word ptr ds:[72eh]
 mov dx,2a2h
 mul dx
 mov cx,word ptr ds:[99d4h]
 mov bx,word ptr ds:[99d2h]
 add bx,ax
 push cx
 push bx
 push word ptr ds:[738h]
 push word ptr ds:[736h]
 CALL_REL 17258 ; RUNTIME_BLOCK
 add sp,0ah
L_C068:
 cmp word ptr [bp-0eh],0dh
 jnz short $+90
 mov dx,word ptr ds:[0b78h]
 mov ax,word ptr ds:[0b76h]
 cmp dx,[bp-92h]
 jc short $+56
 ja short $+8
 cmp ax,[bp-94h]
 jna short $+48
L_C083:
 mov al,0h
 mov word ptr ds:[0b54fh],al
 mov ah,0h
 mov word ptr ds:[730h],ax
 dec word ptr [bp-0ah]
 jnl short $+7
 mov word ptr [bp-0ah],0h
L_C097:
 cmp word ptr [bp-0ah],0h
 jnz short $+22
 CALL_REL -5046 ; F_ACE7
 or ax,ax
 jnz short $+15
 CALL_REL 24105 ; F_1ECD
 mov ax,45h
 push ax
 CALL_REL 5454 ; F_D5F9
 pop cx
 jmp short $+26
L_C0B1:
 mov dx,word ptr ds:[0b78h]
 mov ax,word ptr ds:[0b76h]
 add ax,1f4h
 adc dx,0h
 mov [bp-92h],dx
 mov [bp-94h],ax
L_C0C6:
 CALL_REL 24071 ; F_1ECD
L_C0C9:
 CALL_REL -21594 ; F_6C6F
 jmp near ptr $-1704
 mov word ptr ds:[0bch],0h
 mov ax,1h
 jmp short $+2
L_C0DA:
 pop di
 pop si
 mov sp,bp
 pop bp
 ret
_level_run_loop endp
_TEXT ends
end
