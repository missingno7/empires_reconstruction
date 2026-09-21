; Candidate complete arithmetic module.
_TEXT segment byte public 'CODE'
DGROUP group _DATA,_BSS
assume cs:_TEXT, ds:DGROUP
_TEXT ends
_DATA segment word public 'DATA'
d@ label byte
_DATA ends
_BSS segment word public 'BSS'
b@ label byte
_BSS ends
_TEXT segment byte public 'CODE'

extrn LXMUL@:far, LDIV@:far, LXLSH@:far
extrn _gc6c3:word, _gca24:word, _gca6d:word, _gc5ea:word, _gc64a:word
public _music_note_to_divisor, _music_build_octave_table, _music_reset_tuning_tables, _fdf98

; Emit source contributions in descending address order so TASM emits the historical descending FIXUPP sequence.
org 2bbh
L_02BB:
 ret
org 2bah
L_02BA:
 pop bp
org 2b8h
L_02B8:
 mov sp,bp
org 2b7h
L_02B7:
 pop si
org 2b6h
L_02B6:
 pop di
org 2b3h
L_02B3:
 mov word ptr ds:[3752h],ax
org 2afh
L_02AF:
 mov word ptr ds:[3754h],dx
org 2ach
L_02AC:
 mov ax,[bp-4h]
org 2a9h
L_02A9:
 mov dx,[bp-2h]
org 2a6h
L_02A6:
 mov word ptr ds:[0c5e6h],ax
org 2a2h
L_02A2:
 mov word ptr ds:[0c5e8h],dx
org 29eh
L_029E:
 mov [bx-35dch],ax
org 29ah
L_029A:
 mov [bx-35dah],dx
org 298h
L_0298:
 shl bx,1
org 296h
L_0296:
 shl bx,1
org 293h
L_0293:
 mov bx,[bp+4h]
org 291h
L_0291:
 mov dx,ds
org 28eh
L_028E:
 add ax,0c6c3h
org 28ch
L_028C:
 mul dx
org 289h
L_0289:
 mov dx,18h
org 287h
L_0287:
 mov ax,si
org 285h
L_0285:
 mov si,dx
org 283h
L_0283:
 idiv bx
org 282h
L_0282:
 cwd
org 27fh
L_027F:
 mov bx,19h
org 27dh
L_027D:
 mov ax,di
org 27ah
L_027A:
 mov word ptr ds:[0c5e4h],ax
org 276h
L_0276:
 mov [bx-3593h],ax
org 274h
L_0274:
 shl bx,1
org 271h
L_0271:
 mov bx,[bp+4h]
org 26fh
L_026F:
 idiv bx
org 26eh
L_026E:
 cwd
org 26bh
L_026B:
 mov bx,19h
org 269h
L_0269:
 mov ax,di
org 267h
L_0267:
 jmp short L_0287
org 265h
L_0265:
 mov si,ax
org 263h
L_0263:
 sub ax,si
org 260h
L_0260:
 mov ax,19h
org 25eh
L_025E:
 jz L_0267
org 25ch
L_025C:
 or si,si
org 25ah
L_025A:
 mov si,dx
org 258h
L_0258:
 idiv bx
org 257h
L_0257:
 cwd
org 254h
L_0254:
 mov bx,19h
org 251h
L_0251:
 add ax,0ffe8h
org 24eh
L_024E:
 mov ax,[bp-6h]
org 24bh
L_024B:
 mov word ptr ds:[0c5e4h],ax
org 247h
L_0247:
 mov [bx-3593h],ax
org 245h
L_0245:
 shl bx,1
org 242h
L_0242:
 mov bx,[bp+4h]
org 240h
L_0240:
 neg ax
org 23eh
L_023E:
 idiv bx
org 23dh
L_023D:
 cwd
org 23ah
L_023A:
 mov bx,19h
org 237h
L_0237:
 mov ax,[bp-6h]
org 234h
L_0234:
 mov [bp-6h],ax
org 232h
L_0232:
 sub ax,di
org 22fh
L_022F:
 mov ax,18h
org 22dh
L_022D:
 jnl L_0269
org 22bh
L_022B:
 or di,di
org 229h
L_0229:
 mov di,ax
org 224h
L_0224:
 call far ptr LDIV@
org 221h
L_0221:
 push word ptr [bp-4h]
org 21eh
L_021E:
 push word ptr [bp-2h]
org 21dh
L_021D:
 push ax
org 21ch
L_021C:
 push dx
org 219h
L_0219:
 mov ax,2000h
org 217h
L_0217:
 xor dx,dx
org 214h
L_0214:
 jmp near ptr L_02B6
org 210h
L_0210:
 mov [bx-3593h],ax
org 20eh
L_020E:
 shl bx,1
org 20bh
L_020B:
 mov bx,[bp+4h]
org 208h
L_0208:
 mov ax,ds:[0c5e4h]
org 204h
L_0204:
 mov [bx-35dch],ax
org 200h
L_0200:
 mov [bx-35dah],dx
org 1feh
L_01FE:
 shl bx,1
org 1fch
L_01FC:
 shl bx,1
org 1f9h
L_01F9:
 mov bx,[bp+4h]
org 1f6h
L_01F6:
 mov ax,ds:[0c5e6h]
org 1f2h
L_01F2:
 mov dx,ds:[0c5e8h]
org 1f0h
L_01F0:
 jnz L_0217
org 1edh
L_01ED:
 cmp ax,[bp-4h]
org 1ebh
L_01EB:
 jnz L_0217
org 1e8h
L_01E8:
 cmp dx,[bp-2h]
org 1e5h
L_01E5:
 mov ax,ds:[3752h]
org 1e1h
L_01E1:
 mov dx,ds:[3754h]
org 1deh
L_01DE:
 mov [bp-4h],ax
org 1dbh
L_01DB:
 mov [bp-2h],dx
org 1d6h
L_01D6:
 call far ptr LXMUL@
org 1d5h
L_01D5:
 pop cx
org 1d4h
L_01D4:
 pop bx
org 1d3h
L_01D3:
 cwd
org 1d0h
L_01D0:
 mov ax,ds:[0ca6bh]
org 1cfh
L_01CF:
 push ax
org 1ceh
L_01CE:
 push dx
org 1cdh
L_01CD:
 cwd
org 1cah
L_01CA:
 add ax,0e000h
org 1c7h
L_01C7:
 mov ax,[bp+6h]
org 1c6h
L_01C6:
 push di
org 1c5h
L_01C5:
 push si
org 1c2h
L_01C2:
 sub sp,byte ptr +6h
org 1c0h
L_01C0:
 mov bp,sp
org 1bfh
_fdf98:
 push bp
org 1beh
L_01BE:
 ret
org 1bdh
L_01BD:
 pop bp
org 1bbh
L_01BB:
 mov sp,bp
org 1bah
L_01BA:
 pop si
org 1b9h
L_01B9:
 pop di
org 1b7h
L_01B7:
 jc L_0191
org 1b3h
L_01B3:
 cmp word ptr [bp-8h],byte ptr +8h
org 1b0h
L_01B0:
 inc word ptr [bp-8h]
org 1aeh
L_01AE:
 jc L_0198
org 1aah
L_01AA:
 cmp word ptr [bp-6h],byte ptr +0ch
org 1a9h
L_01A9:
 inc si
org 1a6h
L_01A6:
 inc word ptr [bp-6h]
org 1a2h
L_01A2:
 mov [si+offset DGROUP:_gc64a],al
org 19fh
L_019F:
 mov al,[bp-6h]
org 19bh
L_019B:
 mov [si+offset DGROUP:_gc5ea],al
org 198h
L_0198:
 mov al,[bp-8h]
org 196h
L_0196:
 jmp short L_01AA
org 191h
L_0191:
 mov word ptr [bp-6h],0h
org 18fh
L_018F:
 jmp short L_01B3
org 18ah
L_018A:
 mov word ptr [bp-8h],0h
org 188h
L_0188:
 xor si,si
org 186h
L_0186:
 jc L_0163
org 182h
L_0182:
 cmp word ptr [bp-8h],byte ptr +0bh
org 17fh
L_017F:
 inc word ptr [bp-8h]
org 179h
L_0179:
 mov word ptr [bx+offset DGROUP:_gca6d],0
org 177h
L_0177:
 shl bx,1
org 174h
L_0174:
 mov bx,[bp-8h]
org 16eh
L_016E:
 mov word ptr [bx+offset DGROUP:_gca24],offset DGROUP:_gc6c3
org 16ah
L_016A:
 mov word ptr [bx+offset DGROUP:_gca24+2],ds
org 168h
L_0168:
 shl bx,1
org 166h
L_0166:
 shl bx,1
org 163h
L_0163:
 mov bx,[bp-8h]
org 161h
L_0161:
 jmp short L_0182
org 15ch
L_015C:
 mov word ptr [bp-8h],0h
org 15ah
L_015A:
 jc L_0137
org 157h
L_0157:
 cmp di,byte ptr +19h
org 154h
L_0154:
 add [bp-4h],ax
org 151h
L_0151:
 mov ax,[bp-2h]
org 150h
L_0150:
 inc di
org 14dh
L_014D:
 add sp,byte ptr +8h
org 14ah
L_014A:
 call _music_build_octave_table
org 149h
L_0149:
 push ax
org 146h
L_0146:
 add ax,offset DGROUP:_gc6c3
org 144h
L_0144:
 mul dx
org 141h
L_0141:
 mov dx,18h
org 13fh
L_013F:
 mov ax,di
org 13eh
L_013E:
 push ds
org 13bh
L_013B:
 push word ptr [bp-4h]
org 13ah
L_013A:
 push ax
org 137h
L_0137:
 mov ax,64h
org 135h
L_0135:
 jmp short L_0157
org 132h
L_0132:
 mov [bp-4h],ax
org 130h
L_0130:
 mov ax,di
org 12eh
L_012E:
 xor di,di
org 129h
L_0129:
 mov word ptr [bp-2h],4h
org 128h
L_0128:
 push di
org 127h
L_0127:
 push si
org 124h
L_0124:
 sub sp,byte ptr +8h
org 122h
L_0122:
 mov bp,sp
org 121h
_music_reset_tuning_tables:
 push bp
org 120h
L_0120:
 ret
org 11fh
L_011F:
 pop bp
org 11dh
L_011D:
 mov sp,bp
org 11ch
L_011C:
 pop si
org 11ah
L_011A:
 jl L_00D5
org 117h
L_0117:
 cmp si,byte ptr +0ch
org 116h
L_0116:
 inc si
org 112h
L_0112:
 add word ptr [bp+4h],byte ptr +2h
org 10fh
L_010F:
 mov [es:bx],ax
org 10ch
L_010C:
 les bx,[bp+4h]
org 10ah
L_010A:
 shr ax,1
org 108h
L_0108:
 shr ax,1
org 106h
L_0106:
 shr ax,1
org 103h
L_0103:
 add ax,4h
org 100h
L_0100:
 mov [bp-4h],ax
org 0fdh
L_00FD:
 mov [bp-2h],dx
org 0f8h
L_00F8:
 call far ptr LDIV@
org 0f5h
L_00F5:
 push word ptr [bp-4h]
org 0f2h
L_00F2:
 push word ptr [bp-2h]
org 0f1h
L_00F1:
 push ax
org 0f0h
L_00F0:
 push dx
org 0edh
L_00ED:
 mov ax,64h
org 0ebh
L_00EB:
 xor dx,dx
org 0e8h
L_00E8:
 mov [bp-4h],ax
org 0e5h
L_00E5:
 mov [bp-2h],dx
org 0e0h
L_00E0:
 call far ptr LXMUL@
org 0ddh
L_00DD:
 mov bx,6ah
org 0dbh
L_00DB:
 xor cx,cx
org 0d8h
L_00D8:
 mov ax,[bp-4h]
org 0d5h
L_00D5:
 mov dx,[bp-2h]
org 0d3h
L_00D3:
 jmp short L_0117
org 0d0h
L_00D0:
 mov si,1h
org 0cch
L_00CC:
 add word ptr [bp+4h],byte ptr +2h
org 0c9h
L_00C9:
 mov [es:bx],ax
org 0c6h
L_00C6:
 les bx,[bp+4h]
org 0c4h
L_00C4:
 shr ax,1
org 0c2h
L_00C2:
 shr ax,1
org 0c0h
L_00C0:
 shr ax,1
org 0bdh
L_00BD:
 add ax,4h
org 0bah
L_00BA:
 mov [bp-4h],ax
org 0b7h
L_00B7:
 mov [bp-2h],dx
org 0b6h
L_00B6:
 pop cx
org 0b5h
L_00B5:
 pop cx
org 0b2h
L_00B2:
 call _music_note_to_divisor
org 0afh
L_00AF:
 push word ptr [bp+8h]
org 0ach
L_00AC:
 push word ptr [bp+0ah]
org 0abh
L_00AB:
 push si
org 0a8h
L_00A8:
 sub sp,byte ptr +4h
org 0a6h
L_00A6:
 mov bp,sp
org 0a5h
_music_build_octave_table:
 push bp
org 0a4h
L_00A4:
 ret
org 0a3h
L_00A3:
 pop bp
org 0a1h
L_00A1:
 mov sp,bp
org 9fh
L_009F:
 jmp short L_00A1
org 9ch
L_009C:
 mov ax,[bp-8h]
org 99h
L_0099:
 mov dx,[bp-6h]
org 96h
L_0096:
 mov [bp-8h],ax
org 93h
L_0093:
 mov [bp-6h],dx
org 8eh
L_008E:
 call far ptr LDIV@
org 8bh
L_008B:
 push word ptr [bp-8h]
org 88h
L_0088:
 push word ptr [bp-6h]
org 87h
L_0087:
 push ax
org 86h
L_0086:
 push dx
org 83h
L_0083:
 mov ax,0b503h
org 80h
L_0080:
 mov dx,1h
org 7dh
L_007D:
 mov [bp-8h],ax
org 7ah
L_007A:
 mov [bp-6h],dx
org 75h
L_0075:
 call far ptr LXMUL@
org 72h
L_0072:
 mov bx,9h
org 70h
L_0070:
 xor cx,cx
org 6dh
L_006D:
 mov ax,[bp-8h]
org 6ah
L_006A:
 mov dx,[bp-6h]
org 67h
L_0067:
 mov [bp-8h],ax
org 64h
L_0064:
 mov [bp-6h],dx
org 5fh
L_005F:
 call far ptr LXLSH@
org 5dh
L_005D:
 mov cl,0eh
org 5ah
L_005A:
 mov ax,[bp-0ch]
org 57h
L_0057:
 mov dx,[bp-0ah]
org 54h
L_0054:
 mov [bp-0ch],ax
org 51h
L_0051:
 mov [bp-0ah],dx
org 4ch
L_004C:
 call far ptr LDIV@
org 49h
L_0049:
 push word ptr [bp-0ch]
org 46h
L_0046:
 push word ptr [bp-0ah]
org 45h
L_0045:
 push ax
org 44h
L_0044:
 push dx
org 3fh
L_003F:
 call far ptr LXMUL@
org 3ch
L_003C:
 mov bx,19h
org 3ah
L_003A:
 xor cx,cx
org 37h
L_0037:
 mov ax,[bp-4h]
org 34h
L_0034:
 mov dx,[bp-2h]
org 31h
L_0031:
 mov [bp-0ch],ax
org 2eh
L_002E:
 mov [bp-0ah],dx
org 29h
L_0029:
 call far ptr LXMUL@
org 26h
L_0026:
 mov bx,0cb78h
org 24h
L_0024:
 xor cx,cx
org 21h
L_0021:
 adc dx,[bp-2h]
org 1eh
L_001E:
 add ax,[bp-4h]
org 1dh
L_001D:
 cwd
org 1bh
L_001B:
 mul dx
org 18h
L_0018:
 mov dx,6h
org 15h
L_0015:
 mov ax,[bp+4h]
org 12h
L_0012:
 mov [bp-4h],ax
org 0fh
L_000F:
 mov [bp-2h],dx
org 0eh
L_000E:
 cwd
org 0ch
L_000C:
 mul dx
org 9h
L_0009:
 mov dx,64h
org 6h
L_0006:
 mov ax,[bp+6h]
org 3h
L_0003:
 sub sp,byte ptr +0ch
org 1h
L_0001:
 mov bp,sp
org 0h
_music_note_to_divisor proc near
 push bp
_music_note_to_divisor endp
_TEXT ends
end _music_note_to_divisor
